/*****< linuxcgmp.c >**********************************************************/
/*      Copyright 2014 Stonestreet One.                                       */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LinuxCGMP - Continous Glucose Monitoring Sample Application.              */
/*             (Based on LinuxGLP).                                           */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/14  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "LinuxCGMP.h"           /* Sample Application Header.                */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */

#include "SS1BTCGM.h"            /* Main SS1 CGM Service Header.              */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTBMS.h"            /* Main SS1 BMS Service Header.              */

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

#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define MAX_INQUIRY_RESULTS                        (32)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define MAX_NUM_OF_PARAMETERS                       (16) /* Denotes the max   */
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

#define DATE_STRING_LENGTH                         (14)  /* Denotes the number*/
                                                         /* of characters     */
                                                         /* required to       */
                                                         /* represent a Date  */
                                                         /* string of form    */
                                                         /* YYYYMMDDHHMMSS    */

#define DATE_TIME_FIELD_LENGTH                      (6)  /* Denotes the number*/
                                                         /* of variable fields*/
                                                         /* in Date/Time      */
                                                         /* structure of type */
                                                         /* CGMS_Date_Time_   */
                                                         /* Data_t.           */
/* Server Specific Defines.                                             */

#define SENSOR_FEATURES                            (0x01FFFF) /* Denotes the default */
                                                              /* supported features  */
                                                              /* of the sensor.      */

#define SENSOR_SESSION_RUN_TIME                    (30)       /* Denotes the static  */
                                                              /* supported features  */
                                                              /* of the sensor.      */

#define SENSOR_SESSION_RUN_TIME                    (30)       /* Denotes the static  */
                                                              /* session run time of */
                                                              /* the sensor.         */

#define SENSOR_MEASUREMENT_INTERVAL                (1)        /* Denotes the static  */
                                                              /* measurement interval*/
                                                              /* that measurements   */
                                                              /* will be generated at*/
                                                              /* on the sensor.      */

#define SENSOR_COMMUNICATION_INTERVAL              (2)        /* Denotes the default */
                                                              /* communication       */
                                                              /* interval that       */
                                                              /* measurements will be*/
                                                              /* periodically        */
                                                              /* notified at.        */

#define SENSOR_FASTEST_SUPPORTED_COMM_INTERVAL     (1)        /* Denotes the fastest */
                                                              /* communication       */
                                                              /* interval the sensor */
                                                              /* supports.           */

#define MAX_SENSOR_MEASUREMENTS                    (100)      /* Denotes the number*/
                                                              /* of CGMS           */
                                                              /* measurements      */
                                                              /* supported by the  */
                                                              /* sensor.           */

#define MAX_CALIBRATION_DATA_RECORDS               (20)       /* Denotes the       */
                                                              /* maximum number of */
                                                              /* calibration data  */
                                                              /* records supported */
                                                              /* by the sensor.    */

#define MINIMUM_SUPPORTED_CONCENTRATION            (10)       /* Denotes the       */
                                                              /* minimum supported */
                                                              /* glucose           */
                                                              /* concentration by  */
                                                              /* the sensor.       */

#define MAXIMUM_SUPPORTED_CONCENTRATION            (200)      /* Denotes the       */
                                                              /* maximum supported */
                                                              /* glucose           */
                                                              /* concentration by  */
                                                              /* the sensor.       */

#define SENSOR_TYPE                                (CGMS_FEATURE_TYPE_CAPILLARY_WHOLE_BLOOD)
                                                              /* Denotes the       */
                                                              /* default sensor    */
                                                              /* type.             */

#define SENSOR_SAMPLE_LOCATION                     (CGMS_FEATURE_SAMPLE_LOCATION_FINGER)
                                                              /* Denotes the       */
                                                              /* default sensor    */
                                                              /* sample location.  */

#define RACP_OP_CODE_NOT_SUPPORTED                 (-2)       /* Denotes that a   */
                                                              /* received RACP OP */
                                                              /* code is not      */
                                                              /* supported by the */
                                                              /* sensor.          */

#define RACP_INVALID_OPERATOR                      (-3)       /* Denotes that a   */
                                                              /* RACP operator for*/
                                                              /* an RACP Procedure*/
                                                              /* is invalid.      */

#define RACP_OPERATOR_NOT_SUPPORTED                (-4)       /* Denotes that a   */
                                                              /* RACP operator for*/
                                                              /* an RACP Procedure*/
                                                              /* is not supported */
                                                              /* by the sensor.   */

#define RACP_INVALID_OPERAND                       (-5)       /* Denotes that an  */
                                                              /* operand for an   */
                                                              /* RACP Procedure is*/
                                                              /* invalid.         */

#define RACP_NO_RECORDS_FOUND                      (-6)       /* Denotes that     */
                                                              /* there are no     */
                                                              /* measurements on  */
                                                              /* sensor that match*/
                                                              /* the specifed     */
                                                              /* criteria.        */

#define RACP_ABORT_UNSUCCESSFUL                    (-7)       /* Denotes that an  */
                                                              /* Abort RACP       */
                                                              /* Procedure has    */
                                                              /* failed.          */

#define RACP_PROCEDURE_NOT_COMPLETED               (-8)       /* Denotes that an  */
                                                              /* RACP Procedure   */
                                                              /* not completed due*/
                                                              /* to some error.   */

#define RACP_OPERAND_NOT_SUPPORTED                 (-9)       /* Denotes that a   */
                                                              /* RACP operand for */
                                                              /* an RACP Procedure*/
                                                              /* is not supported */
                                                              /* by the sensor.   */

#define SOCP_OP_CODE_NOT_SUPPORTED                 (-2)       /* Denotes that a   */
                                                              /* received SOCP OP */
                                                              /* code is not      */
                                                              /* supported by the */
                                                              /* sensor.          */

#define SOCP_INVALID_OPERAND                       (-3)       /* Denotes that an  */
                                                              /* operand for an   */
                                                              /* SOCP Procedure   */
                                                              /* is invalid.      */

#define SOCP_PROCEDURE_NOT_COMPLETED               (-4)       /* Denotes that an  */
                                                              /* SOCP Procedure   */
                                                              /* not completed due*/
                                                              /* to some error.   */

#define SOCP_PARAMETER_OUT_OF_RANGE                (-5)       /* Denotes that     */
                                                              /* there are no     */
                                                              /* measurements on  */
                                                              /* sensor that match*/
                                                              /* the specifed     */
                                                              /* criteria.        */

/* Client Specific Defines.                                             */

#define MAX_COLLECTOR_MEASUREMENTS                 (500) /* Denotes the number*/
                                                         /* of CGMS           */
                                                         /* measurements      */
                                                         /* supported by the  */
                                                         /* Client.           */

   /* The following converts an ASCII character to an integer value.    */
#define ToInt(_x)                      (((_x) > 0x39)?((_x)-0x37):((_x)-0x30))

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxCGMP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxCGMP_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxCGMP"

   /* The following represent the possible values of UI_Mode variable.  */
#define UI_MODE_IS_CLIENT                          (2)
#define UI_MODE_IS_SERVER                          (1)

   /* The following correspond to the current Reserved for Future Use   */
   /* bits in OpCode field of Record Access Control Point characteristic*/
#define CGMS_RECORD_ACCESS_OPCODE_RFU_0                       0x00
#define CGMS_RECORD_ACCESS_OPCODE_RFU_7                       0x07
#define CGMS_RECORD_ACCESS_OPCODE_RFU_255                     0xFF

   /* The following type definition represents the container type which */
   /* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
   /* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t
{
   BD_ADDR_t  BD_ADDR;
   Link_Key_t LinkKey;
} LinkKeyInfo_t;

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
   char             *CommandName;
   CommandFunction_t CommandFunction;
} CommandTable_t;

   /* The following enumerated type definition defines the different    */
   /* types of service discovery that can be performed.                 */
typedef enum
{
   sdDIS,
   sdGAPS,
   sdCGMS,
   sdBMS
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

   /* The following enum represents the handle names of DIS Service     */
   /* These enum values are 1 to 1 mapped with handle names defined in  */
   /* DIS_Client_Info_t structure. These enum values will be used       */
   /* as user input in GATTRead command to read Specific DIS            */
   /* Characteristic attribute                                          */
typedef enum
{
   daManufacturerName         = 1,
   daModelNumber              = 2,
   daSerialNumber             = 3,
   daHardwareRevision         = 4,
   daFirmwareRevision         = 5,
   daSoftwareRevision         = 6,
   daSystemID                 = 7,
   daIEEEDataList             = 8
} DIS_Attribute_Handle_Type_t;

   /* The following structure represents the attribute handles of       */
   /* DIS Service that will hold DIS attribute handles after DIS        */
   /* Service Discovery.                                                */
typedef struct _tagDIS_Client_Info_t
{
   Word_t ManufacturerName;
   Word_t ModelNumber;
   Word_t SerialNumber;
   Word_t HardwareRevision;
   Word_t FirmwareRevision;
   Word_t SoftwareRevision;
   Word_t SystemID;
   Word_t IEEECertificationDataList;
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
   unsigned int              Flags;
   Byte_t                    EncryptionKeySize;
   GAP_LE_Address_Type_t     ConnectionAddressType;
   BD_ADDR_t                 ConnectionBD_ADDR;
   Link_Key_t                LinkKey;
   Long_Term_Key_t           LTK;
   Random_Number_t           Rand;
   Word_t                    EDIV;
   DIS_Client_Info_t         DISClientInfo;
   GAPS_Client_Info_t        GAPSClientInfo;
   BMS_Client_Information_t  BMSClientInfo;
   CGMS_Client_Information_t ClientInfo;
   CGMS_Server_Information_t ServerInfo;
   struct _tagDeviceInfo_t  *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x0001
#define DEVICE_INFO_FLAGS_LINK_KEY_VALID                    0x0002
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x0004
#define DEVICE_INFO_FLAGS_BR_EDR_CONNECTED                  0x0008
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x0010
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE        0x0020
#define DEVICE_INFO_FLAGS_CT_NOTIFY_SUPPORTED               0x0040
#define DEVICE_INFO_FLAGS_RACP_OUTSTANDING                  0x0080
#define DEVICE_INFO_FLAGS_SOCP_OUTSTANDING                  0x0100

   /* The following structure defines a CGMS Measurement that is a Node  */
   /* for a CGMS Measurement doubly Linked List (DeQueue).               */
typedef struct _tagMeasurement_t
{
   DWord_t                   Number;
   Boolean_t                 Notified;
   Byte_t                    Size;
   Byte_t                    Flags;
   Word_t                    GlucoseConcentration;
   Word_t                    TimeOffset;
   Word_t                    Trend;
   Word_t                    Quality;
   Byte_t                    Status;
   Byte_t                    CalTemp;
   Byte_t                    Warning;
   struct _tagMeasurement_t *Next;
   struct _tagMeasurement_t *Previous;
} Measurement_t;

   /* The following enumeration is used to specify the list type.       */
typedef enum
{
   ltTemp,
   ltSensor,
   ltCollector
} List_Type_t;

   /* The following structure defines a doubly linked list (Queue) made */
   /* up of CGMS Measurement Nodes.                                     */
typedef struct _tagMeasurement_List_t
{
   DWord_t            NumberOfMeasurements;
   Measurement_t     *Head;
   Measurement_t     *Tail;
   List_Type_t        ListType;
} Measurement_List_t;

   /* The following structure defines a node for a linked list of       */
   /* calibration data records.                                         */
typedef struct _tagCalibrationDataRecord_t
{
   Word_t                              GlucoseConcentration;
   Word_t                              CalibrationTime;
   Byte_t                              TypeSampleLocation;
   Word_t                              NextCalibrationTime;
   Word_t                              CalibrationDataRecord;
   Byte_t                              Status;
   struct _tagCalibrationDataRecord_t *Next;
} CalibrationDataRecord_t;

   /* The following structure defines a list made up of calibration data*/
   /* record nodes.                                                     */
typedef struct _tagCalibrationDataRecordList_t
{
   DWord_t                  NumberOfRecords;
   CalibrationDataRecord_t *Head;
   CalibrationDataRecord_t *Tail;
} CalibrationDataRecordList_t;

   /* The following structure defines a way to specify a range or       */
   /* records or a specific record, which will be needed by some RACP   */
   /* procedures.                                                       */
typedef struct _tagRACP_Procedure_Data_t
{
   CGMS_RACP_Operator_Type_t        OperatorType;
   union
   {
      Word_t                        TimeOffset;
      CGMS_Time_Offset_Range_Data_t Range;
   } ParameterData;

} RACP_Procedure_Data_t;

typedef struct _tagBMSCommandInfo_t
{
   char        *Name;
   unsigned int Command;
} BMSCommandInfo_t;

   /* The following string table is used to map the BMS Command types   */
   /* to an easily displayable string.                                  */
static BMSCommandInfo_t BMSCommandTable[] =
{
   { "Delete Requesting Device Bond (BR/EDR and LE)", bmcDeleteBondRequestingBREDR_LE },
   { "Delete Requesting Device Bond (BR/EDR)"       , bmcDeleteBondRequestingBREDR },
   { "Delete Requesting Device Bond (LE)",            bmcDeleteBondRequestingLE },
   { "Delete All Device Bonds (BR/EDR and LE)",       bmcDeleteAllBondsBREDR_LE },
   { "Delete All Device Bonds (BR/EDR)"       ,       bmcDeleteAllBondsBREDR },
   { "Delete All Device Bonds (LE)",                  bmcDeleteAllBondsLE },
   { "Delete Other Device Bonds (BR/EDR and LE)",     bmcDeleteAllOtherBondsBREDR_LE },
   { "Delete Other Device Bonds (BR/EDR)"       ,     bmcDeleteAllOtherBondsBREDR },
   { "Delete Other Device Bonds (LE)",                bmcDeleteAllOtherBondsLE },
} ;

#define NUM_BMS_COMMANDS                  (sizeof(BMSCommandTable)/sizeof(BMSCommandInfo_t))

   /* Defines the Authorization Code to be used with the Bond           */
   /* Management Service, in Little Endian Format.                      */
static Byte_t BMS_Authorization_Code[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

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

                        /* The following array represents the IEEE      */
                        /* 11073-20601 Regulatory Certification Data    */
                        /* List.                                        */
static Byte_t IEEE_DATA_LIST[] = {0x12, 0x34, 0x56, 0x78};

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

static int                 UI_Mode;                 /* Holds the UI Mode.              */

static unsigned int        CGMSInstanceID;          /* The following holds the CGMS    */
                                                    /* Instance ID that is returned    */
                                                    /* from CGMS_Initialize_Service(). */

static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

static unsigned int        BMSInstanceID;
                                                    /* The following holds the BMS     */
                                                    /* Instance IDs that are returned  */
                                                    /* from BMS_Initialize_Service().  */

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

static GAP_Encryption_Mode_t GAPEncryptionMode;     /* Holds the encryption mode of    */
                                                    /* currently connected device.     */

static unsigned int        ConnectionID;            /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */

static Boolean_t           LocalDeviceIsMaster;     /* Boolean that tells if the local */
                                                    /* device is the master of the     */
                                                    /* current connection.             */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS];  /* Variable which  */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int        NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static Link_Key_t          CreatedLinkKey;          /* Variable which holds the created*/
                                                    /* link key when bonding with a    */
                                                    /* remote device.                  */

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

static Boolean_t           ScanningInProgress = FALSE;
                                                    /* A boolean flag to show if a sc- */
                                                    /*an is currently in process.      */

static unsigned int        DebugID;                 /* Variable which holds the ID of  */
                                                    /* the currently enabled debugging */
                                                    /* session.                        */

static unsigned int        BMSCommandBufferLength;  /* Size of the command buffer.     */

static Byte_t             *BMSCommandBuffer;        /* The command buffer used to hold */
                                                    /* a command to be sent to a remote*/
                                                    /* server.                         */

static unsigned int        BMSCommandTransactionID; /* Holds the transactionID for a   */
                                                    /* Write request.                  */

static unsigned int        BMSCommandValueOffset;   /* Holds the offset value of the   */
                                                    /* command.                        */

static BMS_BM_Control_Point_Format_Data_t FormatData;
                                                    /* Holds the format data of the    */
                                                    /* command.                        */

/* Internal Variables specific to the Server Module (Remember that all  */
/* variables declared static are initialized to 0 automatically by the  */
/* compiler as part of standard C/C++).                                 */

static DWord_t                       SensorStatus;                  /* Variable which holds the */
                                                                    /* current status of the    */
                                                                    /* sensor.                  */

static DWord_t                       SensorSupportedFeatures;       /* Variable which holds the */
                                                                    /* bit mask for the         */
                                                                    /* supported features       */
                                                                    /* of the sensor.           */

static Byte_t                        SensorTypeSampleLocation;      /* Byte which holds the type*/
                                                                    /* and sample location of   */
                                                                    /* the sensor.              */

static CGMS_Session_Start_Time_Data_t SensorSessionStartTime;       /* Variable which holds the */
                                                                    /* Session Start Time or    */
                                                                    /* the time the first       */
                                                                    /* measurement was taken    */
                                                                    /* (from client time).      */

static Word_t                        SensorSessionRunTime;          /* Variable which holds the */
                                                                    /* Session Run Time or the  */
                                                                    /* static time that session */
                                                                    /* can remain running on the*/
                                                                    /* sensor.                  */

static Word_t                        SensorRunTimeMinutes;          /* Variable which holds the */
                                                                    /* current number of minutes*/
                                                                    /* that have elasped since  */
                                                                    /* the session started.     */

static Byte_t                        SensorMeasurementInterval;     /* Variable which holds the */
                                                                    /* the static measurement   */
                                                                    /* interval, which indicates*/
                                                                    /* how often measurements   */
                                                                    /* are generated on the     */
                                                                    /* server.                  */

static Word_t                        TimeOffset;                    /* Variable used to         */
                                                                    /* represent Time Offset of */
                                                                    /* Glucose Measurement      */
                                                                    /* Record since the session */
                                                                    /* started.                 */

static Byte_t                        SensorCommunicationInterval;   /* Variable which holds the */
                                                                    /* Communication interval,  */
                                                                    /* which controls how often */
                                                                    /* the recent measurements  */
                                                                    /* are notified.            */

static Measurement_List_t            SensorMeasurementList;         /* Variable which holds the */
                                                                    /* measurement list on the  */
                                                                    /* sensor.                  */

static DWord_t                       MeasurementCounter;            /* Variable which holds a   */
                                                                    /* counter that will be used*/
                                                                    /* to assig a unique        */
                                                                    /* number to each           */
                                                                    /* measurement that is      */
                                                                    /* generated.               */

static Word_t                        PatientHighAlertLevel;         /* Variable which holds the */
                                                                    /* Patient high alert level.*/

static Word_t                        PatientLowAlertLevel;          /* Variable which holds the */
                                                                    /* Patient low alert level. */

static Word_t                        HypoAlertLevel;                /* Variable which holds the */
                                                                    /* Hypo Alert Level.        */

static Word_t                        HyperAlertLevel;               /* Variable which holds the */
                                                                    /* Hyper Alert Level.       */

static Word_t                        RateOfDecreaseAlertLevel;      /* Variable which holds the */
                                                                    /* rate of decrease alert   */
                                                                    /* level.                   */

static Word_t                        RateOfIncreaseAlertLevel;      /* Variable which holds the */
                                                                    /* rate of increase alert   */
                                                                    /* level.                   */

static CalibrationDataRecordList_t   CalibrationDataRecordList;     /* Variable which holds the */
                                                                    /* list of calibration data */
                                                                    /* records on the sensor.   */

static Word_t                        CalibrationDataRecordCtr;      /* Variable which is a      */
                                                                    /* counter used to assign   */
                                                                    /* a unique number for each */
                                                                    /* calibration data record  */
                                                                    /* that is added.           */

static Byte_t                        CalibrationStatus;             /* Variable which holds the */
                                                                    /* current calibration      */
                                                                    /* status.                  */

static Boolean_t                     AbortProcedureFlag = FALSE;    /* Variable which holds a   */
                                                                    /* flag to indicate that an */
                                                                    /* abort procedure has been */
                                                                    /* received.                */

static unsigned int                  TimerID;                       /* Variable which holds the */
                                                                    /* timer ID for the sensor  */
                                                                    /* measurement timer        */
                                                                    /* callback.                */

static DWord_t                       MinuteCounter;                 /* Variable which holds the */
                                                                    /* current number of minutes*/
                                                                    /* that have elasped since  */
                                                                    /* the sensor timer was     */
                                                                    /* started.                 */

/* Internal Variables specific to the Client Module (Remember that all  */
/* variables declared static are initialized to 0 automatically by the  */
/* compiler as part of standard C/C++).                                 */

static Measurement_List_t            CollectorMeasurementList;      /* Variable which is used to */
                                                                    /* to hold the measurement   */
                                                                    /* list on the collector.    */

static DWord_t                       ClientFeatures;                /* Variable which is used to */
                                                                    /* to hold the Features      */
                                                                    /* of the sensor, which is   */
                                                                    /* read by the collector.    */

static CGMS_Session_Start_Time_Data_t ClientSessionStartTime;       /* Variable which is used to */
                                                                    /* to hold the Session Start */
                                                                    /* Time of the session,      */
                                                                    /* which is read by the      */
                                                                    /* collector.                */

static Word_t                        ClientSessionRunTime;          /* Variable which is used to */
                                                                    /* hold the Session Run Time */
                                                                    /* of the session, which is  */
                                                                    /* read by the collector.    */

static DWord_t                       ClientStatus;                  /* Variable which is used to */
                                                                    /* hold the status of the    */
                                                                    /* sensor, which is read by  */
                                                                    /* the collector.            */

static Byte_t                        ClientTypeSampleLocation;      /* Variable which is used    */
                                                                    /* to hold the type sample   */
                                                                    /* location of the sensor,   */
                                                                    /* which is read, along with */
                                                                    /* the sensor features, by   */
                                                                    /* the collector.            */

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

   /* The following string table is used to map the CGMS Feature Bits to*/
   /* an easily displayable string.                                     */
static BTPSCONST char *CGMSFeatureBitStrings[] =
{
   "Calibration",
   "Patient High/Low Alerts",
   "Hypo Alerts",
   "Hyper Alerts",
   "Rate of Increase/Decrease",
   "Device Specific Alert",
   "Sensor Malfunction Detection",
   "Sensor Temperature High/Low Detection",
   "Sensor Result High/Low Detection",
   "Low Battery Detection",
   "Sensor Type Error",
   "General Device Fault",
   "E2E-CRC",
   "Multiple Bond",
   "Multiple Sessions",
   "Trend Information",
   "Quality"
} ;

#define NUM_CGMS_FEATURE_BIT_STRINGS              ((sizeof(CGMSFeatureBitStrings)/sizeof(char *)) - 1)

#define LONGEST_CGMS_FEATURE_IDX                  (7)

   /* The following string table is used to map the CGMS Feature Type   */
   /* Bits to an easily displayable string.                             */
static BTPSCONST char *CGMSFeatureTypeBitStrings[] =
{
   "Capillary Whole Blood",
   "Capillary Plasma",
   "Venous Whole Blood",
   "Venous Plasma",
   "Arterial Whole Blood",
   "Arterial Plasma",
   "Undetermined Whole Blood",
   "Undetermined Plasma",
   "Interstitial Fluid",
   "Control Solution"
} ;

#define NUM_CGMS_FEATURE_TYPE_BIT_STRINGS         ((sizeof(CGMSFeatureTypeBitStrings)/sizeof(char *)) - 1)

   /* The following string table is used to map the CGMS Feature Sample */
   /* Location Bits to an easily displayable string.                    */
static BTPSCONST char *CGMSFeatureSampleLocationBitStrings[] =
{
   "Finger",
   "Alternate Site Test",
   "Earlobe",
   "Control Solution",
   "Subcutaneous Tissue",
   "Not Available"
} ;

#define NUM_CGMS_FEATURE_SAMPLE_LOCATION_BIT_STRINGS  ((sizeof(CGMSFeatureSampleLocationBitStrings)/sizeof(char *)) - 1)

   /* Internal function prototypes.                                     */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree);
static void FreeDeviceInfoList(DeviceInfo_t **ListHead);

static void UserInterface_Selection(void);
static void UserInterface_Server(void);
static void UserInterface_Client(void);
static int CommandLineInterpreter(char *Command);
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
static void DisplayGATTReadUsage(void);
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

static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);

/* Bond Management Service Helper Functions                          */
static void BMSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
Boolean_t IsBMSFeatureHandle(Word_t Handle, DeviceInfo_t *DeviceInfo);
Boolean_t IsBMSControlPointHandle(Word_t Handle, DeviceInfo_t *DeviceInfo);

static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback);

static int StartScan(unsigned int BluetoothStackID);
static int StopScan(unsigned int BluetoothStackID);

static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, int AddressType, Boolean_t UseWhiteList);
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
static int CheckForBond(void);
static int GetLocalAddress(ParameterList_t *TempParam);

static int AdvertiseLE(ParameterList_t *TempParam);

static int StartScanning(ParameterList_t *TempParam);
static int StopScanning(ParameterList_t *TempParam);

static int Connect(ParameterList_t *TempParam);
static int Disconnect(ParameterList_t *TempParam);

static int PairLE(ParameterList_t *TempParam);

   /* GAPS Function Commands.                                           */
static int DiscoverGAPS(ParameterList_t *TempParam);
static int ReadLocalName(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int ReadRemoteName(ParameterList_t *TempParam);
static int ReadLocalAppearance(ParameterList_t *TempParam);
static int SetLocalAppearance(ParameterList_t *TempParam);
static int ReadRemoteAppearance(ParameterList_t *TempParam);

   /* DIS Function Commands.                                            */
static int DiscoverDIS(ParameterList_t *TempParam);
static int ReadDeviceInfo(ParameterList_t *TempParam);

static int EnableDebug(ParameterList_t *TempParam);
static int GetGATTMTU(ParameterList_t *TempParam);
static int SetGATTMTU(ParameterList_t *TempParam);

   /* User Interface Function Commands.                                 */
static int ServerMode(ParameterList_t *TempParam);
static int ClientMode(ParameterList_t *TempParam);

   /* CGMS Function Commands                                            */
static int RegisterCGMS(ParameterList_t *TempParam);
static int UnregisterCGMS(ParameterList_t *TempParam);
static int DiscoverCGMS(ParameterList_t *TempParam);
static int ConfigureRemoteCGMS(ParameterList_t *TempParam);
static int SetSupportedFeatures(ParameterList_t *TempParam);
static int GetSupportedFeatures(ParameterList_t *TempParam);
static int SetSessionStartTime(ParameterList_t *TempParam);
static int GetSessionStartTime(ParameterList_t *TempParam);
static int GetSessionRunTime(ParameterList_t *TempParam);
static int SetStatus(ParameterList_t *TempParam);
static int GetStatus(ParameterList_t *TempParam);
static int IndicateNumberOfStoredRecords(Word_t NumberOfStoredRecords);
static int IndicateRACPResult(Byte_t CommandType, Byte_t ResponseCode);
static int ConfigureRACP(ParameterList_t *TempParam);
static int IndicateSOCPResult(CGMS_SOCP_Command_Type_t RequestOpCode, Byte_t Response);
static int IndicateSOCPCommunicationInterval(void);
static int IndicateSOCPCalibrationData(CGMS_Calibration_Data_Record_t CalibrationData);
static int IndicateSOCPAlertLevel(CGMS_SOCP_Response_Type_t ResponseOpCode, Word_t AlertLevel);
static int ConfigureSOCP(ParameterList_t *TempParam);
static int GenerateMeasurement(ParameterList_t *TempParam);
static int DeleteMeasurements(ParameterList_t *TempParam);
static int PrintMeasurements(ParameterList_t *TempParam);
static int GenerateCalRecord(ParameterList_t *TempParam);
static int PrintCalibrationRecords(ParameterList_t *TempParam);
static int DeleteCalibrationRecords(ParameterList_t *TempParam);
static int StartSessionCommand(ParameterList_t *TempParam);
static int StopSessionCommand(ParameterList_t *TempParam);

   /* CGMS RACP Functions                                               */
static int ReportStoredRecordsProcedure(RACP_Procedure_Data_t RACPData);
static int ReportNumberOfStoredRecordsProcedure(RACP_Procedure_Data_t RACPData, unsigned int *NumberOfMeasurementsFound);
static int AbortProcedure(void);
static int DeleteStoredRecordsProcedure(RACP_Procedure_Data_t RACPData);

   /* CGMS SOCP Functions                                               */
static int SetCommunicationInterval(Byte_t *CommunicationInterval);
static int GetCommunicationInterval(Byte_t *CommunicationInterval);
static int SetGlucoseCalibrationValue(CGMS_Calibration_Data_Record_t *CalDataRecord);
static int GetGlucoseCalibrationValue(Word_t RecordNumber, CGMS_Calibration_Data_Record_t *CalDataRecord);
static int SetPatientHighAlertLevel(Word_t *AlertLevel);
static int GetPatientHighAlertLevel(Word_t *AlertLevel);
static int SetPatientLowAlertLevel(Word_t *AlertLevel);
static int GetPatientLowAlertLevel(Word_t *AlertLevel);
static int SetHypoAlertLevel(Word_t *AlertLevel);
static int GetHypoAlertLevel(Word_t *AlertLevel);
static int SetHyperAlertLevel(Word_t *AlertLevel);
static int GetHyperAlertLevel(Word_t *AlertLevel);
static int SetRateOfDecreaseAlertLevel(Word_t *AlertLevel);
static int GetRateOfDecreaseAlertLevel(Word_t *AlertLevel);
static int SetRateOfIncreaseAlertLevel(Word_t *AlertLevel);
static int GetRateOfIncreaseAlertLevel(Word_t *AlertLevel);
static int ResetDeviceSpecificAlert(void);
static int StartSession(void);
static int StopSession(void);

   /* CGMS Helper Functions                                             */
static void CGMSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void DisplayUsageCGMSFeatureBitMask(void);
static void DecodeDisplayStoreCGMSMeasurement(unsigned int ValueLength, Byte_t *Value);
static void DisplayRACPCommandUsage(void);
static void DecodeDisplayRACPResponse(Word_t BufferLength, Byte_t *Buffer);
static void DisplaySOCPCommandUsage(void);
static void DecodeDisplaySOCPResponse(Word_t BufferLength, Byte_t *Buffer);
static void DisplaySensorStatusUsage(void);
static void DisplayDISCharacteristicValue(char* Value);

   /* CGMS Initialize Funcitons.                                        */
static int InitializeServer(void);
static int InitializeClient(void);

static void CalculateSessionStartTime(void);

   /* CGMS Measurement List Functions.                                  */
static int AddNewMeasurementToList(Measurement_t *MeasurementToAdd, Measurement_List_t *List);
static void RemoveNodefromSensorMeasurementList(Measurement_t *MeasurementToRemove);
static int DeleteMeasurementList(Measurement_List_t *ListToDelete);
static void CopyMeasurement(Measurement_t *Measurement1, Measurement_t *Measurement2);

   /* CGMS Calibration Record Functions.                                */
static int AddNewCalibrationDataRecord(CalibrationDataRecord_t *RecordToAdd);
static int DeleteCalibrationRecordList(void);

   /* BMS Functions.                                                    */
static int DiscoverBMS(ParameterList_t *TempParam);
static Boolean_t VerifyBMSCommand(BMS_BM_Control_Point_Format_Data_t *FormatData);
static int       ExecuteBMSCommand(BMS_BMCP_Command_Type_t CommandType);
static int BMSCommand(ParameterList_t *TempParam);
static void DisplaySupportedFeatures(DWord_t SupportedFeatures);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI SensorTimerCallback(unsigned int BluetoothStackID, unsigned int CTimerID, unsigned long CallbackParameter);
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI BMS_EventCallback(unsigned int BluetoothStackID, BMS_Event_Data_t *BMS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI CGMS_EventCallback(unsigned int BluetoothStackID, CGMS_Event_Data_t *CGMS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_BMS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_CGMS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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

   /* The following function is responsible for taking the input from   */
   /* the user in order to select the sample application user interface */
   /* to run as the server or the client.                               */
static void UserInterface_Selection(void)
{
   /* Display the available commands.  Note the UI_MODE should be zero, */
   /* which will display the user interface options.                    */
   DisplayHelp(NULL);

   /* Clear the installed command.                                      */
   ClearCommands();

   AddCommand("SERVER", ServerMode);
   AddCommand("CLIENT", ClientMode);

   AddCommand("HELP", DisplayHelp);
}

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Server(void)
{
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
   AddCommand("DISCOVERGAPS", DiscoverGAPS);
   AddCommand("DISCOVERDIS", DiscoverDIS);
   AddCommand("READDEVICEINFO", ReadDeviceInfo);
   AddCommand("GETLOCALNAME", ReadLocalName);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETREMOTENAME", ReadRemoteName);
   AddCommand("GETLOCALAPPEARANCE", ReadLocalAppearance);
   AddCommand("SETLOCALAPPEARANCE", SetLocalAppearance);
   AddCommand("GETREMOTEAPPEARANCE", ReadRemoteAppearance);
   AddCommand("REGISTERCGMS", RegisterCGMS);
   AddCommand("UNREGISTERCGMS", UnregisterCGMS);
   AddCommand("GETFEATURES", GetSupportedFeatures);
   AddCommand("SETFEATURES", SetSupportedFeatures);
   AddCommand("GETSESSIONSTARTTIME", GetSessionStartTime);
   AddCommand("GETSESSIONRUNTIME", GetSessionRunTime);
   AddCommand("GETSTATUS", GetStatus);
   AddCommand("SETSTATUS", SetStatus);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   AddCommand("CREATE", GenerateMeasurement);
   AddCommand("DELETELIST", DeleteMeasurements);
   AddCommand("PRINTLIST", PrintMeasurements);
   AddCommand("CREATECALRECORD", GenerateCalRecord);
   AddCommand("DELETECALRECORDS", DeleteCalibrationRecords);
   AddCommand("PRINTCALRECORDS", PrintCalibrationRecords);
   AddCommand("START", StartSessionCommand);
   AddCommand("STOP", StopSessionCommand);
   AddCommand("HELP", DisplayHelp);
}

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Client(void)
{
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
   AddCommand("DISCOVERGAPS", DiscoverGAPS);
   AddCommand("DISCOVERDIS", DiscoverDIS);
   AddCommand("READDEVICEINFO", ReadDeviceInfo);
   AddCommand("GETLOCALNAME", ReadLocalName);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETREMOTENAME", ReadRemoteName);
   AddCommand("GETLOCALAPPEARANCE", ReadLocalAppearance);
   AddCommand("SETLOCALAPPEARANCE", SetLocalAppearance);
   AddCommand("GETREMOTEAPPEARANCE", ReadRemoteAppearance);
   AddCommand("DISCOVERCGMS", DiscoverCGMS);
   AddCommand("CONFIGUREREMOTECGMS", ConfigureRemoteCGMS);
   AddCommand("CONFIGURERACP", ConfigureRACP);
   AddCommand("CONFIGURESOCP", ConfigureSOCP);
   AddCommand("GETFEATURES", GetSupportedFeatures);
   AddCommand("SETSESSIONSTARTTIME", SetSessionStartTime);
   AddCommand("GETSESSIONSTARTTIME", GetSessionStartTime);
   AddCommand("GETSESSIONRUNTIME", GetSessionRunTime);
   AddCommand("GETSTATUS", GetStatus);
   AddCommand("ENABLEDEBUG", EnableDebug);
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   AddCommand("DELETELIST", DeleteMeasurements);
   AddCommand("PRINTLIST", PrintMeasurements);
   AddCommand("DISCOVERBMS", DiscoverBMS);
   AddCommand("DELETEBOND", BMSCommand);

   AddCommand("HELP", DisplayHelp);
}

   /* The following function is responsible for parsing user input      */
   /* and call appropriate command function.                            */
static int CommandLineInterpreter(char *Command)
{
   int           ret_val = 0;
   UserCommand_t TempCommand;

   /* The string input by the user contains a value, now run the string */
   /* through the Command Parser.                                       */
   if(CommandParser(&TempCommand, Command) >= 0)
   {
      /* The Command was successfully parsed run the Command.           */
      ret_val = CommandInterpreter(&TempCommand);

      switch(ret_val)
      {
         case INVALID_COMMAND_ERROR:
            printf("Invalid Command: %s.\r\n",TempCommand.Command);
            break;
         case FUNCTION_ERROR:
            printf("Function Error.\r\n");
            break;
         case EXIT_CODE:
            CloseStack();
            break;
      }
   }
   else
      printf("\r\nInvalid Command.\r\n");

   /* Display a prompt.                                                 */
   DisplayPrompt();

   return(ret_val);
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
      for(i=0;i<BTPS_StringLength(TempCommand->Command);i++)
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
   /* programmatically add Commands the Global (to this module) Command */
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
         if((BTPS_StringLength(Command) == BTPS_StringLength(CommandTable[Index].CommandName)) && (BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(Command)) == 0))
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
   /* Displays GATTRead function usage.                                 */
static void DisplayGATTReadUsage()
{
   DisplayUsage("GATTRead [Handle] ");
   printf(" Where Handle is:\r\n");
   printf("  1 = Remote Device Manufacture Name\r\n");
   printf("  2 = Remote Device Model Number\r\n");
   printf("  3 = Remote Device Serial Number\r\n");
   printf("  4 = Remote Device HardWare Revision\r\n");
   printf("  5 = Remote Device Firmware Revision\r\n");
   printf("  6 = Remote Device Software Revision\r\n");
   printf("  7 = Remote Device System ID\r\n");
   printf("  8 = Remote Device IEEE Data List\r\n");
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
         printf("AD Type: 0x%02X.\r\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Type));
         printf("AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data->Data_Entries[Index].AD_Data_Length));
         if(Advertising_Data->Data_Entries[Index].AD_Data_Buffer)
         {
            printf("AD Data: ");
            for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
            {
               printf("0x%02X ", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]);
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
   if(UI_Mode == UI_MODE_IS_SERVER)
   {
      printf("\r\nSensor>");
   }
   else
   {
      if(UI_Mode == UI_MODE_IS_CLIENT)
      {
         printf("\r\nCollector>");
      }
      else
         printf("\r\nLinuxCGMP>");
   }

   fflush(stdout);
}

   /* Displays a usage string..                                         */
static void DisplayUsage(char *UsageString)
{
   printf("Usage: %s\r\n\r\n",UsageString);
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
                     DIS_Set_IEEE_Certification_Data(BluetoothStackID, DISInstanceID, sizeof(IEEE_DATA_LIST), IEEE_DATA_LIST);

                     /* Return success to the caller.                   */
                     ret_val = 0;
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

      /* Cleanup CGMS Service.                                          */
      if(CGMSInstanceID)
      {
         CGMS_Cleanup_Service(BluetoothStackID, CGMSInstanceID);
         CGMSInstanceID = 0;
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

      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, dmGeneralDiscoverableMode, 0);

      /* Next, check the return value of the GAP Set Discoverability    */
      /* Mode command for successful execution.                         */
      if(!ret_val)
      {
         /* The command appears to have been successful.  The attached  */
         /* Device is now in General Discoverability Mode.              */
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
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* * NOTE * Connectability is only an applicable when advertising */
      /*          so we will just save the default connectability for   */
      /*          the next time we enable advertising.                  */
      LE_Parameters.ConnectableMode = lcmConnectable;

      /* Attempt to set the attached Device to be Connectable.          */
      ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);

      /* Next, check the return value of the                            */
      /* GAP_Set_Connectability_Mode() function for successful          */
      /* execution.                                                     */
      if(!ret_val)
      {
         /* The command appears to have been successful.  The attached  */
         /* Device is now in Connectable Mode.                          */
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
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      ret_val = GAP_LE_Set_Pairability_Mode(BluetoothStackID, lpmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!ret_val)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         ret_val = GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

         /* Attempt to set the attached device to be pairable.          */
         ret_val = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode_EnableSecureSimplePairing);

         /* Next, check the return value of the GAP Set Pairability mode*/
         /* command for successful execution.                           */
         if(!ret_val)
         {
            /* The device has been set to pairable mode, now register an*/
            /* Authentication Callback to handle the Authentication     */
            /* events if required.                                      */
            ret_val = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

            /* Next, check the return value of the GAP Register Remote  */
            /* Authentication command for successful execution.         */
            if(!ret_val)
            {
               /* The command appears to have been successful.          */
            }
            else
            {
               /* An error occurred while trying to execute this        */
               /* function.                                             */
               printf("GAP_Register_Remote_Authentication() Failure: %d\r\n", ret_val);
               ret_val = FUNCTION_ERROR;
            }
         }

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(ret_val)
         {
            /* An error occurred while trying to execute this function. */
            DisplayFunctionError("GAP_LE_Register_Remote_Authentication", ret_val);
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         DisplayFunctionError("GAP_LE_Set_Pairability_Mode", ret_val);
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
   /* mechanism of populating discovered DIS Service Handles.           */
static void DISPopulateHandles(DIS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                       Index1;
   GATT_Characteristic_Information_t            *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (DIS_COMPARE_DIS_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1=0;Index1<ServiceInfo->NumberOfCharacteristics;Index1++,CurrentCharacteristic++)
         {
            /* All DIS Service UUIDs are defined to be 16 bit UUIDs.    */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               if(DIS_COMPARE_DIS_MANUFACTURER_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  printf("Manufacturer Name UUID ");
                  DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                  ClientInfo->ManufacturerName = CurrentCharacteristic->Characteristic_Handle;
                  printf("Manufacturer Name Handle 0x%04X\n\n",ClientInfo->ManufacturerName);
               }
               else
               {
                  if(DIS_COMPARE_DIS_MODEL_NUMBER_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                  {
                     printf("Model Number String UUID " );
                     DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                     ClientInfo->ModelNumber = CurrentCharacteristic->Characteristic_Handle;
                     printf("ModelNumber Handle 0x%04X\n\n",ClientInfo->ModelNumber);
                  }
                  else
                  {
                     if(DIS_COMPARE_DIS_SERIAL_NUMBER_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     {
                        printf("Serial Number    UUID " );
                        DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                        ClientInfo->SerialNumber = CurrentCharacteristic->Characteristic_Handle;
                        printf("SerialNumber Handle 0x%04X\n\n",ClientInfo->SerialNumber);

                     }
                     else
                     {
                        if(DIS_COMPARE_DIS_HARDWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                        {
                           printf("\r\nHardware Revision UUID " );
                           DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                           ClientInfo->HardwareRevision = CurrentCharacteristic->Characteristic_Handle;
                           printf("HardwareRevision Handle 0x%04X\n\n",ClientInfo->HardwareRevision);
                        }
                        else
                        {
                           if(DIS_COMPARE_DIS_FIRMWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                           {
                              printf("Firmware Revision UUID " );
                              DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                              ClientInfo->FirmwareRevision = CurrentCharacteristic->Characteristic_Handle;
                              printf("FirmwareRevision Handle 0x%04X\n\n",ClientInfo->FirmwareRevision);
                           }
                           else
                           {
                              if(DIS_COMPARE_DIS_SOFTWARE_REVISION_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                              {
                                 printf("Software Revision UUID " );
                                 DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                                 ClientInfo->SoftwareRevision = CurrentCharacteristic->Characteristic_Handle;
                                 printf("SoftwareRevision Handle 0x%04X\n\n",ClientInfo->SoftwareRevision);
                              }
                              else
                              {
                                 if(DIS_COMPARE_DIS_SYSTEM_ID_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                                 {
                                    printf("System ID UUID " );
                                    DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                                    ClientInfo->SystemID = CurrentCharacteristic->Characteristic_Handle;
                                    printf("SystemID Handle 0x%04X\n\n",ClientInfo->SystemID);
                                 }
                                 else
                                 {
                                    if(DIS_COMPARE_DIS_IEEE_CERTIFICATION_DATA_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                                    {
                                       printf("IEEE 11073-20601 Regulatory Certification Data List UUID " );
                                       DisplayUUID(&(CurrentCharacteristic->Characteristic_UUID));
                                       ClientInfo->IEEECertificationDataList = CurrentCharacteristic->Characteristic_Handle;
                                       printf("IEEECertification Handle 0x%04X\n\n",ClientInfo->IEEECertificationDataList);
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
   /* mechanism of populating a BMS Client Information structure with   */
   /* the information discovered from a GDIS Discovery operation.       */
static void BMSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index1;
   GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (BMS_COMPARE_BMS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
     /* Loop through all characteristics discovered in the service and  */
     /* populate the correct entry.                                     */
     CurrentCharacteristic = ServiceDiscoveryData->CharacteristicInformationList;
     if(CurrentCharacteristic)
     {
        for(Index1 = 0; Index1 < ServiceDiscoveryData->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
        {
           if(BMS_COMPARE_BM_FEATURE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
           {
              /* All BMS UUIDs are defined to be 16 bit UUIDs.          */
              if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
              {
                 DeviceInfo->BMSClientInfo.BM_Feature = CurrentCharacteristic->Characteristic_Handle;

                 /* Verify that read is supported.                      */
                 if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                    printf("Warning - Mandatory read property of Supported Features characteristic not supported!\r\n");
              }
           }
           else
           {
              if(BMS_COMPARE_BM_CONTROL_POINT_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
              {
                 /* All BMS UUIDs are defined to be 16 bit UUIDs.       */
                 if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
                 {
                    DeviceInfo->BMSClientInfo.BM_Control_Point = CurrentCharacteristic->Characteristic_Handle;

                    /* Verify that read is supported.                   */
                    if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                       printf("Warning - Mandatory write property of Control Point characteristic not supported!\r\n");
                 }
              }
           }
        }
     }
   }
}

   /* The following function is a utility function that checks if a     */
   /* given handle is a Handle of Supported Feature characteristic.     */
   /* This function returns TRUE if handle matches with Supported       */
   /* Feature characteristic of any of the Service Instance else        */
   /* returns FALSE.                                                    */
Boolean_t IsBMSFeatureHandle(Word_t Handle, DeviceInfo_t *DeviceInfo)
{
   Boolean_t ret_val = FALSE;

   if(Handle == DeviceInfo->BMSClientInfo.BM_Feature)
      ret_val = TRUE;

   return ret_val;
}

   /* The following function is a utility function that checks if a     */
   /* given handle is a Handle of Control Point characteristic. This    */
   /* function returns TRUE if handle matches with Supported Feature    */
   /* characteristic of any of the Service Instance else returns FALSE. */
Boolean_t IsBMSControlPointHandle(Word_t Handle, DeviceInfo_t *DeviceInfo)
{
   Boolean_t ret_val = FALSE;

   if(Handle == DeviceInfo->BMSClientInfo.BM_Control_Point)
      ret_val = TRUE;

   return ret_val;
}

   /* The following function function is used to enable/disable         */
   /* notifications on a specified handle.  This function returns the   */
   /* positive non-zero Transaction ID of the Write Request or a        */
   /* negative error code.                                              */
static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback)
{
   int              ret_val;
   NonAlignedWord_t Buffer;

   /* Verify the input parameters.                                      */
   if((BluetoothStackID) && (ConnectionID) && (ClientConfigurationHandle))
   {
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, ClientConfigurationValue);

      ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, ClientConfigurationHandle, sizeof(Buffer), &Buffer, ClientEventCallback, ClientConfigurationHandle);
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

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
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, int AddressType, Boolean_t UseWhiteList)
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
         if(AddressType == 0)
         {
            WhiteListEntry.Address_Type = latPublic;
         }
         if(AddressType == 1)
         {
            WhiteListEntry.Address_Type = latRandom;
         }

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
            Result = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, Result?fpNoFilter:fpWhiteList, latPublic, Result?&BD_ADDR:NULL, WhiteListEntry.Address_Type, &ConnectionParameters, GAP_LE_Event_Callback, 0);

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
         printf("Calling GAP_LE_Generate_Long_Term_Key.\r\n");

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            printf("Encryption Information Request Response.\r\n");

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = larEncryptionInformation;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               printf("GAP_LE_Authentication_Response (larEncryptionInformation) success.\r\n");
            }
            else
            {
               printf("Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val);
            }
         }
         else
         {
            printf("Error - SM_Generate_Long_Term_Key returned %d.\r\n", ret_val);
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

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a CGMS Client Information structure with  */
   /* the information discovered from a GDIS Discovery operation.       */
static void CGMSPopulateHandles(DeviceInfo_t *DeviceInfo,  GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   Word_t                                       *ClientConfigurationHandle;
   unsigned int                                  Index1;
   unsigned int                                  Index2;
   GATT_Characteristic_Information_t            *CurrentCharacteristic;
   GATT_Characteristic_Descriptor_Information_t *CurrentDescriptor;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (CGMS_COMPARE_CGMS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceDiscoveryData->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1 = 0; Index1 < ServiceDiscoveryData->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
         {
            /* All CGMS UUIDs are defined to be 16 bit UUIDs.           */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               ClientConfigurationHandle = NULL;

               /* Use ClientConfigurationHandle here to avoid compiler  */
               /* warning                                               */
               if(ClientConfigurationHandle != NULL)
                  ;

               /* Determine which characteristic this is.               */
               if(!CGMS_COMPARE_MEASUREMENT_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  if(!CGMS_COMPARE_FEATURE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                  {
                     if(!CGMS_COMPARE_STATUS_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     {
                        if(!CGMS_COMPARE_CGMS_SESSION_START_TIME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                        {
                           if(!CGMS_COMPARE_CGMS_SESSION_RUN_TIME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                           {
                              if(!CGMS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                              {
                                 if(!CGMS_COMPARE_CGMS_SPECIFIC_OPS_CONTROL_POINT_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                                 {
                                    continue;
                                 }
                                 else /* CGMS_SPECIFIC_OPS_CONTROL_POINT */
                                 {
                                    DeviceInfo->ClientInfo.Specific_Ops_Control_Point = CurrentCharacteristic->Characteristic_Handle;

                                    /* Verify that write is supported.  */
                                    if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                                       printf("Warning - Mandatory Write property of Specific Ops Control Point characteristic not supported!\r\n");

                                     /* Verify that Indicate is         */
                                     /* supported.                      */
                                    if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                                       printf("Warning - Mandatory Indicate property of Specific Ops Control Point characteristic not supported!\r\n");

                                    CurrentDescriptor = CurrentCharacteristic->DescriptorList;

                                    /* Get SOCP Descriptor              */
                                    for(Index2 = 0; Index2 < CurrentCharacteristic->NumberOfDescriptors; Index2++)
                                    {
                                       if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                                       {
                                          if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16))
                                          {
                                             DeviceInfo->ClientInfo.SOCP_Client_Configuration = CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                                          }
                                       }
                                    }

                                    continue;
                                 }
                              }
                              else  /* CGMS_RECORD_ACCESS_CONTROL_POINT  */
                              {
                                 DeviceInfo->ClientInfo.Record_Access_Control_Point = CurrentCharacteristic->Characteristic_Handle;

                                 /* Verify that write is supported.     */
                                 if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                                    printf("Warning - Mandatory Write property of Record Access Control Point characteristic not supported!\r\n");

                                  /* Verify that Indicate is supported. */
                                 if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                                    printf("Warning - Mandatory Indicate property of Record Access Control Point characteristic not supported!\r\n");

                                 CurrentDescriptor = CurrentCharacteristic->DescriptorList;

                                 /* Get RACP Descriptor                 */
                                 for(Index2 = 0; Index2 < CurrentCharacteristic->NumberOfDescriptors; Index2++)
                                 {
                                    if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                                    {
                                       if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16))
                                       {
                                          DeviceInfo->ClientInfo.RACP_Client_Configuration = CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                                       }
                                    }
                                 }

                                 continue;
                              }
                           }
                           else /* CGMS_SESSION_RUN_TIME              */
                           {
                              DeviceInfo->ClientInfo.CGMS_Session_Run_Time = CurrentCharacteristic->Characteristic_Handle;

                              /* Verify that read is supported.         */
                              if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                                 printf("Warning - Mandatory read property of CGMS Session Run Time characteristic not supported!\r\n");

                              continue;
                           }
                        }
                        else /* CGMS_SESSION_START_TIME               */
                        {
                           DeviceInfo->ClientInfo.CGMS_Session_Start_Time = CurrentCharacteristic->Characteristic_Handle;

                           /* Verify that read is supported.            */
                           if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                              printf("Warning - Mandatory read property of CGMS Session Start Time characteristic not supported!\r\n");

                           /* Verify that write is supported.           */
                           if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                              printf("Warning - Mandatory write property of CGMS Session Start Time characteristic not supported!\r\n");

                           continue;
                        }
                     }
                     else /* CGMS_STATUS                              */
                     {
                        DeviceInfo->ClientInfo.CGMS_Status = CurrentCharacteristic->Characteristic_Handle;

                        /* Verify that read is supported.               */
                        if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                           printf("Warning - Mandatory read property of CGMS Status characteristic not supported!\r\n");

                        continue;
                     }
                  }
                  else /* CGMS_FEATURE                             */
                  {
                     DeviceInfo->ClientInfo.CGMS_Feature = CurrentCharacteristic->Characteristic_Handle;

                     /* Verify that read is supported.                  */
                     if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                        printf("Warning - Mandatory read property of CGMS Feature characteristic not supported!\r\n");

                     continue;
                  }
               }
               else /* CGMS_MEASUREMENT                            */
               {
                  DeviceInfo->ClientInfo.CGMS_Measurement = CurrentCharacteristic->Characteristic_Handle;

                  /* Verify that Notify is supported.                   */
                  if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                     printf("Warning - Mandatory Notify property of CGMS Measurement characteristic not supported!\r\n");

                  CurrentDescriptor = CurrentCharacteristic->DescriptorList;

                  /* Get RACP Descriptor                                */
                  for(Index2 = 0; Index2 < CurrentCharacteristic->NumberOfDescriptors; Index2++)
                  {
                     if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                     {
                        if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_UUID.UUID.UUID_16))
                        {
                           DeviceInfo->ClientInfo.CGMS_Measurement_Client_Configuration = CurrentCharacteristic->DescriptorList[Index2].Characteristic_Descriptor_Handle;
                        }
                     }
                  }

                  continue;
               }
            }
         }
      }
   }
}

   /* The following function is responsible for displaying usage        */
   /* information for a command line CGMS Features bitmask input field. */
static void DisplayUsageCGMSFeatureBitMask(void)
{
   unsigned int i;

   printf("Where setting a bit in BitMask enables the feature.\r\n");
   printf("\r\n     Bit         Category\r\n     ---         --------\r\n");

   for(i = 0; i < NUM_CGMS_FEATURE_BIT_STRINGS+1; i++)
      printf("     0x%08X  %s\r\n", (DWord_t)pow(2,i), CGMSFeatureBitStrings[i]);

   printf("\r\n");
}

   /* The following function is used to decode and display a CGMS       */
   /* measurement from a notification event.  The first parameter       */
   /* provides the buffer length of the data to be decoded, the second  */
   /* parameter provides the data packet to be decoded.  This function  */
   /* will also store the measurement into the collector measurement    */
   /* list.                                                             */
static void DecodeDisplayStoreCGMSMeasurement(unsigned int BufferLength, Byte_t *Buffer)
{
   int                        Result = 0;
   int                        Index;
   int                        NumberOfMeasurements;
   CGMS_Measurement_Data_t  **MeasurementData;
   Measurement_t             *NewMeasurement;

   /* Determine the number of measurements in the buffer.               */
   if((NumberOfMeasurements = CGMS_Decode_CGMS_Measurement(BufferLength, Buffer, 0, NULL)) > 0)
   {
      /* Allocate memory for the CGMS Measurement Data structure        */
      /* pointers.                                                      */
      if((MeasurementData = (CGMS_Measurement_Data_t **)BTPS_AllocateMemory(sizeof(CGMS_Measurement_Data_t *)*NumberOfMeasurements)) != NULL)
      {
         /* Allocate memory for the CGMS Measurements Data structures.  */
         for(Index = 0; Index < NumberOfMeasurements; Index++)
         {
            if((MeasurementData[Index] = (CGMS_Measurement_Data_t *)BTPS_AllocateMemory(CGMS_MEASUREMENT_DATA_SIZE)) != NULL)
            {
               /* Initialize the memory so that if a E2E-CRC calculation*/
               /* is performed there is not an error due to             */
               /* uninitialized memory.                                 */
               BTPS_MemInitialize(MeasurementData[Index], 0, sizeof(CGMS_Measurement_Data_t));
            }
            else
               Result = FUNCTION_ERROR;
         }

         if(Result == 0)
         {
            /* Decode the measurements.                                 */
            if((Result = CGMS_Decode_CGMS_Measurement(BufferLength, Buffer, NumberOfMeasurements, MeasurementData)) == NumberOfMeasurements)
            {
               printf("\r\nCGMS Measurement Data:\r\n");

               /* For each measurement, allocate an internal measurement*/
               /* structure and store it in the collector measurement   */
               /* list.  Also print the measurement to the user.        */
               for(Index = 0; Index < NumberOfMeasurements; Index++)
               {
                  if((NewMeasurement = (Measurement_t *)BTPS_AllocateMemory(sizeof(Measurement_t))) != NULL)
                  {
                     NewMeasurement->Size                 = MeasurementData[Index]->Size;
                     NewMeasurement->Flags                = MeasurementData[Index]->Flags;
                     NewMeasurement->GlucoseConcentration = MeasurementData[Index]->GlucoseConcentration;
                     NewMeasurement->TimeOffset           = MeasurementData[Index]->TimeOffset;

                     NewMeasurement->Number               = MeasurementCounter++;
                     printf("\r\nMeasurement Number:         %u\r\n",      NewMeasurement->Number);
                     printf("Size:                       %u (octets)\r\n", NewMeasurement->Size);
                     printf("Glucose Concentration:      %u (mg/dL)\r\n",  NewMeasurement->GlucoseConcentration);
                     printf("Flags:                      0x%02X\r\n",      NewMeasurement->Flags);
                     printf("TimeOffset:                 %u (Min)\r\n",    NewMeasurement->TimeOffset);

                     /* Following fields are optional so we will need to*/
                     /* check the measurement flags to make sure that   */
                     /* they exist before displaying them.              */
                     if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_PRESENT)
                     {
                        if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT)
                        {
                           NewMeasurement->Status  = MeasurementData[Index]->SensorStatus;
                           printf("Status:                     0x%02X\r\n", NewMeasurement->Status);
                        }
                        else
                           NewMeasurement->Status  = 0;

                        if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT)
                        {
                           NewMeasurement->CalTemp = MeasurementData[Index]->SensorCalTemp;
                           printf("Cal/Temp:                   0x%02X\r\n", NewMeasurement->CalTemp);
                        }
                        else
                           NewMeasurement->CalTemp  = 0;

                        if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT)
                        {
                           NewMeasurement->Warning = MeasurementData[Index]->SensorWarning;
                           printf("Warning:                    0x%02X\r\n", NewMeasurement->Warning);
                        }
                        else
                           NewMeasurement->Warning  = 0;
                     }

                     if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT)
                     {
                        NewMeasurement->Trend = MeasurementData[Index]->TrendInformation;
                        printf("Trend:                      %u ((mg/dL)/Min)\r\n", NewMeasurement->Trend);
                     }
                     else
                        NewMeasurement->Trend = 0;

                     if(NewMeasurement->Flags & CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT)
                     {
                        NewMeasurement->Quality = MeasurementData[Index]->Quality;
                        printf("Quality:                    %u (((mg/dL)/Min) %%)\r\n", NewMeasurement->Quality);
                     }
                     else
                        NewMeasurement->Quality = 0;

                     if((ClientFeatures & CGMS_E2E_CRC_SUPPORTED))
                     {
                        if(MeasurementData[Index]->CRCFlags & CGMS_E2E_CRC_PRESENT)
                        {
                           if(!(MeasurementData[Index]->CRCFlags & CGMS_E2E_CRC_VALID))
                           {
                              printf("CRC is invalid for measurement with timeoffset: %u.\r\n", NewMeasurement->TimeOffset);
                              break;
                           }
                        }
                        else
                        {
                           printf("CRC is missing for measurement with timeoffset: %u.\r\n", NewMeasurement->TimeOffset);
                           break;
                        }
                     }

                     NewMeasurement->Next      = NULL;
                     NewMeasurement->Previous  = NULL;

                     /* Add the new measurement to the collector list.  */
                     AddNewMeasurementToList(NewMeasurement, &CollectorMeasurementList);
                  }
                  else
                  {
                     printf("Could not allocate memory for an internal measurement structure.\r\n");
                     break;
                  }
               }
            }
            else
               DisplayFunctionError("CGMS_Decode_CGMS_Measurement", Result);
         }
         else
            printf("Could not allocate memory for one or more CGMS Measurement Data structures.\r\n");

         /* Free the memory for each CGMS Measurement Data structure.   */
         for(Index = 0; Index < NumberOfMeasurements; Index++)
         {
            if(MeasurementData[Index] != NULL)
            {
               BTPS_FreeMemory(MeasurementData[Index]);
               MeasurementData[Index] = NULL;
            }
         }

         /* Free the memory allocated for the measurement data strcuture*/
         /* pointers.                                                   */
         BTPS_FreeMemory(MeasurementData);
      }
      else
         printf("Could not allocate memory CGMS Measurement Data structures.\r\n");
   }
   else
   {
      DisplayFunctionError("CGMS_Decode_CGMS_Measurement", Result);
      printf("Could not determine the number of measurements to decode.\r\n");
   }
}

   /* This function displays the usage of ConfigureRACP.                */
static void DisplayRACPCommandUsage(void)
{
   DisplayUsage("ConfigureRACP [Command] [Filter] [Parameters(If required will be listed below)].");

   printf(" NOTE: The Filter Type Value 'Time Offset' is always used.\r\n\r\n");

   printf(" Where Command is:\r\n");
   printf("  1 = ReportStoredRecordsRequest\r\n");
   printf("  2 = DeleteStoredRecordsRequest\r\n");
   printf("  3 = AbortOperationRequest\r\n");
   printf("  4 = NumberOfStoredRecordsRequest\r\n\r\n");

   printf(" Where Filter and Parameters are:\r\n");
   printf("  1 = AllRecords\r\n");
   printf("  2 = LessThanOrEqualTo [Time Offset(UINT16)]\r\n");
   printf("  3 = GreaterThanOrEqualTo [Time Offset(UINT16)]\r\n");
   printf("  4 = WithinRangeOf [Time Offset Lower(UINT16)] [Time Offset Upper(UINT16)]\r\n");
   printf("  5 = FirstRecord\r\n");
   printf("  6 = LastRecord\r\n\n");
}

   /* The following function is used to decode and display a Record     */
   /* Access Control Point Response Data at client end while getting    */
   /* read response from CGMS Service.  The first parameter provides te */
   /* buffer length of the data to be decoded, the second parameter     */
   /* provides the data packet to be decoded.                           */
static void DecodeDisplayRACPResponse(Word_t BufferLength, Byte_t *Buffer)
{
   CGMS_RACP_Response_Data_t  RACPResponseData;
   int                        Result;

   /* Verify that the input parameters seem semi-valid.                 */
   if((BufferLength) && (Buffer) )
   {
      /* Decode the CGMS Context.                                       */
      Result = CGMS_Decode_Record_Access_Control_Point_Response(BufferLength, Buffer, &RACPResponseData);
      if(!Result)
      {
         if(RACPResponseData.ResponseType == rarCGMSResponseCode)
         {
            printf("      ResponseCode: 0x%02X", (Byte_t)RACPResponseData.ResponseData.ResponseCodeValue.ResponseCodeValue);
            switch(RACPResponseData.ResponseData.ResponseCodeValue.ResponseCodeValue)
            {
               case CGMS_RACP_RESPONSE_CODE_SUCCESS:
                  printf(" (Success)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_OPCODE_NOT_SUPPORTED:
                  printf(" (Op Code not supported)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_INVALID_OPERATOR:
                  printf(" (Invalid operator)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED:
                  printf(" (Operator not supported)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_INVALID_OPERAND:
                  printf(" (Invalid operand)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_NO_RECORDS_FOUND:
                  printf(" (No records found)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_ABORT_UNSUCCESSFUL:
                  printf(" (Abort unsuccessful)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED:
                  printf(" (Procedure not completed)\r\n");
                  break;
               case CGMS_RACP_RESPONSE_CODE_OPERAND_NOT_SUPPORTED:
                  printf(" (Operand not supported)\r\n");
                  break;
               default:
                  printf(" (Unknown response code)\r\n");
                  break;
            }
         }
         else
         {
            if(RACPResponseData.ResponseType == rarCGMSNumberOfStoredRecords)
            {
               printf("      NumberOfStoredRecords: %u\r\n", RACPResponseData.ResponseData.NumberOfStoredRecordsResult);
            }
            else
            {
               printf("Invalid ResponseType.\r\n");
            }
         }
      }
      else
         DisplayFunctionError("CGMS_Decode_Record_Access_Control_Point_Response", Result);
   }
   else
   {
      printf("Invalid RACP Response buffer.\r\n");
   }
}

   /* This function displays the usage of ConfigureSOCP.                */
static void DisplaySOCPCommandUsage(void)
{
   DisplayUsage("ConfigureSOCP [Command] [Parameters(If required will be listed below)].");

   printf(" Where Command is:\r\n");
   printf("  1  = Set Communication Interval [Communication Interval in Minutes(UINT8)]\r\n");
   printf("  2  = Get Communication Interval\r\n");
   printf("  4  = Set Glucose Calibration Value\r\n");
   printf("  5  = Get Glucose Calibration Value [Calibration Data Record Number (UINT16)]\r\n");
   printf("  7  = Set Patient High Alert Level [Alert Level (UINT16)]\r\n");
   printf("  8  = Get Patient High Alert Level\r\n");
   printf("  10 = Set Patient Low Alert Level [Alert Level (UINT16)]\r\n");
   printf("  11 = Get Patient Low Alert Level\r\n");
   printf("  13 = Set Hypo Alert Level [Alert Level (UINT16)]\r\n");
   printf("  14 = Get Hypo Alert Level\r\n");
   printf("  16 = Set Hyper Alert Level [Alert Level (UINT16)]\r\n");
   printf("  17 = Get Hyper Alert Level\r\n");
   printf("  19 = Set Rate of Decrease Alert Level [Alert Level (UINT16)]\r\n");
   printf("  20 = Get Rate of Decrease Alert Level\r\n");
   printf("  22 = Set Rate of Increase Alert Level [Alert Level (UINT16)]\r\n");
   printf("  23 = Get Rate of Increase Alert Level\r\n");
   printf("  25 = Reset Device Specific Alert Level\r\n");
   printf("  26 = Start Session\r\n");
   printf("  27 = Stop Session\r\n\r\n");
}

   /* The following function is used to decode and display a Specific   */
   /* Ops Control Point Response Data at client end while getting read  */
   /* response from CGMS Service.  The first parameter provides the     */
   /* buffer length of the data to be decoded, the second parameter     */
   /* provides the data packet to be decoded.                           */
static void DecodeDisplaySOCPResponse(Word_t BufferLength, Byte_t *Buffer)
{
   CGMS_Specific_Ops_Control_Point_Response_Data_t SOCPResponseData;
   int                                             Result;

   /* Verify that the input parameters seem semi-valid.                 */
   if((BufferLength) && (Buffer) )
   {
      /* Decode the CGMS Context.                                       */
      Result = CGMS_Decode_CGMS_Specific_Ops_Control_Point_Response(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), BufferLength, Buffer, &SOCPResponseData);
      if(!Result)
      {
         switch(SOCPResponseData.ResponseType)
         {
            case cgrResponse:
               printf("      ResponseCode: 0x%02X", (Byte_t)SOCPResponseData.ResponseData.ResponseCodeValue.ResponseCodeValue);
               switch(SOCPResponseData.ResponseData.ResponseCodeValue.ResponseCodeValue)
               {
                  case CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_SUCCESS:
                     printf(" (Success)\r\n");
                     break;
                  case CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_NOT_SUPPORTED:
                     printf(" (Op Code not supported)\r\n");
                     break;
                  case CGMS_SPECIFIC_OPS_CP_RESPONSE_INVALID_OPERAND:
                     printf(" (Invalid operand)\r\n");
                     break;
                  case CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PROCEDURE_NOT_COMPLETED:
                     printf(" (Procedure Not Completed)\r\n");
                     break;
                  case CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PARAMETER_OUT_OF_RANGE:
                     printf(" (Parameter out of range)\r\n");
                     break;
                  default:
                     printf(" (Unknown response code)\r\n");
                     break;
               }
               break;
            case cgrCommunicationIntervalResponse:
               printf("      Communication Interval: %u (Min)\r\n",      SOCPResponseData.ResponseData.CommunicationIntervalMinutes);
               break;
            case cgrCalibrationValueResponse:
               printf("      Calibration Data Record:\r\n");
               printf("         Record Number:         %u\r\n",          SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationDataRecordNumber);
               printf("         Status:                0x%02X\r\n",      SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationStatus);
               printf("         Glucose Concentration: %u (mg/dL)\r\n",  SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationGlucoseConcentration);
               printf("         Time:                  %u (Min)\r\n",    SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationTime);
               printf("         Next Time:             %u (Min)\r\n",    SOCPResponseData.ResponseData.CalibrationDataRecord.NextCalibrationTime);
               printf("         Type Sample-Location:  0x%02X\r\n",      SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationTypeSampleLocation);
               printf("            Type:               %s\r\n",          CGMSFeatureTypeBitStrings[(SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationTypeSampleLocation & 0x0F) - 1]);
               printf("            Sample Location:    %s\r\n",          CGMSFeatureSampleLocationBitStrings[(SOCPResponseData.ResponseData.CalibrationDataRecord.CalibrationTypeSampleLocation >> 4) - 1]);
               break;
            case cgrPatientHighAlertLevelResponse:
               printf("      Patient High Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            case cgrPatientLowAlertLevelResponse:
               printf("      Patient Low Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            case cgrHypoAlertLevelResponse:
               printf("      Hypo Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            case cgrHyperAlertLevelResponse:
               printf("      Hyper Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            case cgrRateOfDecreaseAlertLevelResponse:
               printf("      Rate of Increase Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            case cgrRateOfIncreaseAlertLevelResponse:
               printf("      Rate of Decrease Alert Level Response:\r\n");
               printf("         Alert Level: %u (mg/dL)\r\n", SOCPResponseData.ResponseData.AlertLevel);
               break;
            default:
               printf("Invalid ResponseType.\r\n");
         }
      }
      else
         DisplayFunctionError("CGMS_Decode_CGMS_Specific_Ops_Control_Point_Response", Result);
   }
   else
   {
      printf("Invalid SOCP Response buffer.\r\n");
   }
}

   /* This function displays the usage of SetStatus.                    */
static void DisplaySensorStatusUsage(void)
{
   DisplayUsage("SetStatus [Status Flag(UINT8)] [Cal/Temp Flag(UINT8)] [Warning Flag(UINT8)]");

   printf(" Where Status Flag bits are: \r\n");
   printf("  0x01 = Time Synchronization between sensor and collector required (Set Automatically. Can be overwritten.)\r\n");
   printf("  0x02 = Calibration Not Allowed\r\n");
   printf("  0x04 = Calibration Recommended (Set Automatically. Can be overwritten.)\r\n");
   printf("  0x08 = Calibration Required\r\n");
   printf("  0x10 = Sensor Result Lower than the Patient Low Level\r\n");
   printf("  0x20 = Sensor Result Higher than the Patient High Level\r\n");
   printf("  0x40 = Sensor Result Lower than the Hypo Level\r\n");
   printf("  0x80 = Sensor Result Higer than the Hyper Level\r\n\r\n");

   printf(" Where Cal/Temp Flag bits are: \r\n");
   printf("  0x01 = Sensor Rate of Decrease Exceeded\r\n");
   printf("  0x02 = Sensor Rate of Increase Exceeded\r\n");
   printf("  0x04 = Device Specific Alert\r\n");
   printf("  0x08 = Sensor Malfunction\r\n");
   printf("  0x10 = Temperature Too High For Valid Test at Time of Measurement\r\n");
   printf("  0x20 = Temperature Too Low For Valid Test at Time of Measurement\r\n");
   printf("  0x40 = Sensor Result Lower than the Device can Process\r\n");
   printf("  0x80 = Sensor Result Higher than the Device can Process\r\n\r\n");

   printf(" Where Warning Flag bits are: \r\n");
   printf("  0x01 = Session Stopped (Set Automatically. Will be ignored.)\r\n");
   printf("  0x02 = Device Battery Low\r\n");
   printf("  0x04 = Sensor Type Incorrect for Device\r\n");
   printf("  0x08 = General Device Fault has Occured in the Sensor\r\n");
}

   /* The following displays the DIS Characteristic in either ASCII     */
   /* Printable format or Hex format as per the value passed in it.     */
   /* If any of the character in the String value is non ASCII then     */
   /* String value will be printed in Hex format else in ASCII String   */
   /* format.                                                           */
   /* ** NOTE ** Value string parameter must be '\0' terminated string. */
static void DisplayDISCharacteristicValue(char* Value)
{
   Boolean_t   ASCIIPrintable = TRUE;
   char       *Ptr            = Value;
   /* Check if String is ASCII Printable                                */
   while((*Ptr != '\0') && (ASCIIPrintable == TRUE))
   {
      /* check if the character is out of ASCII printable range         */
      if(*Ptr < 0x20 || *Ptr > 0x7E)
      {
         ASCIIPrintable = FALSE;
      }
      ++Ptr;
   }
   if(ASCIIPrintable)
   {
       printf("%s",Value);
   }
   else
   {
      Ptr = Value;
      printf("0x");
      while(*Ptr != '\0')
      {
         printf("%02X",*Ptr++);
      }
   }
   printf("\r\n");
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
   if(UI_Mode == UI_MODE_IS_SERVER)
   {
      printf("\r\n");
      printf("******************************************************************\r\n");
      printf("* LinuxCGMP Server                                               *\r\n");
      printf("******************************************************************\r\n");
      printf("* Command Options General: Help, Quit, GetLocalAddress,          *\r\n");
      printf("*                          EnableDebug, GetMTU, SetMTU           *\r\n");
      printf("* Command Options GAPLE:   SetDiscoverabilityMode,               *\r\n");
      printf("*                          SetConnectabilityMode,                *\r\n");
      printf("*                          SetPairabilityMode,                   *\r\n");
      printf("*                          ChangePairingParameters,              *\r\n");
      printf("*                          AdvertiseLE, StartScanning,           *\r\n");
      printf("*                          StopScanning,                         *\r\n");
      printf("*                          Connect, Disconnect,                  *\r\n");
      printf("*                          PairLE,                               *\r\n");
      printf("*                          LEPasskeyResponse,                    *\r\n");
      printf("*                          QueryEncryptionMode, SetPasskey,      *\r\n");
      printf("* Command Options DIS:     DiscoverDIS, DISGATTRead,             *\r\n");
      printf("* Command Options GAPS:    DiscoverGAPS, GetLocalName,           *\r\n");
      printf("*                          SetLocalName, GetRemoteName,          *\r\n");
      printf("*                          SetLocalAppearance,                   *\r\n");
      printf("*                          GetLocalAppearance,                   *\r\n");
      printf("*                          GetRemoteAppearance,                  *\r\n");
      printf("* Command Options CGMS:    RegisterCGMS, UnRegisterCGMS,         *\r\n");
      printf("*                          GetFeatures,                          *\r\n");
      printf("*                          SetFeatures,                          *\r\n");
      printf("*                          GetSessionStartTime,                  *\r\n");
      printf("*                          GetSessionRunTime,                    *\r\n");
      printf("*                          SetStatus,                            *\r\n");
      printf("*                          GetStatus,                            *\r\n");
      printf("* Measurement Options:     Create,                               *\r\n");
      printf("*                          DeleteList,                           *\r\n");
      printf("*                          PrintList,                            *\r\n");
      printf("* Cal Record Options:      CreateCalRecord,                      *\r\n");
      printf("*                          DeleteCalRecords,                     *\r\n");
      printf("*                          PrintCalRecords,                      *\r\n");
      printf("* Session Options:         Start,                                *\r\n");
      printf("*                          Stop,                                 *\r\n");
      printf("* Exit LinuxCGMP:          Quit                                  *\r\n");
      printf("******************************************************************\r\n");
   }
   else
   {
      if(UI_Mode == UI_MODE_IS_CLIENT)
      {
         printf("\r\n");
         printf("******************************************************************\r\n");
         printf("* LinuxCGMP Client                                               *\r\n");
         printf("******************************************************************\r\n");
         printf("* Command Options General: Help, Quit, GetLocalAddress,          *\r\n");
         printf("*                          EnableDebug, GetMTU, SetMTU           *\r\n");
         printf("* Command Options GAPLE:   SetDiscoverabilityMode,               *\r\n");
         printf("*                          SetConnectabilityMode,                *\r\n");
         printf("*                          SetPairabilityMode,                   *\r\n");
         printf("*                          ChangePairingParameters,              *\r\n");
         printf("*                          AdvertiseLE, StartScanning,           *\r\n");
         printf("*                          StopScanning,                         *\r\n");
         printf("*                          Connect, Disconnect,                  *\r\n");
         printf("*                          PairLE,                               *\r\n");
         printf("*                          LEPasskeyResponse,                    *\r\n");
         printf("*                          QueryEncryptionMode, SetPasskey,      *\r\n");
         printf("* Command Options DIS:     DiscoverDIS, DISGATTRead,             *\r\n");
         printf("* Command Options GAPS:    DiscoverGAPS, GetLocalName,           *\r\n");
         printf("*                          SetLocalName, GetRemoteName,          *\r\n");
         printf("*                          SetLocalAppearance,                   *\r\n");
         printf("*                          GetLocalAppearance,                   *\r\n");
         printf("*                          GetRemoteAppearance,                  *\r\n");
         printf("* Command Options BMS:     DiscoverBMS, DeleteBond,              *\r\n");
         printf("* Command Options CGMS:    DiscoverCGMS, ConfigureRemoteCGMS,    *\r\n");
         printf("*                          ConfigureRACP,                        *\r\n");
         printf("*                          ConfigureSOCP,                        *\r\n");
         printf("*                          GetFeatures,                          *\r\n");
         printf("*                          SetSessionStartTime,                  *\r\n");
         printf("*                          GetSessionStartTime,                  *\r\n");
         printf("*                          GetSessionRunTime,                    *\r\n");
         printf("*                          GetStatus,                            *\r\n");
         printf("* Measurement Options:     DeleteList,                           *\r\n");
         printf("*                          PrintList,                            *\r\n");
         printf("* Exit LinuxCGMP:          Quit                                  *\r\n");
         printf("******************************************************************\r\n");
      }
      else
      {
         printf("\r\n");
         printf("******************************************************************\r\n");
         printf("* LinuxCGMP Sample Application                                   *\r\n");
         printf("******************************************************************\r\n");
         printf("* Command Options: Server, Client, Help                          *\r\n");
         printf("* Exit LinuxCGMP: Quit                                           *\r\n");
         printf("******************************************************************\r\n");
      }
   }

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
         if(TempParam->Params[1].intParam == 0)
            LE_Parameters.ConnectableMode = lcmNonConnectable;
         else
            LE_Parameters.ConnectableMode = lcmConnectable;

         /* The Mode was changed successfully.                          */
         printf("Connectability Mode: %s.\r\n", (LE_Parameters.ConnectableMode == lcmNonConnectable)?"Non Connectable":"Connectable");

         GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);

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
   int                              Result;
   int                              ret_val;
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

   /* The following function is responsible for determining if a Bond   */
   /* exists between the collector and the sensor (LE or BR/EDR).  This */
   /* function returns one if a Bond exists, otherwise it returns a     */
   /* negative integer indicating that an error occured or zero if no   */
   /* Bond exists.                                                      */
static int CheckForBond(void)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Check to make sure we are currently connected.                    */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         if(((DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID) && (!GAP_LE_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAPEncryptionMode) && (GAPEncryptionMode != emDisabled))) ||
            ((DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID) && (!GAP_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAPEncryptionMode) && (GAPEncryptionMode != emDisabled))))
         {
            /* Simply return success to the caller.                     */
            ret_val = 1;
         }
      }

      /* Inform the user if a bond has not been found.                  */
      if(ret_val == 0)
         printf("No bond exists.\r\n");
   }
   else
      printf("No connection.\r\n");

   return ret_val;
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
   Word_t    Appearance;

   /* First, check that valid Bluetooth Stack ID exists.                */
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

            if(CGMSInstanceID)
            {
               /* Advertise the CGMS Server (1 byte type & 2 bytes      */
               /* UUID).                                                */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               CGMS_ASSIGN_CGMS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));

               /* Advertise the appearance.                             */
               if(!GAPS_Query_Device_Appearance(BluetoothStackID, GAPSInstanceID, &Appearance))
               {
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[7] = 3;
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[8] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[9] = ((Appearance >> 8) | (Appearance >> 8));
               }

               /* Advertise Public Target Address.                      */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[11] = 7;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[12] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_PUBLIC_TARGET_ADDRESS;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[13] = 0xDE;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[14] = 0x5A;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[15] = 0x05;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[16] = 0xDC;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[17] = 0x1B;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[18] = 0x00;

               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[19] = 7;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[20] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_RANDOM_TARGET_ADDRESS;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[21] = 0xDE;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[22] = 0x5A;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[23] = 0x05;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[24] = 0xDC;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[25] = 0x1B;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[26] = 0x00;
            }

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[7] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[10] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[19] + 6), &(Advertisement_Data_Buffer.AdvertisingData));
            if(!ret_val)
            {
               BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

               /* Set the Scan Response Data.  First include the device */
               /* name.                                                 */
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
      if(!ScanningInProgress)
      {
         ScanningInProgress = TRUE;
         /* Simply start scanning.                                      */
         if(!StartScan(BluetoothStackID))
         ret_val = 0;
         else
         ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("\r\nThe Scanning is Already in Progress!\r\n");
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
      if(ScanningInProgress)
      {
         ScanningInProgress = FALSE;
         /* Simply stop scanning.                                       */
         if(!StopScan(BluetoothStackID))
            ret_val = 0;
            else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         printf("\r\nScan is not in progress, does not need to stop scanning.\r\n");
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
      /* Initialize the Name Buffer to all ZEROs.                       */
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

   /* The following function is responsible for performing a DIS Service*/
   /* Discovery Operation.  This function will return zero on successful*/
   /* execution and a negative value on errors.                         */
static int DiscoverDIS(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID[1];
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
            UUID[0].UUID_Type = guUUID_16;
            DIS_ASSIGN_DIS_SERVICE_UUID_16(UUID[0].UUID.UUID_16);

            /* Start the service discovery process.                     */
            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdDIS);
            if(!ret_val)
            {
               /* Display success message.                              */
               printf("GATT_Service_Discovery_Start success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
            }
            else
            {
               /* An error occur so just clean-up.                      */
               DisplayFunctionError("GDIS_Service_Discovery_Start", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("Service Discovery Operation Outsanding for Device.\r\n");

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

   /* The following function is responsible for retrieving a DIS Service*/
   /* characterstic value.  This function will return zero on successful*/
   /* execution and a negative value on errors.                         */
static int ReadDeviceInfo(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   Word_t        AttributeHandle;
   Word_t        InputAttributeHandle;
   DeviceInfo_t *DeviceInfo;

   if((TempParam) && (TempParam->NumberofParameters > 0))
   {
      InputAttributeHandle = (Word_t)TempParam->Params[0].intParam;
      if((InputAttributeHandle >= daManufacturerName) && (InputAttributeHandle <= daIEEEDataList))
      {
         /* Verify that there is a connection that is established.      */
         if(ConnectionID)
         {
            /* Get the device info for the connection device.           */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               switch(InputAttributeHandle)
               {
                  case daManufacturerName:
                     AttributeHandle = DeviceInfo->DISClientInfo.ManufacturerName;
                     break;
                  case daModelNumber:
                     AttributeHandle = DeviceInfo->DISClientInfo.ModelNumber;
                     break;
                  case daSerialNumber:
                     AttributeHandle = DeviceInfo->DISClientInfo.SerialNumber;
                     break;
                  case daHardwareRevision:
                     AttributeHandle = DeviceInfo->DISClientInfo.HardwareRevision;
                     break;
                  case daFirmwareRevision:
                     AttributeHandle = DeviceInfo->DISClientInfo.FirmwareRevision;
                     break;
                  case daSoftwareRevision:
                     AttributeHandle = DeviceInfo->DISClientInfo.SoftwareRevision;
                     break;
                  case daSystemID:
                     AttributeHandle = DeviceInfo->DISClientInfo.SystemID;
                     break;
                  case daIEEEDataList:
                     AttributeHandle = DeviceInfo->DISClientInfo.IEEECertificationDataList;
                     break;
                  default:
                     printf("Unknown Attribute Handle.\r\n");
                     ret_val = FUNCTION_ERROR;
                     break;
               }

               if(!ret_val)
               {
                  ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_DIS, InputAttributeHandle);
                  if(ret_val > 0)
                  {
                     /* Display success message.                        */
                     printf("GATT_Read_Value_Request success.\r\n");
                     ret_val = 0;
                  }
                  else
                  {
                     /* An error occur so just clean-up.                */
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                     ret_val = FUNCTION_ERROR;
                  }
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
      }
      else
      {
         DisplayGATTReadUsage();
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      DisplayGATTReadUsage();
      ret_val = FUNCTION_ERROR;
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
                        printf("Log File Name: %s\r\n", LogFileName);

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

   /* The following function is responsible for changing the User       */
   /* Interface Mode to Server and starting the server.                 */
static int ServerMode(ParameterList_t *TempParam)
{
   int ret_val = EXIT_CODE;

   /* Automatically register the OTS service.                           */

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!CGMSInstanceID)
      {
         /* Attempt to perform any needed initialization on the server. */
         if(!InitializeServer())
         {
            /* Register the CGMS Service with GATT.  Note enables all   */
            /* features.                                                */
            ret_val = CGMS_Initialize_Service(BluetoothStackID, CGMS_SERVICE_FLAGS_DUAL_MODE, CGMS_EventCallback, 0, &CGMSInstanceID);
            if((ret_val > 0) && (CGMSInstanceID > 0))
            {
               /* Display success message.                              */
               printf("Successfully Registered The CGMS Service.\r\n");

               /* Save the InstanceID of the registered service.        */
               CGMSInstanceID = (unsigned int)ret_val;

               /* Display the server user interface.                    */
               UI_Mode = UI_MODE_IS_SERVER;
               UserInterface_Server();

               /* Simply return that the server has been initialized.   */
               ret_val = 0;
            }
            else
            {
               DisplayFunctionError("CGMS_Initialize_Service", ret_val);
               ret_val = EXIT_CODE;
            }
         }
         else
         {
            printf("Unable to initialize the Server.\r\n");
            ret_val = EXIT_CODE;
         }
      }
      else
      {
         printf("CGMS Service already registered.\r\n");
         ret_val = EXIT_CODE;
      }
   }
   else
   {
      printf("Connection currently active.\r\n");
      ret_val = EXIT_CODE;
   }

   return(ret_val);
}

   /* The following function is responsible for changing the User       */
   /* Interface Mode to Client.                                         */
static int ClientMode(ParameterList_t *TempParam)
{
   int ret_val = EXIT_CODE;

   /* Attempt to perform any needed initialization on the Client.       */
   if((ret_val = InitializeClient()) == 0)
   {
      UI_Mode = UI_MODE_IS_CLIENT;
      UserInterface_Client();
   }
   else
   {
      printf("Unable to initialize the Client.\r\n");
      ret_val = EXIT_CODE;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a CGMS      */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterCGMS(ParameterList_t *TempParam)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!CGMSInstanceID)
      {
         /* Register the CGMS Service with GATT.                        */
         ret_val = CGMS_Initialize_Service(BluetoothStackID, CGMS_SERVICE_FLAGS_DUAL_MODE, CGMS_EventCallback, 0, &CGMSInstanceID);
         if((ret_val > 0) && (CGMSInstanceID > 0))
         {
            /* Display success message.                                 */
            printf("Successfully registered CGMS Service.\r\n");

            /* Save the ServiceID of the registered service.            */
            CGMSInstanceID  = (unsigned int)ret_val;
            ret_val        = 0;
         }
      }
      else
      {
         printf("CGMS Service already registered.\r\n");

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

   /* The following function is responsible for unregistering a CGMS    */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnregisterCGMS(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that a service is registered.                              */
   if(CGMSInstanceID)
   {
      /* If there is a connected device, then first disconnect it.      */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR);

      /* Unregister the CGMS Service with GATT.                         */
      ret_val = CGMS_Cleanup_Service(BluetoothStackID, CGMSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("Successfully unregistered CGMS Service.\r\n");

         /* Save the ServiceID of the registered service.               */
         CGMSInstanceID = 0;
      }
      else
         DisplayFunctionError("CGMS_Cleanup_Service", ret_val);
   }
   else
      printf("CGMS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function is responsible for performing a CGMS       */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverCGMS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID[1];
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!CGMSInstanceID)
   {
      /* Verify that there is a connection that is established.         */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               /* Verify that no service discovery is outstanding for   */
               /* this device.                                          */
               if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
               {
                  /* Configure the filter so that only the CGMS Service */
                  /* is discovered.                                     */
                  UUID[0].UUID_Type = guUUID_16;
                  CGMS_ASSIGN_CGMS_SERVICE_UUID_16(&(UUID[0].UUID.UUID_16));

                  /* Start the service discovery process.               */
                  ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdCGMS);
                  if(!ret_val)
                  {
                     /* Display success message.                        */
                     printf("GATT_Service_Discovery_Start success.\r\n");

                     /* Flag that a Service Discovery Operation is      */
                     /* outstanding.                                    */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
                  }
                  else
                     DisplayFunctionError("GDIS_Service_Discovery_Start", ret_val);
               }
               else
                  printf("Service Discovery Operation Outsanding for Device.\r\n");
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("No Connection Established\r\n");
   }
   else
      printf("Cannot discover CGMS Services when registered as a service.\r\n");

   return(ret_val);
}

   /* The following function is responsible for configuring a CGMS      */
   /* Service on a remote device.  This function will return zero on    */
   /* successful execution and a negative value on errors.              */
static int ConfigureRemoteCGMS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   int           gm_ret_val   = FUNCTION_ERROR;
   int           mc_ret_val   = FUNCTION_ERROR;
   int           rcap_ret_val = FUNCTION_ERROR;
   int           scap_ret_val = FUNCTION_ERROR;
   int           ret_val      = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 2))
   {
      /* Verify that we are not configured as a server                  */
      if(!CGMSInstanceID)
      {
         /* Verify that there is a connection that is established.      */
         if(ConnectionID)
         {
            /* Get the device info for the connection device.           */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Check to make sure we are bonded.                     */
               if(CheckForBond())
               {
                  /* Determine if service discovery has been performed  */
                  /* on this device                                     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     ret_val      = 0;
                     gm_ret_val   = 0;
                     mc_ret_val   = 0;
                     rcap_ret_val = 0;
                     scap_ret_val = 0;

                     printf("Attempting to configure CCCDs...\r\n");

                     /* Determine if Glucose Measurement CC is supported*/
                     /* (mandatory).                                    */
                     if(DeviceInfo->ClientInfo.CGMS_Measurement_Client_Configuration)
                     {
                        gm_ret_val = EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.CGMS_Measurement_Client_Configuration, (TempParam->Params[0].intParam ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE : 0), GATT_ClientEventCallback_CGMS);
                     }

                     if(gm_ret_val > 0)
                     {
                        printf("CGMS Measurement Configuration Success.\r\n");

                        gm_ret_val = 0;
                     }
                     else
                     {
                        /* CC Configuration failed, check to see if it  */
                        /* was from a call to                           */
                        /* EnableDisableNotificationsIndications        */
                        if(gm_ret_val < 0)
                        {
                           printf("EnableDisableNotificationsIndications %d", ret_val);
                        }

                        gm_ret_val = FUNCTION_ERROR;
                     }

                     if(DeviceInfo->ClientInfo.RACP_Client_Configuration)
                     {
                        rcap_ret_val = EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.RACP_Client_Configuration, (TempParam->Params[1].intParam ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE : 0), GATT_ClientEventCallback_CGMS);
                     }

                     if(rcap_ret_val > 0)
                     {
                        printf("RACP Configuration Success.\r\n");

                        rcap_ret_val = 0;
                     }
                     else
                     {
                        /* CC Configuration failed, check to see if it  */
                        /* was from a call to                           */
                        /* EnableDisableNotificationsIndications        */
                        if(rcap_ret_val < 0 )
                        {
                           DisplayFunctionError("EnableDisableNotificationsIndications", ret_val);
                        }

                        rcap_ret_val = FUNCTION_ERROR;
                     }

                     if(DeviceInfo->ClientInfo.SOCP_Client_Configuration)
                     {
                        scap_ret_val = EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.SOCP_Client_Configuration, (TempParam->Params[2].intParam ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE : 0), GATT_ClientEventCallback_CGMS);
                     }

                     if(scap_ret_val > 0)
                     {
                        printf("SOCP Configuration Success.\r\n");

                        scap_ret_val = 0;
                     }
                     else
                     {
                        /* CC Configuration failed, check to see if it  */
                        /* was from a call to                           */
                        /* EnableDisableNotificationsIndications        */
                        if(scap_ret_val < 0 )
                        {
                           DisplayFunctionError("EnableDisableNotificationsIndications", ret_val);
                        }

                        scap_ret_val = FUNCTION_ERROR;
                     }
                  }
                  else
                     printf("Service discovery has not been performed on this device.\r\n");
               }
            }
            else
               printf("No Device Info.\r\n");
         }
         else
            printf("No Connection Established.\r\n");
      }
      else
         printf("Cannot configure remote CGMS Services when registered as a service.\r\n");
   }
   else
   {
      printf("Usage: ConfigureRemoteCGMS [Notify CGMS Measurement (0 = disable, 1 = enable)]\r\n");
      printf("                          [Indicate Record Access Control Point (0 = disable, 1 = enable)]\r\n");
      printf("                          [Indicate Specific Ops Control Point (0 = disable, 1 = enable)]\r\n");
   }

   if((gm_ret_val == 0) && (mc_ret_val == 0) && (rcap_ret_val == 0))
   {
      ret_val = 0;
   }
   else
   {
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for writing the CGMS        */
   /* Feature.  It can be executed only by a server.  This function will*/
   /* return zero on successful execution and a negative value on errors*/
static int SetSupportedFeatures(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Check for a registered CGMS Server                                */
   if(CGMSInstanceID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Set the supported features.                                 */
         SensorSupportedFeatures = (DWord_t)TempParam->Params[0].intParam;
      }
      else
      {
         DisplayUsage("SetFeatures [BitMask]");
         DisplayUsageCGMSFeatureBitMask();
      }
   }
   else
   {
      if(ConnectionID)
         printf("Cannot write Glucose Feature data as a client.\r\n");
      else
         printf("CGMS server not registered.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for reading the CGMS        */
   /* Feature.  It can be executed by a server or as a client with an   */
   /* open connection to a remote server If executed as a client, a GATT*/
   /* read request will be generated, and the results will be returned  */
   /* as a response in the GATT client event callback.  This function   */
   /* will return zero on successful execution and a negative value on  */
   /* errors                                                            */
static int GetSupportedFeatures(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   int           ret_val = 0;
   int           Index;

   /* First check for a registered CGMS Server                          */
   if(CGMSInstanceID)
   {
      /* Print the supported features of the server.                    */
      printf("CGMS Features:         0x%08X\r\n", SensorSupportedFeatures);
      for(Index = 0; Index < (NUM_CGMS_FEATURE_BIT_STRINGS+1); Index++)
      {
         if(SensorSupportedFeatures & ((DWord_t)pow(2,Index)))
         {
            printf("   %s\r\n", CGMSFeatureBitStrings[Index]);
         }
      }

      printf("\r\nType Sample Location: 0x%02X\r\n", SensorTypeSampleLocation);
      printf("   Type:              %s.\r\n", CGMSFeatureTypeBitStrings[(SensorTypeSampleLocation & 0x0F) - 1]);
      printf("   Sample Location:   %s.\r\n", CGMSFeatureSampleLocationBitStrings[(SensorTypeSampleLocation >> 4) - 1]);
   }
   else
   {
      /* Check to see if we are configured as a client with an active   */
      /* connection                                                     */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               /* Verify that the client has received a valid Glucose   */
               /* Feature Attribute Handle.                             */
               if(DeviceInfo->ClientInfo.CGMS_Feature != 0)
               {
                  /* Finally, submit a read request to the server       */
                  if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.CGMS_Feature, GATT_ClientEventCallback_CGMS, DeviceInfo->ClientInfo.CGMS_Feature)) > 0)
                  {
                     printf("Get Supported Features sent, Transaction ID = %u", ret_val);

                     ret_val = 0;
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
               else
                  printf("Error - CGMS Feature attribute handle is invalid!\r\n");
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Either a CGMS server must be registered or a CGMS client must be connected.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for writing the session     */
   /* start time characateristics.  It can only be executed by the      */
   /* client.  This function returns zero on success or a negative      */
   /* integer indicating that an error occured.                         */
static int SetSessionStartTime(ParameterList_t *TempParam)
{
   int                            ret_val = FUNCTION_ERROR;
   DeviceInfo_t                  *DeviceInfo;
   time_t                         CollectorUserFacingTime;
   struct tm                     *CollectorUserFacingTimeTM;
   CGMS_Session_Start_Time_Data_t SessionStartTime;
   int                            BufferLength;
   Byte_t                        *Buffer;

   /* CGMS Service is not registered so we are configured as the client.*/
   if((!CGMSInstanceID) && (ConnectionID))
   {
      /* Get the device info for the connected device.                  */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Check to make sure we are bonded.                           */
         if(CheckForBond())
         {
            /* Verify that the client has received a valid Session Start*/
            /* Time Attribute Handle.                                   */
            if(DeviceInfo->ClientInfo.CGMS_Session_Start_Time != 0)
            {
               /* We need to determine the user facing time of the      */
               /* client.                                               */
               CollectorUserFacingTime       = time(NULL);
               CollectorUserFacingTimeTM     = localtime(&CollectorUserFacingTime);

               /* Format the session start time.                        */
               SessionStartTime.Time.Seconds = CollectorUserFacingTimeTM->tm_sec;
               SessionStartTime.Time.Minutes = CollectorUserFacingTimeTM->tm_min;
               SessionStartTime.Time.Hours   = CollectorUserFacingTimeTM->tm_hour;
               SessionStartTime.Time.Day     = CollectorUserFacingTimeTM->tm_mday;
               SessionStartTime.Time.Month   = CollectorUserFacingTimeTM->tm_mon+1;
               SessionStartTime.Time.Year    = CollectorUserFacingTimeTM->tm_year+1900;

               SessionStartTime.DSTOffset    = dsoDaylightTime;
               SessionStartTime.TimeZone     = tizUTCMinus700;

               /* Determine the buffer length.                          */
               if((BufferLength = CGMS_Format_Session_Start_Time(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), &SessionStartTime, 0, NULL)) > 0)
               {
                  /* Allocate memory for the buffer.                    */
                  if((Buffer = (Byte_t *)BTPS_AllocateMemory((Word_t)BufferLength)) != NULL)
                  {
                     /* Format the session start time.                  */
                     if((ret_val = CGMS_Format_Session_Start_Time(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), &SessionStartTime, (Word_t)BufferLength, Buffer)) == BufferLength)
                     {
                        /* Finally, submit a write request to the       */
                        /* server.                                      */
                        ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.CGMS_Session_Start_Time, BufferLength, Buffer, GATT_ClientEventCallback_CGMS, (unsigned long)DeviceInfo->ClientInfo.CGMS_Session_Start_Time);
                        if(ret_val > 0)
                        {
                           printf("\r\nWrite Request For The Session Start Time Is Sent, Transaction ID = %d\r\n", ret_val);
                           ret_val = 0;
                        }
                        else
                           DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                     }
                     else
                        DisplayFunctionError("CGMS_Format_Session_Start_Time", ret_val);

                     /* Free the buffer.                                */
                     BTPS_FreeMemory(Buffer);
                  }
                  else
                     printf("Could not allocate memory for the buffer.\r\n");
               }
               else
                  DisplayFunctionError("CGMS_Format_Session_Start_Time", BufferLength);
            }
            else
               printf("Error - Session Start Time Attribute Handle Is Invalid!\r\n");
         }
      }
      else
         printf("No Device Info.\r\n");
   }
   else
      printf("Must be a CGMS client and connected.\r\n");

   return ret_val;
}

   /* The following function is responsible for reading the session     */
   /* start time characteristic.  It can be executed by the client and  */
   /* the server.  This function returns zero on success or a negative  */
   /* integer indicating that an error occured.                         */
static int GetSessionStartTime(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;
   struct tm     TempTime;
   char          Time[80];

   /* Verify that the CGMS Service is registered.                       */
   if(CGMSInstanceID)
   {
      /* Print the Session Start Time to the user.                      */
      TempTime.tm_year = SensorSessionStartTime.Time.Year-1900;
      TempTime.tm_mon  = SensorSessionStartTime.Time.Month-1;
      TempTime.tm_mday = SensorSessionStartTime.Time.Day;
      TempTime.tm_hour = SensorSessionStartTime.Time.Hours;
      TempTime.tm_min  = SensorSessionStartTime.Time.Minutes;
      TempTime.tm_sec  = SensorSessionStartTime.Time.Seconds;
      strftime(Time, 80, "%Y/%m/%d %T", &TempTime);
      printf("CGMS Session Start Time:\r\n");
      printf("   Time:       %s\r\n",Time);
      printf("   Timezone:   %d\r\n", SensorSessionStartTime.TimeZone);
      printf("   DST Offset: %u\r\n", SensorSessionStartTime.DSTOffset);
   }
   else
   {
      /* CGMS Service is not registered so we are configured as the     */
      /* client.                                                        */
      if(ConnectionID)
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               /* Verify that the client has received a valid Session   */
               /* Start Time Attribute Handle.                          */
               if(DeviceInfo->ClientInfo.CGMS_Session_Start_Time != 0)
               {
                  /* Finally, submit a read request to the server.      */
                  ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.CGMS_Session_Start_Time, GATT_ClientEventCallback_CGMS, (unsigned long)DeviceInfo->ClientInfo.CGMS_Session_Start_Time);
                  if(ret_val > 0)
                  {
                     printf("Read Request For The Session Start Time Is Sent, Transaction ID = %d\r\n", ret_val);
                     ret_val = 0;
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
               else
                  printf("Error - Session Start Time Attribute Handle Is Invalid!\r\n");
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Either a CGMS server must be registered or a CGMS client must be connected.\r\n");
   }

   return ret_val;
}

   /* The following function is reponsible for reading the Session Run  */
   /* Time characteristic.  It can be executed by the client and the    */
   /* server.  This function returns zero on success or a negative      */
   /* integer indicating that an error occured.                         */
static int GetSessionRunTime(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that the CGMS Service is registered.                       */
   if(CGMSInstanceID)
   {
      /* Print the Session Run Time to the user.                        */
      printf("CGMS Session Run Time:   %u (Min)\r\n", SensorSessionRunTime);
   }
   else
   {
      /* CGMS Service is not registered so we are configured as the     */
      /* client.                                                        */
      if(ConnectionID)
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               /* Verify that the client has received a valid Session   */
               /* Run Time Attribute Handle.                            */
               if(DeviceInfo->ClientInfo.CGMS_Session_Run_Time != 0)
               {
                  /* Finally, submit a read request to the server.      */
                  ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.CGMS_Session_Run_Time, GATT_ClientEventCallback_CGMS, (unsigned long)DeviceInfo->ClientInfo.CGMS_Session_Run_Time);
                  if(ret_val > 0)
                  {
                     printf("Read Request For The Session Run Time Is Sent, Transaction ID = %d\r\n", ret_val);
                     ret_val = 0;
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
               else
                  printf("Error - Session Run Time Attribute Handle Is Invalid!\r\n");
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Either a CGMS server must be registered or a CGMS client must be connected.\r\n");
   }

   return ret_val;
}

   /* The following function is a utility function to set the bits of   */
   /* the sensor status annunciation octets that will be included with  */
   /* each generated measurement.  This function takes three parameters,*/
   /* one byte for the Status bits, one byte for the Cal/Temp bits, and */
   /* one byte for Warning bits.  This function returns zero for success*/
   /* or a negative integer indicating that an error occured.           */
static int SetStatus(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Check that the parameters are semi-valid.  Note the session       */
   /* stopped bit will be ignored.                                      */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
      SensorStatus = ((Byte_t)TempParam->Params[0].intParam | ((Byte_t)TempParam->Params[1].intParam << 8) | ((((Byte_t)TempParam->Params[2].intParam & 0xFE) | (((Byte_t)(SensorStatus >> 16) & 0x01))) << 16));
   else
      DisplaySensorStatusUsage();

   return ret_val;
}

   /* The following function is reponsible for reading the status       */
   /* characteristic.  It can be excuted by the client and the server.  */
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
static int GetStatus(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that the CGMS Service is registered.                       */
   if(CGMSInstanceID)
   {
      /* Print the Status to the user.                                  */
      printf("Sensor Status:   0x%08X.\r\n", SensorStatus);
   }
   else
   {
      /* CGMS Service is not registered so we are configured as the     */
      /* client.                                                        */
      if(ConnectionID)
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               /* Verify that the client has received a valid Status    */
               /* Attribute Handle.                                     */
               if(DeviceInfo->ClientInfo.CGMS_Status != 0)
               {
                  /* Finally, submit a read request to the server.      */
                  ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.CGMS_Status, GATT_ClientEventCallback_CGMS, (unsigned long)DeviceInfo->ClientInfo.CGMS_Status);
                  if(ret_val > 0)
                  {
                     printf("Read Request For The Status Is Sent, Transaction ID = %d\r\n", ret_val);
                     ret_val = 0;
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
               else
                  printf("Error - Status Attribute Handle Is Invalid!\r\n");
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Either a CGMS server must be registered or a CGMS client must be connected.\r\n");
   }

   return ret_val;
}

   /* The following function is responsible for indicating number of    */
   /* stored records to the connected remote device.  This function     */
   /* will return zero on successful execution and a negative value on  */
   /* errors.                                                           */
static int IndicateNumberOfStoredRecords(Word_t NumberOfStoredRecords)
{
   DeviceInfo_t *DeviceInfo;
   int           ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding RACP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.RACP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     /* Finally, indicate the number of stored records. */
                     ret_val = CGMS_Indicate_Number_Of_Stored_Records(BluetoothStackID,CGMSInstanceID,ConnectionID, NumberOfStoredRecords);
                     if(ret_val < 0)
                     {
                        DisplayFunctionError("CGMS_Indicate_Number_Of_Stored_Records", ret_val);
                     }
                     else
                     {
                        /* Clear the current outstanding RACP command   */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_RACP_OUTSTANDING;
                     }
                  }
                  else
                  {
                     printf("Client has not registered for Number Of Stored Records indications.\r\n");
                  }
               }
               else
               {
                  printf("There are no RACP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered.\r\n");
      }
   }

   return(ret_val);
}

   /* Indicate Record Access Control Point Result                       */
static int IndicateRACPResult(Byte_t CommandType, Byte_t ResponseCode)
{
   DeviceInfo_t *DeviceInfo = NULL;
   int           ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding RACP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.RACP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     ret_val = CGMS_Indicate_Record_Access_Control_Point_Result(BluetoothStackID, CGMSInstanceID, ConnectionID, CommandType, ResponseCode);
                     if(ret_val == 0)
                     {
                        /* Clear the current outstanding RACP command   */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_RACP_OUTSTANDING;
                     }
                     else
                     {
                        DisplayFunctionError("CGMS_Indicate_Record_Access_Control_Point_Result", ret_val);
                     }
                  }
                  else
                  {
                     printf("Client has not registered for Remote Access Control Point Indication \r\n");
                  }
               }
               else
               {
                 printf("There are no RACP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered\r\n");
      }
   }

   return(ret_val);
}


   /* The following function is responsible for configure a RACP Service*/
   /* on a remote device.  This function will return zero on successful */
   /* execution and a negative value on errors.                         */
static int ConfigureRACP(ParameterList_t *TempParam)
{
   CGMS_RACP_Format_Data_t  RACPRequestData;
   DeviceInfo_t            *DeviceInfo;
   unsigned int             BufferLength;
   Byte_t                   Buffer[CGMS_RACP_FORMAT_DATA_SIZE+1];
   int                      ret_val = 0;

   /* Verify that the input parameters are semi-valid.                  */
   if(TempParam->NumberofParameters >= 1)
   {
      /* Verify that there is a valid connection                        */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               RACPRequestData.CommandType = (Byte_t)TempParam->Params[0].intParam;

               /* Make sure the command is semi-valid.                  */
               if((RACPRequestData.CommandType >= racCGMSReportStoredRecordsRequest) && (RACPRequestData.CommandType <= racCGMSNumberOfStoredRecordsRequest))
               {
                  /* Determine the operator type.                       */
                  if(RACPRequestData.CommandType == racCGMSAbortOperationRequest)
                     RACPRequestData.OperatorType = raoCGMSNull;
                  else
                     RACPRequestData.OperatorType = (Byte_t)TempParam->Params[1].intParam;

                  /* Make sure the operator is semi-valid.              */
                  if(((RACPRequestData.OperatorType >= raoCGMSAllRecords) && (RACPRequestData.OperatorType <= raoCGMSLastRecord)) || ((RACPRequestData.CommandType == racCGMSAbortOperationRequest) && (RACPRequestData.OperatorType == raoCGMSNull)))
                  {
                     /* Determine the parameters.                       */
                     switch(RACPRequestData.OperatorType)
                     {
                        case raoCGMSGreaterThanOrEqualTo:
                        case raoCGMSLessThanOrEqualTo:
                           /* Check to make sure that the Parameters are*/
                           /* valid.                                    */
                           if(TempParam->NumberofParameters >= 3)
                              RACPRequestData.FilterParameters.TimeOffset = (Word_t)TempParam->Params[2].intParam;
                           else
                              ret_val = INVALID_PARAMETERS_ERROR;
                           break;
                        case raoCGMSWithinRangeOf:
                           /* Check to make sure that the Parameters are*/
                           /* valid.                                    */
                           if(TempParam->NumberofParameters >= 4)
                           {
                              RACPRequestData.FilterParameters.TimeOffsetRange.Minimum = (Word_t)TempParam->Params[2].intParam;
                              RACPRequestData.FilterParameters.TimeOffsetRange.Maximum = (Word_t)TempParam->Params[3].intParam;
                           }
                           else
                              ret_val = INVALID_PARAMETERS_ERROR;
                           break;
                        default:
                           /* No Parameters.                            */
                           break;
                     }

                     /* FilterType will always be TimeOffset regardless */
                     /* of the Operator.                                */
                     RACPRequestData.FilterType   = 0x01;

                     /* Only continue if no errors occured.             */
                     if(ret_val == 0)
                     {
                        BufferLength = CGMS_RACP_FORMAT_DATA_SIZE;
                        ret_val      = CGMS_Format_Record_Access_Control_Point_Command(&RACPRequestData, &BufferLength, Buffer);
                        if(ret_val == 0)
                        {
                           /* Finally, submit a write request to the    */
                           /* server.                                   */
                           if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Record_Access_Control_Point, BufferLength, ((void *)&Buffer), GATT_ClientEventCallback_CGMS, DeviceInfo->ClientInfo.Record_Access_Control_Point)) > 0)
                           {
                              /* Simply return success to the caller.   */
                              ret_val = 0;
                           }
                           else
                              DisplayFunctionError("GATT_Write_Request", ret_val);
                        }
                        else
                        {
                           DisplayFunctionError("CGMS_Format_Record_Access_Control_Point_Command", ret_val);
                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                        DisplayRACPCommandUsage();
                  }
                  else
                     DisplayRACPCommandUsage();
               }
               else
                  DisplayRACPCommandUsage();
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Connection not established.\r\n");
   }
   else
      DisplayRACPCommandUsage();

   return ret_val;
}

   /* Indicate the Specific Ops Control Point Result.                   */
static int IndicateSOCPResult(CGMS_SOCP_Command_Type_t RequestOpCode, Byte_t Response)
{
   DeviceInfo_t *DeviceInfo;
   int           ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding SOCP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SOCP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.SOCP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                    ret_val = CGMS_Indicate_CGMS_Specific_Ops_Control_Point_Result(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), (Byte_t)RequestOpCode, Response);
                     if(ret_val == 0)
                     {
                        /* Clear the current outstanding SOCP command   */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SOCP_OUTSTANDING;
                     }
                     else
                        DisplayFunctionError("CGMS_Indicate_CGMS_Specific_Ops_Control_Point_Result", ret_val);
                  }
                  else
                  {
                     printf("Client has not registered for Specific Ops Control Point Indication.\r\n");
                  }
               }
               else
               {
                 printf("There are no SOCP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered.\r\n");
      }
   }

   return ret_val;
}

/* Indicate the Specific Ops Communication Interval.                    */
static int IndicateSOCPCommunicationInterval(void)
{
   DeviceInfo_t            *DeviceInfo;
   int                      ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding SOCP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SOCP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.SOCP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     ret_val = CGMS_Indicate_Communication_Interval(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), SensorCommunicationInterval);
                     if(ret_val == 0)
                     {
                        /* Clear the current outstanding SOCP command   */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SOCP_OUTSTANDING;
                     }
                     else
                     {
                        DisplayFunctionError("CGMS_Indicate_Communication_Interval", ret_val);
                     }
                  }
                  else
                  {
                     printf("Client has not registered for Specific Ops Control Point Indication.\r\n");
                  }
               }
               else
               {
                 printf("There are no SOCP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered.\r\n");
      }
   }

   return ret_val;
}

/* Indicate the Specifc Ops Calibration Data.                           */
static int IndicateSOCPCalibrationData(CGMS_Calibration_Data_Record_t CalibrationData)
{
   DeviceInfo_t     *DeviceInfo;
   int               ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding SOCP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SOCP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.SOCP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     ret_val = CGMS_Indicate_Calibration_Data(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), &CalibrationData);
                     if(ret_val == 0)
                     {
                        /* Clear the current outstanding SOCP command.  */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SOCP_OUTSTANDING;
                     }
                     else
                     {
                        DisplayFunctionError("CGMS_Indicate_Calibration_Data", ret_val);
                     }
                  }
                  else
                  {
                     printf("Client has not registered for Specific Ops Control Point Indication.\r\n");
                  }
               }
               else
               {
                 printf("There are no SOCP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered.\r\n");
      }
   }

   return ret_val;
}

/* Indicate the Specific Ops Alert Level.                               */
static int IndicateSOCPAlertLevel(CGMS_SOCP_Response_Type_t ResponseOpCode, Word_t AlertLevel)
{
   DeviceInfo_t             *DeviceInfo;
   int                       ret_val = FUNCTION_ERROR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Verify that we have an open server and a connection.           */
      if(CGMSInstanceID)
      {
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that there is an outstanding SOCP command.     */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SOCP_OUTSTANDING)
               {
                  /* Verify that the client has registered for          */
                  /* indications.                                       */
                  if(DeviceInfo->ServerInfo.SOCP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     ret_val = CGMS_Indicate_Alert_Level(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), ResponseOpCode, AlertLevel);
                     if(ret_val == 0)
                     {
                        /* Clear the current outstanding SOCP command.  */
                        DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SOCP_OUTSTANDING;
                     }
                     else
                     {
                        DisplayFunctionError("CGMS_Indicate_Alert_Level", ret_val);
                     }
                  }
                  else
                  {
                     printf("Client has not registered for Specific Ops Control Point Indication.\r\n");
                  }
               }
               else
               {
                 printf("There are no SOCP commands currently outstanding.\r\n");
               }
            }
            else
               printf("Error - Unknown Client.\r\n");
         }
         else
            printf("Connection not established.\r\n");
      }
      else
      {
         if(ConnectionID)
            printf("Error - Only a server can indicate.\r\n");
         else
            printf("Error - CGMS server not registered.\r\n");
      }
   }

   return ret_val;
}

   /* The following function is responsible for configure a SOCP Service*/
   /* on a remote device.  This function will return zero on successful */
   /* execution and a negative value on errors.                         */
static int ConfigureSOCP(ParameterList_t *TempParam)
{
   int                                            ret_val = 0;
   CGMS_Specific_Ops_Control_Point_Format_Data_t  SOCPRequestData;
   DeviceInfo_t                                  *DeviceInfo;
   unsigned int                                   BufferLength;
   Byte_t                                        *Buffer;
   time_t                                         TempTime;
   struct tm                                      StartTime;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Verify that there is a valid connection                        */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Check to make sure we are bonded.                        */
            if(CheckForBond())
            {
               SOCPRequestData.CommandType = (Byte_t)TempParam->Params[0].intParam;

               /* Set the Command Parameters based on the Command Type. */
               /* Note only (SET) commands and (Get Glucose Calibration */
               /* Value) will have a command parameter.                 */
               switch(SOCPRequestData.CommandType)
               {
                  case cgcGetCGMCommunicationInterval:
                     break;
                  case cgcSetCGMCommunicationInterval:
                     if(TempParam->NumberofParameters >= 2)
                        SOCPRequestData.CommandParameters.CommunicationIntervalMinutes                       = (Byte_t)TempParam->Params[1].intParam;
                     else
                        ret_val = INVALID_PARAMETERS_ERROR;
                     break;
                  case cgcSetGlucoseCalibrationValue:
                     /* Generate a random number between 100-70.        */
                     SOCPRequestData.CommandParameters.CalibrationDataRecord.CalibrationGlucoseConcentration = rand() % 30 + 70;

                     /* We will calculate the number of minutes since   */
                     /* the session started for the calibration time.   */
                     TempTime          = time(NULL);

                     StartTime.tm_year = ClientSessionStartTime.Time.Year - 1900;
                     StartTime.tm_mon  = ClientSessionStartTime.Time.Month - 1;
                     StartTime.tm_mday = ClientSessionStartTime.Time.Day;
                     StartTime.tm_hour = ClientSessionStartTime.Time.Hours;
                     StartTime.tm_min  = ClientSessionStartTime.Time.Minutes;
                     StartTime.tm_sec  = ClientSessionStartTime.Time.Seconds;

                     SOCPRequestData.CommandParameters.CalibrationDataRecord.CalibrationTime                 = (60*(Word_t)difftime(mktime(&StartTime), TempTime));
                     SOCPRequestData.CommandParameters.CalibrationDataRecord.CalibrationTypeSampleLocation   = (((Byte_t)SENSOR_SAMPLE_LOCATION << 4) | (Byte_t)SENSOR_TYPE);

                     /* Next calibration time will always be 10 more    */
                     /* minutes than the current time.                  */
                     SOCPRequestData.CommandParameters.CalibrationDataRecord.NextCalibrationTime             = (((60*(Word_t)difftime(mktime(&StartTime), TempTime)) + 10));

                     /* Note the other fields (status and record number)*/
                     /* supplied as parameters will be ignored by the   */
                     /* sensor, since the sensor will set them          */
                     /* automatically.                                  */
                     break;
                  case cgcGetGlucoseCalibrationValue:
                     if(TempParam->NumberofParameters >= 2)
                        SOCPRequestData.CommandParameters.CalibrationDataRecordNumber                        = (Word_t)TempParam->Params[1].intParam;
                     else
                        ret_val = INVALID_PARAMETERS_ERROR;
                     break;
                  case cgcSetPatientHighAlertLevel:
                  case cgcSetPatientLowAlertLevel:
                  case cgcSetHyperAlertLevel:
                  case cgcSetHypoAlertLevel:
                  case cgcSetRateOfDecreaseAlertLevel:
                  case cgcSetRateOfIncreaseAlertLevel:
                     if(TempParam->NumberofParameters >= 2)
                        SOCPRequestData.CommandParameters.AlertLevel                                         = TempParam->Params[1].intParam;
                     else
                        ret_val = INVALID_PARAMETERS_ERROR;
                     break;
                  case cgcGetPatientHighAlertLevel:
                  case cgcGetPatientLowAlertLevel:
                  case cgcGetHyperAlertLevel:
                  case cgcGetHypoAlertLevel:
                  case cgcGetRateOfDecreaseAlertLevel:
                  case cgcGetRateOfIncreaseAlertLevel:
                  case cgcResetDeviceSpecificAlert:
                  case cgcStartSession:
                  case cgcStopSession:
                     break;
                  default:
                     ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
               }

               /* Do not continue if there was a an error.              */
               if(ret_val == 0)
               {
                  /* Determine the size of the buffer to allocate.      */
                  if((BufferLength = CGMS_Format_CGMS_Specific_Ops_Control_Point_Command(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), &SOCPRequestData, 0, NULL)) > 0)
                  {
                     /* Allocate the buffer.                            */
                     if((Buffer = (Byte_t *)BTPS_AllocateMemory(BufferLength)) != NULL)
                     {
                        /* Finally format the SOCP Request.             */
                        if((ret_val = CGMS_Format_CGMS_Specific_Ops_Control_Point_Command(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), &SOCPRequestData, BufferLength, Buffer)) == BufferLength)
                        {
                           /* Finally, submit a write request to the    */
                           /* server.                                   */
                           if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Specific_Ops_Control_Point, BufferLength, Buffer, GATT_ClientEventCallback_CGMS, DeviceInfo->ClientInfo.Specific_Ops_Control_Point)) > 0)
                           {
                              ret_val = 0;
                           }
                           else
                              DisplayFunctionError("GATT_Write_Request", ret_val);
                        }
                        else
                        {
                           if(ret_val < 0)
                              DisplayFunctionError("CGMS_Format_CGMS_Specific_Ops_Control_Point_Command", ret_val);
                           else
                              printf("The the number of bytes used in the buffer: %d, does not equal the number of bytes allocated for the buffer: %u.\r\n", ret_val, BufferLength);

                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                     {
                        printf("Could not allocate memory for the buffer.\r\n");
                        ret_val = FUNCTION_ERROR;
                     }
                  }
               }
               else
                  DisplaySOCPCommandUsage();
            }
         }
         else
            printf("No Device Info.\r\n");
      }
      else
         printf("Connection not established.\r\n");
   }
   else
      DisplaySOCPCommandUsage();

   return ret_val;
}

   /* The following function is a utility function to generate a        */
   /* measurement and add that measurement to the sensor measurement    */
   /* list.  This function will perform two tasks depending on how the  */
   /* function is called.  If the function is called as a command, then */
   /* it will take as parameters in the following order: the number of  */
   /* measurements to create, the Measurement's Flag field bitmask, the */
   /* Measurement's Status field, the Measurements Cal/Temp field, and  */
   /* the Measurements Warning field.  This will allow the user to      */
   /* create specific measurements with certain fields for testing.  If */
   /* this function is called not as a command, with the parameter set  */
   /* to NULL, then this function is used to auto generate a measurement*/
   /* at a specific measurement interval defined at the top of this     */
   /* file.  This is so that when a session is started measurements will*/
   /* be auto generated at this interval.  The flags and optional fields*/
   /* will be included and set appropriately depending on the supported */
   /* features of the sensor, as well as what bits are currently set in */
   /* the SensorStatus.  This function returns zero if successful or a  */
   /* negative integer if an error occured.                             */
static int GenerateMeasurement(ParameterList_t *TempParam)
{
   int                    ret_val                      = 0;
   Measurement_t         *Measurement;
   Word_t                 NumberOfMeasurements         = 1;
   static Word_t          PreviousGlucoseConcentration = 0;
   Byte_t                 Flags                        = 0;
   Byte_t                 Status                       = 0;
   Byte_t                 CalTemp                      = 0;
   Byte_t                 Warning                      = 0;

   /* If this function is called as a command.                          */
   if(TempParam)
   {
      /* Check for the parameters.                                      */
      if(TempParam->NumberofParameters == 5)
      {
         NumberOfMeasurements = (Word_t)TempParam->Params[0].intParam;
         Flags                = (Byte_t)TempParam->Params[1].intParam;
         Status               = (Byte_t)TempParam->Params[2].intParam;
         CalTemp              = (Byte_t)TempParam->Params[3].intParam;
         Warning              = (Byte_t)TempParam->Params[4].intParam;
      }
      else
      {
         /* Print the usage.                                            */
         printf("Usage: Create [Number of Measurements(UINT16)] [Measurement Flags(UINT8)] [Status(UINT8)] [Cal/Temp(UINT8)] [Warning(UINT8)]\r\n");
         printf(" Where [Measurement Flags] is a bit mask:\r\n");
         printf("  0x01 = Trend\r\n");
         printf("  0x02 = Quality\r\n");
         printf("  0x20 = Status\r\n");
         printf("  0x40 = Cal/Temp\r\n");
         printf("  0x80 = Warning\r\n\r\n");

         printf(" Where [Status] is a bit mask: \r\n");
         printf("  0x01 = Time Synchronization between sensor and collector required\r\n");
         printf("  0x02 = Calibration Not Allowed\r\n");
         printf("  0x04 = Calibration Recommended\r\n");
         printf("  0x08 = Calibration Required\r\n");
         printf("  0x10 = Sensor Result Lower than the Patient Low Level\r\n");
         printf("  0x20 = Sensor Result Higher than the Patient High Level\r\n");
         printf("  0x40 = Sensor Result Lower than the Hypo Level\r\n");
         printf("  0x80 = Sensor Result Higer than the Hyper Level\r\n\r\n");

         printf(" Where [Cal/Temp] is a bit mask: \r\n");
         printf("  0x01 = Sensor Rate of Decrease Exceeded\r\n");
         printf("  0x02 = Sensor Rate of Increase Exceeded\r\n");
         printf("  0x04 = Device Specific Alert\r\n");
         printf("  0x08 = Sensor Malfunction\r\n");
         printf("  0x10 = Temperature Too High For Valid Test at Time of Measurement\r\n");
         printf("  0x20 = Temperature Too Low For Valid Test at Time of Measurement\r\n");
         printf("  0x40 = Sensor Result Lower than the Device can Process\r\n");
         printf("  0x80 = Sensor Result Higher than the Device can Process\r\n\r\n");

         printf(" Where [Warning Flag] is a bit mask: \r\n");
         printf("  0x01 = Session Stopped\r\n");
         printf("  0x02 = Device Battery Low\r\n");
         printf("  0x04 = Sensor Type Incorrect for Device\r\n");
         printf("  0x08 = General Device Fault has Occured in the Sensor\r\n\r\n");
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }

   /* While there are measurements to generate.                         */
   while((ret_val == 0) && (NumberOfMeasurements--))
   {
      /* Attempt to allocate memory for a measurement.                  */
      if((Measurement = (Measurement_t *)BTPS_AllocateMemory(sizeof(Measurement_t))) != NULL)
      {
         /* Store the Measurement Information.                          */
         Measurement->Number               = ++MeasurementCounter;
         Measurement->Notified             = FALSE;
         Measurement->Flags                = Flags;
         Measurement->GlucoseConcentration = (rand() % 30) + 70;
         Measurement->TimeOffset           = TimeOffset;

         TimeOffset                       += (Word_t)SensorMeasurementInterval;

         /* Set the initial size of the measurement (Size is included). */
         /* Note the optional fields will add to the mandatory minimum  */
         /* value of 6 octets.                                          */
         Measurement->Size                 = 6;

         /* Determine if the status sensor annunciation field needs to  */
         /* be included.                                                */
         if(!TempParam)
         {
            /* If the required bits in the features are supported for   */
            /* the sensor status and one of the bits of the sensor      */
            /* status is set then we need to include the field in the   */
            /* measurement.                                             */
            if((SensorSupportedFeatures & 0x000E) && (SensorStatus & 0x000000FF))
            {
               Measurement->Status             = (Byte_t)SensorStatus;
               Measurement->Flags             |= CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT;
               Measurement->Size              += 1;
            }
            else
               Measurement->Status             = 0;
         }
         else
         {
            if(Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT)
            {
               Measurement->Status             = Status;
               Measurement->Size              += 1;
            }
            else
               Measurement->Status             = 0;
         }

         /* Determine if the cal/temp sensor annunciation field needs to*/
         /* be included.                                                */
         if(!TempParam)
         {
            /* If the required bits in the features are supported for   */
            /* the sensor cal/temp and one of the bits of the sensor    */
            /* cal/temp is set then we need to include the field in the */
            /* measurement.                                             */
            if((SensorSupportedFeatures & 0x01F0) && (SensorStatus & 0x0000FF00))
            {
               Measurement->CalTemp            = (Byte_t)(SensorStatus >> 8);
               Measurement->Flags             |= CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT;
               Measurement->Size              += 1;
            }
            else
               Measurement->CalTemp            = 0;
         }
         else
         {
            if(Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT)
            {
               Measurement->CalTemp            = CalTemp;
               Measurement->Size              += 1;
            }
            else
               Measurement->CalTemp            = 0;
         }

         /* Determine if the warning sensor status annunciation field   */
         /* needs to be included.                                       */
         if(!TempParam)
         {
            /* If the required bits in the features are supported for   */
            /* the sensor warning and one of the bits of the sensor     */
            /* warning is set then we need to include the field in the  */
            /* measurement.                                             */
            if((SensorSupportedFeatures & 0x6E00) && (SensorStatus & 0x00FF0000))
            {
               Measurement->Warning            = (Byte_t)(SensorStatus >> 16);
               Measurement->Flags             |= CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT;
               Measurement->Size              += 1;
            }
            else
               Measurement->Warning            = 0;
         }
         else
         {
            if(Flags & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT)
            {
               Measurement->Warning            = Warning;
               Measurement->Size              += 1;
            }
            else
               Measurement->Warning            = 0;
         }

         /* Determine if the Trend information is supported and set the */
         /* measurement flags.                                          */
         if(((!TempParam) && (SensorSupportedFeatures & CGMS_FEATURE_FLAG_TREND_INFORMATION_SUPPORTED)) || ((TempParam) && (Flags & CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT)))
         {
            /* Calculate the trend information.                         */
            if(PreviousGlucoseConcentration == 0)
            {
               Measurement->Trend              = Measurement->GlucoseConcentration;
            }
            else
            {
               /* Take the average of the previous concentration with   */
               /* the current concentration and store it in the Trend.  */
               Measurement->Trend              = ((PreviousGlucoseConcentration + Measurement->GlucoseConcentration)/2);
            }
            Measurement->Size                 += 2;
            Measurement->Flags                |= CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT;
         }
         else
            Measurement->Trend                 = 0;

         /* Determine if the Quality information is supported and set   */
         /* the measurement flags.                                      */
         if(((!TempParam) && (SensorSupportedFeatures & CGMS_FEATURE_FLAG_QUALITY_SUPPORTED)) || ((TempParam) && (Flags & CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT)))
         {
            /* Calculate the quality information.                       */
            Measurement->Quality            = (rand() % 20 + 80);
            Measurement->Size              += 2;
            Measurement->Flags             |= CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT;
         }
         else
            Measurement->Quality           = 0;

         /* Determine if E2E-CRC field size needs to be added to the    */
         /* size.                                                       */
         if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED)
         {
            /* The CRC will take up two octets so let's add that to the */
            /* size.                                                    */
            Measurement->Size             += 2;
         }

         /* Set the pointers to the next and previous measurement to    */
         /* NULL.                                                       */
         Measurement->Next                 = NULL;
         Measurement->Previous             = NULL;

         /* Add the measurement to the sensor measurement list.         */
         AddNewMeasurementToList(Measurement, &SensorMeasurementList);

         /* Save the previous Glucose Concentration.                    */
         PreviousGlucoseConcentration      = Measurement->GlucoseConcentration;
      }
      else
      {
         printf("Memory could not be allocated for the new measurement.\r\n");
         ret_val = FUNCTION_ERROR;
         break;
      }
   }

   return ret_val;
}

   /* The following function is a utility command that will delete the  */
   /* measurements lists.                                               */
static int DeleteMeasurements(ParameterList_t *TempParam)
{
   int ret_val = 0;

   switch(UI_Mode)
   {
      case UI_MODE_IS_CLIENT:
         DeleteMeasurementList(&CollectorMeasurementList);
         break;
      case UI_MODE_IS_SERVER:
         DeleteMeasurementList(&SensorMeasurementList);
         break;
      default:
         break;
   }

   return ret_val;
}

   /* The following function is responsible for printing off the        */
   /* measurements in a list.  This is a utility function that will     */
   /* allow the user to view the measurements on the screen.  This      */
   /* function will print off the measurements starting at the beginning*/
   /* of the linked list.  This function returns zero on success or a   */
   /* negative integer indicating that an error orccured.               */
static int PrintMeasurements(ParameterList_t *TempParam)
{
   int                ret_val = 0;
   Measurement_t     *Index;

   /* Check if we are the server.                                       */
   if(UI_Mode == UI_MODE_IS_SERVER)
   {
      /* Check to make sure that the list exists.                       */
      if((SensorMeasurementList.Tail != NULL) && (SensorMeasurementList.NumberOfMeasurements != 0))
      {
         Index = SensorMeasurementList.Tail;

         /* Print the list contents.                                    */
         while(Index != NULL)
         {
            printf("\r\nMeasurement Number:         %u\r\n",                Index->Number);
            printf("Size:                       %u (octets)\r\n",           Index->Size);
            printf("Glucose Concentration:      %u (mg/dL)\r\n",            Index->GlucoseConcentration);
            printf("Flags:                      0x%02X\r\n",                Index->Flags);
            printf("TimeOffset:                 %u (Min)\r\n",              Index->TimeOffset);
            printf("Trend:                      %u ((mg/dL)/Min)\r\n",      Index->Trend);
            printf("Quality:                    %u (((mg/dL)/Min) %%)\r\n", Index->Quality);
            printf("Status:                     0x%02X\r\n",                Index->Status);
            printf("Cal/Temp:                   0x%02X\r\n",                Index->CalTemp);
            printf("Warning:                    0x%02X\r\n",                Index->Warning);

            Index = Index->Previous;
         }
      }
      else
         printf("There are no sensor measurements.\r\n");
   }
   else
   {
      /* Check to make sure that we are the client.                     */
      if(UI_Mode == UI_MODE_IS_CLIENT)
      {
         /* Check to make sure that the list exists.                    */
         if((CollectorMeasurementList.Tail != NULL) && (CollectorMeasurementList.NumberOfMeasurements != 0))
         {
            Index = CollectorMeasurementList.Tail;

            /* Print the list contents.                                 */
            while(Index != NULL)
            {
               printf("\r\nMeasurement Number:         %u\r\n",                Index->Number);
               printf("Size:                       %u (octets)\r\n",           Index->Size);
               printf("Glucose Concentration:      %u (mg/dL)\r\n",            Index->GlucoseConcentration);
               printf("Flags:                      0x%02X\r\n",                Index->Flags);
               printf("TimeOffset:                 %u (Min)\r\n",              Index->TimeOffset);
               printf("Trend:                      %u ((mg/dL)/Min)\r\n",      Index->Trend);
               printf("Quality:                    %u (((mg/dL)/Min) %%)\r\n", Index->Quality);
               printf("Status:                     0x%02X\r\n",                Index->Status);
               printf("Cal/Temp:                   0x%02X\r\n",                Index->CalTemp);
               printf("Warning:                    0x%02X\r\n",                Index->Warning);

               Index = Index->Previous;
            }
         }
         else
            printf("There are no collector measurements.\r\n");
      }
   }

   return ret_val;
}

   /* The following function is responsible for generating a calibration*/
   /* data record.  This function is a utility function to aid in       */
   /* testing.  This function returns zero if successful or a negative  */
   /* integer if an error occured.                                      */
static int GenerateCalRecord(ParameterList_t *TempParam)
{
   int                      ret_val = 0;
   CalibrationDataRecord_t *NewCalRecord;

   /* Check to make sure the parameters are semi-valid.                 */
   if((TempParam) && (TempParam->NumberofParameters == 1))
   {
      if((NewCalRecord = BTPS_AllocateMemory(sizeof(CalibrationDataRecord_t))) != NULL)
      {
         /* Generate a random number between 100-70.                    */
         NewCalRecord->GlucoseConcentration  = ((rand() % 30) + 70);
         NewCalRecord->CalibrationDataRecord = ++CalibrationDataRecordCtr;
         NewCalRecord->CalibrationTime       = (Word_t)TempParam->Params[0].intParam;
         NewCalRecord->TypeSampleLocation    = (((Byte_t)SENSOR_SAMPLE_LOCATION << 4) | (Byte_t)SENSOR_TYPE);
         NewCalRecord->Status                = CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_CALIBRATION_PROCESS_PENDING;
         NewCalRecord->Next                  = NULL;

         /* Next calibration time will always be 10 more minutes than   */
         /* the current time.                                           */
         NewCalRecord->NextCalibrationTime   = (NewCalRecord->CalibrationTime + 10);

         /* Add the new calibration data record.                        */
         AddNewCalibrationDataRecord(NewCalRecord);
      }
      else
      {
         printf("Memory could not be allocated for the new calibration data record.\r\n");
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Usage: CreateCalRecord [Calibration Time(UINT16)]\r\n   Note: Next Calibration Time will be 10 (Min) more than [Calibration Time].\r\n\r\n");
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return ret_val;
}

   /* The following function is responsible for printing off the        */
   /* calibration records in a list.  This is a utility function that   */
   /* will allow the user to view the records on the screen.  This      */
   /* function will print off the records starting at the beginning of  */
   /* the linked list.  This function returns zero on success or a      */
   /* negative integer indicating that an error orccured.               */
static int PrintCalibrationRecords(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   CalibrationDataRecord_t   *Index;

      /* Check to make sure that the list exists.                       */
      if((CalibrationDataRecordList.Head != NULL) && (CalibrationDataRecordList.Tail != NULL) && (CalibrationDataRecordList.NumberOfRecords != 0))
      {
         Index = CalibrationDataRecordList.Head;

         /* Print the list contents.                                    */
         while(Index != NULL)
         {
            printf("Glucose Concentration:      %u (mg/dL)\r\n", Index->GlucoseConcentration);
            printf("Time:                       %u (Min)\r\n",   Index->CalibrationTime);
            printf("Type-Sample Location:       0x%02X\r\n",     Index->TypeSampleLocation);
            printf("Next Calibration Time:      %u (Min)\r\n",   Index->NextCalibrationTime);
            printf("Data Record Number:         %u\r\n",         Index->CalibrationDataRecord);
            printf("Status:                     0x%04X\r\n\r\n", Index->Status);

            Index = Index->Next;
         }
      }
      else
         printf("There are no calibration data records.\r\n");

   return ret_val;
}

   /* The following function is a utility function responsible for      */
   /* deleting the calibration data records.  This function returns zero*/
   /* for success or a negative integer indicating that an error        */
   /* occured.                                                          */
static int DeleteCalibrationRecords(ParameterList_t *TempParam)
{
   int ret_val = 0;

   DeleteCalibrationRecordList();

   return ret_val;
}

   /* The following function is a utility function responsible for      */
   /* starting a session on the sensor without having a client use a    */
   /* SOCP session start command.  This function returns zero on success*/
   /* or a negative integer indicating a SOCP error since this function */
   /* simply wraps the internal SOCP session start function.            */
static int StartSessionCommand(ParameterList_t *TempParam)
{
   return(StartSession());
}

   /* The following function is a utility function responsible for      */
   /* stopping a session on the sensor without having a client use a    */
   /* SOCP session stop command.  This function returns zero on success */
   /* or a negative integer indicating a SOCP error since this function */
   /* simply wraps the internal SOCP session stop function.             */
static int StopSessionCommand(ParameterList_t *TempParam)
{
   if(StopSession() < 0)
      printf("Session is already stopped.\r\n");

   return (0);
}

   /* The following function is responsible for performing a Report     */
   /* Stored Records Procedure, which will cause all measurement records*/
   /* specifed by the time offset range in the RACP Data to be notified.*/
   /* This function takes a parameter for the RACP Data, which contains */
   /* an operator for what measurement records need to be notified and a*/
   /* time offset range, which may be needed to distinguish the         */
   /* measurement records depending on the value of the operator.  This */
   /* function returns zero on success or a negative integer            */
   /* representing an RACP error code.                                  */
static int ReportStoredRecordsProcedure(RACP_Procedure_Data_t RACPData)
{
   int                       ret_val                  = 0;
   Measurement_t            *Index                    = NULL;
   Boolean_t                 MeasurementFound         = FALSE;
   Measurement_List_t        NotificationList;
   Measurement_t            *NewMeasurement           = NULL;
   CGMS_Measurement_Data_t **NotificationListArray    = NULL;
   CGMS_Measurement_Data_t  *NewNotification          = NULL;
   int                       ArrayIndex               = 0;
   DWord_t                   NumberOfMeasurementsSent = 0;
   DeviceInfo_t             *DeviceInfo               = NULL;

   /* Make sure that we are the server and valid service has been       */
   /* registered.                                                       */
   if(CGMSInstanceID)
   {
      /* Getting DeviceInfo to verify that the collector has registered */
      /* for notifications.  Note we must be connected.                 */
      if((ConnectionID) && ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL))
      {
         /* Need to check if the collector has registered for           */
         /* notifications.  Otherwise we do not want to send            */
         /* notifications.                                              */
         if(DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
         {
            /* Make sure that the sensor list exists and we have        */
            /* measurements.                                            */
            if((SensorMeasurementList.Head != NULL) && (SensorMeasurementList.Tail != NULL) && (SensorMeasurementList.NumberOfMeasurements > 0))
            {
               /* Initialize the temporary list.                        */
               NotificationList.NumberOfMeasurements = 0;
               NotificationList.ListType             = ltTemp;
               NotificationList.Head                 = NULL;
               NotificationList.Tail                 = NULL;

               /* Go through the sensor list and add the measurements   */
               /* that match the operator type and optional range to a  */
               /* temporary list.  Note the order we find the           */
               /* measurements will be the reverse order that they are  */
               /* notified (FIFO notify order).                         */
               Index = SensorMeasurementList.Head;
               while((ret_val == 0) && (Index != NULL))
               {
                  /* Determine if the measurement needs to be notified. */
                  switch(RACPData.OperatorType)
                  {
                     case raoCGMSNull:
                        ret_val = RACP_INVALID_OPERATOR;
                        break;
                     case raoCGMSAllRecords:
                        MeasurementFound    = TRUE;
                        break;
                     case raoCGMSLessThanOrEqualTo:
                        if(Index->TimeOffset <= RACPData.ParameterData.TimeOffset)
                           MeasurementFound = TRUE;
                        break;
                     case raoCGMSGreaterThanOrEqualTo:
                        if(Index->TimeOffset >= RACPData.ParameterData.TimeOffset)
                           MeasurementFound = TRUE;
                        break;
                     case raoCGMSWithinRangeOf:
                        if((Index->TimeOffset >= RACPData.ParameterData.Range.Minimum) && (Index->TimeOffset <= RACPData.ParameterData.Range.Maximum))
                           MeasurementFound = TRUE;
                        break;
                     case raoCGMSFirstRecord:
                        /* Re-adjust the index to the tail of queue     */
                        /* since the first measurement is at the tail of*/
                        /* the linked list or front of the queue.       */
                        Index            = SensorMeasurementList.Tail;

                        MeasurementFound = TRUE;
                        break;
                     case raoCGMSLastRecord:
                        /* Note the index should already be pointing to */
                        /* the tail of the queue, where the oldest      */
                        /* measurement is.                              */

                        MeasurementFound = TRUE;
                        break;
                     default:
                        ret_val = RACP_OPERATOR_NOT_SUPPORTED;
                        break;
                  }

                  /* If the measurement was a match we need to create a */
                  /* new measurements, copy the data, and add it to the */
                  /* temporary list.                                    */
                  if((ret_val == 0) && (MeasurementFound == TRUE))
                  {
                     /* Allocate a new measurement and add it to the    */
                     /* temporary list.                                 */
                     if((NewMeasurement = (Measurement_t *)BTPS_AllocateMemory(sizeof(Measurement_t))) != NULL)
                     {
                        /* Copy the current measurement information to  */
                        /* the new measurement .                        */
                        CopyMeasurement(Index,  NewMeasurement);

                        /* Initialize the pointers to null.             */
                        NewMeasurement->Next     = NULL;
                        NewMeasurement->Previous = NULL;

                        /* Add the new measurement to the notification  */
                        /* list.                                        */
                        AddNewMeasurementToList(NewMeasurement, &NotificationList);

                        /* If the operator type is for the first record */
                        /* or last record we need to break from the loop*/
                        /* after the measurement is added so that we    */
                        /* will have only one measurement to notify.    */
                        if((RACPData.OperatorType == raoCGMSFirstRecord) || (RACPData.OperatorType == raoCGMSLastRecord))
                           break;
                     }
                     else
                     {
                        printf("Memory could not be allocated for the new measurement.\r\n");
                        ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                        break;
                     }

                     /* Reset the flag.                                 */
                     MeasurementFound = FALSE;
                  }

                  /* Go to the next measurement in the sensor           */
                  /* measurement list.                                  */
                  Index = Index->Next;
               }

               /* Only continue if there are no errors.                 */
               if(ret_val == 0)
               {
                  /* Send recent notifications.  Determine if we have   */
                  /* measurements to notify.                            */
                  if(NotificationList.NumberOfMeasurements != 0)
                  {
                     /* Allocate memory for an array of                 */
                     /* CGMS_Measurement_Data_t pointers since that is  */
                     /* what the API expects.                           */
                     if((NotificationListArray = (CGMS_Measurement_Data_t **)BTPS_AllocateMemory(sizeof(CGMS_Measurement_Data_t *)*NotificationList.NumberOfMeasurements)) != NULL)
                     {
                        /* Allocate memory for the pointers of the      */
                        /* measurements.                                */
                        Index = NotificationList.Head;
                        for(ArrayIndex = 0; ArrayIndex < NotificationList.NumberOfMeasurements; ArrayIndex++, Index = Index->Next)
                        {
                           /* Check to make sure that the index into the*/
                           /* notification list is not NULL.            */
                           if(Index != NULL)
                           {
                              /* Allocate memory for the measurements.  */
                              if((NewNotification = (CGMS_Measurement_Data_t *)BTPS_AllocateMemory(CGMS_MEASUREMENT_DATA_SIZE)) != NULL)
                              {
                                 /* Copy the data.                      */
                                 NewNotification->Flags                       = Index->Flags;
                                 NewNotification->Size                        = Index->Size;
                                 NewNotification->GlucoseConcentration        = Index->GlucoseConcentration;
                                 NewNotification->TimeOffset                  = Index->TimeOffset;
                                 NewNotification->SensorStatus                = Index->Status;
                                 NewNotification->SensorCalTemp               = Index->CalTemp;
                                 NewNotification->SensorWarning               = Index->Warning;
                                 NewNotification->TrendInformation            = Index->Trend;
                                 NewNotification->Quality                     = Index->Quality;

                                 /* Store the new notification.         */
                                 NotificationListArray[ArrayIndex]            = NewNotification;
                              }
                              else
                              {
                                 printf("Could not allocate memory for a CGMS Measurement.\r\n");
                                 ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                                 NotificationListArray[ArrayIndex] = NULL;
                              }
                           }
                           else
                           {
                              printf("The end of the notification list has been reached.\r\n");
                              ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                              NotificationListArray[ArrayIndex] = NULL;
                           }
                        }

                        /* Only continue if there are no errors.        */
                        if(ret_val == 0)
                        {
                           /* While there are notifications to be sent..*/
                           while((ret_val = CGMS_Notify_CGMS_Measurements(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), NotificationList.NumberOfMeasurements-NumberOfMeasurementsSent, NotificationListArray+NumberOfMeasurementsSent)))
                           {
                              /* Check if the abort command has been    */
                              /* issued.                                */
                              if(AbortProcedureFlag == TRUE)
                              {
                                 /* Reset the abort flag and inform the */
                                 /* user that the procedure has been    */
                                 /* terminated.                         */
                                 AbortProcedureFlag = FALSE;
                                 printf("The reporting of stored records has been aborted.\r\n");
                                 ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                                 break;
                              }

                              /* If measurements were notified.         */
                              if(ret_val > 0)
                              {
                                 /* Store the number of measurements    */
                                 /* that were sent.                     */
                                 NumberOfMeasurementsSent += ret_val;

                                 /* Check if we are done sending        */
                                 /* notifications.                      */
                                 if(NumberOfMeasurementsSent >= NotificationList.NumberOfMeasurements)
                                 {
                                    /* Check to make sure that an       */
                                    /* unexpected error did not occur   */
                                    if(NumberOfMeasurementsSent > NotificationList.NumberOfMeasurements)
                                       printf("More measurements notified: %u, than expected: %u.\r\n", NumberOfMeasurementsSent, NotificationList.NumberOfMeasurements);

                                    /* All notifications have been sent */
                                    /* and we are finished.  Simply     */
                                    /* return success.                  */
                                    ret_val = 0;
                                    break;
                                 }
                              }
                              else
                              {
                                 DisplayFunctionError("CGMS_Notify_CGMS_Measurements", ret_val);
                                 ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                                 break;
                              }
                           }

                          /* A 6 second delay has been added in order to*/
                          /* work properly with PTS test cases.  PTS    */
                          /* will not ask for the RACP indication until */
                          /* all measurements have been notified.       */
                          /* Without the delay, the RACP indication will*/
                          /* be sent before all measurements have been  */
                          /* notified (Still queued to be sent).  PTS   */
                          /* will ignore this indication and ask for the*/
                          /* RACP indication to be sent again after all */
                          /* notifications have been received.  This    */
                          /* delay therefore gives time for all the     */
                          /* notifications to successfully be sent      */
                          /* before the indication.                     */
                           BTPS_Delay(6000);
                        }

                        /* Free the notification array list memory.     */
                        for(ArrayIndex = 0; ArrayIndex < NotificationList.NumberOfMeasurements; ArrayIndex++)
                        {
                           BTPS_FreeMemory(NotificationListArray[ArrayIndex]);
                           NotificationListArray[ArrayIndex] = NULL;
                        }

                        BTPS_FreeMemory(NotificationListArray);
                        NotificationListArray = NULL;
                     }
                     else
                     {
                        printf("Memory could not be allocated for the notification measurement list.\r\n");
                        ret_val = RACP_PROCEDURE_NOT_COMPLETED;
                     }

                     /* Free the temporary list data.                   */
                     DeleteMeasurementList(&NotificationList);
                  }
                  else
                     ret_val = RACP_NO_RECORDS_FOUND;
               }
            }
            else
               ret_val = RACP_NO_RECORDS_FOUND;
         }
         else
            ret_val = RACP_PROCEDURE_NOT_COMPLETED;
      }
      else
         ret_val = RACP_PROCEDURE_NOT_COMPLETED;
   }
   else
   {
      printf("Server not registered.\r\n");
      ret_val = RACP_PROCEDURE_NOT_COMPLETED;
   }

   return ret_val;
}

   /* The following function is responsible for performing a Report     */
   /* Number Of Stored Records Procedure, which will find the number of */
   /* measurement records that match the specified criteria in the      */
   /* parameter.  This function takes a parameter for the RACP Data,    */
   /* which contains an operator for what measurement records need to be*/
   /* notified and a time offset range, which may be needed to          */
   /* distinguish the measurement records depending on the value of the */
   /* operator.  This function returns zero on success or a negative    */
   /* integer representing an RACP error code.                          */
static int ReportNumberOfStoredRecordsProcedure(RACP_Procedure_Data_t RACPData, unsigned int *NumberOfMeasurementsFound)
{
   int             ret_val = 0;
   Measurement_t  *Index;

   /* Make sure that we are the server and valid service has been       */
   /* registered.                                                       */
   if(CGMSInstanceID)
   {
      /* Let's initialize the number of measurements found.             */
      *NumberOfMeasurementsFound = 0;

      /* Make sure that the sensor list exists and we have measurements.*/
      if((SensorMeasurementList.Head != NULL) && (SensorMeasurementList.Tail != NULL) && (SensorMeasurementList.NumberOfMeasurements >= 1))
      {
         switch(RACPData.OperatorType)
         {
            case raoCGMSNull:
               ret_val = RACP_INVALID_OPERATOR;
               break;
            case raoCGMSAllRecords:
               /* If the Operator Type is all records simply return the */
               /* total number of records.                              */
               if(RACPData.OperatorType == raoCGMSAllRecords)
                  *NumberOfMeasurementsFound = SensorMeasurementList.NumberOfMeasurements;
               break;
            case raoCGMSLessThanOrEqualTo:
            case raoCGMSGreaterThanOrEqualTo:
            case raoCGMSWithinRangeOf:
               /* Go through the sensor list and count the measurments  */
               /* that match the operator type and range.               */
               Index = SensorMeasurementList.Head;
               while(Index != NULL)
               {
                  /* If the time offset is less than or equal to.       */
                  if((RACPData.OperatorType == raoCGMSLessThanOrEqualTo) && (Index->TimeOffset <= RACPData.ParameterData.TimeOffset))
                     (*NumberOfMeasurementsFound)++;

                  /* If the time offset is greater than or equal to.    */
                  if((RACPData.OperatorType == raoCGMSGreaterThanOrEqualTo) && (Index->TimeOffset >= RACPData.ParameterData.TimeOffset))
                     (*NumberOfMeasurementsFound)++;

                  /* If the time offset is in range.                    */
                  if((RACPData.OperatorType == raoCGMSWithinRangeOf) && (Index->TimeOffset >= RACPData.ParameterData.Range.Minimum) && (Index->TimeOffset <= RACPData.ParameterData.Range.Maximum))
                     (*NumberOfMeasurementsFound)++;

                  /* Go to the next node.                               */
                  Index = Index->Next;
               }
               break;
            case raoCGMSFirstRecord:
               /* Only one possibility since we have already checked the*/
               /* tail pointer.                                         */
               *NumberOfMeasurementsFound = 1;
               break;
            case raoCGMSLastRecord:
               /* Only one possibility since we have already checked the*/
               /* head pointer.                                         */
               *NumberOfMeasurementsFound = 1;
               break;
            default:
               ret_val = RACP_OPERATOR_NOT_SUPPORTED;
         }
      }
      else
         *NumberOfMeasurementsFound = 0;
   }
   else
   {
      printf("Error - CGMS server not registered.\r\n");
      ret_val =  RACP_PROCEDURE_NOT_COMPLETED;
   }

   return ret_val;
}

   /* The following function is responsible for performing a Report     */
   /* Stored Records Procedure, which will cause any RACP procedure to  */
   /* be aborted.  This function takes a parameter for the RACP Data,   */
   /* which contains an operator for what measurement records need to be*/
   /* notified and a time offset range, which may be needed to          */
   /* distinguish the measurement records depending on the value of the */
   /* operator.  This function returns zero on success or a negative    */
   /* integer representing an RACP error code.                          */
static int AbortProcedure(void)
{
   int ret_val = 0;

   AbortProcedureFlag = TRUE;

   return ret_val;
}

   /* The following function is responsible for performing a Report     */
   /* Stored Records Procedure, which will delete specified measurement */
   /* records.  This function takes a parameter for the RACP Data, which*/
   /* contains an operator for what measurement records need to be      */
   /* notified and a time offset range, which may be needed to          */
   /* distinguish the measurement records depending on the value of the */
   /* operator.  This function returns zero on success or a negative    */
   /* integer representing an RACP error code.                          */
static int DeleteStoredRecordsProcedure(RACP_Procedure_Data_t RACPData)
{
   int                    ret_val       = 0;
   Measurement_t         *Index;
   Measurement_t         *MeasurementToDelete;
   Measurement_t         *MeasurementToRemove;
   Measurement_List_t     TempList;
   Boolean_t              RemoveTheNode = FALSE;

   /* Make sure that we are the server and valid service has been       */
   /* registered.                                                       */
   if(CGMSInstanceID)
   {
      /* Make sure that the sensor list exists and we have measurements.*/
      if((SensorMeasurementList.Head != NULL) && (SensorMeasurementList.Tail != NULL) && (SensorMeasurementList.NumberOfMeasurements >= 1))
      {
         /* Initialize the temporary list.                              */
         TempList.NumberOfMeasurements = 0;
         TempList.ListType             = ltTemp;
         TempList.Head                 = NULL;
         TempList.Tail                 = NULL;

         switch(RACPData.OperatorType)
         {
            case raoCGMSNull:
               ret_val = RACP_INVALID_OPERATOR;
               break;
            case raoCGMSAllRecords:
               /* Call the Delete Measurement List function to remove   */
               /* all records.                                          */
               if(DeleteMeasurementList(&SensorMeasurementList) != 0)
                  ret_val = RACP_PROCEDURE_NOT_COMPLETED;
               break;
            case raoCGMSLessThanOrEqualTo:
            case raoCGMSGreaterThanOrEqualTo:
            case raoCGMSWithinRangeOf:
               /* Go through the sensor measurement list and find nodes */
               /* that need to be removed.                              */
               Index = SensorMeasurementList.Head;
               while(Index != NULL)
               {
                  /* If the time offset is less than or equal to.       */
                  if((RACPData.OperatorType == raoCGMSLessThanOrEqualTo) && (Index->TimeOffset <= RACPData.ParameterData.TimeOffset))
                     RemoveTheNode = TRUE;

                  /* If the time offset is greater than or equal to.    */
                  if((RACPData.OperatorType == raoCGMSGreaterThanOrEqualTo) && (Index->TimeOffset >= RACPData.ParameterData.TimeOffset))
                     RemoveTheNode = TRUE;

                  /* If the time offset is in range.                    */
                  if((RACPData.OperatorType == raoCGMSWithinRangeOf) && (Index->TimeOffset >= RACPData.ParameterData.Range.Minimum) && (Index->TimeOffset <= RACPData.ParameterData.Range.Maximum))
                     RemoveTheNode = TRUE;

                  /* If we found a node that needs to be removed.       */
                  if(RemoveTheNode == TRUE)
                  {
                     /* Remove the node from the sensor list and add it */
                     /* to the temporary list.  Note we could use Index,*/
                     /* but for readability we will use another pointer.*/
                     MeasurementToRemove = Index;

                     /* Go ahead an adjust the Index to the next node   */
                     /* before we remove the measurement currently      */
                     /* pointed to by Index.  Note this may be NULL.    */
                     Index = Index->Next;

                     /* Remove the node.                                */
                     RemoveNodefromSensorMeasurementList(MeasurementToRemove);

                     /* Add the node to the temporary list, which will  */
                     /* be deleted after all nodes have been added.     */
                     AddNewMeasurementToList(MeasurementToRemove, &TempList);

                     /* Reset the flag.                                 */
                     RemoveTheNode = FALSE;
                  }
                  else
                  {
                     /* Go to the next index.                           */
                     Index = Index->Next;
                  }
               }

               /* If measurements were deleted.                         */
               if(TempList.NumberOfMeasurements != 0)
               {
                  /* Delete the temporary list and free the memory.     */
                  if(DeleteMeasurementList(&TempList) != 0)
                     ret_val = RACP_PROCEDURE_NOT_COMPLETED;
               }
               else
                  ret_val = RACP_NO_RECORDS_FOUND;
               break;
            case raoCGMSFirstRecord:
               /* Set the Measurement to be deleted.                    */
               MeasurementToDelete = SensorMeasurementList.Tail;

               /* Check to make sure that the measurement to be         */
               /* deleted's previous Node exists.  This is a special    */
               /* case and could cause an error if it is NULL.          */
               if(MeasurementToDelete->Previous != NULL)
               {
                  /* Set the measurement to be deleted node's previous  */
                  /* node's next pointer to NULL since we are removing  */
                  /* the tail Node.                                     */
                  MeasurementToDelete->Previous->Next = NULL;
               }

               /* Set the new tail of the list.  Note this may be NULL. */
               SensorMeasurementList.Tail = MeasurementToDelete->Previous;

               /* Delete the measurement.                               */
               BTPS_FreeMemory(MeasurementToDelete);

               /* Decrement the number of file objects.                 */
               SensorMeasurementList.NumberOfMeasurements--;

               /* If the list is now empty we need to set the head and  */
               /* tail pointers to null.                                */
               if(SensorMeasurementList.NumberOfMeasurements == 0)
               {
                  SensorMeasurementList.Head = NULL;
                  SensorMeasurementList.Tail = NULL;
               }
               break;
            case raoCGMSLastRecord:
               /* Set the Measurement to be deleted.                    */
               MeasurementToDelete = SensorMeasurementList.Head;

               /* Check to make sure that the measurement to be         */
               /* deleted's next Node exists.  This is a special case   */
               /* and could cause an error if it is NULL.               */
               if(MeasurementToDelete->Next != NULL)
               {
                  /* Set the measurement to be deleted node's next      */
                  /* node's previous pointer to NULL since we are       */
                  /* removing the head Node.                            */
                  MeasurementToDelete->Next->Previous = NULL;
               }

               /* Set the new head of the list.  Note this may be NULL. */
               SensorMeasurementList.Head = MeasurementToDelete->Next;

               /* Delete the measurement.                               */
               BTPS_FreeMemory(MeasurementToDelete);

               /* Decrement the number of file objects.                 */
               SensorMeasurementList.NumberOfMeasurements--;

               /* If the list is now empty we need to set the head and  */
               /* tail pointers to null.                                */
               if(SensorMeasurementList.NumberOfMeasurements == 0)
               {
                  SensorMeasurementList.Head = NULL;
                  SensorMeasurementList.Tail = NULL;
               }
               break;
            default:
               ret_val = RACP_OPERATOR_NOT_SUPPORTED;
         }
      }
      else
         ret_val = RACP_NO_RECORDS_FOUND;
   }
   else
   {
      printf("Error - CGMS server not registered.\r\n");
      ret_val =  RACP_PROCEDURE_NOT_COMPLETED;
   }

   return (ret_val);
}

   /* The following function is responsible for setting the             */
   /* communication interval, which controls how often measurements are */
   /* notified to the collector from the sensor.  This function takes a */
   /* parameter, which is a byte to indicate how fast the interval      */
   /* should be.  Note 0xFF will force the sensor to notify measurements*/
   /* as quickly as possible, while 0x00 disables the periodic          */
   /* notification.  This function returns zero on success or a negative*/
   /* integer indicating that an error occured.                         */
static int SetCommunicationInterval(Byte_t *CommunicationInterval)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(CommunicationInterval)
   {
      /* Set Communication Interval is a mandatory SOCP procedure so no */
      /* check is needed.                                               */

      /* If we are setting the communication interval.                  */
      if(*CommunicationInterval == 0xFF)
      {
         SensorCommunicationInterval = SENSOR_FASTEST_SUPPORTED_COMM_INTERVAL;
      }
      else
      {
         /* Set the communication interval.                             */
         SensorCommunicationInterval = *CommunicationInterval;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val = 0;
}

   /* The following function is responsible for getting the             */
   /* communication interval, which controls how often measurements are */
   /* notified to the collector from the sensor.  This function takes a */
   /* parameter, which is a pointer byte that will be set to the current*/
   /* communication interval.  This function returns zero on success or */
   /* a negative integer indicating that an error occured.              */
static int GetCommunicationInterval(Byte_t *CommunicationInterval)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(CommunicationInterval)
   {
      /* Get Communication Interval is a mandatory SOCP procedure so no */
      /* check is needed.                                               */

      /* Get the communication interval and return it to the user.      */
      *CommunicationInterval = SensorCommunicationInterval;
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the glucose     */
   /* calibration value.  This function takes as a parameter a          */
   /* calibration data record, which contains the necessary information */
   /* to populate the data record on the server.  This function returns */
   /* zero on success or a negative integer indicating that an error    */
   /* occured.                                                          */
static int SetGlucoseCalibrationValue(CGMS_Calibration_Data_Record_t *CalDataRecord)
{
   int                      ret_val = 0;
   CalibrationDataRecord_t *NewRecord;

   /* Check to make sure that the parameters are semi-valid.            */
   if(CalDataRecord)
   {
      /* Check to make sure that the SetGlucoseCalibrationValue is      */
      /* supported.                                                     */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_CALIBRATION_SUPPORTED)
      {
         /* Allocate memory for the new calibration data record.        */
         if((NewRecord = (CalibrationDataRecord_t *)BTPS_AllocateMemory(sizeof(CalibrationDataRecord_t))) != NULL)
         {
            /* Prepare the new record.                                  */
            NewRecord->GlucoseConcentration  = CalDataRecord->CalibrationGlucoseConcentration;
            NewRecord->CalibrationTime       = CalDataRecord->CalibrationTime;
            NewRecord->NextCalibrationTime   = CalDataRecord->NextCalibrationTime;
            NewRecord->TypeSampleLocation    = CalDataRecord->CalibrationTypeSampleLocation;
            NewRecord->Next                  = NULL;

            /* Other values in the parameter will be ignored set by the */
            /* server.                                                  */
            NewRecord->CalibrationDataRecord = ++CalibrationDataRecordCtr;
            NewRecord->Status                = CalibrationStatus;

            /* Check to see if we have reached the maximum number of    */
            /* data records.                                            */
            if(CalibrationDataRecordCtr <= MAX_CALIBRATION_DATA_RECORDS)
            {
               /* Store the glucose calibration data record.            */
               AddNewCalibrationDataRecord(NewRecord);
            }
            else
            {
               printf("The maximum number of calibration data records has been reached.\r\n");
               ret_val = SOCP_PROCEDURE_NOT_COMPLETED;
            }
         }
         else
            ret_val = SOCP_PROCEDURE_NOT_COMPLETED;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val = 0;
}

   /* The following function is responsible for getting the glucose     */
   /* calibration value.  This function takes as a parameter a pointer  */
   /* to a calibration data record, so that the information retrieved   */
   /* from the sensor can be passed to the caller.  This function       */
   /* returns zero on success or a negative integer indicating that an  */
   /* error occured.                                                    */
static int GetGlucoseCalibrationValue(Word_t RecordNumber, CGMS_Calibration_Data_Record_t *CalDataRecord)
{
   int                      ret_val = 0;
   CalibrationDataRecord_t *Index;

   /* Check to make sure that the parameters are semi-valid.            */
   if(CalDataRecord)
   {
      /* Check to make sure that the GetGlucoseCalibrationValue is      */
      /* supported.                                                     */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_CALIBRATION_SUPPORTED)
      {
         /* Check to make sure that there are data records available.   */
         if((CalibrationDataRecordList.Head != NULL) && (CalibrationDataRecordList.Tail != NULL) && (CalibrationDataRecordList.NumberOfRecords != 0))
         {
            if(RecordNumber != 0)
            {
               /* Check if we are not looking for the last record.      */
               if(RecordNumber != 0xFFFF)
               {
                  /* Set index to the head of the list.                 */
                  Index = CalibrationDataRecordList.Head;

                  /* Search for the glucose calibration data record.    */
                  /* Note they will be in order for 0 to N.             */
                  while(Index != NULL)
                  {
                     /* If we find a match.                             */
                     if(Index->CalibrationDataRecord == RecordNumber)
                     {
                        /* Set the information and break from the loop. */
                        CalDataRecord->CalibrationGlucoseConcentration = Index->GlucoseConcentration;
                        CalDataRecord->CalibrationTime                 = Index->CalibrationTime;
                        CalDataRecord->NextCalibrationTime             = Index->NextCalibrationTime;
                        CalDataRecord->CalibrationTypeSampleLocation   = Index->TypeSampleLocation;
                        CalDataRecord->CalibrationDataRecordNumber     = Index->CalibrationDataRecord;
                        CalDataRecord->CalibrationStatus               = Index->Status;
                        break;
                     }

                     /* Get the next record.                            */
                     Index = Index->Next;
                  }

                  /* If we didn't find a record return an error.        */
                  if(Index == NULL)
                     ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
               }
               else
               {
                  /* Set index to the head of the list.                 */
                  Index = CalibrationDataRecordList.Tail;

                  /* Last record will be at the tail of the list.       */
                  CalDataRecord->CalibrationGlucoseConcentration       = Index->GlucoseConcentration;
                  CalDataRecord->CalibrationTime                       = Index->CalibrationTime;
                  CalDataRecord->NextCalibrationTime                   = Index->NextCalibrationTime;
                  CalDataRecord->CalibrationTypeSampleLocation         = Index->TypeSampleLocation;
                  CalDataRecord->CalibrationDataRecordNumber           = Index->CalibrationDataRecord;
                  CalDataRecord->CalibrationStatus                     = Index->Status;
               }
            }
            else
               ret_val = SOCP_INVALID_OPERAND;
         }
         else
         {
            printf("There are no calibration data records.\r\n");
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
         }
      }
      else
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the patient high*/
   /* alert level.  This function takes (UINT16) as a parameter for the */
   /* SFLOAT data type.  This function returns zero on success or a     */
   /* negative integer indicating that an error occured.                */
static int SetPatientHighAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetPatientAlertLevel is supported. */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_PATIENT_HIGH_LOW_ALERTS_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            PatientHighAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}


   /* The following function is responsible for getting the patient high*/
   /* alert level.  This function takes a (UINT16) pointer as a         */
   /* parameter for the SFLOAT data type, which will be passed back up  */
   /* to the caller .  This function returns zero on success or a       */
   /* negative integer indicating that an error occured.                */
static int GetPatientHighAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the GetPatientAlertLevel is supported. */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_PATIENT_HIGH_LOW_ALERTS_SUPPORTED)
      {
         /* Get the Alert Level.                                        */
         *AlertLevel = PatientHighAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the patient low */
   /* alert level.  This function takes (UINT16) as a parameter for the */
   /* SFLOAT data type.  This function returns zero on success or a     */
   /* negative integer indicating that an error occured.                */
static int SetPatientLowAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetPatientAlertLevel is supported. */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_PATIENT_HIGH_LOW_ALERTS_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            PatientLowAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for getting the patient low */
   /* alert level.  This function takes a (UINT16) pointer as a         */
   /* parameter for the SFLOAT data type, which will be passed back up  */
   /* to the caller .  This function returns zero on success or a       */
   /* negative integer indicating that an error occured.                */
static int GetPatientLowAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the GetPatientAlertLevel is supported. */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_PATIENT_HIGH_LOW_ALERTS_SUPPORTED)
      {
         /* Get the Alert Level.                                        */
         *AlertLevel = PatientLowAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the hypo alert  */
   /* level.  This function takes (UINT16) as a parameter for the SFLOAT*/
   /* data type.  This function returns zero on success or a negative   */
   /* integer indicating that an error occured.                         */
static int SetHypoAlertLevel(Word_t *AlertLevel)
{
   int   ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetHypoAlertLevel is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_HYPO_ALERTS_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            HypoAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for getting the hypo alert  */
   /* level.  This function takes a (UINT16) pointer as a parameter for */
   /* the SFLOAT data type, which will be passed back up to the caller .*/
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
static int GetHypoAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the GetHypoAlertLevel is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_HYPO_ALERTS_SUPPORTED)
      {
         /* Get the Alert Level.                                        */
         *AlertLevel = HypoAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the hyper alert */
   /* level.  This function takes (UINT16) as a parameter for the SFLOAT*/
   /* data type.  This function returns zero on success or a negative   */
   /* integer indicating that an error occured.                         */
static int SetHyperAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetHyperAlertLevel is supported.   */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_HYPER_ALERTS_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            HyperAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for getting the hyper alert */
   /* level.  This function takes a (UINT16) pointer as a parameter for */
   /* the SFLOAT data type, which will be passed back up to the caller .*/
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
static int GetHyperAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the GetHyperAlertLevel is supported.   */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_HYPER_ALERTS_SUPPORTED)
      {
         /* Get the Alert Level.                                        */
         *AlertLevel = HyperAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the rate of     */
   /* decrease alert level.  This function takes (UINT16) as a parameter*/
   /* for the SFLOAT data type.  This function returns zero on success  */
   /* or a negative integer indicating that an error occured.           */
static int SetRateOfDecreaseAlertLevel(Word_t *AlertLevel)
{
   int   ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetRateOfDecrease is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_RATE_OF_INCREASE_DECREASE_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            RateOfDecreaseAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for getting the rate of     */
   /* decrease alert level.  This function takes a (UINT16) pointer as a*/
   /* parameter for the SFLOAT data type, which will be passed back up  */
   /* to the caller .  This function returns zero on success or a       */
   /* negative integer indicating that an error occured.                */
static int GetRateOfDecreaseAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the getRateOfDecrease is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_RATE_OF_INCREASE_DECREASE_SUPPORTED)
      {
         /* Get the Alert Level.                                        */
         *AlertLevel = RateOfDecreaseAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for setting the rate of     */
   /* increase alert level.  This function takes (UINT16) as a parameter*/
   /* for the SFLOAT data type.  This function returns zero on success  */
   /* or a negative integer indicating that an error occured.           */
static int SetRateOfIncreaseAlertLevel(Word_t *AlertLevel)
{
   int   ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetRateOfIncrease is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_RATE_OF_INCREASE_DECREASE_SUPPORTED)
      {
         if((*AlertLevel <= MAXIMUM_SUPPORTED_CONCENTRATION) && (*AlertLevel >= MINIMUM_SUPPORTED_CONCENTRATION))
         {
            /* Set the Alert Level.                                     */
            RateOfIncreaseAlertLevel = *AlertLevel;
         }
         else
            ret_val = SOCP_PARAMETER_OUT_OF_RANGE;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for getting the rate of     */
   /* increase alert level.  This function takes a (UINT16) pointer as a*/
   /* parameter for the SFLOAT data type, which will be passed back up  */
   /* to the caller .  This function returns zero on success or a       */
   /* negative integer indicating that an error occured.                */
static int GetRateOfIncreaseAlertLevel(Word_t *AlertLevel)
{
   int ret_val = 0;

   /* Check to make sure that the parameters are semi-valid.            */
   if(AlertLevel)
   {
      /* Check to make sure that the SetRateOfIncrease is supported.    */
      if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_RATE_OF_INCREASE_DECREASE_SUPPORTED)
      {
         /* Set the Alert Level.                                        */
         *AlertLevel = RateOfIncreaseAlertLevel;
      }
      else
      {
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
      }
   }
   else
      ret_val = SOCP_INVALID_OPERAND;

   return ret_val;
}

   /* The following function is responsible for resetting the device    */
   /* specific alert.  This function takes no parameters and returns    */
   /* zero for success or a negative integer indicating that an error   */
   /* occured.                                                          */
static int ResetDeviceSpecificAlert(void)
{
   int ret_val = 0;

   /* Check to make sure that the DeviceSpecificAlert is supported.     */
   if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_DEVICE_SPECIFIC_ALERT_SUPPORTED)
   {
      /* Reset the Alert.                                               */
      SensorStatus &= ~CGMS_SENSOR_STATUS_DEVICE_SPECIFIC_ALERT;
   }
   else
   {
      ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
   }

   return ret_val;
}


   /* The following function is responsible for starting a CGMS.  This  */
   /* function takes no parameters and returns zero for success or a    */
   /* negative integer indicating that an error occured.                */
static int StartSession(void)
{
   int ret_val = 0;

   /* Check to make sure that the MultiSession is supported.            */
   if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_MULTIPLE_SESSIONS_SUPPORTED)
   {
      if(SensorStatus & CGMS_SENSOR_STATUS_SESSION_STOPPED)
      {
         /* Delete the sensor measurements list.                        */
         DeleteMeasurementList(&SensorMeasurementList);

         /* Delete the sensor calibration data records.                 */
         DeleteCalibrationRecordList();

         /* Reset the time offset from the first measurement.           */
         TimeOffset           = 0;

         /* Reset the minute counter.                                   */
         MinuteCounter        = 0;

         /* Reset the minutes elapsed for the session run time.         */
         SensorRunTimeMinutes = 0;

         /* Reset the sensor session start time.                        */
         SensorSessionStartTime.Time.Year    = 0;
         SensorSessionStartTime.Time.Month   = 0;
         SensorSessionStartTime.Time.Day     = 0;
         SensorSessionStartTime.Time.Hours   = 0;
         SensorSessionStartTime.Time.Minutes = 0;
         SensorSessionStartTime.Time.Seconds = 0;
         SensorSessionStartTime.TimeZone     = 0;
         SensorSessionStartTime.DSTOffset    = 0;

         /* Flag that a session has started.                            */
         SensorStatus &= ~CGMS_SENSOR_STATUS_SESSION_STOPPED;

         /* Generate the first measurement.                             */
         GenerateMeasurement(NULL);

         /* Start the minute timer.                                     */
         if((TimerID = BSC_StartTimer(BluetoothStackID, 60000, SensorTimerCallback, 1)) <= 0)
         {
            printf("Major Error: Could not start the timer.\r\n");
            ret_val = SOCP_PROCEDURE_NOT_COMPLETED;
         }
      }
      else
         ret_val = SOCP_PROCEDURE_NOT_COMPLETED;
   }
   else
   {
      ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
   }

   return ret_val;
}


   /* The following function is responsible for stopping a CGMS.  This  */
   /* function takes no parameters and returns zero for success or a    */
   /* negative integer indicating that an error occured.                */
static int StopSession(void)
{
   int ret_val = 0;

   /* Check to make sure that the MultiSession is supported.            */
   if(SensorSupportedFeatures & CGMS_FEATURE_FLAG_MULTIPLE_SESSIONS_SUPPORTED)
   {
      /* Check if there is a session started.                           */
      if(!(SensorStatus & CGMS_SENSOR_STATUS_SESSION_STOPPED))
      {
         /* Stop the minute timer.                                      */
         if(!(BSC_StopTimer(BluetoothStackID, TimerID)))
         {
            /* Reset the minutes elapsed for the session run time.      */
            SensorRunTimeMinutes = 0;

            /* Reset the time offset from the first measurement.        */
            TimeOffset           = 0;

            /* Set the appropriate Status Bits.                         */
            SensorStatus |= CGMS_SENSOR_STATUS_SESSION_STOPPED;
            SensorStatus |= CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED;
         }
         else
         {
            printf("Error: Could not stop the timer.\r\n");
            ret_val = SOCP_PROCEDURE_NOT_COMPLETED;
         }
      }
      else
         ret_val = SOCP_OP_CODE_NOT_SUPPORTED;
   }
   else
      ret_val = SOCP_OP_CODE_NOT_SUPPORTED;

   return ret_val;
}

   /* The following function is responsible for initializing the Server.*/
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
static int InitializeServer(void)
{
   int           ret_val = FUNCTION_ERROR;
   unsigned int  ServiceID;

   /* Make sure that the stack has been initialized.                    */
   if(BluetoothStackID)
   {
      /* Initialize the sensor measurement list.                        */
      SensorMeasurementList.Head                  = NULL;
      SensorMeasurementList.Tail                  = NULL;
      SensorMeasurementList.NumberOfMeasurements  = 0;
      SensorMeasurementList.ListType              = ltSensor;

      /* Set Default Configuration.                                     */
      SensorMeasurementInterval                   = SENSOR_MEASUREMENT_INTERVAL;
      SensorCommunicationInterval                 = SENSOR_COMMUNICATION_INTERVAL;
      SensorSessionRunTime                        = SENSOR_SESSION_RUN_TIME;

      /* Set the sensor default features.  Note the type is the upper   */
      /* nibble and the sample location is the lower nibble for the     */
      /* type-sample location byte.                                     */
      SensorSupportedFeatures                     = SENSOR_FEATURES;
      SensorTypeSampleLocation                   |= (Byte_t)SENSOR_TYPE;
      SensorTypeSampleLocation                   |= ((Byte_t)SENSOR_SAMPLE_LOCATION << 4);

      /* Set the initial status flags.                                  */
      SensorStatus                               |= CGMS_SENSOR_STATUS_SESSION_STOPPED;
      SensorStatus                               |= CGMS_SENSOR_STATUS_CALIBRATION_RECOMMENDED;
      SensorStatus                               |= CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED;

      /* Generate random seed for random number generation.             */
      srand(time(NULL));

      /* Initialize BMS Service on the server.                          */
      ret_val = BMS_Initialize_Service(BluetoothStackID, BMS_FLAGS_SUPPORT_DUAL_MODE, BMS_EventCallback, 0, &ServiceID);
      if(ret_val > 0)
      {
         /* Save the BMSinstanceID.                                     */
         BMSInstanceID = (unsigned int)ret_val;

         /* Register all features for BMS.  Set the default features.   */
         if((ret_val = BMS_Set_BM_Features(BluetoothStackID, BMSInstanceID, (DEFAULT_BM_LE_FEATURES_BIT_MASK|DEFAULT_BM_BR_EDR_FEATURES_BIT_MASK|DEFAULT_BM_DUAL_MODE_FEATURES_BIT_MASK))) == 0)
         {
            /* Return success to the caller.                      */
            ret_val = 0;
         }
      }
   }
   else
   {
      printf("The stack has not been initialized.\r\n");
      ret_val = FUNCTION_ERROR;
   }

   return ret_val;
}

   /* The following function is responsible for initializing the Client.*/
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
static int InitializeClient(void)
{
   int ret_val = 0;

   /* Initialize the collector measurement list.                        */
   CollectorMeasurementList.Head                 = NULL;
   CollectorMeasurementList.Tail                 = NULL;
   CollectorMeasurementList.NumberOfMeasurements = 0;
   CollectorMeasurementList.ListType             = ltCollector;

   return ret_val;
}

/* The following function is responsible for calculating the actual     */
/* session start time on the client.  This function will also determine */
/* the session run time once the session start time has been determined.*/
/* This function takes no parameters and returns nothing.               */
static void CalculateSessionStartTime(void)
{
   DWord_t Minutes;

   /* Adjust for the number of minutes since the first measurement was  */
   /* taken.  Note we need to store the value in case it changes.       */
   Minutes = MinuteCounter;

   /* If there are excess minutes that is not divisble by an hour then  */
   /* we need to calculate the correct number of minutes and increment  */
   /* the other time fields if necessary.                               */
   if(Minutes % 60)
   {
      /* Add the excess minutes.  1st Case: The excess minutes plus the */
      /* current minutes do not go over an hour.  2nd Case: They sum    */
      /* does go over or is equal to an hour.                           */
      if((SensorSessionStartTime.Time.Minutes + (Minutes % 60)) < 60)
      {
         /* Simply store the excess minutes.                            */
         SensorSessionStartTime.Time.Minutes += (Minutes % 60);
      }
      else
      {
         /* Store the remainder of minutes that go over an hour.        */
         SensorSessionStartTime.Time.Minutes = ((SensorSessionStartTime.Time.Minutes + (Minutes % 60)) % 60);

         /* Check the hours and adjust accordingly.                     */
         if(++SensorSessionStartTime.Time.Hours == 24)
         {
            /* Reset the hours and increment the day.                   */
            SensorSessionStartTime.Time.Hours = 0;
            if(++SensorSessionStartTime.Time.Day == 32)
            {
               /* Reset the day and increment the month.                */
               SensorSessionStartTime.Time.Day = 1;
               if(++SensorSessionStartTime.Time.Month == 13)
               {
                  /* Reset the month and increment the year.            */
                  SensorSessionStartTime.Time.Month = 1;
                  ++SensorSessionStartTime.Time.Year;
               }
            }
         }
      }

      /* Let's go ahead and subtract the extra minutes so that the      */
      /* remaining minutes are divisble by an hour with no remainder.   */
      Minutes -= Minutes % 60;
   }

   /* Now lets divide the rest of the minutes up by hours.              */
   if((SensorSessionStartTime.Time.Hours + (Minutes / 60)) >= 24)
   {
      /* Store the remainder of hours that go over the number of hours  */
      /* in a day and subtract those hours (in minutes) from the number */
      /* of minutes.                                                    */
      SensorSessionStartTime.Time.Hours = (SensorSessionStartTime.Time.Hours + ((Minutes / 60)) % 24);
      Minutes                          -= (SensorSessionStartTime.Time.Hours + ((Minutes / 60)) % 24) * 60;

      /* Now lets divide the rest of the minutes up by day.             */
      if((SensorSessionStartTime.Time.Day + (Minutes / 1440)) >= 31)
      {
         /* Store the remainder of days that go over the number of days */
         /* in a month and subtract those days (in minutes) from the    */
         /* number of minutes.                                          */
         SensorSessionStartTime.Time.Day = (SensorSessionStartTime.Time.Day + ((Minutes / 1440)) % 31);
         Minutes                        -= (SensorSessionStartTime.Time.Day + ((Minutes / 1440)) % 31) * 1440;

         /* Now lets divide the rest of minutes up by month.            */
         if((SensorSessionStartTime.Time.Month + (Minutes / 44640)) >= 12)
         {
            /* Store the remainder of days that go over the number of   */
            /* days in a month and subtract those days (in minutes) from*/
            /* the number of minutes.                                   */
            SensorSessionStartTime.Time.Month = (SensorSessionStartTime.Time.Month + ((Minutes / 44640)) % 12);
            Minutes                          -= (SensorSessionStartTime.Time.Month + ((Minutes / 44640)) % 12) * 44640;

            /* Remaining minutes must be in years.                      */
            SensorSessionStartTime.Time.Year  += (Minutes / 525600);

         }
         else
         {
            /* Otherwise add the month.                                 */
            SensorSessionStartTime.Time.Month += (Minutes / 44640);
         }
      }
      else
      {
         /* Otherwise add the day.                                      */
         SensorSessionStartTime.Time.Day      += (Minutes / 1440);
      }
   }
   else
   {
      /* Otherwise add the hours.                                       */
      SensorSessionStartTime.Time.Hours       += (Minutes / 60);
   }
}

   /* The following function is responsible for adding a new measurement*/
   /* to either the Sensor Measurement List or the Collector Measurement*/
   /* List.  This function takes a pointer to the measurement to add to */
   /* list specified by the next parameter pointer.  If the maximum     */
   /* number of supported measurements has been reached by the client   */
   /* then the oldest measurement shall be removed and the new          */
   /* measurement shall be placed at the front of the list (Queue).     */
   /* This function returns zero on success or a negative integer       */
   /* indicating that an error occured.                                 */
   /* *** NOTE *** It is the caller's responsibility to make sure that  */
   /*            the new measurement to be added has been initialized   */
   /*            properly, including the next and previous pointers     */
   /*            being set to NULL.                                     */
   /* *** NOTE *** For adding measurements to a temporary list from the */
   /*            sensor measurement list it is impossible to go over the*/
   /*            maximum number of supported measurements since the     */
   /*            temporary list must contain less than or equal to the  */
   /*            number of measurements in the sensor measurement list. */
static int AddNewMeasurementToList(Measurement_t *MeasurementToAdd, Measurement_List_t *List)
{
   int                ret_val = 0;
   Measurement_t     *OldestToDelete = NULL;

   /* Check to make sure the parameters are semi-valid.                 */
   if((MeasurementToAdd) && (List))
   {
      /* If this is not the first measurement in the list then we need  */
      /* to point it to the current head of the list and update the     */
      /* necessary pointers.                                            */
      if((List->Head != NULL) && (List->Tail != NULL) && (List->NumberOfMeasurements != 0))
      {
         /* Determine if the maximum number of measurements has been    */
         /* reached.  If it has then we need to delete the oldest       */
         /* measurement from the list since we cannot hold anymore      */
         /* measurements.                                               */
         if(((List->ListType == ltSensor)    && (List->NumberOfMeasurements == MAX_SENSOR_MEASUREMENTS)) ||
            ((List->ListType == ltCollector) && (List->NumberOfMeasurements == MAX_COLLECTOR_MEASUREMENTS)))
         {
            /* Set a pointer to the oldest measurement that will be     */
            /* deleted.                                                 */
            OldestToDelete          = List->Tail;

            /* Update the new tail pointer and set the new tail         */
            /* pointer's next pointer to null.  Note since we have the  */
            /* maximum number of measurements we are guranteed to have a*/
            /* previous pointer from the tail.  Note we don't care about*/
            /* the oldest's previous pointer since it is going to be    */
            /* deleted.                                                 */
            List->Tail              = OldestToDelete->Previous;
            List->Tail->Next        = NULL;

            /* Delete the oldest measurement since we have successfully */
            /* removed it from the list.                                */
            BTPS_FreeMemory(OldestToDelete);
            OldestToDelete          = NULL;

            /* Add the new measurement to the beginning of the list     */
            /* (Queue).  Set the current head of the list's previous    */
            /* pointer to point to the new measurement.  Set the new    */
            /* measurement's next pointer to point to the current head  */
            /* of the list.  Update the head pointer to point to the new*/
            /* measurement.                                             */
            List->Head->Previous    = MeasurementToAdd;
            MeasurementToAdd->Next  = List->Head;
            List->Head              = MeasurementToAdd;

            /* Note we do not need to increment the number of           */
            /* measurements since they should be the same.              */
         }
         else
         {
            /* Add the new measurement to the beginning of the list.    */
            /* Set the current head of the list's previous pointer to   */
            /* point to the new measurement.  Set the new measurement's */
            /* next pointer to point to the current head of the list.   */
            /* Update the head pointer to point to the new measurement. */
            List->Head->Previous    = MeasurementToAdd;
            MeasurementToAdd->Next  = List->Head;
            List->Head              = MeasurementToAdd;

            /* Increment the number of file objects in the list.        */
            List->NumberOfMeasurements++;
         }
      }
      else
      {
         /* Otherwise this is the first measurement in the list and we  */
         /* need to point the head and tail pointers to the new         */
         /* measurement.                                                */
         List->Head                 = List->Tail = MeasurementToAdd;

         /* Increment the number of measurements in the list.           */
         List->NumberOfMeasurements++;
      }
   }
   else
   {
      printf("The pointer to the measurement or list of measurements is invalid.\r\n");
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return ret_val;
}

   /* The following function is a helper function for removing a node   */
   /* from the sensor measurement list.  This function takes a parameter*/
   /* for the node that needs to be removed and detaches it from the    */
   /* list by updating its pointers and adjacent node pointers.  This   */
   /* function will return zero for success meaning that the node has   */
   /* been detached and will be return via parameter to the caller.     */
   /* Otherwise a negative integer will be return indicating that an    */
   /* error occcured.                                                   */
static void RemoveNodefromSensorMeasurementList(Measurement_t *MeasurementToRemove)
{
   /* If the node to be removed has a non-NULL next and previous pointer*/
   /* then we need to change the pointers so that the two adjacent nodes*/
   /* skip over the one being removed.                                  */
   if((MeasurementToRemove->Next != NULL) && (MeasurementToRemove->Previous != NULL))
   {
      /* Note both values assigned will not be NULL.                    */
      MeasurementToRemove->Previous->Next = MeasurementToRemove->Next;
      MeasurementToRemove->Next->Previous = MeasurementToRemove->Previous;
   }
   else
   {
      /* If the node to be removed only has a next Node.  ie.  the node */
      /* to be removed is the head node.                                */
      if((MeasurementToRemove->Next != NULL) && (MeasurementToRemove->Previous == NULL))
      {
         /* We need to set the next node's previous pointer to NULL and */
         /* update the head pointer.                                    */
         MeasurementToRemove->Next->Previous = NULL;
         SensorMeasurementList.Head = MeasurementToRemove->Next;
      }
      else
      {
         /* If the node to be removed only has a previous Node.  ie.    */
         /* the node to be removed is the tail node.                    */
         if((MeasurementToRemove->Next == NULL) && (MeasurementToRemove->Previous != NULL))
         {
            /* We need to set the previous node's next pointer to NULL  */
            /* and update the tail pointer.                             */
            MeasurementToRemove->Previous->Next = NULL;
            SensorMeasurementList.Tail = MeasurementToRemove->Previous;
         }
         else
         {
            /* Otherwise we need to set the head and tail pointers to   */
            /* NULL since the node is the last node in the list because */
            /* both pointers are NULL.                                  */
            SensorMeasurementList.Head = NULL;
            SensorMeasurementList.Tail = NULL;
         }
      }
   }

   /* Let's go ahead and set the measurement to remove's next and       */
   /* previous pointers to null.                                        */
   MeasurementToRemove->Next     = NULL;
   MeasurementToRemove->Previous = NULL;

   /* Decrement the number of measurments.                              */
   SensorMeasurementList.NumberOfMeasurements--;
}

   /* The following function is responsible for deleting a measurement  */
   /* list.  This function takes a pointer to the measurement list to be*/
   /* deleted.  This function returns zero on success or a negative     */
   /* integer indicating that an error occured.                         */
static int DeleteMeasurementList(Measurement_List_t *ListToDelete)
{
   int                ret_val                 = 0;
   Measurement_t     *MeasurementToDelete     = NULL;
   Measurement_t     *NextMeasurementToDelete = NULL;

   /* Determine if there is anything to delete.                         */
   if(ListToDelete->NumberOfMeasurements != 0)
   {
      /* Set the head of the measurement list to the first measurement  */
      /* to be deleted.                                                 */
      MeasurementToDelete         = ListToDelete->Head;

      /* While there is a measurement to delete.                        */
      while(MeasurementToDelete)
      {
         /* Save a pointer to the next measurement to be deleted.  Note */
         /* we do not need to do anything with the previous pointers    */
         /* since we are guranteed to remove every measurement from the */
         /* list.                                                       */
         NextMeasurementToDelete  = MeasurementToDelete->Next;

         /* Delete the measurement.                                     */
         BTPS_FreeMemory(MeasurementToDelete);

         /* Decrement the number of measurements.                       */
         ListToDelete->NumberOfMeasurements--;

         /* Set the next measurement to be deleted.                     */
         MeasurementToDelete      = NextMeasurementToDelete;
      }

      /* Now that the list is empty set the file object list head to    */
      /* null.                                                          */
      ListToDelete->Head = NULL;
      ListToDelete->Tail = NULL;

      /* Check to make sure that all file objects in the list have been */
      /* removed.                                                       */
      if(ListToDelete->NumberOfMeasurements != 0)
      {
         /* Report the serious error to the user.                       */
         printf("DeleteMeasurementList: The number of measurements that remain are not zero: 0x%04X\r\n", ListToDelete->NumberOfMeasurements);
         ret_val = FUNCTION_ERROR;
      }
   }

   return ret_val;
}

   /* The following function is a utililty function to copy one         */
   /* measurement node to another measurement node.                     */
static void CopyMeasurement(Measurement_t *Measurement1, Measurement_t *Measurement2)
{
   /* Check to make sure that the parameters are semi valid.            */
   if((Measurement1) && (Measurement2))
   {
      Measurement2->Number               = Measurement1->Number;
      Measurement2->Size                 = Measurement1->Size;
      Measurement2->Notified             = Measurement1->Notified;
      Measurement2->Flags                = Measurement1->Flags;
      Measurement2->GlucoseConcentration = Measurement1->GlucoseConcentration;
      Measurement2->TimeOffset           = Measurement1->TimeOffset;
      Measurement2->Trend                = Measurement1->Trend;
      Measurement2->Quality              = Measurement1->Quality;
      Measurement2->Status               = Measurement1->Status;
      Measurement2->CalTemp              = Measurement1->CalTemp;
      Measurement2->Warning              = Measurement1->Warning;

      /* We do not want to copy the pointers because the measurement    */
      /* will point to one or more different memory locations.          */
   }
   else
      DisplayFunctionError("CopyMeasurement", FUNCTION_ERROR);
}

   /* The following function is responsible for adding a new calibration*/
   /* record to the calibration data record list.  This function takes a*/
   /* pointer to the record that is to be added.  This function returns */
   /* zero if successful or a negative integer to indicate that an error*/
   /* occured.                                                          */
   /* *** NOTE *** It is the caller's responsibility to make sure that  */
   /*            the new calibration data record to be added has been   */
   /*            initialized properly, including the next pointer being */
   /*            set to NULL.                                           */
static int AddNewCalibrationDataRecord(CalibrationDataRecord_t *RecordToAdd)
{
   int ret_val = 0;

   /* Make sure that the parameters are semi valid.                     */
   if(RecordToAdd)
   {
      /* If the list exists insert the new record at the end of the list*/
      /* and update the tail pointer.                                   */
      if(CalibrationDataRecordList.Head != NULL)
      {
         CalibrationDataRecordList.Tail->Next = RecordToAdd;
         CalibrationDataRecordList.Tail       = RecordToAdd;
      }
      else
      {
         /* Otherwise this is the first record in the list and we need  */
         /* to point the head and tail pointers to the record.          */
         CalibrationDataRecordList.Head = CalibrationDataRecordList.Tail = RecordToAdd;
      }

      /* Increment the number of file objects in the list.              */
      CalibrationDataRecordList.NumberOfRecords++;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;


   return (ret_val);
}

   /* The following function is responsible for deleting the calibration*/
   /* data record list.  This function takes no parameters and returns  */
   /* zero for success or a negative integer to indicate that an error  */
   /* occured.                                                          */
static int DeleteCalibrationRecordList(void)
{
   int                      ret_val = 0;
   CalibrationDataRecord_t *RecordToDelete;
   CalibrationDataRecord_t *NextRecordToDelete;

   /* Determine if there are any records to delete.                     */
   if((CalibrationDataRecordList.Head != NULL) && (CalibrationDataRecordList.Tail != NULL) && (CalibrationDataRecordList.NumberOfRecords != 0))
   {
      /* Set the head of the calibration record list to the first record*/
      /* to be deleted.                                                 */
      RecordToDelete = CalibrationDataRecordList.Head;

      /* While there is a record to delete.                             */
      while(RecordToDelete)
      {
         /* Set the next record to be deleted to the current record's   */
         /* next pointer.                                               */
         NextRecordToDelete = RecordToDelete->Next;

         /* Delete the record.                                          */
         BTPS_FreeMemory(RecordToDelete);

         /* Decrement the number of records.                            */
         CalibrationDataRecordList.NumberOfRecords--;

         /* Set the next record to be deleted.                          */
         RecordToDelete = NextRecordToDelete;
      }

      /* Now that the list is empty, set the records list head/tail to  */
      /* null.                                                          */
      CalibrationDataRecordList.Head = NULL;
      CalibrationDataRecordList.Tail = NULL;

      /* Reset the data record counter.                                 */
      CalibrationDataRecordCtr = 0;

      /* Check to make sure that all records in the list have been      */
      /* removed.                                                       */
      if(CalibrationDataRecordList.NumberOfRecords != 0)
      {
         printf("DeleteCalibrationRecordList: The number of records that remain are not zero: %u\r\n", CalibrationDataRecordList.NumberOfRecords);
         ret_val = FUNCTION_ERROR;
      }
   }

   return ret_val;
}

   /* The following function is responsible for performing a BAP        */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverBMS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!BMSInstanceID)
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
               /* Configure the filter so that only the BMS Service is  */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               BMS_ASSIGN_BMS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdBMS);
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
      printf("Only the client can discover BMS.\r\n");

   return(ret_val);
}

   /* Function to compare a received BMS Authorization Code to the      */
   /* stored value.                                                     */
static Boolean_t VerifyBMSCommand(BMS_BM_Control_Point_Format_Data_t *FormatData)
{
   Boolean_t ret_val = FALSE;

   if(FormatData->AuthorizationCodeLength == sizeof(BMS_Authorization_Code))
   {
      if ((BTPS_MemCompare(FormatData->AuthorizationCode, BMS_Authorization_Code, FormatData->AuthorizationCodeLength)) == 0)
         ret_val = TRUE;
   }

   return(ret_val);
}

   /* Function to execute a BMS command by deleting bonding information.*/
   /* * NOTE * The bond information won't be cleared until the transport*/
   /*          has been closed.                                         */
static int ExecuteBMSCommand(BMS_BMCP_Command_Type_t CommandType)
{
   int          ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   switch(CommandType)
   {
      /* Delete Bond Requesting device case.                            */
      /* * NOTE * The service will catch unsupported op codes so we do  */
      /*          not need to check the transport here.                 */
      case bmcDeleteBondRequestingBREDR_LE:
      case bmcDeleteBondRequestingBREDR:
      case bmcDeleteBondRequestingLE:
         /* Get the DeviceInfo (that stores the requesting device's     */
         /* bonding information).                                       */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
             /* Simply clear the requesting device bond information by  */
             /* resetting the flags so that the stored keys are no      */
             /* longer valid.  This will force the requesting device to */
             /* establish a new bond when bonding occurs.               */
             /* * NOTE * Since this application only stores the LTK (LE)*/
             /*          and Link Key (BR/EDR) we will not remove any   */
             /*          other stored keys.                             */
             /* * NOTE * This application only stores the LTK (LE) on   */
             /*          the Master device, the LTK should already be   */
             /*          cleared, but we wil include it for             */
             /*          demonstrational purposes.                      */
            switch(CommandType)
            {
               case bmcDeleteBondRequestingBREDR_LE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteBondRequestingBREDR:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteBondRequestingLE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  break;
               default:
                  ret_val = BMS_ERROR_CODE_OPERATION_FAILED;
            }
         }
         else
         {
            printf("Failed to find the requesting device's bond information.\r\n");
            ret_val = BMS_ERROR_CODE_OPERATION_FAILED;
         }
         break;
      /* Delete all bonds.                                              */
      /* * NOTE * The service will catch unsupported op codes so we do  */
      /*          not need to check the transport here.                 */
      case bmcDeleteAllBondsBREDR_LE:
      case bmcDeleteAllBondsBREDR:
      case bmcDeleteAllBondsLE:
         /* We will simply loop through the device list an clear all    */
         /* bonds.                                                      */
         DeviceInfo = DeviceInfoList;
         while(DeviceInfo != NULL)
         {
            /* Simply clear the device's bond information by resetting  */
            /* the flags so that the stored keys are no longer valid.   */
            /* This will force the requesting device to establish a new */
            /* bond when bonding occurs.                                */
            /* * NOTE * Since this application only stores the LTK (LE) */
            /*          and Link Key (BR/EDR) we will not remove any    */
            /*          other stored keys.                              */
            /* * NOTE * This application only stores the LTK (LE) on the*/
            /*          Master device, the LTK should already be        */
            /*          cleared, but we wil include it for              */
            /*          demonstrational purposes.                       */
            switch(CommandType)
            {
               case bmcDeleteAllBondsBREDR_LE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteAllBondsBREDR:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteAllBondsLE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  break;
               default:
                  ret_val = BMS_ERROR_CODE_OPERATION_FAILED;
            }

            /* Get the next device's information.                       */
            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }
         break;
      /* Delete all bonds except the requesting device.                 */
      /* * NOTE * The service will catch unsupported op codes so we do  */
      /*          not need to check the transport here.                 */
      case bmcDeleteAllOtherBondsBREDR_LE:
      case bmcDeleteAllOtherBondsBREDR:
      case bmcDeleteAllOtherBondsLE:
         /* We will simply loop through the device list an clear all    */
         /* bonds.                                                      */
         DeviceInfo = DeviceInfoList;
         while(DeviceInfo != NULL)
         {
            /* If the device information does not match the requesting  */
            /* device then we will remove the device information.       */
            if(!COMPARE_BD_ADDR(DeviceInfo->ConnectionBD_ADDR, ConnectionBD_ADDR))
            {
               /* Simply clear the device's bond information by         */
               /* resetting the flags so that the stored keys are no    */
               /* longer valid.  This will force the requesting device  */
               /* to establish a new bond when bonding occurs.          */
               /* * NOTE * Since this application only stores the LTK   */
               /*          (LE) and Link Key (BR/EDR) we will not remove*/
               /*          any other stored keys.                       */
               /* * NOTE * This application only stores the LTK (LE) on */
               /*          the Master device, the LTK should already be */
               /*          cleared, but we wil include it for           */
               /*          demonstrational purposes.                    */
               switch(CommandType)
               {
               case bmcDeleteAllOtherBondsBREDR_LE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteAllOtherBondsBREDR:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                  break;
               case bmcDeleteAllOtherBondsLE:
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                  break;
               default:
                  ret_val = BMS_ERROR_CODE_OPERATION_FAILED;
               }
            }

            /* Get the next device's information.                       */
            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }
         break;
      default:
         ret_val = BMS_ERROR_CODE_OPCODE_NOT_SUPPORTED;
         break;
   }

   return(ret_val);
}

   /* The following function will send a BMS Command to a remote BMS    */
   /* server.                                                           */
static int BMSCommand(ParameterList_t *TempParam)
{
   int                                 ret_val = FUNCTION_ERROR;
   unsigned int                        Index;
   DeviceInfo_t                       *DeviceInfo;
   Word_t                              MTU;

   /* Verify that we are the client.                                    */
   if(!BMSInstanceID)
   {
      /* Verify the parameters.                                         */
      if((TempParam) && (TempParam->NumberofParameters == 2))
      {
         /* Verify that we have not allocated memory for the command    */
         /* buffer, meaning a write is already in progress.             */
         if(!(BMSCommandBufferLength))
         {
            /* Initialize the structure.                                */
            BTPS_MemInitialize(&FormatData, 0, BMS_BM_CONTROL_POINT_FORMAT_DATA_SIZE);

            /* Get the selected BMS Command.                            */
            FormatData.CommandType = (BMS_BMCP_Command_Type_t)(BMSCommandTable[(TempParam->Params[0].intParam)-1].Command);

            /* Get the Use Authorization Code checkbox value.           */
            if(TempParam->Params[1].intParam == 0)
            {
               FormatData.AuthorizationCode = NULL;
               FormatData.AuthorizationCodeLength = 0;
            }
            else
            {
               FormatData.AuthorizationCode = BMS_Authorization_Code;
               FormatData.AuthorizationCodeLength = sizeof(BMS_Authorization_Code);
            }

            /* Set the buffer size.                                     */
            BMSCommandBufferLength = BMS_BM_CONTROL_POINT_FORMAT_DATA_SIZE + FormatData.AuthorizationCodeLength;

            /* Allocate memory for the Command Buffer.                  */
            if((BMSCommandBuffer = BTPS_AllocateMemory(BMSCommandBufferLength)) != NULL)
            {
               ret_val = BMS_Format_BM_Control_Point_Command(&FormatData, &BMSCommandBufferLength, BMSCommandBuffer);

               if(ret_val == 0)
               {
                  /* Check to see if we are configured as a client with */
                  /* an active connection                               */
                  if(ConnectionID)
                  {
                     /* Get the device info for the connection device.  */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                     {
                        /* Verify that the client has received a valid  */
                        /* Bond Management Feature and Bond Management  */
                        /* Control Point Attribute Handle.              */
                        if((DeviceInfo->BMSClientInfo.BM_Feature) && (DeviceInfo->BMSClientInfo.BM_Control_Point))
                        {
                           GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU);

                           if((Word_t)BMSCommandBufferLength <= (MTU - 3))
                           {
                              /* Finally, submit a write request to the */
                              /* server                                 */
                              if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->BMSClientInfo.BM_Control_Point, BMSCommandBufferLength, BMSCommandBuffer, GATT_ClientEventCallback_BMS, DeviceInfo->BMSClientInfo.BM_Control_Point)) > 0)
                              {
                                 printf("\r\nBMS Command Sent Successfully, TransactionID: 0x%02X.\r\n", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                              }
                              else
                                 DisplayFunctionError("GATT_Write_Request", ret_val);
                           }
                           else
                           {
                              /* Finally, submit a prepare write request*/
                              /* to the server                          */
                              if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->BMSClientInfo.BM_Control_Point, (MTU - 5), 0, BMSCommandBuffer, GATT_ClientEventCallback_BMS, DeviceInfo->BMSClientInfo.BM_Control_Point)) > 0)
                              {
                                 printf("Sent %d.\r\n", (MTU - 5));
                                 BMSCommandBuffer        += (MTU - 5);
                                 BMSCommandBufferLength  -= (MTU - 5);
                                 BMSCommandTransactionID  = ret_val;
                                 BMSCommandValueOffset    = (MTU - 5) - 1;
                                 printf("\r\nBMS Command Sent Successfully, TransactionID: 0x%02X.\r\n", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                              }
                              else
                                 DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                           }
                        }
                        else
                           printf("BMS has not been discovered.\r\n");
                     }
                     else
                        printf("Must be conected to a BMS server.\r\n");
                  }
                  else
                     printf("Must be a client and connected to a BMS server.\r\n");
               }
               else
                  printf("\r\nBMS_Format_BM_Control_Point_Command Failed: %d.\r\n", ret_val);

               /* Free the memory allocated for the buffer and set the  */
               /* length to zero.                                       */
               BTPS_FreeMemory(BMSCommandBuffer);
               BMSCommandBufferLength = 0;
            }
            else
               printf("Insufficient Resources.\r\n");
         }
         else
            printf("BMS Control Point Write in Progress.\r\n");
      }
      else
      {
         printf("Usage: DeleteBond [Command] [Use Authorization Code (0=FALSE, 1=TRUE)]\r\n");
         printf("   Where command is:\r\n\r\n");

         for(Index = 1; Index < (NUM_BMS_COMMANDS+1); Index++)
         {
            printf("%u - %s\r\n", Index, BMSCommandTable[Index-1].Name);
         }
         printf("\r\n");
      }
   }
   else
      printf("Must be client to send command.\r\n");

   return ret_val;
}

   /* The following function is a helper function to display the        */
   /* supported features of the server.                                 */
static void DisplaySupportedFeatures(DWord_t SupportedFeatures)
{
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE)
      printf("DELETE_BOND_REQUESTING_DEVICE_BREDR_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE_AUTH)
      printf("DELETE_BOND_REQUESTING_DEVICE_BREDR_LE_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR)
      printf("DELETE_BOND_REQUESTING_DEVICE_BREDR\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_BREDR_AUTH)
      printf("DELETE_BOND_REQUESTING_DEVICE_BREDR_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE)
      printf("DELETE_BOND_REQUESTING_DEVICE_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_REQUESTING_DEVICE_LE_AUTH)
      printf("DELETE_BOND_REQUESTING_DEVICE_LE_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE)
      printf("DELETE_BOND_ALL_DEVICES_BREDR_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_LE_AUTH)
      printf("DELETE_BOND_ALL_DEVICES_BREDR_LE_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR)
      printf("DELETE_BOND_ALL_DEVICES_BREDR\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_BREDR_AUTH)
      printf("DELETE_BOND_ALL_DEVICES_BREDR_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE)
      printf("DELETE_BOND_ALL_DEVICES_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_ALL_DEVICES_LE_AUTH)
      printf("DELETE_BOND_ALL_DEVICES_LE_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE)
      printf("DELETE_BOND_OTHER_DEVICES_BREDR_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_LE_AUTH)
      printf("DELETE_BOND_OTHER_DEVICES_BREDR_LE_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR)
      printf("DELETE_BOND_OTHER_DEVICES_BREDR\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_BREDR_AUTH)
      printf("DELETE_BOND_OTHER_DEVICES_BREDR_AUTH\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE)
      printf("DELETE_BOND_OTHER_DEVICES_LE\r\n");
   if (SupportedFeatures & BMS_BM_FEATURE_FLAG_DELETE_BOND_OTHER_DEVICES_LE_AUTH)
      printf("DELETE_BOND_OTHER_DEVICES_LE_AUTH\r\n");
}

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */

   /* The following function is the timer callback function for the     */
   /* minute timer.  This function will be called whenever a timer has  */
   /* been created, a Callback has been registered for the associated   */
   /* BluetoothStackID, and a minute has passed.  The purpose of this   */
   /* function is to count the number of minutes and either take a      */
   /* measurement or notify recent measurements.                        */
static void BTPSAPI SensorTimerCallback(unsigned int BluetoothStackID, unsigned int CTimerID, unsigned long CallbackParameter)
{
   int                       ret_val                  = 0;
   Measurement_t            *Index                    = NULL;
   Measurement_t            *NewMeasurement           = NULL;
   Measurement_t            *FirstNotification        = NULL;
   Measurement_List_t        NotificationList;
   CGMS_Measurement_Data_t **NotificationListArray    = NULL;
   CGMS_Measurement_Data_t  *NewNotification          = NULL;
   Word_t                    ArrayIndex               = 0;
   Word_t                    NumberOfMeasurementsSent = 0;
   DeviceInfo_t             *DeviceInfo               = NULL;

   /* We need to increment the minute counter since a minute has        */
   /* occured.                                                          */
   MinuteCounter++;

   /* Check if we need to terminate the session.                        */
   if(SensorRunTimeMinutes != SENSOR_SESSION_RUN_TIME)
   {
      /* Check to see if we have reached the maximum number of minutes  */
      /* before we need to take a measurement.                          */
      if(MinuteCounter % SensorMeasurementInterval == 0)
      {
         /* Generate a measurement.                                     */
         GenerateMeasurement(NULL);
      }

      /* Check to make sure that a session is in progress.              */
      if(!(SensorStatus & CGMS_SENSOR_STATUS_SESSION_STOPPED))
      {
         /* Increment the Session Run Time Minutes.                     */
         SensorRunTimeMinutes++;

         /* Make sure we are connected.                                 */
         if(ConnectionID)
         {
            /* Make sure that periodice communications is not disabled. */
            /* Note we also do not want to take the mod of 0.           */
            if(SensorCommunicationInterval != 0)
            {
               /* Check to see if we need to send a notification of     */
               /* recent measurements.                                  */
               if(MinuteCounter % SensorCommunicationInterval == 0)
               {
                  printf("\r\nSending periodic notification of recent measurements.\r\n");

                  /* Getting DeviceInfo to verify that the collector has*/
                  /* registered for notifications.  Note we must be     */
                  /* connected.                                         */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                  {
                     /* Need to check if the collector has registered   */
                     /* for notifications.  Otherwise we do not want to */
                     /* send notifications.                             */
                     if(DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                     {
                        /* Make sure that the sensor measurement list is*/
                        /* not empty.                                   */
                        if((SensorMeasurementList.Head != NULL) && (SensorMeasurementList.Tail != NULL) && (SensorMeasurementList.NumberOfMeasurements > 0))
                        {
                           /* Initialize the temporary list.            */
                           NotificationList.NumberOfMeasurements = 0;
                           NotificationList.ListType             = ltTemp;
                           NotificationList.Head                 = NULL;
                           NotificationList.Tail                 = NULL;

                           /* Go through the sensor list and add the    */
                           /* measurements that have not been notified  */
                           /* yet.  Note measurements will be notified  */
                           /* in the reverse order we find them (FIFO). */
                           Index = SensorMeasurementList.Head;
                           while(Index != NULL)
                           {
                              /* If the measurement has not been        */
                              /* notified.                              */
                              if(Index->Notified == FALSE)
                              {
                                 /* If this is the first measurement    */
                                 /* that is going to be notified, let's */
                                 /* save a pointer to it so we do not   */
                                 /* have to traverse back through the   */
                                 /* sensor measurement list to find the */
                                 /* first measurement that was notified.*/
                                 if(NotificationList.NumberOfMeasurements == 0)
                                    FirstNotification = Index;

                                 /* Allocate a new measurement and add  */
                                 /* it to the temporary notification    */
                                 /* list.                               */
                                 if((NewMeasurement = (Measurement_t *)BTPS_AllocateMemory(sizeof(Measurement_t))) != NULL)
                                 {
                                    /* Copy the current measurement     */
                                    /* information to the new           */
                                    /* measurement .                    */
                                    CopyMeasurement(Index, NewMeasurement);

                                    /* Initialize the pointers to null. */
                                    NewMeasurement->Next     = NULL;
                                    NewMeasurement->Previous = NULL;

                                    /* Add the new measurement to the   */
                                    /* notification list.               */
                                    AddNewMeasurementToList(NewMeasurement,  &NotificationList);
                                 }
                                 else
                                 {
                                    printf("Memory could not be allocated for the new measurement.\r\n");
                                    break;
                                 }
                              }

                              /* Go to the next node.                   */
                              Index = Index->Next;
                           }

                           /* Send recent notifications.  Determine if  */
                           /* we have measurements to notify.           */
                           if(NotificationList.NumberOfMeasurements != 0)
                           {
                              /* Allocate memory for an array of        */
                              /* CGMS_Measurement_Data_t pointers since */
                              /* that is what the API expects.          */
                              if((NotificationListArray = (CGMS_Measurement_Data_t **)BTPS_AllocateMemory(sizeof(CGMS_Measurement_Data_t *)*NotificationList.NumberOfMeasurements)) != NULL)
                              {
                                 /* Allocate memory for the pointers of */
                                 /* the measurements.                   */
                                 Index = NotificationList.Head;
                                 for(ArrayIndex = 0; ArrayIndex < NotificationList.NumberOfMeasurements; ArrayIndex++, Index = Index->Next)
                                 {
                                    /* Check to make sure that the index*/
                                    /* into the notification list is not*/
                                    /* NULL.                            */
                                    if(Index != NULL)
                                    {
                                       /* Allocate memory for the       */
                                       /* measurements.                 */
                                       if((NewNotification = (CGMS_Measurement_Data_t *)BTPS_AllocateMemory(CGMS_MEASUREMENT_DATA_SIZE)) != NULL)
                                       {
                                          /* Copy the data.             */
                                          NewNotification->Flags                       = Index->Flags;
                                          NewNotification->Size                        = Index->Size;
                                          NewNotification->GlucoseConcentration        = Index->GlucoseConcentration;
                                          NewNotification->TimeOffset                  = Index->TimeOffset;
                                          NewNotification->SensorStatus                = Index->Status;
                                          NewNotification->SensorCalTemp               = Index->CalTemp;
                                          NewNotification->SensorWarning               = Index->Warning;
                                          NewNotification->TrendInformation            = Index->Trend;
                                          NewNotification->Quality                     = Index->Quality;

                                          /* Store the new notification.*/
                                          NotificationListArray[ArrayIndex]            = NewNotification;
                                       }
                                       else
                                       {
                                          printf("Could not allocate memory for a CGMS Measurement.\r\n");
                                          NotificationListArray[ArrayIndex] = NULL;
                                       }
                                    }
                                    else
                                    {
                                       printf("The end of the notification list has been reached.\r\n");
                                       NotificationListArray[ArrayIndex] = NULL;
                                    }
                                 }

                                 /* While there are notifications to be */
                                 /* sent..                              */
                                 while((ret_val = CGMS_Notify_CGMS_Measurements(BluetoothStackID, CGMSInstanceID, ConnectionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), NotificationList.NumberOfMeasurements-NumberOfMeasurementsSent, NotificationListArray+NumberOfMeasurementsSent)))
                                 {
                                    if(ret_val > 0)
                                    {
                                       /* Store the number of           */
                                       /* measurements that were sent.  */
                                       NumberOfMeasurementsSent += ret_val;

                                       /* Go through the sensor list and*/
                                       /* mark the measurements that    */
                                       /* were successfully notified.   */
                                       /* This way if notification fails*/
                                       /* for any reason we know which  */
                                       /* measurements need to be       */
                                       /* notified.                     */
                                       Index = FirstNotification;
                                       while((Index != NULL) && (ret_val--))
                                       {
                                          Index->Notified = TRUE;
                                          Index           = Index->Next;
                                       }

                                       /* Update the first notification */
                                       /* to the next notification that */
                                       /* we will send on the next API  */
                                       /* call, so that this measurement*/
                                       /* will be flagged as notified   */
                                       /* and all following measurements*/
                                       /* that were successfully sent.  */
                                       FirstNotification = Index;

                                       /* Check if we are done sending  */
                                       /* notifications.                */
                                       if(NumberOfMeasurementsSent >= NotificationList.NumberOfMeasurements)
                                       {
                                          if(NumberOfMeasurementsSent > NotificationList.NumberOfMeasurements)
                                             printf("More measurements notified: %u, than expected: %u.\r\n", NumberOfMeasurementsSent, NotificationList.NumberOfMeasurements);

                                          /* All notifications have been*/
                                          /* sent and we are finished.  */
                                          break;
                                       }
                                    }
                                    else
                                    {
                                       DisplayFunctionError("CGMS_Notify_CGMS_Measurements", ret_val);
                                       break;
                                    }
                                 }

                                 /* Free the notification array list    */
                                 /* memory.                             */
                                 for(ArrayIndex = 0; ArrayIndex < NotificationList.NumberOfMeasurements; ArrayIndex++)
                                 {
                                    BTPS_FreeMemory(NotificationListArray[ArrayIndex]);
                                    NotificationListArray[ArrayIndex] = NULL;
                                 }

                                 BTPS_FreeMemory(NotificationListArray);
                                 NotificationListArray = NULL;
                              }
                              else
                              {
                                 printf("Memory could not be allocated for the notification measurement list.\r\n");
                              }

                              /* Free the temporary list data.          */
                              DeleteMeasurementList(&NotificationList);
                           }
                           else
                              printf("There are no measurements to notify in the Notification List.\r\n");
                        }
                        else
                           printf("There are no sensor measurements.\r\n");
                     }
                     else
                        printf("CGMS Measurement CCCD is not configured for notifications.\r\n");
                  }
                  else
                     printf("Could not get the device information.\r\n");

                  DisplayPrompt();
               }
            }
         }
      }

      /* Restart the timer for the next measurement.                    */
      if((TimerID = BSC_StartTimer(BluetoothStackID, 60000, SensorTimerCallback, 1)) <= 0)
         printf("Major Error: Could not start the timer.\r\n");
   }
   else
   {
      printf("Session Complete.\r\n\r\n");

      DisplayPrompt();

      /* Reset the minutes elapsed for the session run time.            */
      SensorRunTimeMinutes = 0;

      /* Set the appropriate Status Bits.                               */
      SensorStatus |= CGMS_SENSOR_STATUS_SESSION_STOPPED;
      SensorStatus |= CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED;
   }
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
         case etInquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            printf("GAP Inquiry Entry Result: %s.\r\n", BoardStr);
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
   BoardStr_t                                    BoardStr;
   unsigned int                                  Index;
   DeviceInfo_t                                 *DeviceInfo;
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
            printf("%d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     printf("Advertising Type: %s.\r\n", "rtConnectableUndirected");
                     break;
                  case rtConnectableDirected:
                     printf("Advertising Type: %s.\r\n", "rtConnectableDirected");
                     break;
                  case rtScannableUndirected:
                     printf("Advertising Type: %s.\r\n", "rtScannableUndirected");
                     break;
                  case rtNonConnectableUndirected:
                     printf("Advertising Type: %s.\r\n", "rtNonConnectableUndirected");
                     break;
                  case rtScanResponse:
                     printf("Advertising Type: %s.\r\n", "rtScanResponse");
                     break;
               }

               /* Display the Address Type.                             */
               if(DeviceEntryPtr->Address_Type == latPublic)
               {
                  printf("Address Type: %s.\r\n","atPublic");
               }
               else
               {
                  printf("Address Type: %s.\r\n","atRandom");
               }

               /* Display the Device Address.                           */
               printf("Address: 0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);
               printf("RSSI: 0x%02X.\r\n", DeviceEntryPtr->RSSI);
               printf("Data Length: %d.\r\n", DeviceEntryPtr->Raw_Report_Length);

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            printf("etLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               printf("Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
               printf("Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
               printf("Address Type: %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic)?"Public":"Random");
               printf("BD_ADDR:      %s.\r\n", BoardStr);

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
                     GAPEncryptionMode = emEnabled;

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
                              printf("GAP_LE_Reestablish_Security returned %d.\r\n",Result);
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
               printf("Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               printf("Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               printf("BD_ADDR: %s.\r\n", BoardStr);

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* Clear the CCCDs stored for this device.            */
                  DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration = 0;
                  DeviceInfo->ServerInfo.RACP_Client_Configuration             = 0;
                  DeviceInfo->ServerInfo.SOCP_Client_Configuration             = 0;

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
               LocalDeviceIsMaster  = FALSE;

               /* Perform necessary cleanup on the client               */
               if(UI_Mode == UI_MODE_IS_CLIENT)
               {
                  /* Clear the known supported features of the sensor.  */
                  ClientFeatures = 0;
               }
               else
               {
                  /* Clear the number of minutes elapsed for the session*/
                  /* run time.                                          */
                  SensorRunTimeMinutes  = 0;

                  /* Flag that the session has ended.                   */
                  SensorStatus         |= CGMS_SENSOR_STATUS_SESSION_STOPPED;
                  SensorStatus         |= CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED;
               }
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

                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Determine if we have previously paired with  */
                        /* the device.  We will simply restart          */
                        /* encryption with the stored LTK.              */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           printf("      Stored LTK valid.\r\n");

                           /* Respond with the Re-Generated Long Term   */
                           /* Key.                                      */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = larLongTermKey;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = DeviceInfo->LTK;
                        }
                        else
                        {
                           printf("      Stored LTK missing.\r\n");

                           /* Since we failed to find the requested key */
                           /* we should respond with a negative         */
                           /* response.                                 */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larLongTermKey;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;
                        }
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
                     printf("latSecurityRequest:.\r\n");
                     printf("BD_ADDR: %s.\r\n", BoardStr);
                     printf("Bonding Type: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == lbtBonding)?"Bonding":"No Bonding"));
                     printf("MITM: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

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
                     printf("Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);
                     break;
                  case latPairingStatus:
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                     printf("Pairing Status: %s.\r\n", BoardStr);
                     printf("Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        printf("Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);

                        /* Set if encryption is enabled for current     */
                        /* connection                                   */
                        GAP_LE_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAPEncryptionMode);
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

   /* The following is a BMS Server Event Callback.  This function will */
   /* be called whenever an BMS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the BMS Event Data   */
   /* that occurred and the BMS Event Callback Parameter that was       */
   /* specified when this Callback was The caller is free to installed. */
   /* use the contents of the BMS Event Data ONLY in the context If the */
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
   /* possible (this argument holds anyway because another BMS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BMS Event Packets.  */
   /*            A Deadlock WILL occur because NO BMS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI BMS_EventCallback(unsigned int BluetoothStackID, BMS_Event_Data_t *BMS_Event_Data, unsigned long CallbackParameter)
{
   int                                  Result;
   unsigned int                         Index;
   BoardStr_t                           BoardStr;
   unsigned int                         InstanceID;
   unsigned int                         TransactionID;
   BMS_BM_Control_Point_Command_Data_t *CommandData;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (BMS_Event_Data))
   {
      switch(BMS_Event_Data->Event_Data_Type)
      {
         case etBMS_BM_Control_Point_Command:
            printf("\r\netBMS_BM_Control_Point_Command with size %u.\r\n", BMS_Event_Data->Event_Data_Size);

            CommandData = BMS_Event_Data->Event_Data.BMS_BM_Control_Point_Command_Data;

            /* Make sure the command data exists.                       */
            if (CommandData)
            {
               InstanceID    = CommandData->InstanceID;
               TransactionID = CommandData->TransactionID;
               BD_ADDRToStr(CommandData->RemoteDevice, BoardStr);

               printf("   Instance ID:\t%u.\r\n", InstanceID);
               printf("   Connection ID:\t%u.\r\n", CommandData->ConnectionID);
               printf("   Transaction ID:\t%u.\r\n", TransactionID);
               printf("   Connection Type:\t%s.\r\n", ((CommandData->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:\t%s.\r\n", BoardStr);

               /* Display the command type.                             */
               switch(CommandData->FormatData.CommandType)
               {
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_BREDR_LE:
                     printf("\r\nCommand: DELETE_BOND_REQUESTING_DEVICE_BREDR_LE\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_BREDR:
                     printf("\r\nCommand: DELETE_BOND_REQUESTING_DEVICE_BREDR\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_REQUESTING_DEVICE_LE:
                     printf("\r\nCommand: DELETE_BOND_REQUESTING_DEVICE_LE\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_BREDR_LE:
                     printf("\r\nCommand: DELETE_BOND_ALL_DEVICES_BREDR_LE\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_BREDR:
                     printf("\r\nCommand: DELETE_BOND_ALL_DEVICES_BREDR\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_ALL_DEVICES_LE:
                     printf("\r\nCommand: DELETE_BOND_ALL_DEVICES_LE\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_BREDR_LE:
                     printf("\r\nCommand: DELETE_BOND_OTHER_DEVICES_BREDR_LE\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_BREDR:
                     printf("\r\nCommand: DELETE_BOND_OTHER_DEVICES_BREDR\r\n");
                     break;
                  case BMS_BM_CONTROL_POINT_DELETE_BOND_OTHER_DEVICES_LE:
                     printf("\r\nCommand: DELETE_BOND_OTHER_DEVICES_LE\r\n");
                     break;
               }

               /* Check if an Authorization Code was given.             */
               if (CommandData->FormatData.AuthorizationCodeLength)
               {
                  printf("\r\nAuthorization Code Length:\t%d\r\n", CommandData->FormatData.AuthorizationCodeLength);

                  Index  = 0;

                  /* Display the Authorization Code.                    */
                  printf("Authorization Code:\t\t");
                  while(Index < CommandData->FormatData.AuthorizationCodeLength)
                  {
                     printf("0x%02X ", CommandData->FormatData.AuthorizationCode[Index++]);
                     Index++;
                  }

                  printf("\r\n");
               }

               /* Execute the command if the valid Authorization Code   */
               /* was provided (or it was not required).                */
               if((CommandData->FormatData.AuthorizationCodeLength == 0) || (VerifyBMSCommand(&CommandData->FormatData)))
               {
                  Result = ExecuteBMSCommand(CommandData->FormatData.CommandType);

                  if(!Result)
                     Result = BMS_BM_Control_Point_Response(BluetoothStackID, TransactionID, BMS_ERROR_CODE_SUCCESS);
                  else
                     Result = BMS_BM_Control_Point_Response(BluetoothStackID, TransactionID, BMS_ERROR_CODE_OPERATION_FAILED);

               }
               else
                  Result = BMS_BM_Control_Point_Response(BluetoothStackID, TransactionID, ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION);
            }

            /* Display the nBMS_BM_Control_Point_Response result.       */
            if(!(Result))
               printf("\r\nBMS_BM_Control_Point_Response Success.\r\n");
            else
               printf("\r\nBMS_BM_Control_Point_Response Failure: %d.\r\n", Result);

            break;
         default:
            printf("\r\nUnknown BMS Event\r\n");
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

   /* The following is a CGMS Server Event Callback.  This function will*/
   /* be called whenever an CGMS Server Profile Event occurs that is    */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the CGMS Event Data  */
   /* that occurred and the CGMS Event Callback Parameter that was      */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the CGMS Event Data ONLY in the context of    */
   /* this callback.  If the caller requires the Data for a longer      */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another CGMS Event   */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving CGMS Event Packets. */
   /*            A Deadlock WILL occur because NO CGMS Event Callbacks  */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI CGMS_EventCallback(unsigned int BluetoothStackID, CGMS_Event_Data_t *CGMS_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t                  *DeviceInfo                = NULL;
   BoardStr_t                     BoardStr;
   int                            Result;
   unsigned int                   InstanceID;
   unsigned int                   TransactionID;
   int                            RACPResult                = 0;
   int                            SOCPResult                = 0;
   Byte_t                         CommandType;
   Byte_t                         OperatorType;
   RACP_Procedure_Data_t          RACPData;
   unsigned int                   NumberOfMeasurementsFound = 0;
   Byte_t                         CommunicationInterval;
   CGMS_Calibration_Data_Record_t CalibrationDataRecord;
   Word_t                         AlertLevel                = 0;
   struct tm                      TempTime;
   char                           Time[80];
   CGMS_Feature_Data_t            CGMSFeature;
   CGMS_Status_Data_t             CGMSStatus;
   CGMS_Session_Run_Time_Data_t   CGMSRunTime;
   CGMS_SOCP_Response_Type_t      ResponseOpCode            = 0;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (CGMS_Event_Data))
   {
      switch(CGMS_Event_Data->Event_Data_Type)
      {
         case etCGMS_Server_Read_Client_Configuration_Request:
            printf("etCGMS_Server_Read_Client_Configuration_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->TransactionID;

               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:    %s.\r\n", BoardStr);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->RemoteDevice)) != NULL)
               {
                  /* Validate event parameters                          */
                  if(CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->InstanceID == CGMSInstanceID)
                  {
                     Result = 0;

                     switch(CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->ClientConfigurationType)
                     {
                         case ctCGMSMeasurement:
                            printf("   Config Type:      ctCGMSMeasurement.\r\n");
                            Result = CGMS_Read_Client_Configuration_Response(BluetoothStackID, CGMSInstanceID, CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration);
                            break;
                         case ctCGMSRecordAccessControlPoint:
                            printf("   Config Type:      ctCGMSRecordAccessControlPoint.\r\n");
                            Result = CGMS_Read_Client_Configuration_Response(BluetoothStackID, CGMSInstanceID, CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->ServerInfo.RACP_Client_Configuration);
                            break;
                        case ctCGMSSpecificOpsControlPoint:
                            printf("   Config Type:      ctCGMSSpecificOpsControlPoint.\r\n");
                            Result = CGMS_Read_Client_Configuration_Response(BluetoothStackID, CGMSInstanceID, CGMS_Event_Data->Event_Data.CGMS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->ServerInfo.SOCP_Client_Configuration);
                            break;
                         default:
                            printf("   Config Type:      Unknown.\r\n");
                            break;
                     }

                     if(Result)
                        DisplayFunctionError("CGMS_Read_Client_Configuration_Response", Result);
                  }
                  else
                  {
                     printf("\r\nInvalid Event data.\r\n");
                  }
               }
               else
               {
                  printf("\r\nUnknown Client.\r\n");
               }
            }
            break;
         case etCGMS_Server_Client_Configuration_Update:
            printf("etCGMS_Server_Client_Configuration_Update with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->InstanceID;

               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:    %s.\r\n", BoardStr);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->RemoteDevice)) != NULL)
               {
                  /* Validate event parameters                          */
                  if(CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->InstanceID == CGMSInstanceID)
                  {
                     switch(CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ClientConfigurationType)
                     {
                        case ctCGMSMeasurement:
                           printf("   Config Type:      ctCGMSMeasurement.\r\n");
                           DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration = CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ClientConfiguration;
                           break;
                        case ctCGMSRecordAccessControlPoint:
                           printf("   Config Type:      ctCGMSRecordAccessControlPoint.\r\n");
                           DeviceInfo->ServerInfo.RACP_Client_Configuration = CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ClientConfiguration;
                           break;
                        case ctCGMSSpecificOpsControlPoint:
                           printf("   Config Type:      ctCGMSSpecificOpsControlPoint.\r\n");
                           DeviceInfo->ServerInfo.SOCP_Client_Configuration = CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ClientConfiguration;
                           break;
                        default:
                           printf("   Config Type:      Unknown.\r\n");
                           break;
                     }

                     printf("   Value:            0x%04X.\r\n", (unsigned int)CGMS_Event_Data->Event_Data.CGMS_Client_Configuration_Update_Data->ClientConfiguration);
                  }
                  else
                  {
                     printf("\r\nInvalid Event data.\r\n");
                  }
               }
               else
               {
                  printf("\r\nUnknown Client.\r\n");
               }
            }
            break;
         case etCGMS_Server_Read_Feature_Request:
            printf("etCGMS_Server_Read_Feature_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Read_Feature_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* Format the feature data.                              */
               CGMSFeature.Features           = SensorSupportedFeatures;
               CGMSFeature.TypeSampleLocation = SensorTypeSampleLocation;

               /* Send the feature read request response.               */
               if((Result = CGMS_Feature_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), CGMS_ERROR_CODE_SUCCESS, &CGMSFeature)) != 0)
               {
                  DisplayFunctionError("CGMS_Feature_Read_Request_Response", Result);

                  /* Send the error response.                           */
                  if((Result = CGMS_Feature_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CGMSFeature)) != 0)
                     DisplayFunctionError("CGMS_Feature_Read_Request_Response", Result);
               }
            }
            break;
         case etCGMS_Server_Read_Status_Request:
            printf("etCGMS_Server_Read_Status_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Read_Status_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* Format the status.                                    */
               CGMSStatus.TimeOffset = TimeOffset;
               CGMSStatus.Status     = SensorStatus;

               /* Send the response with the status.                    */
               if((Result = CGMS_Status_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), CGMS_ERROR_CODE_SUCCESS, &CGMSStatus)) != 0)
                  DisplayFunctionError("CGMS_Status_Read_Request_Response", Result);
            }
            break;
         case etCGMS_Server_Read_Session_Start_Time_Request:
            printf("etCGMS_Server_Read_Session_Start_Time_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* Send the response with the session start time.        */
               if((Result = CGMS_Session_Start_Time_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), CGMS_ERROR_CODE_SUCCESS, &SensorSessionStartTime)) != 0)
                  DisplayFunctionError("CGMS_Session_Start_Time_Read_Request_Response", Result);
            }
            break;
         case etCGMS_Server_Write_Session_Start_Time_Request:
            printf("etCGMS_Server_Write_Session_Start_Time_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* If the sensor is expecting a CRC make sure that we    */
               /* received one by checking the event data.  Note since  */
               /* CRC calculations are checked internally, whether a CRC*/
               /* is present will be flagged and set up to the          */
               /* application so that the application can determine     */
               /* whether it is missing and return the proper GATT Error*/
               /* Response.                                             */
               if((!(SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED)) || ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) && (CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->Flags & CGMS_E2E_CRC_PRESENT)))
               {
                  /* Check if there is a session running.               */
                  if(!(SensorStatus & CGMS_SENSOR_STATUS_SESSION_STOPPED))
                  {
                     TempTime.tm_year = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Year-1900;
                     TempTime.tm_mon  = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Month-1;
                     TempTime.tm_mday = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Day;
                     TempTime.tm_hour = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Hours;
                     TempTime.tm_min  = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Minutes;
                     TempTime.tm_sec  = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime.Time.Seconds;
                     strftime(Time, 80, "%Y/%m/%d %T", &TempTime);
                     printf("\r\nClient User Facing Time:\r\n");
                     printf("   Time:       %s\r\n\r\n",Time);

                     /* First store the user facing time of the client. */
                     SensorSessionStartTime = CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->SessionStartTime;

                     /* Calculate and print the new session start time. */
                     CalculateSessionStartTime();

                     TempTime.tm_year = SensorSessionStartTime.Time.Year-1900;
                     TempTime.tm_mon  = SensorSessionStartTime.Time.Month-1;
                     TempTime.tm_mday = SensorSessionStartTime.Time.Day;
                     TempTime.tm_hour = SensorSessionStartTime.Time.Hours;
                     TempTime.tm_min  = SensorSessionStartTime.Time.Minutes;
                     TempTime.tm_sec  = SensorSessionStartTime.Time.Seconds;
                     strftime(Time, 80, "%Y/%m/%d %T", &TempTime);
                     printf("Calculated CGMS Session Start Time:\r\n");
                     printf("   Time:       %s\r\n",Time);
                     printf("   Timezone:   %d\r\n", SensorSessionStartTime.TimeZone);
                     printf("   DST Offset: %u\r\n", SensorSessionStartTime.DSTOffset);

                     /* Flag that the session start time has been       */
                     /* written.                                        */
                     SensorStatus &= ~CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED;
                  }
                  else
                     printf("\r\nCannot write the session start time. Session is stopped.\r\n");

                  /* Send the CGMS Session Start Time Write Request     */
                  /* Response.                                          */
                  if((Result = CGMS_Session_Start_Time_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_SUCCESS)) != 0)
                     DisplayFunctionError("CGMS_Session_Start_Time_Write_Request_Response", Result);
               }
               else
               {
                  /* Send the GATT Error Response.                      */
                  if((Result = CGMS_Session_Start_Time_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_MISSING_CRC)) != 0)
                     DisplayFunctionError("CGMS_Session_Start_Time_Write_Request_Response", Result);
               }
            }
            break;
         case etCGMS_Server_Read_Session_Run_Time_Request:
            printf("etCGMS_Server_Read_Session_Run_Time_Request with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Read_Session_Start_Time_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Read_Session_Run_Time_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Read_Session_Run_Time_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Read_Session_Run_Time_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Read_Session_Run_Time_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Read_Session_Run_Time_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* Store the Run Time to the proper format.              */
               CGMSRunTime.SessionRunTime = SensorSessionRunTime;

               /* Send the response with the session run time.          */
               if((Result = CGMS_Session_Run_Time_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED), CGMS_ERROR_CODE_SUCCESS, &CGMSRunTime)) != 0)
                  DisplayFunctionError("CGMS_Session_Run_Time_Read_Request_Response", Result);
            }
            break;
         case etCGMS_Server_Record_Access_Control_Point_Command:
            printf("etCGMS_Server_Record_Access_Control_Point_Command with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* Getting DeviceInfo to verify RACP is configured       */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Verify that RACP CCC is configured for indications.*/
                  if(DeviceInfo->ServerInfo.RACP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                  {
                     /* Verify that no outstanding RACP commands are    */
                     /* outstanding or there is an outstanding RACP     */
                     /* command and the new RACP command is the abort   */
                     /* procedure.                                      */
                     if((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_OUTSTANDING)) || ((DeviceInfo->Flags & DEVICE_INFO_FLAGS_RACP_OUTSTANDING) && (CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.CommandType == racCGMSAbortOperationRequest)))
                     {
                        /* Flag that an RACP command is outstanding.    */
                        DeviceInfo->Flags    |= DEVICE_INFO_FLAGS_RACP_OUTSTANDING;

                        /* Let's send the GATT Response.                */
                        Result = CGMS_Record_Access_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_SUCCESS);
                        if(Result != 0)
                           DisplayFunctionError("CGMS_Record_Access_Control_Point_Response", Result);

                        BTPS_MemInitialize(&RACPData, 0, sizeof(RACP_Procedure_Data_t));

                        /* Go ahead and format the RACP Data for the    */
                        /* RACP Procedures.  Note some data will be     */
                        /* ignored depending on the procedure.          */
                        RACPData.OperatorType = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.OperatorType;

                        CommandType = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.CommandType;
                        switch(CommandType)
                        {
                           case racCGMSReportStoredRecordsRequest:
                              printf("OpCode:           racCGMSReportStoredRecordsRequest.\r\n");
                              break;
                           case racCGMSDeleteStoredRecordsRequest:
                              printf("OpCode:           racCGMSDeleteStoredRecordsRequest.\r\n");
                              break;
                           case racCGMSAbortOperationRequest:
                              printf("OpCode:           racCGMSAbortOperationRequest.\r\n");
                              break;
                           case racCGMSNumberOfStoredRecordsRequest:
                              printf("OpCode:           racCGMSNumberOfStoredRecordsRequest.\r\n");
                              break;
                           default:
                              if((CommandType == CGMS_RECORD_ACCESS_OPCODE_RFU_0) || (CommandType >= CGMS_RECORD_ACCESS_OPCODE_RFU_7 && CommandType <= CGMS_RECORD_ACCESS_OPCODE_RFU_255))
                              {
                                 printf("OpCode:           Reserved.\r\n");
                                 RACPResult = RACP_OP_CODE_NOT_SUPPORTED;
                              }
                              else
                              {
                                 printf("OpCode:           Unknown.\r\n");
                                 RACPResult = RACP_OP_CODE_NOT_SUPPORTED;
                              }
                              break;
                        }

                        OperatorType = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.OperatorType;

                        /* Initialize the parameter data.               */
                        switch(OperatorType)
                        {
                           case raoCGMSNull:
                              printf("Operator:         raoCGMSNull.\r\n");
                              break;
                           case raoCGMSAllRecords:
                              printf("Operator:         raoCGMSAllRecords.\r\n");
                              break;
                           case raoCGMSLessThanOrEqualTo:
                           case raoCGMSGreaterThanOrEqualTo:
                              /* Determine the operator type.           */
                              if((OperatorType == raoCGMSLessThanOrEqualTo))
                                 printf("Operator:         raoCGMSLessThanOrEqualTo.\r\n");
                              else
                                 printf("Operator:         raoCGMSGreaterThanOrEqualTo.\r\n");

                              /* Check the filter type.                 */
                              if(CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterType == rafTimeOffset)
                                 printf("Filter Type:      rafTimeOffeset.\r\n");
                              else
                              {
                                 printf("Filter Type:      0x%02X.\r\n   Invalid.\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterType);
                                 RACPResult = RACP_OPERAND_NOT_SUPPORTED;
                              }

                              printf("Timeoffset:       %u\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffset);
                              RACPData.ParameterData.TimeOffset = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffset;
                              break;
                           case raoCGMSWithinRangeOf:
                              printf("Operator:         raoCGMSWithinRangeOf.\r\n");

                              /* Check the filter type.                 */
                              if(CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterType == rafTimeOffset)
                              {
                                 printf("Filter Type:      rafTimeOffeset.\r\n");

                                 /* Let's check the validity of the     */
                                 /* range.                              */
                                 if(CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Minimum <= CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Maximum)
                                 {
                                    printf("Minimum Timeoffset:   %u\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Minimum);
                                    printf("Maximum Timeoffset:   %u\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Maximum);

                                    RACPData.ParameterData.Range.Minimum = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Minimum;
                                    RACPData.ParameterData.Range.Maximum = CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Maximum;
                                 }
                                 else
                                 {
                                    printf("TimeOffset Range:     Invalid.\r\n");
                                    printf("Minimum Timeoffset:   %u\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Minimum);
                                    printf("Maximum Timeoffset:   %u\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterParameters.TimeOffsetRange.Maximum);
                                    RACPResult = RACP_INVALID_OPERAND;
                                 }
                              }
                              else
                              {
                                 printf("Filter Type:      0x%02X.\r\n   Invalid.\r\n", CGMS_Event_Data->Event_Data.CGMS_Record_Access_Control_Point_Command_Data->FormatData.FilterType);
                                 RACPResult = RACP_OPERAND_NOT_SUPPORTED;
                              }
                              break;
                           case raoCGMSFirstRecord:
                              printf("Operator:         raoCGMSFirstRecord.\r\n");
                              break;
                           case raoCGMSLastRecord:
                              printf("Operator:         raoCGMSLastRecord.\r\n");
                              break;
                           default:
                              printf("Operator:         Reserved.\r\n");
                              break;
                        }

                        /* If no errors have occured thus far, let's    */
                        /* call the proper function to perform the RACP */
                        /* procedure.                                   */
                        if(RACPResult == 0)
                        {
                           switch(CommandType)
                           {
                              case racCGMSReportStoredRecordsRequest:
                                 RACPResult = ReportStoredRecordsProcedure(RACPData);
                                 break;
                              case racCGMSDeleteStoredRecordsRequest:
                                 RACPResult = DeleteStoredRecordsProcedure(RACPData);
                                 break;
                              case racCGMSAbortOperationRequest:
                                 RACPResult = AbortProcedure();
                                 break;
                              case racCGMSNumberOfStoredRecordsRequest:
                                 RACPResult = ReportNumberOfStoredRecordsProcedure(RACPData, &NumberOfMeasurementsFound);
                                 break;
                              default:
                                 RACPResult = RACP_OP_CODE_NOT_SUPPORTED;
                                 break;
                           }
                        }

                        /* If the procedure returns success we need to  */
                        /* set the result to the correct result value.  */
                        if(RACPResult == 0)
                           RACPResult = CGMS_RACP_RESPONSE_CODE_SUCCESS;

                        /* Attempt to send the indication.              */
                        if((RACPResult == CGMS_RACP_RESPONSE_CODE_SUCCESS) && (CommandType == racCGMSNumberOfStoredRecordsRequest))
                        {
                           /* We need to indicate the number of records.*/
                           Result = IndicateNumberOfStoredRecords(NumberOfMeasurementsFound);
                           if(Result)
                              DisplayFunctionError("IndicateNumberOfStoredRecords", Result);
                        }
                        else
                        {
                           /* Otherwise indicate the general RACP       */
                           /* Result.                                   */
                           Result = IndicateRACPResult(CommandType, abs(RACPResult));
                           if(Result)
                              DisplayFunctionError("IndicateRACPResult", Result);
                        }
                     }
                     else
                     {
                        /* Indicate there is a procedure already in     */
                        /* progess.                                     */
                        Result = CGMS_Record_Access_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS);
                        if(Result)
                           DisplayFunctionError("CGMS_Record_Access_Control_Point_Response", Result);
                     }
                  }
                  else
                  {
                     /* Indicate that the CCCD is not configured.       */
                     Result = CGMS_Record_Access_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED);
                        if(Result)
                           DisplayFunctionError("CGMS_Record_Access_Control_Point_Response", Result);
                  }
               }
               else
               {
                  /* Indicate GATT Error Code Unlikely since we couldn't*/
                  /* get the device information.                        */
                  Result = CGMS_Record_Access_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                  if(Result)
                     DisplayFunctionError("CGMS_Record_Access_Control_Point_Response", Result);
               }
            }
            break;
         case etCGMS_Server_Specific_Ops_Control_Point_Command:
            printf("etCGMS_Server_Specific_Ops_Control_Point_Command with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->RemoteDevice, BoardStr);
               InstanceID    = CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->InstanceID;
               TransactionID = CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->TransactionID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Transaction ID:   %u.\r\n", TransactionID);
               printf("Remote Device:    %s.\r\n", BoardStr);

               /* If the sensor is expecting a CRC make sure that we    */
               /* received one by checking the event data.  Note since  */
               /* CRC calculations are checked internally, whether a CRC*/
               /* is present will be flagged and set up to the          */
               /* application so that the application can determine     */
               /* whether it is missing and return the proper GATT Error*/
               /* Response.                                             */
               if((!(SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED)) || ((SensorSupportedFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) && (CGMS_Event_Data->Event_Data.CGMS_Write_Session_Start_Time_Data->Flags & CGMS_E2E_CRC_PRESENT)))
               {
                  /* Getting DeviceInfo to verify SOCP is configured    */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                  {
                     /* Verify that SOCP CCC is configured for          */
                     /* indications.                                    */
                     if(DeviceInfo->ServerInfo.SOCP_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)
                     {
                        /* Verify that no outstanding SOCP commands are */
                        /* outstanding.                                 */
                        if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SOCP_OUTSTANDING))
                        {
                           /* Flag that an SOCP command is outstanding. */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SOCP_OUTSTANDING;

                           /* Send the GATT Response.                   */
                           Result = CGMS_Specific_Ops_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_SUCCESS);
                           if(Result != 0)
                              DisplayFunctionError("CGMS_Specific_Ops_Control_Point_Response", Result);

                           /* Let's initialize the calibration data     */
                           /* record structure.  QNXCGMP.mak will warn  */
                           /* that it may be used uninitialized.  There */
                           /* is no warning for LinuxCGMP.mak.          */
                           BTPS_MemInitialize(&CalibrationDataRecord, 0, CGMS_CALIBRATION_DATA_RECORD_SIZE);

                           CommandType = CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandType;
                           switch(CommandType)
                           {
                              case cgcSetCGMCommunicationInterval:
                                 printf("OpCode:           cgcSetCGMSCommunicationInterval.\r\n");
                                 printf("Communication Interval: %u (Minutes)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CommunicationIntervalMinutes);
                                 SOCPResult     = SetCommunicationInterval(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CommunicationIntervalMinutes);
                                 break;
                              case cgcGetCGMCommunicationInterval:
                                 printf("OpCode:           cgcGetCGMSCommunicationInterval.\r\n");
                                 SOCPResult     = GetCommunicationInterval(&CommunicationInterval);
                                 break;
                              case cgcSetGlucoseCalibrationValue:
                                 printf("OpCode:           cgcSetGlucoseCalibrationValue.\r\n\r\n");
                                 /* Note the last two fields of the     */
                                 /* Calibration Record will be ignored  */
                                 /* by the Server since they are set by */
                                 /* the Server.                         */
                                 printf("Calibration Data Record:\r\n");
                                 printf("   Glucose Concentration:   %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord.CalibrationGlucoseConcentration);
                                 printf("   Time:                    %u (Min)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord.CalibrationTime);
                                 printf("   Next Calibration Time:   %u (Min)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord.NextCalibrationTime);
                                 printf("   Type-Sample Location:\r\n");
                                 printf("      Type:                 %s.\r\n", CGMSFeatureTypeBitStrings[(CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord.CalibrationTypeSampleLocation & 0x0F) - 1]);
                                 printf("      Sample Location:      %s.\r\n", CGMSFeatureSampleLocationBitStrings[(CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord.CalibrationTypeSampleLocation >> 4) - 1]);
                                 SOCPResult     = SetGlucoseCalibrationValue(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecord);
                                 break;
                              case cgcGetGlucoseCalibrationValue:
                                 printf("OpCode:           cgcGetGlucoseCalibrationValue.\r\n");
                                 SOCPResult     = GetGlucoseCalibrationValue(CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.CalibrationDataRecordNumber, &CalibrationDataRecord);
                                 break;
                              case cgcSetPatientHighAlertLevel:
                                 printf("OpCode:           cgcSetPatientHighAlertLevel.\r\n");
                                 printf("Patient High Alert Level:   %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetPatientHighAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetPatientHighAlertLevel:
                                 printf("OpCode:           cgcGetPatientHighAlertLevel.\r\n");
                                 ResponseOpCode = cgrPatientHighAlertLevelResponse;
                                 SOCPResult     = GetPatientHighAlertLevel(&AlertLevel);
                                 break;
                              case cgcSetPatientLowAlertLevel:
                                 printf("OpCode:           cgcSetPatientLowAlertLevel.\r\n");
                                 printf("Patient Low Alert Level:    %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetPatientLowAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetPatientLowAlertLevel:
                                 printf("OpCode:           cgcGetPatientLowAlertLevel.\r\n");
                                 ResponseOpCode = cgrPatientLowAlertLevelResponse;
                                 SOCPResult     = GetPatientLowAlertLevel(&AlertLevel);
                                 break;
                              case cgcSetHypoAlertLevel:
                                 printf("OpCode:           cgcSetHypoAlertLevel.\r\n");
                                 printf("Hypo Alert Level:           %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetHypoAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetHypoAlertLevel:
                                 printf("OpCode:           cgcGetHypoAlertLevel.\r\n");
                                 ResponseOpCode = cgrHypoAlertLevelResponse;
                                 SOCPResult     = GetHypoAlertLevel(&AlertLevel);
                                 break;
                              case cgcSetHyperAlertLevel:
                                 printf("OpCode:           cgcSetHyperAlertLevel.\r\n");
                                 printf("Hyper Alert Level:          %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetHyperAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetHyperAlertLevel:
                                 printf("OpCode:           cgcGetHyperAlertLevel.\r\n");
                                 ResponseOpCode = cgrHyperAlertLevelResponse;
                                 SOCPResult     = GetHyperAlertLevel(&AlertLevel);
                                 break;
                              case cgcSetRateOfDecreaseAlertLevel:
                                 printf("OpCode:           cgcSetRateOfDecreaseAlertLevel.\r\n");
                                 printf("Rate of Decrease Alert Level: %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetRateOfDecreaseAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetRateOfDecreaseAlertLevel:
                                 printf("OpCode:           cgcGetRateOfDecreaseAlertLevel.\r\n");
                                 ResponseOpCode = cgrRateOfDecreaseAlertLevelResponse;
                                 SOCPResult     = GetRateOfDecreaseAlertLevel(&AlertLevel);
                                 break;
                              case cgcSetRateOfIncreaseAlertLevel:
                                 printf("OpCode:           cgcSetRateOfIncreaseAlertLevel.\r\n");
                                 printf("Rate of Increase Alert Level: %u (mg/dL)\r\n", CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 SOCPResult     = SetRateOfIncreaseAlertLevel(&CGMS_Event_Data->Event_Data.CGMS_Specific_Ops_Control_Point_Command_Data->FormatData.CommandParameters.AlertLevel);
                                 break;
                              case cgcGetRateOfIncreaseAlertLevel:
                                 printf("OpCode:           cgcGetRateOfIncreaseAlertLevel.\r\n");
                                 ResponseOpCode = cgrRateOfIncreaseAlertLevelResponse;
                                 SOCPResult     = GetRateOfIncreaseAlertLevel(&AlertLevel);
                                 break;
                              case cgcResetDeviceSpecificAlert:
                                 printf("OpCode:           cgcResetDeviceSpecificAlert.\r\n");
                                 SOCPResult     = ResetDeviceSpecificAlert();
                                 break;
                              case cgcStartSession:
                                 printf("OpCode:           cgcStartSession.\r\n");
                                 SOCPResult     = StartSession();
                                 break;
                              case cgcStopSession:
                                 printf("OpCode:           cgcStopSession.\r\n");
                                 SOCPResult     = StopSession();
                                 break;
                              default:
                                 printf("OpCode:           Unknown.\r\n");
                                 SOCPResult     = SOCP_OP_CODE_NOT_SUPPORTED;
                           }

                           /* If the procedure returns success we need  */
                           /* to set the result to the correct result   */
                           /* value.                                    */
                           if(SOCPResult == 0)
                              SOCPResult = CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_SUCCESS;

                           /* Determine if the SOCP Result is success.  */
                           if(SOCPResult == CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_SUCCESS)
                           {
                              /* Communication Interval indication.     */
                              if(CommandType == cgcGetCGMCommunicationInterval)
                              {
                                 Result = IndicateSOCPCommunicationInterval();
                                 if(Result)
                                    DisplayFunctionError("IndicateSOCPCommunicationInterval", Result);
                              }
                              else
                              {  /* Calibration Value indication.          */
                                 if(CommandType == cgcGetGlucoseCalibrationValue)
                                 {
                                    Result = IndicateSOCPCalibrationData(CalibrationDataRecord);
                                    if(Result)
                                       DisplayFunctionError("IndicateSOCPCalibrationData", Result);
                                 }
                                 else
                                 {
                                    /* Alert Level indication.          */
                                    if((CommandType == cgcGetPatientHighAlertLevel) ||
                                       (CommandType == cgcGetPatientLowAlertLevel) ||
                                       (CommandType == cgcGetHyperAlertLevel) ||
                                       (CommandType == cgcGetHypoAlertLevel) ||
                                       (CommandType == cgcGetRateOfIncreaseAlertLevel) ||
                                       (CommandType == cgcGetRateOfDecreaseAlertLevel))
                                    {
                                       Result = IndicateSOCPAlertLevel(ResponseOpCode, AlertLevel);
                                       if(Result)
                                          DisplayFunctionError("IndicateSOCPAlertLevel", Result);
                                    }
                                    else
                                    {
                                       /* Otherwise indicate SOCP       */
                                       /* Result.                       */
                                       Result = IndicateSOCPResult(CommandType, abs(SOCPResult));
                                       if(Result)
                                          DisplayFunctionError("IndicateSOCPResult", Result);
                                    }
                                 }
                              }
                           }
                           else
                           {
                              /* Indicate the CGCMP error.              */
                              Result = IndicateSOCPResult(CommandType, abs(SOCPResult));
                              if(Result)
                                 DisplayFunctionError("IndicateSOCPResult", Result);
                           }
                        }
                        else
                        {
                           /* Send the Procedure Already in Progress    */
                           /* error.                                    */
                           Result = CGMS_Specific_Ops_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS);
                           if(Result)
                              DisplayFunctionError("CGMS_Specific_Ops_Control_Point_Response", Result);
                        }

                     }
                     else
                     {
                        /* Send the CCCD error.                         */
                        Result = CGMS_Specific_Ops_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED);
                        if(Result)
                           DisplayFunctionError("CGMS_Specific_Ops_Control_Point_Response", Result);
                     }
                  }
                  else
                  {
                     /* Send the general error response.                */
                     Result = CGMS_Specific_Ops_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     if(Result)
                        DisplayFunctionError("CGMS_Specific_Ops_Control_Point_Response", Result);
                  }
               }
               else
               {
                  /* Send the GATT Error Response for the missing CRC.  */
                  Result = CGMS_Specific_Ops_Control_Point_Response(BluetoothStackID, InstanceID, TransactionID, CGMS_ERROR_CODE_MISSING_CRC);
                  if(Result)
                     DisplayFunctionError("CGMS_Specific_Ops_Control_Point_Response", Result);
               }
            }
            break;
         case etCGMS_Server_Confirmation_Data:
            printf("etCGMS_Server_Confirmation_Data with size %u.\r\n", CGMS_Event_Data->Event_Data_Size);

            if(CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data)
            {
               BD_ADDRToStr(CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data->RemoteDevice, BoardStr);
               InstanceID = CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data->InstanceID;

               printf("Instance ID:      %u.\r\n", InstanceID);
               printf("Connection ID:    %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Remote Device:    %s.\r\n", BoardStr);
               printf("Status:           %u.\r\n", CGMS_Event_Data->Event_Data.CGMS_Confirmation_Data->Status);
            }
            break;
         default:
            printf("Unknown CGMS Event\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nCGMS Callback Data: Event_Data = NULL.\r\n");
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
static void BTPSAPI GATT_ClientEventCallback_BMS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   Word_t                i;
   Word_t                ValueLength;
   DWord_t               SupportedFeatures;
   BoardStr_t            BoardStr;
   DeviceInfo_t         *DeviceInfo;
   Word_t                MTU;
   Byte_t               *AttributeValue;
   unsigned int          ret_val;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Prepare_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data)
            {
               printf("etGATT_Client_Prepare_Write_Response\r\n");

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what error response     */
               /* this is.                                              */
               if(BMSCommandBufferLength)
               {
                  DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->RemoteDevice);
                  if((DeviceInfo) && (CallbackParameter != 0) && (IsBMSControlPointHandle((Word_t)CallbackParameter,DeviceInfo)))
                  {
                     /* Get the MTU to find the maximum length value    */
                     /* that can be written.                            */
                     GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU);

                     /* Send the appropriate length write.              */
                     if(BMSCommandBufferLength > (unsigned int)(MTU - 5))
                        ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, (Word_t)CallbackParameter, (MTU - 5), BMSCommandValueOffset, BMSCommandBuffer, GATT_ClientEventCallback_BMS, CallbackParameter);
                     else
                        ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, (Word_t)CallbackParameter, BMSCommandBufferLength, BMSCommandValueOffset, BMSCommandBuffer, GATT_ClientEventCallback_BMS, CallbackParameter);

                     /* Check the return value.                         */
                     if(ret_val > 0)
                     {
                        /* Increment the variables.                     */
                        BMSCommandBuffer += (MTU - 5);

                        if(BMSCommandBufferLength > (unsigned int)(MTU - 5))
                        {
                           BMSCommandValueOffset  += (MTU - 5);
                           BMSCommandBufferLength -= (MTU - 5);
                        }
                        else
                        {
                           BMSCommandValueOffset  = 0;
                           BMSCommandBufferLength = 0;
                        }

                        BMSCommandTransactionID  = ret_val;
                        printf("\r\nBMS Command Sent Successfully, TransactionID: 0x%02X.\r\n", ret_val);
                     }
                     else
                     {
                        printf("GATT_Prepare_Write_Request error %d\r\n", ret_val);
                     }
                  }
               }
               else
               {
                  /* If no more Prepare Writes are necessary, then send */
                  /* an Execute Write Request.                          */
                  ret_val = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, FALSE, GATT_ClientEventCallback_BMS, CallbackParameter);

                  if(ret_val > 0)
                  {
                     printf("\r\nBMS Command Execute Sent Successfully, TransactionID: 0x%02X.\r\n", ret_val);
                  }
                  else
                  {
                    printf("GATT_Execute_Write_Request error %d\r\n", ret_val);
                  }
               }
            }
            break;
         case etGATT_Client_Execute_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data)
            {
               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what write response     */
               /* this is.                                              */
               if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter != 0))
               {
                  if(IsBMSControlPointHandle((Word_t)CallbackParameter,DeviceInfo))
                  {
                     printf("BMS Command Response: Success.\r\n");
                     BMSCommandValueOffset  = 0;
                     BMSCommandBufferLength = 0;
                  }
               }
               else
               {
                  printf("\r\nExecute Write Response.\r\n");
                  BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->RemoteDevice, BoardStr);
                  printf("   Connection ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionID);
                  printf("   Transaction ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->TransactionID);
                  printf("   Connection Type:\t%s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
                  printf("   BD_ADDR:\t%s.\r\n", BoardStr);
               }
            }
            else
               printf("\r\nError - Null Execute Write Response Data.");
            break;
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what error response     */
               /* this is.                                              */
               DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice);
               if((DeviceInfo) && (CallbackParameter != 0) && (IsBMSControlPointHandle((Word_t)CallbackParameter,DeviceInfo)))
               {
                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case BMS_ERROR_CODE_OPCODE_NOT_SUPPORTED:
                        printf("BMS Command Failure: Feature Not Supported.\r\n");
                        break;
                     case BMS_ERROR_CODE_OPERATION_FAILED:
                        printf("BMS Command Failure: Operation Failed.\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION:
                        printf("BMS Command Failure: Insufficient Authorization.\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION:
                        printf("BMS Command Failure: Insufficient Authentication.\r\n");
                        break;
                     default:
                        printf("BMS Command Failure: 0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                        break;
                  }

                  BMSCommandValueOffset  = 0;
                  BMSCommandBufferLength = 0;
               }
               else
               {
                  printf("\r\nError Response.\r\n");
                  BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
                  printf("   Connection ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
                  printf("   Transaction ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
                  printf("   Connection Type:\t%s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
                  printf("   BD_ADDR:\t%s.\r\n", BoardStr);
                  printf("   Error Type:\t%s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

                  /* Only print out the rest if it is valid.            */
                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
                  {
                     printf("   Request Opcode:\t0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                     printf("   Request Handle:\t0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                     printf("   Error Code:\t0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                  }
               }
            }
            else
               printf("Error - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               ValueLength = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               DeviceInfo  = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice);

               if((DeviceInfo) && (CallbackParameter) && (IsBMSFeatureHandle((Word_t)CallbackParameter, DeviceInfo)))
               {
                  AttributeValue = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;

                  /* Read the Supported Features from the Value in the  */
                  /* Read Response.                                     */
                  if(ValueLength == 3)
                  {
                     SupportedFeatures = (AttributeValue[0]) + ((AttributeValue[1] << 8) & 0x00FF00) + ((AttributeValue[2] << 16) & 0xFF0000);
                  }
                  else
                  {
                     if(ValueLength == 2)
                        SupportedFeatures = READ_UNALIGNED_WORD_LITTLE_ENDIAN(AttributeValue);
                     else
                     {
                        if(ValueLength == 1)
                           SupportedFeatures = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(AttributeValue);
                        else
                           SupportedFeatures = 0;
                     }
                  }

                  printf("\r\nBMS Supported Features Received, Value 0x%06X.\r\n", SupportedFeatures);

                  /* Display the supported features to the application. */
                  DisplaySupportedFeatures(SupportedFeatures);
               }
               else
               {
                  printf("\r\nRead Response.\r\n");
                  BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice, BoardStr);
                  printf("   Connection ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID);
                  printf("   Transaction ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID);
                  printf("   Connection Type:\t%s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
                  printf("   BD_ADDR:\t%s.\r\n", BoardStr);
                  printf("   Data Length:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                  /* If the data has not been decoded and displayed,    */
                  /* then just display the raw data                     */
                  printf("   Data:            { ");
                  for(i = 0; i < (ValueLength - 1); i++)
                  {
                     printf("0x%02x, ", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue[i]);
                  }

                  printf("0x%02x\r\n}", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue[i]);
               }
            }
            else
               printf("\r\nError - Null Read Response Data.");
            break;
         case etGATT_Client_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what write response     */
               /* this is.                                              */
               if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter != 0))
               {
                  if(IsBMSControlPointHandle((Word_t)CallbackParameter,DeviceInfo))
                  {
                     printf("BMS Command Response: Success.\r\n");

                     /* If the command that was sent by the client was  */
                     /* not for other device bonds.  Then we need to    */
                     /* remove bond information for this client.        */
                     if((FormatData.CommandType != bmcDeleteAllOtherBondsBREDR_LE) ||
                        (FormatData.CommandType != bmcDeleteAllOtherBondsBREDR) ||
                        (FormatData.CommandType != bmcDeleteAllOtherBondsLE))
                     {
                        /* Simply clear the client's bond information by*/
                        /* resetting the flags so that the stored keys  */
                        /* are no longer valid.  This will force the    */
                        /* requesting device to establish a new bond    */
                        /* when bonding occurs.                         */
                        /* * NOTE * Since this application only stores  */
                        /*          the LTK (LE) and Link Key (BR/EDR)  */
                        /*          we will not remove any other stored */
                        /*          keys.                               */
                        if((FormatData.CommandType == bmcDeleteAllBondsBREDR_LE) ||
                           (FormatData.CommandType == bmcDeleteBondRequestingBREDR_LE))
                        {
                           DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                           DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                        }
                        else
                        {
                           if((FormatData.CommandType == bmcDeleteAllBondsBREDR) ||
                              (FormatData.CommandType == bmcDeleteBondRequestingBREDR))
                           {
                              DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                           }
                           else
                           {
                              if((FormatData.CommandType == bmcDeleteAllBondsLE) ||
                                 (FormatData.CommandType == bmcDeleteBondRequestingLE))
                              {
                                 DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                              }
                              else
                                 printf("Failed to remove the bond information.\r\n");
                           }
                        }
                     }
                  }

                  BMSCommandValueOffset  = 0;
                  BMSCommandBufferLength = 0;
               }
               else
               {
                  printf("\r\nWrite Response.\r\n");
                  BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice, BoardStr);
                  printf("   Connection ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID);
                  printf("   Transaction ID:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID);
                  printf("   Connection Type:\t%s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
                  printf("   BD_ADDR:\t%s.\r\n", BoardStr);
                  printf("   Bytes Written:\t%u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten);
               }
            }
            else
               printf("\r\nError - Null Write Response Data.");
            break;
         default:
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");
      printf("BMS Callback Data: Event_Data = NULL.\r\n");
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
static void BTPSAPI GATT_ClientEventCallback_CGMS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t                  *DeviceInfo;
   BoardStr_t                     BoardStr;
   Byte_t                        *Value;
   Word_t                         ValueLength;
   Word_t                         Index;
   int                            Result = FUNCTION_ERROR;
   CGMS_Feature_Data_t            CGMSFeature;
   CGMS_Status_Data_t             CGMSStatus;
   CGMS_Session_Start_Time_Data_t SessionTime;
   CGMS_Session_Run_Time_Data_t   SessionRunTime;
   struct tm                      TempTime;
   char                           Time[80];

   /* Use "Result" here to avoid compiler warning                       */
   if(Result != 0)
      ;

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
               }
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
               {
                  printf("Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);

                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode == ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)
                  {
                     printf("\nLink must be Encrypted so first execute \"PairLE\" Command \r\n");
                  }
               }
               else
               {
                  /* Check if CGMS Error codes are returned             */
                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case CGMS_ERROR_CODE_MISSING_CRC:
                        printf("Error Mesg:      CGMS_ERROR_CODE_MISSING_CRC.\r\n");
                        break;
                     case CGMS_ERROR_CODE_INVALID_CRC:
                        printf("Error Mesg:      CGMS_ERROR_CODE_INVALID_CRC.\r\n");
                        break;
                     case CGMS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED:
                        printf("Error Mesg:      CGMS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED.\r\n");
                        break;
                     case CGMS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS:
                        printf("Error Mesg:      CGMS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.\r\n");
                        break;
                     default:
                        printf("Error Mesg:      Unknown.\r\n");
                        break;
                  }

               }
            }
            else
               printf("Error - Null Error Response Data.\r\n");
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               ValueLength = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               Value = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;
               printf("\r\nRead Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID);
               printf("Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID);
               printf("Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("BD_ADDR:         %s.\r\n", BoardStr);
               printf("Data Length:     %u.\r\n", ValueLength);

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if(ValueLength != 0)
               {
                  if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter != 0))
                  {
                     if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Measurement_Client_Configuration)
                     {
                        if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == 2)
                        {
                           printf("\r\nRead CGMS Measurement Client Configuration Complete.\r\n");
                        }
                        else
                        {
                           printf("\r\nError - Invalid length (%u) for CGMS Measurement Client Configuration response\r\n", ValueLength);
                        }
                     }
                     else
                     {
                        if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Feature)
                        {
                           if((Result = CGMS_Decode_CGMS_Feature(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength,
                                                               GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue,
                                                               &CGMSFeature)) == 0)
                           {
                              printf("\r\nRead CGMS Features Complete.\r\n\r\n");
                              ClientFeatures           = CGMSFeature.Features;
                              ClientTypeSampleLocation = CGMSFeature.TypeSampleLocation;

                              printf("CGMS Features:        0x%08X\r\n", ClientFeatures);
                              for(Index = 0; Index < (NUM_CGMS_FEATURE_BIT_STRINGS+1); Index++)
                              {
                                 if(ClientFeatures & ((DWord_t)pow(2,Index)))
                                 {
                                    printf("   %s\r\n", CGMSFeatureBitStrings[Index]);
                                 }
                              }
                              printf("\r\nType Sample Location: 0x%02X\r\n", ClientTypeSampleLocation);
                              printf("   Type:              %s.\r\n", CGMSFeatureTypeBitStrings[(ClientTypeSampleLocation & 0x0F) - 1]);
                              printf("   Sample Location:   %s.\r\n", CGMSFeatureSampleLocationBitStrings[(ClientTypeSampleLocation >> 4) - 1]);
                           }
                           else
                           {
                              printf("\r\nError - CGMS_Decode_CGMS_Feature Result: %d\r\n", Result);
                           }
                        }
                        else
                        {
                           if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Status)
                           {
                              if((Result = CGMS_Decode_CGMS_Status(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED),
                                                                 GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength,
                                                                 GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue,
                                                                 &CGMSStatus)) == 0)
                              {
                                 printf("\r\nRead CGMS Status Complete.\r\n\r\n");
                                 ClientStatus = CGMSStatus.Status;
                                 printf("CGMS Status:\r\n");
                                 printf("   Time Offset: %u\r\n",     CGMSStatus.TimeOffset);
                                 printf("   Value:       0x%08X\r\n", ClientStatus);
                              }
                              else
                              {
                                 printf("\r\nError - CGMS_Decode_CGMS_Status Result: %d\r\n", Result);
                              }
                           }
                           else
                           {
                              if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Session_Start_Time)
                              {

                                 if((Result = CGMS_Decode_Session_Start_Time(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED),
                                                     GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength,
                                                     GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue,
                                                     &SessionTime)) == 0)
                                 {
                                    printf("\r\nRead CGMS Session Start Time Complete.\r\n\r\n");
                                    ClientSessionStartTime   = SessionTime;

                                    /* Format and display the session   */
                                    /* start time.                      */
                                    TempTime.tm_year = ClientSessionStartTime.Time.Year-1900;
                                    TempTime.tm_mon  = ClientSessionStartTime.Time.Month-1;
                                    TempTime.tm_mday = ClientSessionStartTime.Time.Day;
                                    TempTime.tm_hour = ClientSessionStartTime.Time.Hours;
                                    TempTime.tm_min  = ClientSessionStartTime.Time.Minutes;
                                    TempTime.tm_sec  = ClientSessionStartTime.Time.Seconds;
                                    strftime(Time, 80, "%Y/%m/%d %T", &TempTime);
                                    printf("CGMS Session Start Time:\r\n");
                                    printf("   Time:       %s\r\n", Time);
                                    printf("   Timezone:   %d\r\n", ClientSessionStartTime.TimeZone);
                                    printf("   DST Offset: %u\r\n", ClientSessionStartTime.DSTOffset);
                                 }
                                 else
                                 {
                                    printf("\r\nError - CGMS_Decode_Session_Time Result: %d\r\n", Result);
                                 }
                              }
                              else
                              {
                                 if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Session_Run_Time)
                                 {
                                    if((Result = CGMS_Decode_Session_Run_Time(((ClientFeatures & CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED) ? CGMS_E2E_CRC_SUPPORTED : CGMS_E2E_CRC_NOT_SUPPORTED),
                                                                             GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength,
                                                                             GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue,
                                                                             &SessionRunTime)) == 0)
                                    {
                                       printf("\r\nRead CGMS Session Run Time Complete.\r\n\r\n");
                                       ClientSessionRunTime = SessionRunTime.SessionRunTime;

                                       printf("CGMS Session Run Time:   %u (Min)\r\n", ClientSessionRunTime);
                                    }
                                    else
                                    {
                                       printf("\r\nError - CGMS_Decode_Session_Run_Time Result: %d\r\n", Result);
                                    }
                                 }
                                 else
                                 {
                                    if(CallbackParameter == DeviceInfo->ClientInfo.RACP_Client_Configuration)
                                    {
                                       if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == 2)
                                       {
                                          printf("\r\nRead RACP Client Configuration Complete.\r\n");
                                       }
                                       else
                                       {
                                          printf("\r\nError - Invalid length (%u) for RACP Client Configuration response\r\n", ValueLength);
                                       }
                                    }
                                    else
                                    {
                                       if(CallbackParameter == DeviceInfo->ClientInfo.SOCP_Client_Configuration)
                                       {
                                          if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == 2)
                                          {
                                             printf("\r\nRead SOCP Client Configuration Complete.\r\n");
                                          }
                                          else
                                          {
                                             printf("\r\nError - Invalid length (%u) for SOCP Client Configuration response\r\n", ValueLength);
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }

                  /* If the data has not been decoded and displayed,    */
                  /* then just display the raw data                     */
                  if((DeviceInfo == NULL) || (CallbackParameter == 0))
                  {
                     printf("   Data:            { ");
                     for(Index = 0; Index < (ValueLength - 1); Index++)
                        printf("0x%02x, ", *Value + Index);

                     printf("0x%02x }\r\n", *Value + Index);
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
               printf("MTU:             %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU);
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

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what write response this*/
               /* is.                                                   */
               if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter != 0))
               {
                  if(CallbackParameter == DeviceInfo->ClientInfo.CGMS_Session_Start_Time)
                  {
                     printf("\r\nWrite Session Start Time Complete.\r\n");
                  }

                  if(CallbackParameter == DeviceInfo->ClientInfo.Record_Access_Control_Point)
                  {
                     printf("\r\nWrite Record_Access_Control_Point Complete.\r\n");
                  }

                  if(CallbackParameter == DeviceInfo->ClientInfo.Specific_Ops_Control_Point)
                  {
                     printf("\r\nWrite Specific_Ops_Control_Point Complete.\r\n");
                  }
               }
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
   char         *NameBuffer;
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
            printf("   Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout");

            /* Only print out the rest if it is valid.                  */
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
            {
               printf("Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
               printf("Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
               printf("Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
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
            printf("Error - Null Error Response Data.\r\n");
         break;
      case etGATT_Client_Read_Response:
         if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
            {
               if((Word_t)CallbackParameter == daManufacturerName)
               {
                  /* Display the remote device Manufacurer name.        */
                  if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                  {
                     BTPS_MemInitialize(NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                     BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                     printf("\r\nManufacturer Name: ");
                     DisplayDISCharacteristicValue(NameBuffer);
                     BTPS_FreeMemory(NameBuffer);
                  }
               }
               else
               {
                  if((Word_t)CallbackParameter == daModelNumber)
                  {
                     /* Display the remote device Model Number.         */
                     if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                     {
                        BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                        BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                        printf("Model Number: ");
                        DisplayDISCharacteristicValue(NameBuffer);
                        BTPS_FreeMemory(NameBuffer);
                     }
                  }
                  else
                  {
                     if((Word_t)CallbackParameter == daSerialNumber)
                     {
                        /* Display the remote device Serial Number.     */
                        if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                        {
                           BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                           BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                           printf("Serial Number: ");
                           DisplayDISCharacteristicValue(NameBuffer);
                           BTPS_FreeMemory(NameBuffer);
                        }
                     }
                     else
                     {
                        if((Word_t)CallbackParameter == daHardwareRevision)
                        {
                           /* Display the remote device Hardware        */
                           /* Revision.                                 */
                           if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                           {
                              BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                              BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                              printf("Hardware Revision: ");
                              DisplayDISCharacteristicValue(NameBuffer);
                              BTPS_FreeMemory(NameBuffer);
                           }
                        }
                        else
                        {
                           if((Word_t)CallbackParameter == daFirmwareRevision)
                           {
                              /* Display the remote device Firmware     */
                              /* Revision.                              */
                              if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                              {
                                 BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                 BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                 printf("Firmware Revision: ");
                                 DisplayDISCharacteristicValue(NameBuffer);
                                 BTPS_FreeMemory(NameBuffer);
                              }
                           }
                           else
                           {
                              if((Word_t)CallbackParameter == daSoftwareRevision)
                              {
                                 /* Display the remote device Software  */
                                 /* Revision.                           */
                                 if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                                 {
                                    BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                    BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                    printf("Software Revision: ");
                                    DisplayDISCharacteristicValue(NameBuffer);
                                    BTPS_FreeMemory(NameBuffer);
                                 }
                              }
                              else
                              {
                                 if((Word_t)CallbackParameter == daSystemID)
                                 {
                                    /* Display the remote device System */
                                    /* ID name.                         */
                                    if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                                    {
                                       BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                       BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                       printf("System ID: ");
                                       DisplayDISCharacteristicValue(NameBuffer);
                                       BTPS_FreeMemory(NameBuffer);
                                    }
                                 }
                                 else
                                 {
                                    if((Word_t)CallbackParameter == daIEEEDataList)
                                    {
                                       /* Display the remote Device     */
                                       /* IEEE  Certification List.     */
                                       if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                                       {
                                          BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                          BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                          printf("IEEE CertificationDataList: ");
                                          DisplayDISCharacteristicValue(NameBuffer);
                                          BTPS_FreeMemory(NameBuffer);
                                       }
                                    }
                                    else
                                    {
                                       /*Unknown                        */
                                       if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                                       {
                                          BTPS_MemInitialize(NameBuffer, '\0', GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                          BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                          printf("Unknown Device Characteristic: ");
                                          DisplayDISCharacteristicValue(NameBuffer);
                                          BTPS_FreeMemory(NameBuffer);
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
   DeviceInfo_t         *DeviceInfo;
   BoardStr_t            BoardStr;
   int                   Result;
   GAP_Encryption_Mode_t EncryptionMode;

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

               printf("\r\netGATT_Connection_Device_Connection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
               printf("Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID);
               printf("Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Remote Device:   %s.\r\n", BoardStr);
               printf("Connection MTU:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU);

               ConnectionBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;

               /* Since there is no connection event in GAP for BR/EDR  */
               /* we will handle any connection setup here when GATT    */
               /* connects for BR/EDR.                                  */
               /* * NOTE * This code should be simliar to the LE        */
               /*          Connection Event.                            */
               if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctBR_EDR)
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
                        printf("Failed to add device to Device Info List.\r\n");
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
                     printf("No device Info.\r\n");
               }
               else
               {
                  /* Must be an LE connection.                          */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) != NULL)
                  {
                     if(LocalDeviceIsMaster)
                     {
                        /* Attempt to update the MTU to the maximum     */
                        /* supported.                                   */
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_CGMS, 0);
                     }
                  }
               }
            }
            else
               printf("Error - Null Connection Data.\r\n");

            /* Print the command line prompt.                           */
            DisplayPrompt();
            break;
         case etGATT_Connection_Device_Disconnection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               printf("\r\netGATT_Connection_Device_Disconnection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
               printf("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID);
               printf("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:   %s.\r\n", BoardStr);

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

                     /* Clear the CCCDs stored for this device.         */
                     DeviceInfo->ServerInfo.CGMS_Measurement_Client_Configuration = 0;
                     DeviceInfo->ServerInfo.RACP_Client_Configuration             = 0;
                     DeviceInfo->ServerInfo.SOCP_Client_Configuration             = 0;

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

                     /* Clear the saved Connection BD_ADDR.             */
                     ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
                     LocalDeviceIsMaster = FALSE;
                  }
               }
            }
            else
               printf("Error - Null Disconnection Data.\r\n");

            /* Print the command line prompt.                           */
            DisplayPrompt();
            break;
       case etGATT_Connection_Server_Indication:
             if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data)
            {
               printf("\r\netGATT_Connection_Server_Indication with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->RemoteDevice, BoardStr);
               printf("Connection ID:    %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID);
               printf("Transaction ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID);
               printf("Connection Type:  %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Remote Device:    %s.\r\n", BoardStr);
               printf("Attribute Handle: 0x%04X.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle);
               printf("Attribute Length: %d.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->RemoteDevice)) != NULL)
               {
                  /* Only send an indication response if this is a known*/
                  /* remote device.                                     */
                  if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->TransactionID)) != 0)
                  {
                     DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);
                  }

                  if(DeviceInfo->ClientInfo.Record_Access_Control_Point == GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle)
                  {
                     /* Decode and display the data                     */
                     printf("\r\n   RecordAccessControlPoint Data:\r\n");
                     DecodeDisplayRACPResponse(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue);
                  }
                  else
                  {
                     if(DeviceInfo->ClientInfo.Specific_Ops_Control_Point == GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle)
                     {
                        /* Decode and display the data                  */
                        printf("\r\n   SpecificOpsControlPoint Data:\r\n");
                        DecodeDisplaySOCPResponse(GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue);
                     }
                  }
               }
            }
            else
               printf("Error - Null Server Indication Data.\r\n");

            /* Print the command line prompt.                           */
            DisplayPrompt();
            break;
         case etGATT_Connection_Server_Notification:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               printf("\r\netGATT_Connection_Server_Notification with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice, BoardStr);
               printf("   Connection ID:    %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Attribute Handle: 0x%04X.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle);
               printf("   Attribute Length: %d.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice)) != NULL)
               {
                  if(DeviceInfo->ClientInfo.CGMS_Measurement == GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle)
                  {
                     /* Decode and display the data.                    */
                     DecodeDisplayStoreCGMSMeasurement(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue);
                  }
               }
               else
                  printf("Error - Remote Server Unknown.\r\n");
            }
            else
               printf("Error - Null Server Notification Data.\r\n");

            /* Print the command line prompt.                           */
            DisplayPrompt();
            break;
         case etGATT_Connection_Device_Connection_Confirmation:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data)
            {
               printf("\r\netGATT_Connection_Device_Connection_Confirmation with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data->RemoteDevice, BoardStr);
               printf("Connect Status:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data->ConnectionStatus);
               printf("Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data->ConnectionID);
               printf("Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("Remote Device:   %s.\r\n", BoardStr);
               printf("Connection MTU:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Confirmation_Data->MTU);
            }
            else
               printf("Error - Null Confirmation Data.\r\n");

            /* Print the command line prompt.                           */
            DisplayPrompt();
            break;
         default:
            break;
      }
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

                  switch(CallbackParameter)
                  {
                     case sdDIS:
                        /* Populate the handles for DIS Service.        */
                        DISPopulateHandles(&(DeviceInfo->DISClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;

                     case sdGAPS:
                        /* Populate the handles for GAPS Service.       */
                        GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;

                     case sdCGMS:
                        /* Populate the handles for CGMS Service.       */
                        CGMSPopulateHandles(DeviceInfo, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                        break;

                     case sdBMS:
                        /* Populate the handles for BMS Service.        */
                        BMSPopulateHandles(DeviceInfo, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);

                     default:
                        printf("Unknown Service Discovery Type");
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

                  /* Check the service discovery type for CGMS          */
                  if(((Service_Discovery_Type_t)CallbackParameter) == sdCGMS)
                  {
                     /* Flag that service discovery has been performed  */
                     /* on for this connection.                         */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE;

                     /* Print a summary of what descriptor were found   */
                     printf("\r\nCGMS Service Discovery Summary\r\n");

                     printf("   CGMS Measurement               : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Measurement                                 ? "Supported" : "Not Supported"));
                     printf("   CGMS Measurement CC            : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Measurement_Client_Configuration            ? "Supported" : "Not Supported"));
                     printf("   CGMS Feature                   : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Feature                                     ? "Supported" : "Not Supported"));
                     printf("   CGMS Status                    : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Status                                      ? "Supported" : "Not Supported"));
                     printf("   CGMS Session Start Time        : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Session_Start_Time                          ? "Supported" : "Not Supported"));
                     printf("   CGMS Session Run Time          : %s\r\n", (DeviceInfo->ClientInfo.CGMS_Session_Run_Time                            ? "Supported" : "Not Supported"));
                     printf("   Record Access Control Point    : %s\r\n", (DeviceInfo->ClientInfo.Record_Access_Control_Point                     ? "Supported" : "Not Supported"));
                     printf("   Record Access Control Point CC : %s\r\n", (DeviceInfo->ClientInfo.RACP_Client_Configuration                       ? "Supported" : "Not Supported"));
                     printf("   Specific Ops Control Point     : %s\r\n", (DeviceInfo->ClientInfo.Specific_Ops_Control_Point                      ? "Supported" : "Not Supported"));
                     printf("   Specific Ops Control Point CC  : %s\r\n", (DeviceInfo->ClientInfo.SOCP_Client_Configuration                      ? "Supported" : "Not Supported"));
                  }
                  else
                  {
                     /* Check the service discovery type for DIS        */
                     if(((Service_Discovery_Type_t)CallbackParameter) == sdDIS)
                     {
                        /* Flag that service discovery has been         */
                        /* performed for this connection.               */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE;

                        /* Print a summary of what descriptor were      */
                        /* found                                        */
                        printf("\r\nDIS Service Discovery Summary\r\n");
                        printf("     Manufacturer Name             : %s\r\n", (DeviceInfo->DISClientInfo.ManufacturerName            ? "Supported" : "Not Supported"));
                        printf("     Model Number                  : %s\r\n", (DeviceInfo->DISClientInfo.ModelNumber                 ? "Supported" : "Not Supported"));
                        printf("     Serial Number                 : %s\r\n", (DeviceInfo->DISClientInfo.SerialNumber                ? "Supported" : "Not Supported"));
                        printf("     Hardware Revision             : %s\r\n", (DeviceInfo->DISClientInfo.HardwareRevision            ? "Supported" : "Not Supported"));
                        printf("     Firmware Revision             : %s\r\n", (DeviceInfo->DISClientInfo.FirmwareRevision            ? "Supported" : "Not Supported"));
                        printf("     Software Revision             : %s\r\n", (DeviceInfo->DISClientInfo.SoftwareRevision            ? "Supported" : "Not Supported"));
                        printf("     System ID                     : %s\r\n", (DeviceInfo->DISClientInfo.SystemID                    ? "Supported" : "Not Supported"));
                        printf("     IEEE Certification Data List  : %s\r\n", (DeviceInfo->DISClientInfo.IEEECertificationDataList   ? "Supported" : "Not Supported"));
                     }
                     else
                     {
                        /* Check the service discovery type for BMS     */
                        if(((Service_Discovery_Type_t)CallbackParameter) == sdBMS)
                        {
                           /* Flag that service discovery has been      */
                           /* performed on for this connection.         */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE;

                           /* Print a summary of what descriptor were   */
                           /* found                                     */
                           printf("\r\nBMS Service Discovery Summary\r\n");
                           printf("   Bond Management Control Point:   %s\r\n", (DeviceInfo->BMSClientInfo.BM_Control_Point ? "Supported" : "Not Supported"));
                           printf("   Bond Management Feature:         %s\r\n", (DeviceInfo->BMSClientInfo.BM_Feature ? "Supported" : "Not Supported"));
                        }
                     }
                  }
               }
               break;
         }

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
   HCI_DriverInformation_t  HCI_DriverInformation;
   HCI_DriverInformation_t *HCI_DriverInformationPtr;
   char                     UserInput[MAX_COMMAND_LENGTH];

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
                     /* Let the user select the proper interface.       */
                     UserInterface_Selection();

                     /* Display the first command prompt.               */
                     DisplayPrompt();

                     /* This is the main loop of the program.  It gets  */
                     /* user input from the command window, make a call */
                     /* to the command parser, and command interpreter. */
                     /* After the function has been ran it then check   */
                     /* the return value and displays an error message  */
                     /* when appropriate.  If the result returned is    */
                     /* ever the EXIT_CODE the loop will exit leading   */
                     /* the the exit of the program.                    */
                     while(Result != EXIT_CODE)
                     {
                        /* Initialize the value of the variable used to */
                        /* store the users input and output "Input: " to*/
                        /* the command window to inform the user that   */
                        /* another command may be entered.              */
                        UserInput[0] = '\0';

                        /* Retrieve the command entered by the user and */
                        /* store it in the User Input Buffer.  Note that*/
                        /* this command will fail if the application    */
                        /* receives a signal which cause the standard   */
                        /* file streams to be closed.  If this happens  */
                        /* the loop will be broken out of so the        */
                        /* application can exit.                        */
                        if(fgets(UserInput, sizeof(UserInput), stdin) != NULL)
                        {
                           /* Start a newline for the results.          */
                           printf("\r\n");

                           /* Next, check to see if a command was input */
                           /* by the user.                              */
                           if(strlen(UserInput))
                           {
                              /* The string input by the user contains a*/
                              /* value, now run the string through the  */
                              /* Command Parser.                        */
                              Result = CommandLineInterpreter(UserInput);
                           }
                        }
                        else
                        {
                           Result = EXIT_CODE;
                        }
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, BCSP = 2 Flag] [IF !USB [Comm Port] [Baud Rate]])\r\n");
   }

   return 0;
}
