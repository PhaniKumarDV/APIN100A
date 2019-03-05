/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< linuxuds.c >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXUDS - Linux Bluetooth User Data Service using GATT (LE/BREDR)        */
/*             sample application.                                            */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/29/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxUDS.h"            /* Application Header.                       */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTUDS.h"            /* Main SS1 UDS Service Header.              */

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

#define UDSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* UDSP.             */

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

#define PRINT_SERVICE_DISCOVERY_INFORMATION     (TRUE)   /* Enables printing  */
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
                                                         /* create a UDS     */
                                                         /* Server.           */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

#define UNABLE_TO_ALLOCATE_MEMORY                  (-11) /* Denotes that we   */
                                                         /* failed because we */
                                                         /* couldn't allocate */
                                                         /* memory.           */

#define UDS_SA_MAX_BUFFER_SIZE                     (BTPS_CONFIGURATION_UDS_MAXIMUM_SUPPORTED_STRING_LENGTH)
                                                         /* Denotes the size  */
                                                         /* of the buffers.   */
                                                         /* The service will  */
                                                         /* not allow reads or*/
                                                         /* writes for strings*/
                                                         /* beyond this       */
                                                         /* length.           */

#define UDS_SA_DEFAULT_CHARACTERISTIC_FLAGS        (UDS_CHARACTERISTIC_FLAGS_FIRST_NAME                        |  \
                                                    UDS_CHARACTERISTIC_FLAGS_LAST_NAME                         |  \
                                                    UDS_CHARACTERISTIC_FLAGS_EMAIL_ADDRESS                     |  \
                                                    UDS_CHARACTERISTIC_FLAGS_AGE                               |  \
                                                    UDS_CHARACTERISTIC_FLAGS_DATE_OF_BIRTH                     |  \
                                                    UDS_CHARACTERISTIC_FLAGS_GENDER                            |  \
                                                    UDS_CHARACTERISTIC_FLAGS_WEIGHT                            |  \
                                                    UDS_CHARACTERISTIC_FLAGS_HEIGHT                            |  \
                                                    UDS_CHARACTERISTIC_FLAGS_VO2_MAX                           |  \
                                                    UDS_CHARACTERISTIC_FLAGS_HEART_RATE_MAX                    |  \
                                                    UDS_CHARACTERISTIC_FLAGS_RESTING_HEART_RATE                |  \
                                                    UDS_CHARACTERISTIC_FLAGS_MAXIMUM_RECOMMENDED_HEART_RATE    |  \
                                                    UDS_CHARACTERISTIC_FLAGS_AEROBIC_THRESHOLD                 |  \
                                                    UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_THRESHOLD               |  \
                                                    UDS_CHARACTERISTIC_FLAGS_SPORT_TYPE                        |  \
                                                    UDS_CHARACTERISTIC_FLAGS_DATE_OF_THRESHOLD                 |  \
                                                    UDS_CHARACTERISTIC_FLAGS_WAIST_CIRCUMFERENCE               |  \
                                                    UDS_CHARACTERISTIC_FLAGS_HIP_CIRCUMFERENCE                 |  \
                                                    UDS_CHARACTERISTIC_FLAGS_FAT_BURN_HEART_RATE_LOWER_LIMIT   |  \
                                                    UDS_CHARACTERISTIC_FLAGS_FAT_BURN_HEART_RATE_UPPER_LIMIT   |  \
                                                    UDS_CHARACTERISTIC_FLAGS_AEROBIC_HEART_RATE_LOWER_LIMIT    |  \
                                                    UDS_CHARACTERISTIC_FLAGS_AEROBIC_HEART_RATE_UPPER_LIMIT    |  \
                                                    UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_HEART_RATE_LOWER_LIMIT  |  \
                                                    UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_HEART_RATE_UPPER_LIMIT  |  \
                                                    UDS_CHARACTERISTIC_FLAGS_FIVE_ZONE_HEART_RATE_LIMITS       |  \
                                                    UDS_CHARACTERISTIC_FLAGS_THREE_ZONE_HEART_RATE_LIMITS      |  \
                                                    UDS_CHARACTERISTIC_FLAGS_TWO_ZONE_HEART_RATE_LIMIT         |  \
                                                    UDS_CHARACTERISTIC_FLAGS_LANGUAGE)
                                                         /* Denotes the UDS   */
                                                         /* Characteristics   */
                                                         /* supported by the  */
                                                         /* UDS Server.       */

#define UDS_SA_MAXIMUM_SUPPORTED_USERS             (5)
                                                         /* Denotes the       */
                                                         /* maximum number of */
                                                         /* supported users.  */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxUDS_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxUDS_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxUDS"

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
   sdUDS
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

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x0001
#define DEVICE_INFO_FLAGS_LINK_KEY_VALID                    0x0002
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x0004
#define DEVICE_INFO_FLAGS_BR_EDR_CONNECTED                  0x0008
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x0010
#define DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE    0x0020
#define DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE   0x0040
#define DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE    0x0080
#define DEVICE_INFO_FLAGS_UDS_PROCEDURE_IN_PROGRESS         0x0100

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

   /* The following enumeration defines the attribute handle types.     */
   /* This has been included to make the sample application easier to   */
   /* read since we do not need to use if-else trees.                   */
typedef enum
{
   ahtUDS_Characteristic,
   ahtDatabase_Change_Increment,
   ahtDatabase_Change_Increment_CCCD,
   ahtUser_Index,
   ahtUser_Control_Point,
   ahtUser_Control_Point_CCCD
} UDS_Attribute_Handle_Type_t;

   /* The following structure holds the data that is needed to hold     */
   /* parameters for function commands and to correctly handle a GATT   */
   /* response received from a remote UDS Server.                       */
   /* * NOTE * We will add one to the maximum size of the buffer for the*/
   /*          NULL terminator so we can display UDS Characteristic     */
   /*          strings as a character string.                           */
   /* * NOTE * If the AttributeHandleType is ahtUDS_Characteristic, then*/
   /*          the Type field MUST be valid since it will be used to    */
   /*          distinguish the type of UDS Characteristic.              */
   /* * NOTE * The UserIndex is only used by the UDS Server to specify  */
   /*          the user index to write a UDS Characteristic into a      */
   /*          user's data.                                             */
   /* * NOTE * The remaining fields are used by the UDS Client for GATT */
   /*          Write requests.  The Data field holds the possible fields*/
   /*          that the UDS Client may send.  The ReadLong,             */
   /*          BufferOffset, BufferLength, and Buffer will be used for  */
   /*          GATT Read/Write Long procedure requests and responses.   */
typedef struct _tagUDS_Request_Data_t
{
   UDS_Attribute_Handle_Type_t               AttributeHandleType;
   Word_t                                    AttributeHandle;
   UDS_Characteristic_Type_t                 Type;
   UDS_CCCD_Characteristic_Type_t            CCCDType;

   /* UDS Server (Only) request data.                                   */
   Byte_t                                    UserIndex;

   /* UDS Client (Only) request data.                                   */
   Boolean_t                                 ReadLong;
   Word_t                                    BufferOffset;
   Word_t                                    BufferLength;
   Byte_t                                    Buffer[UDS_SA_MAX_BUFFER_SIZE+1];
   union
   {
      Byte_t                                *Buffer;
      UDS_Characteristic_t                   Characteristic;
      Word_t                                 ClientConfiguration;
      DWord_t                                DatabaseChangeIncrement;
      UDS_User_Control_Point_Request_Data_t  RequestData;
   } Data;

} UDS_Request_Data_t;

   /* The following structure defines the UDS String data for the sample*/
   /* application, for the following UDS Characteristics: First Name,   */
   /* Last Name, Email Address, and Language.                           */
   /* * NOTE * We will include room for the NULL terminator so we can   */
   /*          display the Buffer as a character string.                */
typedef struct _tagUDS_Sample_String_Data_t
{
   Word_t  Buffer_Length;
   Byte_t  Buffer[UDS_SA_MAX_BUFFER_SIZE+1];
} UDS_Sample_String_Data_t;

   /* The following union defines the possible data types for a UDS     */
   /* Characteristic used by the sample application.                    */
typedef union
{
   UDS_Sample_String_Data_t                 First_Name;
   UDS_Sample_String_Data_t                 Last_Name;
   UDS_Sample_String_Data_t                 Email_Address;
   Byte_t                                   Age;
   UDS_Date_Data_t                          Date_Of_Birth;
   Byte_t                                   Gender;
   Word_t                                   Weight;
   Word_t                                   Height;
   Byte_t                                   VO2_Max;
   Byte_t                                   Heart_Rate_Max;
   Byte_t                                   Resting_Heart_Rate;
   Byte_t                                   Maximum_Recommended_Heart_Rate;
   Byte_t                                   Aerobic_Threshold;
   Byte_t                                   Anaerobic_Threshold;
   Byte_t                                   Sport_Type;
   UDS_Date_Data_t                          Date_Of_Threshold;
   Word_t                                   Waist_Circumference;
   Word_t                                   Hip_Circumference;
   Byte_t                                   Fat_Burn_Heart_Rate_Lower_Limit;
   Byte_t                                   Fat_Burn_Heart_Rate_Upper_Limit;
   Byte_t                                   Aerobic_Heart_Rate_Lower_Limit;
   Byte_t                                   Aerobic_Heart_Rate_Upper_Limit;
   Byte_t                                   Anaerobic_Heart_Rate_Lower_Limit;
   Byte_t                                   Anaerobic_Heart_Rate_Upper_Limit;
   UDS_Five_Zone_Heart_Rate_Limits_Data_t   Five_Zone_Heart_Rate_Limits;
   UDS_Three_Zone_Heart_Rate_Limits_Data_t  Three_Zone_Heart_Rate_Limits;
   Byte_t                                   Two_Zone_Heart_Rate_Limit;
   UDS_Sample_String_Data_t                 Language;
} UDS_Sample_Characteristic_t;

/* We will hardcode the number of UDS Characteristics that each user */
/* MUST support.  This will be the maximum number of supported UDS   */
/* Characteristics since we MUST support all of them for this sample */
/* application.                                                      */
#define UDS_SA_NUMBER_OF_SUPPORTED_UDS_CHARACTERISTICS  (28)

   /* The following structure defines the data that needs to be stored  */
   /* for each user that is supported by the UDS Server.                */
   /* * NOTE * The Registered field will be used to indicate that a UDS */
   /*          Client has registered the user.                          */
   /* * NOTE * The Consent_Code field will store the Consent Code       */
   /*          received in the User Control Point Register New User     */
   /*          procedure, that is required to be compared so that a     */
   /*          user's data may be allowed to be accessed via the        */
   /*          subsequent User Control Point Consent Procedure..        */
   /* * NOTE * The Consent_Attempts field is used to control how many   */
   /*          times a UDS Client may attempt or use the User Control   */
   /*          Point Consent Procedure to access a user's data.  This   */
   /*          sample application will support 3 attempts and will not  */
   /*          allow the UDS Server to reset this value once all        */
   /*          attempts are used.  The User MUST be deleted using the   */
   /*          User Control Point Delete User Procedure.                */
   /* * NOTE * The Database Change Increment is used to indicate to a   */
   /*          UDS Client changes that have been made to a user's data  */
   /*          stored on the UDS Server.  This provides synchronization.*/
   /* * NOTE * The Data field stores all supported UDS Characteristics  */
   /*          that the user MUST hold.  Consent MUST be given before   */
   /*          this structure may be accessed.                          */
typedef struct _tagUDS_User_Data_t
{
   Boolean_t                    Registered;
   Word_t                       Consent_Code;
   Byte_t                       Consent_Attempts;
   DWord_t                      Database_Change_Increment;
   UDS_Sample_Characteristic_t  Data[UDS_SA_NUMBER_OF_SUPPORTED_UDS_CHARACTERISTICS];
} UDS_User_Data_t;

   /* The following structure for a Master/Slave is used to hold a list */
   /* of information on all paired.                                     */
   /* * NOTE * For each UDS Client we will need to store if the UDS     */
   /*          Client has obtained consent before it can access a user's*/
   /*          UDS Characteristic data.  This is dependent on the number*/
   /*          of supported users, which is why it is not stored in the */
   /*          UDS_Server_Information_t structure.  The Consent field   */
   /*          will be indexed by the User Index for the user.          */
typedef struct _tagDeviceInfo_t
{
   Word_t                    Flags;
   Byte_t                    EncryptionKeySize;
   GAP_LE_Address_Type_t     ConnectionAddressType;
   BD_ADDR_t                 ConnectionBD_ADDR;
   Link_Key_t                LinkKey;
   Long_Term_Key_t           LTK;
   Random_Number_t           Rand;
   Word_t                    EDIV;
   DIS_Client_Info_t         DISClientInfo;
   GAPS_Client_Info_t        GAPSClientInfo;
   UDS_Client_Information_t  UDSClientInfo;
   UDS_Request_Data_t        UDSRequestData;
   UDS_Server_Information_t  UDSServerInfo;
   Boolean_t                 Consent[UDS_SA_MAXIMUM_SUPPORTED_USERS];
   struct _tagDeviceInfo_t  *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

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
static unsigned int        UDSInstanceID;
                                                    /* The following holds the UDS     */
                                                    /* Instance IDs that are returned  */
                                                    /* from UDS_Initialize_XXX().      */

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

static UDS_User_Data_t     UserList[UDS_SA_MAXIMUM_SUPPORTED_USERS];
                                                    /* The user data that the UDS      */
                                                    /* Server MUST store.              */

static Byte_t              User_Index;
                                                    /* The selected User Index of the  */
                                                    /* user's data that has consent    */
                                                    /* to be read/written.             */

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

   /* User Data Service Functions and Commands.                         */
static void UDSPopulateHandles(UDS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreDescriptorHandles(UDS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle);

static int RegisterUDS(void);
static int UnregisterUDS(void);

static void MapUDSCharacteristicToAttributeHandle(UDS_Characteristic_Type_t Type, UDS_Characteristic_Handles_t *Handles, Word_t *AttributeHandle);

static int ReadUDSCharacteristic(UDS_Request_Data_t *RequestData);
static int WriteUDSCharacteristic(UDS_Request_Data_t *RequestData);

static void StoreUDSCharacteristicData(UDS_User_Data_t *UserData, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);
static void FormatUDSCharacteristicData(UDS_User_Data_t *UserData, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);

static void RegisterNewUserProcedure(UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData);
static void ConsentProcedure(DeviceInfo_t *DeviceInfo, UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData);
static void DeleteUserDataProcedure(UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData);

static void DisplayUDSCharacteristicType(UDS_Characteristic_Type_t Type);
static void DisplayUDSCharacteristicData(UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);
static void DisplayUDSSampleCharacteristicData(UDS_Characteristic_Type_t Type, UDS_Sample_Characteristic_t *UDS_Characteristic);
static void DisplayUserControlPointRequestData(UDS_User_Control_Point_Request_Data_t *RequestData);
static void DisplayUserControlPointResponseData(UDS_User_Control_Point_Response_Data_t *ResponseData);

static int RegisterUDSCommand(ParameterList_t *TempParam);
static int UnregisterUDSCommand(ParameterList_t *TempParam);
static int DiscoverUDSCommand(ParameterList_t *TempParam);
static int ReadUDSCharacteristicCommand(ParameterList_t *TempParam);
static int WriteUDSCharacteristicCommand(ParameterList_t *TempParam);
static int WriteUDSCCCDCommand(ParameterList_t *TempParam);
static int WriteUDSDatabaseChangeIncrement(ParameterList_t *TempParam);
static int WriteUDSUserControlPointCommand(ParameterList_t *TempParam);
static int RegisterUserCommand(ParameterList_t *TempParam);

static void DisplayReadCharacteristicCommandUsage(void);
static void DisplayWriteCharacteristicCommandUsage(void);
static void DisplayWriteUDSCCCDCommandUsage(void);
static void DisplayWriteUDSUserControlPointCommandUsage(void);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI UDS_EventCallback(unsigned int BluetoothStackID, UDS_Event_Data_t *UDS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_UDS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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
   /* * NOTE * This function has been modified from other sample        */
   /*          applications for use with UDS/UDS.                       */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List.  Upon return of this      */
   /* function, the Head Pointer is set to NULL.                        */
   /* * NOTE * This function has been modified from other sample        */
   /*          applications for use with UDS/UDS.                       */
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
   AddCommand("REGISTERUDS", RegisterUDSCommand);
   AddCommand("UNREGISTERUDS", UnregisterUDSCommand);
   AddCommand("DISCOVERUDS", DiscoverUDSCommand);
   AddCommand("READ", ReadUDSCharacteristicCommand);
   AddCommand("WRITE", WriteUDSCharacteristicCommand);
   AddCommand("WRITECCCD", WriteUDSCCCDCommand);
   AddCommand("WRITEDCI", WriteUDSDatabaseChangeIncrement);
   AddCommand("WRITEUSERCP", WriteUDSUserControlPointCommand);
   AddCommand("REGISTERUSER", RegisterUserCommand);
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

               /* We will check for the UDS Service data.  Make sure the*/
               /* mandatory UUID is present.                            */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the UDS Service data.             */
                  if(UDS_COMPARE_UDS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
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
   printf("\r\nLinuxUDS>");

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

      /* Cleanup UDS Service.                                           */
      if(UDSInstanceID)
      {
         UDS_Cleanup_Service(BluetoothStackID, UDSInstanceID);
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
   printf("* UDS Sample Application                                         *\r\n");
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
   printf("* Command Options UDS:                                           *\r\n");
   printf("*                Server:   RegisterUDS,                          *\r\n");
   printf("*                          UnRegisterUDS,                        *\r\n");
   printf("*                          RegisterUser,                         *\r\n");
   printf("*                Client:   DiscoverUDS,                          *\r\n");
   printf("*                          WriteCCCD,                            *\r\n");
   printf("*                          WriteDCI,                             *\r\n");
   printf("*                          WriteUserCP,                          *\r\n");
   printf("*                Both:     Read,                                 *\r\n");
   printf("*                          Write                                 *\r\n");
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
   int                                 Length;
   GAP_LE_Advertising_Parameters_t     AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t  ConnectabilityParameters;
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

            /* Configure the flags field udsed on the Discoverability   */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            if(UDSInstanceID)
            {
               /* Advertise the Battery Server(1 byte type and 2 bytes  */
               /* UUID)                                                 */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               UDS_ASSIGN_UDS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));
            }

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] + 2), &(Advertisement_Data_Buffer.AdvertisingData));
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
   /* mechanism of populating a UDS Client Information structure with   */
   /* the information discovered from a UDS Discovery operation.        */
static void UDSPopulateHandles(UDS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (UDS_COMPARE_UDS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
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
            printf("\r\nUDS Characteristic:\r\n");
            printf("   Handle:       0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
            printf("   Properties:   0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
            printf("   UUID:         0x");
            DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
            printf("   Descriptors:  %u\r\n", CharacteristicInfoPtr->NumberOfDescriptors);
         }

         /* All UDS UUIDs are defined to be 16 bit UUIDs.               */
         if(CharacteristicInfoPtr->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* Check if this is a UDS Characteristic first.             */

            /* First name.                                              */
            if(UDS_COMPARE_FIRST_NAME_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.First_Name = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Last name.                                               */
            if(UDS_COMPARE_LAST_NAME_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Last_Name = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Email Address.                                           */
            if(UDS_COMPARE_EMAIL_ADDRESS_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Email_Address = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Age.                                                     */
            if(UDS_COMPARE_AGE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Age = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Date of Birth.                                           */
            if(UDS_COMPARE_DATE_OF_BIRTH_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Date_Of_Birth = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Gender.                                                  */
            if(UDS_COMPARE_GENDER_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Gender = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Weight.                                                  */
            if(UDS_COMPARE_WEIGHT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Weight = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Height.                                                  */
            if(UDS_COMPARE_HEIGHT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Height = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* VO2 Max.                                                 */
            if(UDS_COMPARE_VO2_MAX_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.VO2_Max = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Heart Rate Max.                                          */
            if(UDS_COMPARE_HEART_RATE_MAX_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Heart_Rate_Max = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Resting Heart Rate.                                      */
            if(UDS_COMPARE_RESTING_HEART_RATE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Resting_Heart_Rate = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Maximum recommended heart rate.                          */
            if(UDS_COMPARE_MAXIMUM_RECOMMENDED_HEART_RATE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Maximum_Recommended_Heart_Rate = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Aerobic Threshold.                                       */
            if(UDS_COMPARE_AEROBIC_THRESHOLD_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Aerobic_Threshold = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Anaerobic Threshold.                                     */
            if(UDS_COMPARE_ANAEROBIC_THRESHOLD_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Anaerobic_Threshold = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Sport Type.                                              */
            if(UDS_COMPARE_SPORT_TYPE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Sport_Type = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Date of Threshold.                                       */
            if(UDS_COMPARE_DATE_OF_THRESHOLD_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Date_Of_Threshold = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Waist Circumference.                                     */
            if(UDS_COMPARE_WAIST_CIRCUMFERENCE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Waist_Circumference = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Hip Circumference.                                       */
            if(UDS_COMPARE_HIP_CIRCUMFERENCE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Hip_Circumference = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Fat Burn Heart Rate Lower Limit.                         */
            if(UDS_COMPARE_FAT_BURN_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Fat_Burn_Heart_Rate_Lower_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Aerobic Heart Rate Lower Limit.                          */
            if(UDS_COMPARE_FAT_BURN_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Fat_Burn_Heart_Rate_Upper_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Aerobic Heart Rate Lower Limit.                          */
            if(UDS_COMPARE_AEROBIC_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Aerobic_Heart_Rate_Lower_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Aerobic Heart Rate Upper Limit.                          */
            if(UDS_COMPARE_AEROBIC_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Aerobic_Heart_Rate_Upper_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Anaerobic Heart Rate Lower Limit.                        */
            if(UDS_COMPARE_ANAEROBIC_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Anaerobic_Heart_Rate_Lower_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Anaerobic Heart Rate Upper Limit.                        */
            if(UDS_COMPARE_ANAEROBIC_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Anaerobic_Heart_Rate_Upper_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Five Zone Heart Rate Limits.                             */
            if(UDS_COMPARE_FIVE_ZONE_HEART_RATE_LIMITS_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Five_Zone_Heart_Rate_Limits = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Three Zone Heart Rate Limits.                            */
            if(UDS_COMPARE_THREE_ZONE_HEART_RATE_LIMITS_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Three_Zone_Heart_Rate_Limits = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Two Zone Heart Rate Limit.                               */
            if(UDS_COMPARE_TWO_ZONE_HEART_RATE_LIMIT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Two_Zone_Heart_Rate_Limit = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Language.                                                */
            if(UDS_COMPARE_LANGUAGE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->UDS_Characteristic.Language = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* Check for the remaining UDS Characteristics.             */

            /* Database Change Increment Characteristic.                */
            if(UDS_COMPARE_DATABASE_CHANGE_INCREMENT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Database_Change_Increment = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               /* * NOTE * The CCCD may not exist if the UDS Server does*/
               /*          not support notifications.                   */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Database_Change_Increment_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* User Index Characteristic.                               */
            if(UDS_COMPARE_USER_INDEX_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                 printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->User_Index = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* User Control Point Characteristic.                       */
            if(UDS_COMPARE_USER_CONTROL_POINT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                 printf("Warning - Mandatory write property not supported!\r\n");

               /* Verify that indicate is supported.                    */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                 printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->User_Control_Point = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->User_Control_Point_CCCD));

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
   /* handles for a UDS Characteristic.                                 */
static void StoreDescriptorHandles(UDS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle)
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

   /* The following function is responsible for registering UDS.        */
static int RegisterUDS(void)
{
   int                   ret_val = 0;
   UDS_Initialize_Data_t InitializeData;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!UDSInstanceID)
      {
         /* We will initialize all features for UDS.                    */
         InitializeData.UDS_Characteristic_Flags = UDS_SA_DEFAULT_CHARACTERISTIC_FLAGS;
         InitializeData.Server_Update_Supported  = TRUE;


         /* Initialize the service.                                     */
         ret_val = UDS_Initialize_Service(BluetoothStackID, (unsigned int)UDS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, UDS_EventCallback, 0, &UDSInstanceID);
         if((ret_val > 0) && (UDSInstanceID > 0))
         {
            /* Display succuds message.                                 */
            printf("Successfully registered UDS Service, UDSInstanceID = %u.\r\n", ret_val);

            /* Save the ServiceID of the registered service.            */
            UDSInstanceID = (unsigned int)ret_val;

            /* Simply return success to the caller.                     */
            ret_val = 0;
         }
         else
            DisplayFunctionError("UDS_Initialize_Service", ret_val);
      }
      else
      {
         printf("\r\nUDS is already registered.\r\n");
      }
   }
   else
   {
      printf("\r\nConnection currently active.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for un-registering UDS.     */
static int UnregisterUDS(void)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that a service is registered.                              */
   if(UDSInstanceID)
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

      /* Unregister the UDS Service with GATT.                          */
      ret_val = UDS_Cleanup_Service(BluetoothStackID, UDSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("Successfully unregistered UDS Service InstanceID %u.\r\n", UDSInstanceID);

         /* Clear the InstanceID.                                       */
         UDSInstanceID = 0;
      }
      else
         DisplayFunctionError("UDS_Cleanup_Service", ret_val);
   }
   else
      printf("\r\nUDS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function is responsible for mapping an attribute    */
   /* handle for a UDS Characteristic based on the UDS Characteristic   */
   /* type for UDS Client requests.                                     */
   /* * NOTE * This function will not set the attribute handle if the   */
   /*          Type is invalid.                                         */
static void MapUDSCharacteristicToAttributeHandle(UDS_Characteristic_Type_t Type, UDS_Characteristic_Handles_t *Handles, Word_t *AttributeHandle)
{
   /* Make sure the parameters are semi-valid.                          */
   if((Handles) && (AttributeHandle))
   {
      /* Determine the UDS Characteristic Type to find the correct      */
      /* attribute handle.                                              */
      switch(Type)
      {
         case uctFirstName:
            *AttributeHandle = Handles->First_Name;
            break;
         case uctLastName:
            *AttributeHandle = Handles->Last_Name;
            break;
         case uctEmailAddress:
            *AttributeHandle = Handles->Email_Address;
            break;
         case uctAge:
            *AttributeHandle = Handles->Age;
            break;
         case uctDateOfBirth:
            *AttributeHandle = Handles->Date_Of_Birth;
            break;
         case uctGender:
            *AttributeHandle = Handles->Gender;
            break;
         case uctWeight:
            *AttributeHandle = Handles->Weight;
            break;
         case uctHeight:
            *AttributeHandle = Handles->Height;
            break;
         case uctVO2Max:
            *AttributeHandle = Handles->VO2_Max;
            break;
         case uctHeartRateMax:
            *AttributeHandle = Handles->Heart_Rate_Max;
            break;
         case uctRestingHeartRate:
            *AttributeHandle = Handles->Resting_Heart_Rate;
            break;
         case uctMaximumRecommendedHeartRate:
            *AttributeHandle = Handles->Maximum_Recommended_Heart_Rate;
            break;
         case uctAerobicThreshold:
            *AttributeHandle = Handles->Aerobic_Threshold;
            break;
         case uctAnaerobicThreshold:
             *AttributeHandle = Handles->Anaerobic_Threshold;
            break;
         case uctSportType:
            *AttributeHandle = Handles->Sport_Type;
            break;
         case uctDateOfThreshold:
            *AttributeHandle = Handles->Date_Of_Threshold;
            break;
         case uctWaistCircumference:
            *AttributeHandle = Handles->Waist_Circumference;
            break;
         case uctHipCircumference:
            *AttributeHandle = Handles->Hip_Circumference;
            break;
         case uctFatBurnHeartRateLowerLimit:
            *AttributeHandle = Handles->Fat_Burn_Heart_Rate_Lower_Limit;
            break;
         case uctFatBurnHeartRateUpperLimit:
            *AttributeHandle = Handles->Fat_Burn_Heart_Rate_Upper_Limit;
            break;
         case uctAerobicHeartRateLowerLimit:
            *AttributeHandle = Handles->Aerobic_Heart_Rate_Lower_Limit;
            break;
         case uctAerobicHeartRateUpperLimit:
            *AttributeHandle = Handles->Aerobic_Heart_Rate_Upper_Limit;
            break;
         case uctAnaerobicHeartRateLowerLimit:
            *AttributeHandle = Handles->Anaerobic_Heart_Rate_Lower_Limit;
            break;
         case uctAnaerobicHeartRateUpperLimit:
            *AttributeHandle = Handles->Anaerobic_Heart_Rate_Upper_Limit;
            break;
         case uctFiveZoneHeartRateLimits:
            *AttributeHandle = Handles->Five_Zone_Heart_Rate_Limits;
            break;
         case uctThreeZoneHeartRateLimits:
            *AttributeHandle = Handles->Three_Zone_Heart_Rate_Limits;
            break;
         case uctTwoZoneHeartRateLimit:
            *AttributeHandle = Handles->Two_Zone_Heart_Rate_Limit;
            break;
         case uctLanguage:
            *AttributeHandle = Handles->Language;
            break;
         default:
            printf("\r\nInvalid UDS Characteristic type.\r\n");
            break;
      }
   }
   else
      printf("\r\nInvalid parameters.\r\n");
}

/* The following function is responsible for reading a UDS              */
/* Characteristic.  This function may be called by the UDS Client or UDS*/
/* Server.  If called by the UDS Client a connection MUST exist to the  */
/* UDS Server.                                                          */
static int ReadUDSCharacteristic(UDS_Request_Data_t *RequestData)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;
   Word_t        AttributeHandle = 0;


   /* If we are the UDS Server.                                         */
   if(UDSInstanceID)
   {
      /* Detemermine the UDS request based on the attribute handle type.*/
      switch(RequestData->AttributeHandleType)
      {
         case ahtUDS_Characteristic:
            /* Make sure the User Index specified is valid.             */
            if(RequestData->UserIndex < (Byte_t)UDS_SA_MAXIMUM_SUPPORTED_USERS)
            {
               /* Simply call the internal function to display the      */
               /* user's UDS Characteristic data based on the specified */
               /* UDS Characteristic type.                              */
               DisplayUDSSampleCharacteristicData(RequestData->Type, &(UserList[RequestData->UserIndex].Data[RequestData->Type]));
            }
            else
               printf("\r\nThe User Index is invalid.\r\n");
            break;
         case ahtDatabase_Change_Increment:
            /* Make sure the User Index specified is valid.             */
            if(RequestData->UserIndex < (Byte_t)UDS_SA_MAXIMUM_SUPPORTED_USERS)
            {
               /* Make sure the user has been registered.  Only the UDS */
               /* Client may register users.                            */
               if(UserList[RequestData->UserIndex].Registered == TRUE)
               {
                  printf("\r\nDatabase Change Increment:\r\n");
                  printf("   Value: 0x%08X\r\n", UserList[RequestData->UserIndex].Database_Change_Increment);
               }
               else
               {
                  /* Zero indicates that the Database Change Increment  */
                  /* is not valid.                                      */
                  printf("\r\nDatabase Change Increment:\r\n");
                  printf("   Value: 0x%08X\r\n", 0);
               }
            }
            else
               printf("\r\nThe User Index is invalid.\r\n");
            break;
         case ahtDatabase_Change_Increment_CCCD:
         case ahtUser_Control_Point_CCCD:
            /* Intentional fall-through since we can handle this request*/
            /* similarly.                                               */

            /* Make sure there is a connection.                         */
            if(ConnectionID)
            {
               /* Get the device info for the connection device.        */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Display the Characteristic based on the attribute  */
                  /* handle type.                                       */
                  if(ahtDatabase_Change_Increment_CCCD)
                  {
                     printf("\r\nDatabase Chance Increment CCCD:\r\n");
                     printf("   Value: 0x%04X\r\n", DeviceInfo->UDSServerInfo.Database_Change_Increment_Configuration);
                  }
                  else
                  {
                     printf("\r\nUser Control Point CCCD:\r\n");
                     printf("   Value: 0x%04X\r\n", DeviceInfo->UDSServerInfo.User_Control_Point_Configuration);
                  }
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nNo connection exists.\r\n");
            break;
         case ahtUser_Index:
            printf("\r\nUser Index:\r\n");
            printf("   Value: 0x%04X\r\n", User_Index);
            break;
         case ahtUser_Control_Point:
            printf("\r\nThe User Control Point CANNOT be read.\r\n");
            break;
         default:
            printf("\r\nInvalid attribute handle type.\r\n");
            break;
      }
   }
   else
   {
      /* Get the device info for the connection device.                 */
      /* * NOTE * ConnectionBD_ADDR should be valid if ConnectionID is. */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Make sure that a connection exists.                         */
         if(ConnectionID)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Detemermine the UDS request based on the attribute    */
               /* handle type.                                          */
               switch(RequestData->AttributeHandleType)
               {
                  case ahtUDS_Characteristic:
                     /* Simply call the internal function to map the    */
                     /* discovered UDS Characteristic attribute handle  */
                     /* based on the specified UDS Characteristic type. */
                     MapUDSCharacteristicToAttributeHandle(RequestData->Type, &(DeviceInfo->UDSClientInfo.UDS_Characteristic), &AttributeHandle);
                     break;
                  case ahtDatabase_Change_Increment:
                     AttributeHandle = DeviceInfo->UDSClientInfo.Database_Change_Increment;
                     break;
                  case ahtDatabase_Change_Increment_CCCD:
                     AttributeHandle = DeviceInfo->UDSClientInfo.Database_Change_Increment_CCCD;
                     break;
                  case ahtUser_Index:
                     AttributeHandle = DeviceInfo->UDSClientInfo.User_Index;
                     break;
                  case ahtUser_Control_Point:
                     printf("\r\nThe User Control Point CANNOT be read.\r\n");
                     break;
                  case ahtUser_Control_Point_CCCD:
                     AttributeHandle = DeviceInfo->UDSClientInfo.User_Control_Point_CCCD;
                     break;
                  default:
                     printf("\r\nInvalid attribute handle type.\r\n");
                     break;
               }

               /* Check to make sure the attribute handle is valid.     */
               if(AttributeHandle)
               {
                  /* Let's go ahead and store the request information so*/
                  /* we can handle the response.                        */
                  DeviceInfo->UDSRequestData.AttributeHandleType = RequestData->AttributeHandleType;
                  DeviceInfo->UDSRequestData.AttributeHandle     = AttributeHandle;

                  /* If this is a UDS Characteristic request we need to */
                  /* store the UDS Characteristic type so we can        */
                  /* correctly handle the response.                     */
                  if(RequestData->AttributeHandleType == ahtUDS_Characteristic)
                  {
                     DeviceInfo->UDSRequestData.Type             = RequestData->Type;
                  }

                  /* If this is a read request for a value that may     */
                  /* exceed the GATT Read response size we need to      */
                  /* handle this case separately.  This will be the     */
                  /* First Name, Last Name, Email Address, or Language  */
                  /* UDS Characteristics, which have a variable length. */
                  if((RequestData->AttributeHandleType == ahtUDS_Characteristic) &&
                     ((RequestData->Type == uctFirstName) ||
                      (RequestData->Type == uctLastName) ||
                      (RequestData->Type == uctEmailAddress) ||
                      (RequestData->Type == uctLanguage)))
                  {
                     /* Determine if this a GATT Read or GATT Read Long */
                     /* request.                                        */
                     /* * NOTE * A GATT read request MUST be peformed   */
                     /*          before a subsequent GATT Read Long may */
                     /*          be performed.                          */
                     if(!(RequestData->ReadLong))
                     {
                        /* We will always reset the response buffer so  */
                        /* that the buffer is initially empty for a GATT*/
                        /* Read Long response.                          */
                        /* * NOTE * This is the offset for the GATT Read*/
                        /*          Long request if it needs to be      */
                        /*          issued and will be updated when the */
                        /*          GATT Read response is received.     */
                        DeviceInfo->UDSRequestData.BufferLength = 0;

                        /* Send the GATT read request.                  */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Read Value Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n",     ret_val);
                           printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                        }
                        else
                           DisplayFunctionError("GATT_Read_Value_Request", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                     {
                        /* Make sure a GATT Read response has previously*/
                        /* updated the current length of the response   */
                        /* buffer.  This will also be the next offset to*/
                        /* read from.                                   */
                        if(DeviceInfo->UDSRequestData.BufferLength)
                        {
                           /* Send the GATT read request.               */
                           /* * NOTE * We will not save the             */
                           /*          transactionID returned by this   */
                           /*          function, which we could use to  */
                           /*          cancel the request.              */
                           if((ret_val = GATT_Read_Long_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, DeviceInfo->UDSRequestData.BufferLength, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                           {
                              printf("\r\nGATT Read Long Value Request sent:\r\n");
                              printf("   TransactionID:     %d\r\n",     ret_val);
                              printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                           }
                           else
                              DisplayFunctionError("GATT_Read_Long_Value_Request", ret_val);

                           /* Simply return success to the caller.      */
                           ret_val = 0;
                        }
                        else
                           printf("\r\nA GATT Read request MUST be performed before a GATT Read Long request.\r\n");
                     }
                  }
                  else
                  {
                     /* Send the GATT read request.                     */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
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
               }
               else
                  printf("\r\nInvalid attribute handle.\r\n");
            }
            else
               printf("\r\nService discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo connection exists.\r\n");
      }
      else
         printf("\r\nNo device information.\r\n");
   }

   return ret_val;
}

   /* The following function is responsible for writing a UDS           */
   /* Characteristic.  This function may be called by the UDS Client or */
   /* UDS Server.  If called by the UDS Client a connection MUST exist  */
   /* to the UDS Server.                                                */
static int WriteUDSCharacteristic(UDS_Request_Data_t *RequestData)
{
   int                ret_val = 0;
   DeviceInfo_t      *DeviceInfo;
   Word_t             AttributeHandle = 0;
   NonAlignedWord_t   Client_Configuration;
   NonAlignedDWord_t  Database_Change_Increment;
   Word_t             BufferLength;
   Byte_t            *Buffer;
   Word_t             MTU;

   /* If we are the UDS Server.                                         */
   if(UDSInstanceID)
   {
      /* Detemermine the UDS request based on the attribute handle type.*/
      switch(RequestData->AttributeHandleType)
      {
         case ahtUDS_Characteristic:
            /* Make sure the User Index specified is valid.             */
            if(RequestData->UserIndex < (Byte_t)UDS_SA_MAXIMUM_SUPPORTED_USERS)
            {
               /* Simply call the internal function to store the UDS    */
               /* Characteristic data into the user data based on the   */
               /* specified UDS Characteristic type.                    */
               StoreUDSCharacteristicData(&(UserList[RequestData->UserIndex]), RequestData->Type, &(RequestData->Data.Characteristic));
            }
            else
               printf("\r\nThe User Index is invalid.\r\n");
            break;
         case ahtDatabase_Change_Increment:
            /* We will not allow the UDS Server to directly set the     */
            /* Database Change Increment.  Instead it will be set when a*/
            /* UDS Characteristic is updated and will be notified to the*/
            /* UDS Client.                                              */
            printf("\r\nDatabase Change Increment CANNOT be written.\r\n");
            break;
         case ahtDatabase_Change_Increment_CCCD:
            printf("\r\nThe Database Change Increment CCCD CANNOT be written.\r\n");
            break;
         case ahtUser_Index:
            printf("\r\nThe User Index CANNOT be written.\r\n");
            break;
         case ahtUser_Control_Point:
            printf("\r\nThe User Control Point CANNOT be written.\r\n");
            break;
         case ahtUser_Control_Point_CCCD:
            printf("\r\nThe User Control Point CCCD CANNOT be written.\r\n");
            break;
         default:
            printf("\r\nInvalid attribute handle type.\r\n");
            break;
      }
   }
   else
   {
      /* Make sure that a connection exists.                            */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         /* * NOTE * ConnectionBD_ADDR should be valid if ConnectionID  */
         /*          is.                                                */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Detemermine the UDS request based on the attribute    */
               /* handle type.                                          */
               switch(RequestData->AttributeHandleType)
               {
                  case ahtUDS_Characteristic:
                     /* Simply call the internal function to map the    */
                     /* discovered UDS Characteristic attribute handle  */
                     /* based on the specified UDS Characteristic type. */
                     MapUDSCharacteristicToAttributeHandle(RequestData->Type, &(DeviceInfo->UDSClientInfo.UDS_Characteristic), &AttributeHandle);

                     /* Make sure the attrubute handle is valid.        */
                     if(AttributeHandle)
                     {
                        /* Store the request data so we can handle the  */
                        /* GATT response.                               */
                        DeviceInfo->UDSRequestData = *RequestData;

                        /* Determine how to format and send the UDS     */
                        /* Characteristic.                              */
                        /* * NOTE * UTF-8 string datay types are already*/
                        /*          formmated.                          */
                        switch(DeviceInfo->UDSRequestData.Type)
                        {
                           case uctFirstName:
                           case uctLastName:
                           case uctEmailAddress:
                           case uctLanguage:
                              /* Intentional fall-through.              */
                              /* * NOTE * We will use the First Name    */
                              /*          Characteristic since the      */
                              /*          strings will align.           */

                              /* Store the attribute handle needed for  */
                              /* the GATT response.                     */
                              DeviceInfo->UDSRequestData.AttributeHandle = AttributeHandle;

                              /* Query the GATT MTU so we can determine */
                              /* if we can send the string in a GATT    */
                              /* Write request or a GATT Prepare Write  */
                              /* request.                               */
                              if((ret_val = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                              {
                                 /* We can only send a GATT Write       */
                                 /* request if the string length is less*/
                                 /* than or equal to the (ATT_MTU-3)    */
                                 /* size.                               */
                                 if(RequestData->Data.Characteristic.First_Name.Buffer_Length <= (MTU-3))
                                 {
                                    /* Send the GATT Write request.     */
                                    if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, RequestData->Data.Characteristic.First_Name.Buffer_Length, RequestData->Data.Characteristic.First_Name.Buffer, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Write Request sent:\r\n");
                                       printf("   Type:              ");
                                       DisplayUDSCharacteristicType(RequestData->Type);
                                       printf("   TransactionID:     %d\r\n", ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Write_Request", ret_val);
                                 }
                                 else
                                 {
                                    /* Other wise we will simply send   */
                                    /* the GATT Prepare Write request to*/
                                    /* prepare the UDS Characteristic to*/
                                    /* be written.                      */
                                    /* * NOTE * We will need to call the*/
                                    /*          GATT_Execute_Write_Reque*/
                                    /*          function after the data */
                                    /*          has been prepared to    */
                                    /*          actually write the value*/
                                    /*          or cancel the GATT      */
                                    /*          Prepare Write request at*/
                                    /*          any time.               */

                                    /* Set the buffer offset to zero for*/
                                    /* the first request.               */
                                    /* * NOTE * This will be updated for*/
                                    /*          each GATT Prepare Write */
                                    /*          response that is        */
                                    /*          received.               */
                                    DeviceInfo->UDSRequestData.BufferOffset = 0;

                                    /* Copy the string into the buffer  */
                                    /* since we will need to accesss it */
                                    /* for each subsequent GATT Prepare */
                                    /* Write request that may be sent   */
                                    /* when the GATT Prepare Write      */
                                    /* response is received.            */
                                    DeviceInfo->UDSRequestData.BufferLength = RequestData->Data.Characteristic.First_Name.Buffer_Length;
                                    BTPS_MemCopy(DeviceInfo->UDSRequestData.Buffer, RequestData->Data.Characteristic.First_Name.Buffer, RequestData->Data.Characteristic.First_Name.Buffer_Length);

                                    /* Send the GATT Prepare Write      */
                                    /* request.                         */
                                    /* * NOTE * Offset will always be 0 */
                                    /*          for the first request.  */
                                    if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, RequestData->Data.Characteristic.First_Name.Buffer_Length, 0, RequestData->Data.Characteristic.First_Name.Buffer, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                                    {
                                       printf("\r\nGATT Prepare Write Request sent:\r\n");
                                       printf("   Type:              ");
                                       DisplayUDSCharacteristicType(RequestData->Type);
                                       printf("   TransactionID:     %d\r\n", ret_val);
                                       printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                    }
                                    else
                                       DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                 }
                              }
                              else
                                 DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
                              break;
                           default:
                              /* Determine the size of the buffer needed*/
                              /* to hold the formatted UDS              */
                              /* Characteristic.                        */
                              if((ret_val = UDS_Format_UDS_Characteristic_Request(RequestData->Type, &(RequestData->Data.Characteristic), 0, NULL)) > 0)
                              {
                                 /* Store the buffer length.            */
                                 BufferLength = (Word_t)ret_val;

                                 /* Allocate memory for the buffer.     */
                                 if((Buffer = (Byte_t *)BTPS_AllocateMemory(ret_val * BYTE_SIZE)) != NULL)
                                 {
                                    /* Format the UDS Characteristic.   */
                                    if((ret_val = UDS_Format_UDS_Characteristic_Request(RequestData->Type, &(RequestData->Data.Characteristic), BufferLength, Buffer)) == 0)
                                    {
                                       /* Send the GATT Write request.  */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   Type:              ");
                                          DisplayUDSCharacteristicType(RequestData->Type);
                                          printf("   TransactionID:     %d\r\n", ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       DisplayFunctionError("UDS_Format_UDS_Characteristic_Request", ret_val);

                                    /* Free the memory.                 */
                                    BTPS_FreeMemory(Buffer);
                                    Buffer = NULL;
                                 }
                                 else
                                    printf("\r\nInsufficient resources.\r\n");
                              }
                              else
                                 DisplayFunctionError("UDS_Format_UDS_Characteristic_Request", ret_val);
                              break;
                        }

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        printf("\r\nInvalid attribute handle.\r\n");
                     break;
                  case ahtDatabase_Change_Increment:
                     AttributeHandle = DeviceInfo->UDSClientInfo.Database_Change_Increment;

                     /* Make sure the attrubute handle is valid.        */
                     if(AttributeHandle)
                     {
                        /* Format the Database Change Icnrement.        */
                        ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(&Database_Change_Increment, RequestData->Data.DatabaseChangeIncrement);

                        /* Send the write request.                      */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_DWORD_SIZE, (Byte_t *)&Database_Change_Increment, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Write Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n", ret_val);
                           printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                        }
                        else
                           DisplayFunctionError("GATT_Write_Request", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        printf("\r\nInvalid attribute handle.\r\n");
                     break;
                  case ahtDatabase_Change_Increment_CCCD:
                  case ahtUser_Control_Point_CCCD:
                     /* Intentional fall-through since we can handle    */
                     /* these requests similarly.                       */

                     /* Set the attribute handle based on the attribute */
                     /* handle type.                                    */
                     if(RequestData->AttributeHandleType == ahtDatabase_Change_Increment_CCCD)
                        AttributeHandle = DeviceInfo->UDSClientInfo.Database_Change_Increment_CCCD;
                     else
                        AttributeHandle = DeviceInfo->UDSClientInfo.User_Control_Point_CCCD;

                     /* Make sure the attrubute handle is valid.        */
                     if(AttributeHandle)
                     {
                        /* Format the client configuration.             */
                        ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Client_Configuration, RequestData->Data.ClientConfiguration);

                        /* Send the write request.                      */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Client_Configuration, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Write Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n", ret_val);
                           printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                        }
                        else
                           DisplayFunctionError("GATT_Write_Request", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        printf("\r\nInvalid attribute handle.\r\n");
                     break;
                  case ahtUser_Index:
                     printf("\r\nThe User Index CANNOT be written.\r\n");
                     break;
                  case ahtUser_Control_Point:
                     AttributeHandle = DeviceInfo->UDSClientInfo.User_Control_Point;

                     /* Make sure the attrubute handle is valid.        */
                     if(AttributeHandle)
                     {
                        /* Determine the size of the buffer needed to   */
                        /* hold the formatted UDS Characteristic.       */
                        if((ret_val = UDS_Format_User_Control_Point_Request(&(RequestData->Data.RequestData), 0, NULL)) > 0)
                        {
                           /* Store the buffer length.                  */
                           BufferLength = (Word_t)ret_val;

                           /* Allocate memory for the buffer.           */
                           if((Buffer = (Byte_t *)BTPS_AllocateMemory(ret_val * BYTE_SIZE)) != NULL)
                           {
                              /* Format the UDS Characteristic.         */
                              if((ret_val = UDS_Format_User_Control_Point_Request(&(RequestData->Data.RequestData), BufferLength, Buffer)) == 0)
                              {
                                 /* Send the GATT Write request.        */
                                 if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                 }
                                 else
                                    DisplayFunctionError("GATT_Write_Request", ret_val);
                              }
                              else
                                 DisplayFunctionError("UDS_Format_User_Control_Point_Request", ret_val);

                              /* Free the memory.                       */
                              BTPS_FreeMemory(Buffer);
                              Buffer = NULL;
                           }
                           else
                              printf("\r\nInsufficient resources.\r\n");
                        }
                        else
                           DisplayFunctionError("UDS_Format_User_Control_Point_Request", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        printf("\r\nInvalid attribute handle.\r\n");
                     break;
                  default:
                     printf("\r\nInvalid attribute handle type.\r\n");
                     break;
               }
            }
            else
               printf("\r\nService discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nNo connection exists.\r\n");
   }

   return ret_val;
}

   /* The following function is responsible for storing the UDS         */
   /* Characteristic data into a user's data.                           */
static void StoreUDSCharacteristicData(UDS_User_Data_t *UserData, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic)
{
   int     Result;
   Byte_t *Buffer;
   Word_t  BufferLength;

   /* Use the UDS Characteristic type to store the data.                */
   switch(Type)
   {
      case uctFirstName:
      case uctLastName:
      case uctEmailAddress:
      case uctLanguage:
         /* Intentional fall-through since these events align.          */
         /* * NOTE * We will simply use the First Name.                 */

         /* Store the fields to make this more readable.                */
         Buffer       = UDS_Characteristic->First_Name.Buffer;
         BufferLength = UDS_Characteristic->First_Name.Buffer_Length;

         /* Copy the buffer and update the length.                      */
         BTPS_MemCopy(UserData->Data[Type].First_Name.Buffer, Buffer, BufferLength);
         UserData->Data[Type].First_Name.Buffer_Length = BufferLength;

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string if we choose to read the current value later.    */
         /* * NOTE * We will not include this as part of the length so  */
         /*          the NULL terminator is not sent.                   */
         UserData->Data[Type].First_Name.Buffer[BufferLength] = '\0';
         break;
      case uctDateOfBirth:
      case uctDateOfThreshold:
         /* Intentional fall-through since these events align.          */
         /* * NOTE * We will simply use the Date of Birth.              */

         /* Store the data.                                             */
         UserData->Data[Type].Date_Of_Birth = UDS_Characteristic->Date_Of_Birth;
         break;
      case uctAge:
      case uctGender:
      case uctVO2Max:
      case uctHeartRateMax:
      case uctRestingHeartRate:
      case uctMaximumRecommendedHeartRate:
      case uctAerobicThreshold:
      case uctAnaerobicThreshold:
      case uctSportType:
      case uctFatBurnHeartRateLowerLimit:
      case uctFatBurnHeartRateUpperLimit:
      case uctAerobicHeartRateLowerLimit:
      case uctAerobicHeartRateUpperLimit:
      case uctAnaerobicHeartRateLowerLimit:
      case uctAnaerobicHeartRateUpperLimit:
      case uctTwoZoneHeartRateLimit:
         /* Intentional fall-through since these events align.          */
         /* * NOTE * We will simply use the Age.                        */

         /* Store the data.                                             */
         UserData->Data[Type].Age = UDS_Characteristic->Age;
         break;
      case uctWeight:
      case uctHeight:
      case uctWaistCircumference:
      case uctHipCircumference:
         /* Intentional fall-through since these events align.          */
         /* * NOTE * We will simply use the Weight.                     */

         /* Store the data.                                             */
         UserData->Data[Type].Weight = UDS_Characteristic->Weight;
         break;
      case uctFiveZoneHeartRateLimits:
         /* Simply store the data.                                      */
         UserData->Data[Type].Five_Zone_Heart_Rate_Limits = UDS_Characteristic->Five_Zone_Heart_Rate_Limits;
         break;
      case uctThreeZoneHeartRateLimits:
         /* Simply store the data.                                      */
         UserData->Data[Type].Three_Zone_Heart_Rate_Limits = UDS_Characteristic->Three_Zone_Heart_Rate_Limits;
         break;
      default:
         /* Shoudn't occur.                                             */
         printf("\r\nUnknown UDS characteristic type received.\r\n");
         break;
   }

   /* We will update the User's Database Change Increment each time a   */
   /* UDS Characteristic is updated by the UDS Server.  This could also */
   /* be incremented once a number of UDS Characteristics have been     */
   /* updated and the UDS Server interface has a save feature.          */
   /* * NOTE * We will increment this for the UDS Client, however the   */
   /*          UDS Client really should be the one to update this value */
   /*          after the UDS Client writes a UDS Characteristic.        */
   UserData->Database_Change_Increment++;

   /* Make sure we are connected to the UDS Client.                     */
   /* * NOTE * We could loop through the device information and notify  */
   /*          all UDS Clients, however this sample application only    */
   /*          supports one UDS Client.                                 */
   if(ConnectionID)
   {
      /* Notify the UDS Client.                                         */
      /* * NOTE * We have no gurantee that the UDS Client received the  */
      /*          notification even if this function is successful.     */
      if((Result = UDS_Notify_Database_Change_Increment(BluetoothStackID, UDSInstanceID, ConnectionID, UserData->Database_Change_Increment)) > 0)
      {
         printf("\r\nDatabase Change Increment notification sent:\r\n");
         printf("   Length: %d\r\n", Result);
      }
   }

}

   /* The following function is responsible for formatting the UDS      */
   /* Characteristic data into a user's data.                           */
static void FormatUDSCharacteristicData(UDS_User_Data_t *UserData, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic)
{
   /* Use the UDS Characteristic type to store the data.                */
   switch(Type)
   {
      case uctFirstName:
      case uctLastName:
      case uctEmailAddress:
      case uctLanguage:
         /* Intentional fall-through since these events align.          */

         /* Simply assign a pointer to the buffer and set the buffer    */
         /* length.                                                     */
         /* * NOTE * We will simply use the First Name since the above  */
         /*          types align.                                       */
         UDS_Characteristic->First_Name.Buffer        = UserData->Data[Type].First_Name.Buffer;
         UDS_Characteristic->First_Name.Buffer_Length = UserData->Data[Type].First_Name.Buffer_Length;
         break;
      case uctDateOfBirth:
      case uctDateOfThreshold:
         /* Intentional fall-through since these events align.          */

         /* Simply store the value.                                     */
         /* * NOTE * We will simply use the Date of Birth since the     */
         /*          above types align.                                 */
         UDS_Characteristic->Date_Of_Birth = UserData->Data[Type].Date_Of_Birth;
         break;
      case uctAge:
      case uctGender:
      case uctVO2Max:
      case uctHeartRateMax:
      case uctRestingHeartRate:
      case uctMaximumRecommendedHeartRate:
      case uctAerobicThreshold:
      case uctAnaerobicThreshold:
      case uctSportType:
      case uctFatBurnHeartRateLowerLimit:
      case uctFatBurnHeartRateUpperLimit:
      case uctAerobicHeartRateLowerLimit:
      case uctAerobicHeartRateUpperLimit:
      case uctAnaerobicHeartRateLowerLimit:
      case uctAnaerobicHeartRateUpperLimit:
      case uctTwoZoneHeartRateLimit:
         /* Intentional fall-through since these events align.          */

         /* Simply store the value.                                     */
         /* * NOTE * We will simply use the Age since the above types   */
         /*          align.                                             */
         UDS_Characteristic->Age = UserData->Data[Type].Age;
         break;
      case uctWeight:
      case uctHeight:
      case uctWaistCircumference:
      case uctHipCircumference:
         /* Intentional fall-through since these events align.          */

         /* Simply store the value.                                     */
         /* * NOTE * We will simply use the Weight since the above types*/
         /*          align.                                             */
         UDS_Characteristic->Weight = UserData->Data[Type].Weight;
         break;
      case uctFiveZoneHeartRateLimits:
         /* Intentional fall-through since these events align.          */

         /* Simply store the value.                                     */
         UDS_Characteristic->Five_Zone_Heart_Rate_Limits = UserData->Data[Type].Five_Zone_Heart_Rate_Limits;
         break;
      case uctThreeZoneHeartRateLimits:
         /* Intentional fall-through since these events align.          */

         /* Simply store the value.                                     */
         UDS_Characteristic->Three_Zone_Heart_Rate_Limits = UserData->Data[Type].Three_Zone_Heart_Rate_Limits;
         break;
      default:
         /* Shoudn't occur.                                             */
         printf("\r\nUnknown UDS characteristic type received.\r\n");
         break;
   }
}

      /* The following function is responsible for executing the        */
      /* Register New User procedure.                                   */
static void RegisterNewUserProcedure(UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData)
{
   unsigned int Index;

   /* Make sure the parameters are semi-valid.                          */
   if((RequestData) && (ResponseData))
   {
      /* Set the response data Request Op Code.                         */
      ResponseData->Request_Op_Code = RequestData->Op_Code;

      /* Make sure the Consent Code is in the valid range.              */
      if(UDS_CONSENT_CODE_VALID(RequestData->Parameter.Consent_Code))
      {
         /* Find the first un-registered user.                          */
         /* * NOTE * The response data Response Code Value is already   */
         /*          set to Operation Failed by the caller so we only   */
         /*          need to set succcess if we register a user.  This  */
         /*          MUST be returned if we do not have room for a new  */
         /*          user.                                              */
         for(Index = 0; Index < (unsigned int)UDS_SA_MAXIMUM_SUPPORTED_USERS; Index++)
         {
            /* If we find an un-registered user.                        */
            if(UserList[Index].Registered == FALSE)
            {
               /* Flag that this user is valid and store the consent    */
               /* code so we can verify it in the consent procedure.    */
               UserList[Index].Registered                = TRUE;
               UserList[Index].Consent_Code              = RequestData->Parameter.Consent_Code;
               UserList[Index].Consent_Attempts          = 3;
               UserList[Index].Database_Change_Increment = 0;

               /* Set the response data.                                */
               ResponseData->Response_Code_Value  = rcvUDSSuccess;
               ResponseData->Parameter.User_Index = (Byte_t)Index;

               break;
            }
         }
      }
      else
         ResponseData->Response_Code_Value = rcvUDSInvalidParameter;
   }
}

   /* The following function is responsible for executing the Consent   */
   /* procedure.                                                        */
static void ConsentProcedure(DeviceInfo_t *DeviceInfo, UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData)
{
   /* Make sure the parameters are semi-valid.                          */
   if((RequestData) && (ResponseData))
   {
      /* Set the response data Request Op Code.                         */
      ResponseData->Request_Op_Code = RequestData->Op_Code;

      /* Make sure the User Index is valid.                             */
      if(RequestData->Parameter.User_Index < (Byte_t)UDS_SA_MAXIMUM_SUPPORTED_USERS)
      {
         /* Make sure the user is registered.                           */
         if(UserList[RequestData->Parameter.User_Index].Registered)
         {
            /* Make sure the Consent Code is in the valid range.        */
            if(UDS_CONSENT_CODE_VALID(RequestData->Parameter.Consent_Code))
            {
               /* Validate the Consent Code.                            */
               if(UserList[RequestData->Parameter.User_Index].Consent_Code == RequestData->Parameter.Consent_Code)
               {
                  /* Mark that the UDS Client has consent for the       */
                  /* specified User Index.                              */
                  DeviceInfo->Consent[RequestData->Parameter.User_Index] = TRUE;

                  /* Update the User Index to the current user.         */
                  User_Index = RequestData->Parameter.User_Index;

                  /* Reset the number of Consent Attempts.              */
                  UserList[RequestData->Parameter.User_Index].Consent_Attempts = 3;

                  /* Set the Response Code Value.                       */
                  ResponseData->Response_Code_Value = rcvUDSSuccess;
               }
               else
               {
                  /* Decrement the number of attempts.                  */
                  if(UserList[RequestData->Parameter.User_Index].Consent_Attempts == 0)
                  {
                     /* We will always sent operation failed if UDS     */
                     /* Client does not have any more consent attemps.  */
                     ResponseData->Response_Code_Value = rcvUDSOperationFailed;
                  }
                  else
                  {
                     ResponseData->Response_Code_Value = rcvUDSUserNotAuthorized;

                     /* Decrement the number of Consent Attempts.       */
                     UserList[RequestData->Parameter.User_Index].Consent_Attempts--;
                  }
               }
            }
            else
               ResponseData->Response_Code_Value = rcvUDSInvalidParameter;
         }
         else
            ResponseData->Response_Code_Value = rcvUDSUserNotAuthorized;
      }
      else
         ResponseData->Response_Code_Value = rcvUDSUserNotAuthorized;
   }
}

   /* The following function is responsible for executing the Delete    */
   /* User Data procedure.                                              */
static void DeleteUserDataProcedure(UDS_User_Control_Point_Request_Data_t *RequestData, UDS_User_Control_Point_Response_Data_t *ResponseData)
{
   /* Make sure the parameters are semi-valid.                          */
   if((RequestData) && (ResponseData))
   {
      /* Set the response data Request Op Code.                         */
      ResponseData->Request_Op_Code = RequestData->Op_Code;

      /* Make sure the user is selected.                                */
      if(User_Index != UDS_USER_INDEX_UNKNOWN_USER)
      {
         /* Simply mark that the user is no longer valid.               */
         UserList[User_Index].Registered = 0;

         /* Set the User Index to 'Unknown User' since a user is no     */
         /* longer selected.                                            */
         User_Index = UDS_USER_INDEX_UNKNOWN_USER;

         /* Set the Response Code Value.                                */
         ResponseData->Response_Code_Value = rcvUDSSuccess;
      }
      else
         ResponseData->Response_Code_Value = rcvUDSUserNotAuthorized;
   }
}

   /* This function is a helper function for displaying the UDS         */
   /* Characteristic type.                                              */
static void DisplayUDSCharacteristicType(UDS_Characteristic_Type_t Type)
{
   /* Display the data based on the type.                               */
   switch(Type)
   {
      case uctFirstName:
         printf("uctFirstName\r\n");
         break;
      case uctLastName:
         printf("uctLastName\r\n");
         break;
      case uctEmailAddress:
         printf("uctEmailAddress\r\n");
         break;
      case uctAge:
         printf("uctAge\r\n");
         break;
      case uctDateOfBirth:
         printf("uctDateOfBirth\r\n");
         break;
      case uctGender:
         printf("uctGender\r\n");
         break;
      case uctWeight:
         printf("uctWeight\r\n");
         break;
      case uctHeight:
         printf("uctHeight\r\n");
         break;
      case uctVO2Max:
         printf("uctVO2Max\r\n");
         break;
      case uctHeartRateMax:
         printf("uctHeartRateMax\r\n");
         break;
      case uctRestingHeartRate:
         printf("uctRestingHeartRate\r\n");
         break;
      case uctMaximumRecommendedHeartRate:
         printf("uctMaximumRecommendedHeartRate\r\n");
         break;
      case uctAerobicThreshold:
         printf("uctAerobicThreshold\r\n");
         break;
      case uctAnaerobicThreshold:
         printf("uctAnaerobicThreshold\r\n");
         break;
      case uctSportType:
         printf("uctSportType\r\n");
         break;
      case uctDateOfThreshold:
         printf("uctDateOfThreshold\r\n");
         break;
      case uctWaistCircumference:
         printf("uctWaistCircumference\r\n");
         break;
      case uctHipCircumference:
         printf("uctHipCircumference\r\n");
         break;
      case uctFatBurnHeartRateLowerLimit:
         printf("uctFatBurnHeartRateLowerLimit\r\n");
         break;
      case uctFatBurnHeartRateUpperLimit:
         printf("uctFatBurnHeartRateUpperLimit\r\n");
         break;
      case uctAerobicHeartRateLowerLimit:
         printf("uctAerobicHeartRateLowerLimit\r\n");
         break;
      case uctAerobicHeartRateUpperLimit:
         printf("uctAerobicHeartRateUpperLimit\r\n");
         break;
      case uctAnaerobicHeartRateLowerLimit:
         printf("uctAnaerobicHeartRateLowerLimit\r\n");
         break;
      case uctAnaerobicHeartRateUpperLimit:
         printf("uctAnaerobicHeartRateUpperLimit\r\n");
         break;
      case uctFiveZoneHeartRateLimits:
         printf("uctFiveZoneHeartRateLimits\r\n");
         break;
      case uctThreeZoneHeartRateLimits:
         printf("uctThreeZoneHeartRateLimits\r\n");
         break;
      case uctTwoZoneHeartRateLimit:
         printf("uctTwoZoneHeartRateLimit\r\n");
         break;
      case uctLanguage:
         printf("uctLanguage\r\n");
         break;
      default:
         printf("Invalid\r\n");
         break;
   }
}

   /* This function is a helper function that simply prints the UDS     */
   /* Characteristic data.                                              */
static void DisplayUDSCharacteristicData(UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic)
{
   Byte_t  Buffer[UDS_SA_MAX_BUFFER_SIZE+1];
   Word_t  BufferLength;

   printf("\r\nUDS Characteristic:\r\n");

   /* Display the data based on the type.                               */
   switch(Type)
   {
      case uctFirstName:
         printf("   Type:       ");
         DisplayUDSCharacteristicType(Type);

         /* Simply copy the string into the buffer.                     */
         BufferLength = UDS_Characteristic->First_Name.Buffer_Length;
         BTPS_MemCopy(Buffer, UDS_Characteristic->First_Name.Buffer, BufferLength);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         Buffer[BufferLength] = '\0';

         /* Display the string.                                         */
         printf("   First Name: \"%s\"\r\n", Buffer);
         break;
      case uctLastName:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);

         /* Simply copy the string into the buffer.                     */
         BufferLength = UDS_Characteristic->Last_Name.Buffer_Length;
         BTPS_MemCopy(Buffer, UDS_Characteristic->Last_Name.Buffer, BufferLength);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         Buffer[BufferLength] = '\0';

         /* Display the string.                                         */
         printf("   Last Name: \"%s\"\r\n", Buffer);
         break;
      case uctEmailAddress:
         printf("   Type:          ");
         DisplayUDSCharacteristicType(Type);

         /* Simply copy the string into the buffer.                     */
         BufferLength = UDS_Characteristic->Email_Address.Buffer_Length;
         BTPS_MemCopy(Buffer, UDS_Characteristic->Email_Address.Buffer, BufferLength);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         Buffer[BufferLength] = '\0';

         /* Display the string.                                         */
         printf("   Email Address: \"%s\"\r\n", Buffer);
         break;
      case uctAge:
         printf("   Type: ");
         DisplayUDSCharacteristicType(Type);
         printf("   Age:  %u.\r\n", UDS_Characteristic->Age);
         break;
      case uctDateOfBirth:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Data of Birth:\r\n");
         printf("      Year:   %u\r\n", UDS_Characteristic->Date_Of_Birth.Year);
         printf("      Month:  %u\r\n", UDS_Characteristic->Date_Of_Birth.Month);
         printf("      Day:    %u\r\n", UDS_Characteristic->Date_Of_Birth.Day);
         break;
      case uctGender:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Gender: %s.\r\n", (UDS_Characteristic->Gender) ? "Female" : "Male");
         break;
      case uctWeight:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Weight: %u.\r\n", UDS_Characteristic->Weight);
         break;
      case uctHeight:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Height: %u.\r\n", UDS_Characteristic->Height);
         break;
      case uctVO2Max:
         printf("   Type:    ");
         DisplayUDSCharacteristicType(Type);
         printf("   VO2 Max: %u.\r\n", UDS_Characteristic->VO2_Max);
         break;
      case uctHeartRateMax:
         printf("   Type:           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Heart Rate Max: %u.\r\n", UDS_Characteristic->Heart_Rate_Max);
         break;
      case uctRestingHeartRate:
         printf("   Type:               ");
         DisplayUDSCharacteristicType(Type);
         printf("   Resting Heart Rate: %u.\r\n", UDS_Characteristic->Resting_Heart_Rate);
         break;
      case uctMaximumRecommendedHeartRate:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Maximum Recommended Heart Rate: %u.\r\n", UDS_Characteristic->Maximum_Recommended_Heart_Rate);
         break;
      case uctAerobicThreshold:
         printf("   Type:              ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Threshold: %u.\r\n", UDS_Characteristic->Aerobic_Threshold);
         break;
      case uctAnaerobicThreshold:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Threshold: %u.\r\n", UDS_Characteristic->Anaerobic_Threshold);
         break;
      case uctSportType:
         printf("   Type:       ");
         DisplayUDSCharacteristicType(Type);
         printf("   Sport Type: %u.\r\n", UDS_Characteristic->Sport_Type);
         break;
      case uctDateOfThreshold:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Data of Threshold:\r\n");
         printf("      Year:   %u\r\n", UDS_Characteristic->Date_Of_Threshold.Year);
         printf("      Month:  %u\r\n", UDS_Characteristic->Date_Of_Threshold.Month);
         printf("      Day:    %u\r\n", UDS_Characteristic->Date_Of_Threshold.Day);
         break;
      case uctWaistCircumference:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Waist circumference: %u.\r\n", UDS_Characteristic->Waist_Circumference);
         break;
      case uctHipCircumference:
         printf("   Type:              ");
         DisplayUDSCharacteristicType(Type);
         printf("   Hip circumference: %u.\r\n", UDS_Characteristic->Hip_Circumference);
         break;
      case uctFatBurnHeartRateLowerLimit:
         printf("   Type:                            ");
         DisplayUDSCharacteristicType(Type);
         printf("   Fat Burn Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Fat_Burn_Heart_Rate_Lower_Limit);
         break;
      case uctFatBurnHeartRateUpperLimit:
         printf("   Type:                            ");
         DisplayUDSCharacteristicType(Type);
         printf("   Fat Burn Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Fat_Burn_Heart_Rate_Upper_Limit);
         break;
      case uctAerobicHeartRateLowerLimit:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Aerobic_Heart_Rate_Lower_Limit);
         break;
      case uctAerobicHeartRateUpperLimit:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Aerobic_Heart_Rate_Upper_Limit);
         break;
      case uctAnaerobicHeartRateLowerLimit:
         printf("   Type:                             ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Anaerobic_Heart_Rate_Lower_Limit);
         break;
      case uctAnaerobicHeartRateUpperLimit:
         printf("   Type:                             ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Anaerobic_Heart_Rate_Upper_Limit);
         break;
      case uctFiveZoneHeartRateLimits:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Five Zone Heart Rate Limits:\r\n");
         printf("      Very Light/Light: %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Light_Limit);
         printf("      Light/Moderate:   %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Light_Moderate_Limit);
         printf("      Moderate/Hard:    %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Moderate_Hard_Limit);
         printf("      Hard/Maximum:     %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Hard_Maximum_Limit);
         break;
      case uctThreeZoneHeartRateLimits:
         printf("   Three Zone Heart Rate Limits:\r\n");
         printf("      Light/Moderate: %u\r\n", UDS_Characteristic->Three_Zone_Heart_Rate_Limits.Light_Moderate_Limit);
         printf("      Moderate/Hard:  %u\r\n", UDS_Characteristic->Three_Zone_Heart_Rate_Limits.Moderate_Hard_Limit);
         break;
      case uctTwoZoneHeartRateLimit:
         printf("   Type:                      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Two Zone Heart Rate Limit: %u\r\n", UDS_Characteristic->Two_Zone_Heart_Rate_Limit);
         break;
      case uctLanguage:
         printf("   Type:     ");
         DisplayUDSCharacteristicType(Type);

         /* Simply copy the string into the buffer.                     */
         /* * NOTE * We will simply access the First Name since the     */
         /*          characteristics will align.                        */
         /* * NOTE * Length MUST be valid or we would not have received */
         /*          the event.                                         */
         BufferLength = UDS_Characteristic->Language.Buffer_Length;
         BTPS_MemCopy(Buffer, UDS_Characteristic->Language.Buffer, BufferLength);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         Buffer[BufferLength] = '\0';

         /* Display the string.                                         */
         printf("   Language: \"%s\"\r\n", Buffer);
         break;
      default:
         printf("Invalid UDS Characteristic Type.\r\n");
         break;
   }
}

   /* This function is a helper function that simply prints the UDS     */
   /* Characteristic data.                                              */
static void DisplayUDSSampleCharacteristicData(UDS_Characteristic_Type_t Type, UDS_Sample_Characteristic_t *UDS_Characteristic)
{
   printf("\r\nUDS Characteristic:\r\n");

   /* Display the data based on the type.                               */
   switch(Type)
   {
      case uctFirstName:
         printf("   Type:        ");
         DisplayUDSCharacteristicType(Type);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         UDS_Characteristic->First_Name.Buffer[UDS_Characteristic->First_Name.Buffer_Length] = '\0';

         /* Display the string.                                         */
         printf("   First Name:  \"%s\"\r\n", UDS_Characteristic->First_Name.Buffer);
         break;
      case uctLastName:
         printf("   Type:       ");
         DisplayUDSCharacteristicType(Type);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         UDS_Characteristic->Last_Name.Buffer[UDS_Characteristic->Last_Name.Buffer_Length] = '\0';

         /* Display the string.                                         */
         printf("   Last Name:  \"%s\"\r\n", UDS_Characteristic->Last_Name.Buffer);
         break;
      case uctEmailAddress:
         printf("   Type:           ");
         DisplayUDSCharacteristicType(Type);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         UDS_Characteristic->Email_Address.Buffer[UDS_Characteristic->Email_Address.Buffer_Length] = '\0';

         /* Display the string.                                         */
         printf("   Email Address:  \"%s\"\r\n", UDS_Characteristic->Email_Address.Buffer);
         break;
      case uctAge:
         printf("   Type: ");
         DisplayUDSCharacteristicType(Type);
         printf("   Age:  %u.\r\n", UDS_Characteristic->Age);
         break;
      case uctDateOfBirth:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Data of Birth:\r\n");
         printf("      Year:   %u\r\n", UDS_Characteristic->Date_Of_Birth.Year);
         printf("      Month:  %u\r\n", UDS_Characteristic->Date_Of_Birth.Month);
         printf("      Day:    %u\r\n", UDS_Characteristic->Date_Of_Birth.Day);
         break;
      case uctGender:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Gender: %s.\r\n", (UDS_Characteristic->Gender) ? "Female" : "Male");
         break;
      case uctWeight:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Weight: %u.\r\n", UDS_Characteristic->Weight);
         break;
      case uctHeight:
         printf("   Type:   ");
         DisplayUDSCharacteristicType(Type);
         printf("   Height: %u.\r\n", UDS_Characteristic->Height);
         break;
      case uctVO2Max:
         printf("   Type:    ");
         DisplayUDSCharacteristicType(Type);
         printf("   VO2 Max: %u.\r\n", UDS_Characteristic->VO2_Max);
         break;
      case uctHeartRateMax:
         printf("   Type:           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Heart Rate Max: %u.\r\n", UDS_Characteristic->Heart_Rate_Max);
         break;
      case uctRestingHeartRate:
         printf("   Type:               ");
         DisplayUDSCharacteristicType(Type);
         printf("   Resting Heart Rate: %u.\r\n", UDS_Characteristic->Resting_Heart_Rate);
         break;
      case uctMaximumRecommendedHeartRate:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Maximum Recommended Heart Rate: %u.\r\n", UDS_Characteristic->Maximum_Recommended_Heart_Rate);
         break;
      case uctAerobicThreshold:
         printf("   Type:              ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Threshold: %u.\r\n", UDS_Characteristic->Aerobic_Threshold);
         break;
      case uctAnaerobicThreshold:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Threshold: %u.\r\n", UDS_Characteristic->Anaerobic_Threshold);
         break;
      case uctSportType:
         printf("   Type:       ");
         DisplayUDSCharacteristicType(Type);
         printf("   Sport Type: %u.\r\n", UDS_Characteristic->Sport_Type);
         break;
      case uctDateOfThreshold:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Data of Threshold:\r\n");
         printf("      Year:   %u\r\n", UDS_Characteristic->Date_Of_Threshold.Year);
         printf("      Month:  %u\r\n", UDS_Characteristic->Date_Of_Threshold.Month);
         printf("      Day:    %u\r\n", UDS_Characteristic->Date_Of_Threshold.Day);
         break;
      case uctWaistCircumference:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Waist circumference: %u.\r\n", UDS_Characteristic->Waist_Circumference);
         break;
      case uctHipCircumference:
         printf("   Type:              ");
         DisplayUDSCharacteristicType(Type);
         printf("   Hip circumference: %u.\r\n", UDS_Characteristic->Hip_Circumference);
         break;
      case uctFatBurnHeartRateLowerLimit:
         printf("   Type:                            ");
         DisplayUDSCharacteristicType(Type);
         printf("   Fat Burn Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Fat_Burn_Heart_Rate_Lower_Limit);
         break;
      case uctFatBurnHeartRateUpperLimit:
         printf("   Type:                            ");
         DisplayUDSCharacteristicType(Type);
         printf("   Fat Burn Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Fat_Burn_Heart_Rate_Upper_Limit);
         break;
      case uctAerobicHeartRateLowerLimit:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Aerobic_Heart_Rate_Lower_Limit);
         break;
      case uctAerobicHeartRateUpperLimit:
         printf("   Type:                           ");
         DisplayUDSCharacteristicType(Type);
         printf("   Aerobic Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Aerobic_Heart_Rate_Upper_Limit);
         break;
      case uctAnaerobicHeartRateLowerLimit:
         printf("   Type:                             ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Heart Rate Lower Limit: %u.\r\n", UDS_Characteristic->Anaerobic_Heart_Rate_Lower_Limit);
         break;
      case uctAnaerobicHeartRateUpperLimit:
         printf("   Type:                             ");
         DisplayUDSCharacteristicType(Type);
         printf("   Anaerobic Heart Rate Upper Limit: %u.\r\n", UDS_Characteristic->Anaerobic_Heart_Rate_Upper_Limit);
         break;
      case uctFiveZoneHeartRateLimits:
         printf("   Type:                ");
         DisplayUDSCharacteristicType(Type);
         printf("   Five Zone Heart Rate Limits:\r\n");
         printf("      Very Light/Light: %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Light_Limit);
         printf("      Light/Moderate:   %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Light_Moderate_Limit);
         printf("      Moderate/Hard:    %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Moderate_Hard_Limit);
         printf("      Hard/Maximum:     %u\r\n", UDS_Characteristic->Five_Zone_Heart_Rate_Limits.Hard_Maximum_Limit);
         break;
      case uctThreeZoneHeartRateLimits:
         printf("   Three Zone Heart Rate Limits:\r\n");
         printf("      Light/Moderate: %u\r\n", UDS_Characteristic->Three_Zone_Heart_Rate_Limits.Light_Moderate_Limit);
         printf("      Moderate/Hard:  %u\r\n", UDS_Characteristic->Three_Zone_Heart_Rate_Limits.Moderate_Hard_Limit);
         break;
      case uctTwoZoneHeartRateLimit:
         printf("   Type:                      ");
         DisplayUDSCharacteristicType(Type);
         printf("   Two Zone Heart Rate Limit: %u\r\n", UDS_Characteristic->Two_Zone_Heart_Rate_Limit);
         break;
      case uctLanguage:
         printf("   Type:      ");
         DisplayUDSCharacteristicType(Type);

         /* Insert the NULL terminator so we do not go past the end of  */
         /* the string.                                                 */
         UDS_Characteristic->Language.Buffer[UDS_Characteristic->Language.Buffer_Length] = '\0';

         /* Display the string.                                         */
         printf("   Language:  \"%s\"\r\n", UDS_Characteristic->Language.Buffer);
         break;
      default:
         printf("Invalid UDS Characteristic Type.\r\n");
         break;
   }
}


   /* The following function is a helper function that will display the */
   /* User Control Point request data.                                  */
static void DisplayUserControlPointRequestData(UDS_User_Control_Point_Request_Data_t *RequestData)
{
   printf("\r\nUser Control Point request data:\r\n");

   /* Verify that the input parameters seem semi-valid.                 */
   if(RequestData)
   {
      printf("   Request Op Code:  ");
      switch(RequestData->Op_Code)
      {
         case ucpRegisterNewUser:
            printf("Register New User.\r\n");

            /* Display the parameters.                                  */
            printf("   Consent Code:     0x%04X\r\n", RequestData->Parameter.Consent_Code);
            break;
         case ucpConsent:
            printf("Consent.\r\n");

            /* Display the parameters.                                  */
            printf("   User Index:       %u\r\n", RequestData->Parameter.User_Index);
            printf("   Consent Code:     0x%04X\r\n", RequestData->Parameter.Consent_Code);
            break;
         case ucpDeleteUserData:
            printf("Delete User Data.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
   else
      printf("\r\nInvalid User Control Point response data.\r\n");
}

   /* The following function is a helper function that will display the */
   /* User Control Point response data.                                 */
static void DisplayUserControlPointResponseData(UDS_User_Control_Point_Response_Data_t *ResponseData)
{
   printf("\r\nUser Control Point response data:\r\n");

   /* Verify that the input parameters seem semi-valid.                 */
   if(ResponseData)
   {
      printf("   Request Op Code:      ");
      switch(ResponseData->Request_Op_Code)
      {
         case ucpRegisterNewUser:
            printf("Register New User.\r\n");
            break;
         case ucpConsent:
            printf("Consent.\r\n");
            break;
         case ucpDeleteUserData:
            printf("Delete User Data.\r\n");
            break;
         default:
            printf("Request Code invalid.\r\n");
            break;
      }

      printf("   Response Code Value:  ");
      switch(ResponseData->Response_Code_Value)
      {
         case rcvUDSSuccess:
            printf("Success.\r\n");
            break;
         case rcvUDSOpCodeNotSupported:
            printf("Op Code Not Supported.\r\n");
            break;
         case rcvUDSInvalidParameter:
            printf("Invalid parameter.\r\n");
            break;
         case rcvUDSOperationFailed:
            printf("Operation Failed.\r\n");
            break;
         case rcvUDSUserNotAuthorized:
            printf("User Not Authorized.\r\n");
            break;
         default:
            printf("Response Code Value invalid).\r\n");
            break;
      }

      /* Check if we need to display any optional parameters.           */
      /* * NOTE * This MUST be a successful response and the request    */
      /*          MUST have been to register a new user.  We expect the */
      /*          User Index in the response data.                      */
      if((ResponseData->Response_Code_Value == rcvUDSSuccess) && (ResponseData->Request_Op_Code == ucpRegisterNewUser))
      {
         printf("   User ID:              %u\r\n", ResponseData->Parameter.User_Index);
      }
   }
   else
      printf("\r\nInvalid User Control Point response data.\r\n");
}

   /* The following command is responsible for registering UDS.         */
static int RegisterUDSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(RegisterUDS());
}

   /* The following command is responsible for un-registering UDS.      */
static int UnregisterUDSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(UnregisterUDS());
}

   /* The following command is responsible for discovering attribute    */
   /* handles for UDS on a remote UDS Server.                           */
static int DiscoverUDSCommand(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;

   /* Verify that we are not configured as the UDS Server               */
   if(!UDSInstanceID)
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
               /* Configure the filter so that only the UDS Service is  */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               UDS_ASSIGN_UDS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery procuds.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdUDS);
               if(!ret_val)
               {
                  /* Display succuds mudsage.                           */
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
            printf("\r\nUnknown UDS Server.\r\n");
      }
      else
         printf("\r\nMust be connected to an UDS Server.\r\n");
   }
   else
      printf("\r\nOnly an UDS client can discover UDS.\r\n");

   return(ret_val);
}

   /* The following command is responsible for reading a UDS            */
   /* Characteristic.                                                   */
static int ReadUDSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;
   Boolean_t           RequestReady = FALSE;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = (UDS_Attribute_Handle_Type_t)TempParam->Params[0].intParam;

      /* If this is a UDS Characteristic                                */
      if(RequestData.AttributeHandleType == ahtUDS_Characteristic)
      {
         /* Make sure we have another parameter.                        */
         if(TempParam->NumberofParameters >= 2)
         {
            /* Store the UDS Characteristic type.                       */
            RequestData.Type = (UDS_Characteristic_Type_t)TempParam->Params[1].intParam;

            switch(RequestData.Type)
            {
               case uctFirstName:
               case uctLastName:
               case uctEmailAddress:
               case uctLanguage:
                  /* Intentional fall-through.                          */

                  /* Make sure we have another parameter.               */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the boolean for if this is a GATT Read    */
                     /* Long request.                                   */
                     RequestData.ReadLong = (Boolean_t)TempParam->Params[2].intParam;

                     /* If we are the UDS Server.                       */
                     if(UDSInstanceID)
                     {
                        /* Make sure the User Index is specified.       */
                        if(TempParam->NumberofParameters >= 4)
                        {
                           RequestData.UserIndex = (Byte_t)TempParam->Params[3].intParam;
                           RequestReady = TRUE;
                        }
                        else
                        {
                           printf("\r\nInvalid parameter.\r\n");
                           DisplayReadCharacteristicCommandUsage();
                        }
                     }
                     else
                     {
                        /* Simply flag that the User Index is unknown.  */
                        /* The internal function will display an error. */
                        RequestData.UserIndex = UDS_USER_INDEX_UNKNOWN_USER;
                        RequestReady = TRUE;
                     }


                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayReadCharacteristicCommandUsage();
                  }
                  break;
               default:
                  /* If we are the UDS Server.                          */
                  if(UDSInstanceID)
                  {
                     /* Make sure the User Index is specified.          */
                     if(TempParam->NumberofParameters >= 3)
                     {
                        RequestData.UserIndex = (Byte_t)TempParam->Params[2].intParam;
                        RequestReady = TRUE;
                     }
                     else
                     {
                        printf("\r\nInvalid parameter.\r\n");
                        DisplayReadCharacteristicCommandUsage();
                     }
                  }
                  else
                  {
                     /* Simply flag that the User Index is unknown.  The*/
                     /* internal function will display an error.        */
                     RequestData.UserIndex = UDS_USER_INDEX_UNKNOWN_USER;
                     RequestReady = TRUE;
                  }
                  break;
            }
         }
         else
         {
            printf("\r\nInvalid parameter.\r\n");
            DisplayReadCharacteristicCommandUsage();
         }
      }
      else
      {
         /* If we are the UDS Server.                                   */
        if(UDSInstanceID)
         {
            /* Make sure the User Index is specified.                   */
            if(TempParam->NumberofParameters >= 2)
            {
               RequestData.UserIndex = (Byte_t)TempParam->Params[1].intParam;
               RequestReady = TRUE;
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayReadCharacteristicCommandUsage();
            }
         }
         else
         {
            /* Simply flag that the User Index is unknown.  The internal*/
            /* function will display an error.                          */
            RequestData.UserIndex = UDS_USER_INDEX_UNKNOWN_USER;
            RequestReady = TRUE;
         }
      }

      /* If the request is ready.                                       */
      if(RequestReady)
      {
         /* Simply call the internal function to send the request.      */
         ReadUDSCharacteristic(&RequestData);
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayReadCharacteristicCommandUsage();
   }


   return(ret_val);
}

   /* The following command is responsible for writing a UDS            */
   /* Characteristic.                                                   */
static int WriteUDSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;
   Boolean_t           RequestReady = FALSE;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the UDS Characteristic type for the request.             */
      RequestData.Type = (UDS_Characteristic_Type_t)TempParam->Params[0].intParam;

      /* Format the request data based on the UDS Characteristic type.  */
      switch(RequestData.Type)
      {
         case uctFirstName:
         case uctLastName:
         case uctEmailAddress:
         case uctLanguage:
            /* Intentional fall-through for string requests.            */

            /* Make sure we have a string parameter.                    */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Buffer length.                              */
               /* * NOTE * We will simply use the first name since the  */
               /*          types should align.                          */
               /* * NOTE * We will not include the NULL terminator in   */
               /*          the length.                                  */
               RequestData.Data.Characteristic.First_Name.Buffer_Length = BTPS_StringLength(TempParam->Params[1].strParam);

               /* Simply store the a pointer to the buffer.             */
               RequestData.Data.Characteristic.First_Name.Buffer        = (Byte_t *)TempParam->Params[1].strParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[2].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         case uctDateOfBirth:
         case uctDateOfThreshold:
            /* Intentional fall-through for a Date request.             */

            /* Make sure we have at least three parameters for the year,*/
            /* month, and day.                                          */
            if(TempParam->NumberofParameters >= 4)
            {
               /* Store the Data.                                       */
               /* * NOTE * The fields will align so we will simply set  */
               /*          the Data Of Birth.                           */
               RequestData.Data.Characteristic.Date_Of_Birth.Year  = (Word_t)TempParam->Params[1].intParam;
               RequestData.Data.Characteristic.Date_Of_Birth.Month = (Byte_t)TempParam->Params[2].intParam;
               RequestData.Data.Characteristic.Date_Of_Birth.Day   = (Byte_t)TempParam->Params[3].intParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 5)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[4].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         case uctAge:
         case uctGender:
         case uctVO2Max:
         case uctHeartRateMax:
         case uctRestingHeartRate:
         case uctMaximumRecommendedHeartRate:
         case uctAerobicThreshold:
         case uctAnaerobicThreshold:
         case uctSportType:
         case uctFatBurnHeartRateLowerLimit:
         case uctFatBurnHeartRateUpperLimit:
         case uctAerobicHeartRateLowerLimit:
         case uctAerobicHeartRateUpperLimit:
         case uctAnaerobicHeartRateLowerLimit:
         case uctAnaerobicHeartRateUpperLimit:
         case uctTwoZoneHeartRateLimit:
            /* Intentional fall-through for (UINT8) requests.           */

            /* Make sure we have at least two parameters for the extra  */
            /* byte parameter.                                          */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Data.                                       */
               /* * NOTE * The fields will align so we will simply set  */
               /*          the Age.                                     */
               RequestData.Data.Characteristic.Age  = (Byte_t)TempParam->Params[1].intParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[2].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         case uctWeight:
         case uctHeight:
         case uctWaistCircumference:
         case uctHipCircumference:
            /* Intentional fall-through for (UINT16) requests.          */

            /* Make sure we have at least two parameters for the extra  */
            /* octets.                                                  */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Data.                                       */
               /* * NOTE * The fields will align so we will simply set  */
               /*          the Weight.                                  */
               RequestData.Data.Characteristic.Weight = (Word_t)TempParam->Params[1].intParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[2].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         case uctFiveZoneHeartRateLimits:
            /* Make sure we have at least five parameters.              */
            if(TempParam->NumberofParameters >= 5)
            {
               /* Store the Data.                                       */
               RequestData.Data.Characteristic.Five_Zone_Heart_Rate_Limits.Light_Limit          = (Byte_t)TempParam->Params[1].intParam;
               RequestData.Data.Characteristic.Five_Zone_Heart_Rate_Limits.Light_Moderate_Limit = (Byte_t)TempParam->Params[2].intParam;
               RequestData.Data.Characteristic.Five_Zone_Heart_Rate_Limits.Moderate_Hard_Limit  = (Byte_t)TempParam->Params[3].intParam;
               RequestData.Data.Characteristic.Five_Zone_Heart_Rate_Limits.Hard_Maximum_Limit   = (Byte_t)TempParam->Params[4].intParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 6)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[5].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         case uctThreeZoneHeartRateLimits:
            /* Make sure we have at least three parameters.             */
            if(TempParam->NumberofParameters >= 3)
            {
               /* Store the Data.                                       */
               RequestData.Data.Characteristic.Three_Zone_Heart_Rate_Limits.Light_Moderate_Limit = (Byte_t)TempParam->Params[1].intParam;
               RequestData.Data.Characteristic.Three_Zone_Heart_Rate_Limits.Moderate_Hard_Limit  = (Byte_t)TempParam->Params[2].intParam;

               /* If we are the UDS Server.                             */
               if(UDSInstanceID)
               {
                  /* Make sure the User Index is specified.             */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Store the User Index.                           */
                     RequestData.UserIndex = (Byte_t)TempParam->Params[3].intParam;

                     /* Flag that the request is ready.                 */
                     RequestReady = TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayWriteCharacteristicCommandUsage();
                  }
               }
               else
               {
                  /* Flag that the request is ready.                    */
                  RequestReady = TRUE;
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteCharacteristicCommandUsage();
            }
            break;
         default:
            printf("\r\nInvalid UDS Characteristic type.\r\n");
            break;
      }

      /* If the request is ready.                                       */
      if(RequestReady)
      {
         /* Simply call the internal function to send the request.      */
         WriteUDSCharacteristic(&RequestData);
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayWriteCharacteristicCommandUsage();
   }


   return(ret_val);
}

   /* The following command is responsible for writing a UDS            */
   /* Characteristic CCCD.                                              */
static int WriteUDSCCCDCommand(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the UDS Characteristic type for the request.             */
      RequestData.CCCDType = (UDS_CCCD_Characteristic_Type_t)TempParam->Params[0].intParam;

      /* Format the request data based on the UDS Characteristic type.  */
      switch(RequestData.CCCDType)
      {
         case cctDatabaseChangeIncrement:
         case cctUserControlPoint:
            /* Intentional fall- through since we can handle these types*/
            /* similarly.                                               */

            /* Store the attribute handle type.                         */
            if(RequestData.CCCDType == cctDatabaseChangeIncrement)
               RequestData.AttributeHandleType = ahtDatabase_Change_Increment_CCCD;
            else
               RequestData.AttributeHandleType = ahtUser_Control_Point_CCCD;

            /* Store the CCCD.                                          */
            if(TempParam->Params[1].intParam == 0)
            {
               RequestData.Data.ClientConfiguration = 0;
            }
            else
            {
               if(TempParam->Params[1].intParam == 1)
               {
                  RequestData.Data.ClientConfiguration = (Word_t)UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE;
               }
               else
               {
                  RequestData.Data.ClientConfiguration = (Word_t)UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE;
               }
            }
            break;
         default:
            printf("\r\nInvalid UDS Characteristic type.\r\n");
            break;
      }

      /* Simply call the internal function to send the request.         */
      WriteUDSCharacteristic(&RequestData);
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayWriteUDSCCCDCommandUsage();
   }


   return (ret_val);
}

   /* The following command is responsible for writing a UDS Database   */
   /* Change Increment Characteristic.                                  */
static int WriteUDSDatabaseChangeIncrement(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = ahtDatabase_Change_Increment;

      /* Store the Database Change Increment for the request.           */
      RequestData.Data.DatabaseChangeIncrement = (DWord_t)TempParam->Params[0].intParam;

      /* Simply call the internal function to send the request.         */
      WriteUDSCharacteristic(&RequestData);
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      printf("\r\nUsage: WriteDCI [DCI (UINT32)]\r\n");
   }


   return(ret_val);
}

   /* The following command is responsible for writing a UDS User       */
   /* Control Point Characteristic.                                     */
static int WriteUDSUserControlPointCommand(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;
   Boolean_t           RequestReady = FALSE;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = ahtUser_Control_Point;

      /* Store the request type.                                        */
      RequestData.Data.RequestData.Op_Code = (UDS_User_Control_Point_Request_Type_t)TempParam->Params[0].intParam;

      /* Determine if there are any extra parameters to store.          */
      switch(RequestData.Data.RequestData.Op_Code)
      {
         case ucpRegisterNewUser:
            /* Check if there is room to store the Consent Code         */
            /* parameter.                                               */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Consent Code.                               */
               RequestData.Data.RequestData.Parameter.Consent_Code = (Word_t)TempParam->Params[1].intParam;

               /* Flag that the request is ready.                       */
               RequestReady = TRUE;
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteUDSUserControlPointCommandUsage();
            }
            break;
         case ucpConsent:
            /* Check if there is room to store the Consent Code and User*/
            /* Index parameters.                                        */
            if(TempParam->NumberofParameters >= 3)
            {
               /* Store the Consent Code.                               */
               RequestData.Data.RequestData.Parameter.Consent_Code = (Word_t)TempParam->Params[1].intParam;

               /* Store the User Index.                                 */
               RequestData.Data.RequestData.Parameter.User_Index   = (Byte_t)TempParam->Params[2].intParam;

               /* Flag that the request is ready.                       */
               RequestReady = TRUE;
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayWriteUDSUserControlPointCommandUsage();
            }
            break;
         case ucpDeleteUserData:
            /* Flag that the request is ready.                          */
            RequestReady = TRUE;
            break;
         default:
            printf("\r\nInvalid User Control Point request type.\r\n");
            break;
      }

      /* If the request is ready.                                       */
      if(RequestReady)
      {
         /* Simply call the internal function to send the request.      */
         WriteUDSCharacteristic(&RequestData);
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayWriteUDSUserControlPointCommandUsage();
   }


   return(ret_val);
}

   /* The following command is responsible for registering a user on the*/
   /* UDS Server.                                                       */
static int RegisterUserCommand(ParameterList_t *TempParam)
{
   int                 ret_val = 0;
   UDS_Request_Data_t  RequestData;

   /* Initiailze the data so we do not have any warnings for unused     */
   /* fields.                                                           */
   BTPS_MemInitialize(&RequestData, 0, sizeof(UDS_Request_Data_t));

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 4))
   {
      /* Make sure we are the UDS Server.                               */
      if(UDSInstanceID)
      {
         /* Mark the specified User as registered.                      */
         UserList[TempParam->Params[0].intParam].Registered                = TRUE;
         UserList[TempParam->Params[0].intParam].Consent_Code              = 0;
         UserList[TempParam->Params[0].intParam].Consent_Attempts          = 3;
         UserList[TempParam->Params[0].intParam].Database_Change_Increment = 0;
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      printf("\r\nUsage: RegisterUser [User Index (0-4 UINT8)]\r\n");
   }


   return(ret_val);
}

   /* The following function displays the usage for the                 */
   /* ReadUDSCharacteristic() function.                                 */
static void DisplayReadCharacteristicCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: Read [Attribute Handle Type (UINT8)] [Opt. params (below)] [User Index (0-4 Server ONLY)]\r\n");
   printf("\r\nWhere Attribute Handle Type is:\r\n");
   printf("   0 = UDS Characteristic [Type (UINT8)].\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("       0 = First Name [Read Long(Boolean)].\r\n");
   printf("       1 = Last Name [Read Long(Boolean)].\r\n");
   printf("       2 = Email Address [Read Long(Boolean)].\r\n");
   printf("       3 = Age.\r\n");
   printf("       4 = Date of Birth.\r\n");
   printf("       5 = Gender.\r\n");
   printf("       6 = Weight.\r\n");
   printf("       7 = Height.\r\n");
   printf("       8 = VO2 Max.\r\n");
   printf("       9 = Heart Rate Max.\r\n");
   printf("      10 = Resting Heart Rate.\r\n");
   printf("      11 = Maximum Recommended Heart Rate.\r\n");
   printf("      12 = Aerobic Threshold.\r\n");
   printf("      13 = Anaerobic Threshold.\r\n");
   printf("      14 = Sport Type.\r\n");
   printf("      15 = Date of Threshold.\r\n");
   printf("      16 = Waist Circumference.\r\n");
   printf("      17 = Hip Circumference.\r\n");
   printf("      18 = Fat Burn Heart Rate Lower Limit.\r\n");
   printf("      19 = Fat Burn Heart Rate Upper Limit.\r\n");
   printf("      20 = Aerobic Heart Rate Lower Limit.\r\n");
   printf("      21 = Aerobic Heart Rate Upper Limit.\r\n");
   printf("      22 = Anaerobic Heart Rate Lower Limit.\r\n");
   printf("      23 = Anaerobic Heart Rate Upper Limit.\r\n");
   printf("      24 = Five Zone Heart Rate Limits.\r\n");
   printf("      25 = Three Zone Heart Rate Limits.\r\n");
   printf("      26 = Two Zone Heart Rate Limit.\r\n");
   printf("      27 = Language [Read Long(Boolean)].\r\n");
   printf("   1 = Database Change Increment\r\n");
   printf("   2 = Database Change Increment CCCD\r\n");
   printf("   3 = User Index\r\n");
   printf("   4 = User Control Point (CANNOT BE READ)\r\n");
   printf("   5 = User Control Point CCCD\r\n");
   printf("\r\nNOTE: To use Read Long, the GATT Read request MUST be used first, followed by GATT Read Long for subsequent requests.\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteUDSCharacteristic() function.                                */
static void DisplayWriteCharacteristicCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: Write [Type (UINT8)] [Opt. params (below)] [User Index (0-4 Server ONLY)]\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("    0 = First Name [String (UTF8)].\r\n");
   printf("    1 = Last Name [String (UTF8)].\r\n");
   printf("    2 = Email Address [String (UTF8)].\r\n");
   printf("    3 = Age [(UINT8)].\r\n");
   printf("    4 = Date of Birth [Year (UINT16)] [Month (UINT8)] [Day (UINT8)].\r\n");
   printf("    5 = Gender [(UINT8)].\r\n");
   printf("    6 = Weight [(UINT16)].\r\n");
   printf("    7 = Height [(UINT16)].\r\n");
   printf("    8 = VO2 Max [(UINT8)].\r\n");
   printf("    9 = Heart Rate Max [(UINT8)].\r\n");
   printf("   10 = Resting Heart Rate [(UINT8)].\r\n");
   printf("   11 = Maximum Recommended Heart Rate [(UINT8)].\r\n");
   printf("   12 = Aerobic Threshold [(UINT8)].\r\n");
   printf("   13 = Anaerobic Threshold [(UINT8)].\r\n");
   printf("   14 = Sport Type [(UINT8)].\r\n");
   printf("   15 = Date of Threshold [Year (UINT16)] [Month (UINT8)] [Day (UINT8)].\r\n");
   printf("   16 = Waist Circumference [(UINT16)].\r\n");
   printf("   17 = Hip Circumference [(UINT16)].\r\n");
   printf("   18 = Fat Burn Heart Rate Lower Limit [(UINT8)].\r\n");
   printf("   19 = Fat Burn Heart Rate Upper Limit [(UINT8)].\r\n");
   printf("   20 = Aerobic Heart Rate Lower Limit [(UINT8)].\r\n");
   printf("   21 = Aerobic Heart Rate Upper Limit [(UINT8)].\r\n");
   printf("   22 = Anaerobic Heart Rate Lower Limit [(UINT8)].\r\n");
   printf("   23 = Anaerobic Heart Rate Upper Limit [(UINT8)].\r\n");
   printf("   24 = Five Zone Heart Rate Limits [Light_Limit (UINT8)] [ Light_Moderate_Limit (UINT8)] [ Moderate_Hard_Limit (UINT8)] [Hard_Maximum_Limit (UINT8)].\r\n");
   printf("   25 = Three Zone Heart Rate Limits [ Light_Moderate_Limit (UINT8)] [ Moderate_Hard_Limit (UINT8)].\r\n");
   printf("   26 = Two Zone Heart Rate Limit [(UINT8)].\r\n");
   printf("   27 = Language [String (UTF8)].\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteUDSCharacteristic() function.                                */
static void DisplayWriteUDSCCCDCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteCCCD [Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("   0 = Database Change Increment CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
   printf("   1 = User Control Point CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
}

/* The following function displays the usage for the                 */
/* WriteUDSCharacteristic() function.                                */
static void DisplayWriteUDSUserControlPointCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteUserCP [Request Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Request Type is:\r\n");
   printf("   1 = Register New User [Consent Code (0-9999 UINT16)].\r\n");
   printf("   2 = Consent [Consent Code (0-9999 UINT16)] [User Index (UINT8)].\r\n");
   printf("   3 = Delete User.\r\n");
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
      printf("\r\nGAP Callback Data: Event_Data = NULL.\r\n");

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

   /* The following is a UDS Server Event Callback.  This function will */
   /* be called whenever an UDS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the UDS Event Data   */
   /* that occurred and the UDS Event Callback Parameter that was       */
   /* specified when this Callback was The caller is free to installed. */
   /* use the contents of the UDS Event Data ONLY in the context If the */
   /* caller requires the Data for a longer period of of this callback. */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified this function DOES NOT */
   /* have be re-entrant).  It needs to be installed callback (i.e.     */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will Because of be called serially.      */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another UDS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving UDS Event Packets.  */
   /*            A Deadlock WILL occur because NO UDS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI UDS_EventCallback(unsigned int BluetoothStackID, UDS_Event_Data_t *UDS_Event_Data, unsigned long CallbackParameter)
{
   int                                     Result;
   BoardStr_t                              BoardStr;
   DeviceInfo_t                           *DeviceInfo;

   /* UDS Common Event fields.                                          */
   unsigned int                            InstanceID;
   unsigned int                            ConnectionID;
   unsigned int                            TransactionID;
   BD_ADDR_t                               RemoteDevice;
   GATT_Connection_Type_t                  ConnectionType;

   UDS_Characteristic_Type_t               Type;
   UDS_CCCD_Characteristic_Type_t          CCCD_Type;
   UDS_Characteristic_t                    UDS_Characteristic;
   Word_t                                  Offset;
   Word_t                                  ClientConfiguration;
   DWord_t                                 DatabaseChangeIncrement;
   UDS_User_Control_Point_Request_Data_t   RequestData;
   UDS_User_Control_Point_Response_Data_t  ResponseData;
   Byte_t                                  Status;
   Word_t                                  BytesWritten;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (UDS_Event_Data))
   {
      /* Switch through the event type.                                 */
      switch(UDS_Event_Data->Event_Data_Type)
      {
         case etUDS_Server_Read_Characteristic_Request:
            printf("\r\netUDS_Server_Read_Characteristic_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->RemoteDevice;
               Type           = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->Type;
               Offset         = UDS_Event_Data->Event_Data.UDS_Read_Characteristic_Request_Data->Offset;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Type:                 ");
               DisplayUDSCharacteristicType(Type);

               /* Display the offset.                                   */
               printf("   Offset:               %u.\r\n", Offset);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure the UDS Client has obtained consent to*/
                     /* read the user's data.                           */
                     /* * NOTE * The User Index MUST be valid and       */
                     /*          consent MUST have been given.          */
                     if((User_Index != UDS_USER_INDEX_UNKNOWN_USER) &&
                        (User_Index < UDS_SA_MAXIMUM_SUPPORTED_USERS) &&
                        (DeviceInfo->Consent[User_Index] == TRUE))
                     {
                        /* Simply call the internal function to format  */
                        /* the UDS Characteristic.                      */
                        /* * NOTE * We need to do this since the union  */
                        /*          types are different between the     */
                        /*          sample application and API due to   */
                        /*          the way we handle the string        */
                        /*          structures.                         */
                        FormatUDSCharacteristicData(&(UserList[User_Index]), Type, &UDS_Characteristic);

                        /* Send the response.                           */
                        /* * NOTE * We will use the UDS Characteristic  */
                        /*          type to index the UDS Characteristic*/
                        /*          data to send in the response.       */
                        if((Result = UDS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, Offset, Type, &UDS_Characteristic)) != 0)
                           DisplayFunctionError("UDS_Read_Characteristic_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        if(User_Index == UDS_USER_INDEX_UNKNOWN_USER)
                        {
                           printf("\r\nUnknown user.\r\n");
                        }
                        else
                        {
                           if(User_Index >= UDS_SA_MAXIMUM_SUPPORTED_USERS)
                           {
                              printf("\r\nUser Index is greater than the number of supported users.\r\n");
                           }
                           else
                              printf("\r\nConsent not given.\r\n");
                        }

                        /* Send the error response.                     */
                        if((Result = UDS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED, Offset, Type, NULL)) != 0)
                           DisplayFunctionError("UDS_Read_Characteristic_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Offset, Type, NULL)) != 0)
                        DisplayFunctionError("UDS_Read_Characteristic_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, Offset, Type, NULL)) != 0)
                     DisplayFunctionError("UDS_Read_Characteristic_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Write_Characteristic_Request:
            printf("\r\netUDS_Server_Write_Characteristic_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data)
            {
               /* Store event information.                              */
               InstanceID         = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->InstanceID;
               ConnectionID       = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->ConnectionID;
               ConnectionType     = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->ConnectionType;
               TransactionID      = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->TransactionID;
               RemoteDevice       = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->RemoteDevice;
               Type               = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->Type;
               UDS_Characteristic = UDS_Event_Data->Event_Data.UDS_Write_Characteristic_Request_Data->UDS_Characteristic;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the UDS Write Characteristic data.            */
               DisplayUDSCharacteristicData(Type, &UDS_Characteristic);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure the UDS Client has obtained consent to*/
                     /* write the user's data.                          */
                     /* * NOTE * The User Index MUST be valid and       */
                     /*          consent MUST have been given.          */
                     if((User_Index != UDS_USER_INDEX_UNKNOWN_USER) &&
                        (User_Index < UDS_SA_MAXIMUM_SUPPORTED_USERS) &&
                        (DeviceInfo->Consent[User_Index] == TRUE))
                     {
                        /* Simply call the internal function to store   */
                        /* the UDS Characteristic.                      */
                        /* * NOTE * We need to do this since the union  */
                        /*          types are different between the     */
                        /*          sample application and the received */
                        /*          data type due to the way we handle  */
                        /*          the string structures.              */
                        StoreUDSCharacteristicData(&(UserList[User_Index]), Type, &UDS_Characteristic);

                        /* Send the response.                           */
                        if((Result = UDS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, Type)) != 0)
                           DisplayFunctionError("UDS_Write_Characteristic_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        if(User_Index == UDS_USER_INDEX_UNKNOWN_USER)
                        {
                           printf("\r\nUnknown user.\r\n");
                        }
                        else
                        {
                           if(User_Index >= UDS_SA_MAXIMUM_SUPPORTED_USERS)
                           {
                              printf("\r\nUser Index is greater than the number of supported users.\r\n");
                           }
                           else
                              printf("\r\nConsent not given.\r\n");
                        }

                        /* Send the error response.                     */
                        if((Result = UDS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED, Type)) != 0)
                           DisplayFunctionError("UDS_Write_Characteristic_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Type)) != 0)
                        DisplayFunctionError("UDS_Write_Characteristic_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, Type)) != 0)
                     DisplayFunctionError("UDS_Write_Characteristic_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Prepare_Write_Characteristic_Request:
            printf("\r\netUDS_Server_Prepare_Write_Characteristic_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->RemoteDevice;
               Type           = UDS_Event_Data->Event_Data.UDS_Prepare_Write_Characteristic_Request_Data->Type;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Type:                 ");
               DisplayUDSCharacteristicType(Type);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Send the response.                              */
                     if((Result = UDS_Prepare_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, Type)) != 0)
                        DisplayFunctionError("UDS_Prepare_Write_Characteristic_Request_Response", Result);
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Prepare_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Type)) != 0)
                        DisplayFunctionError("UDS_Prepare_Write_Characteristic_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Prepare_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, Type)) != 0)
                     DisplayFunctionError("UDS_Prepare_Write_Characteristic_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Read_CCCD_Request:
            printf("\r\netUDS_Server_Read_CCCD_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->RemoteDevice;
               CCCD_Type      = UDS_Event_Data->Event_Data.UDS_Read_CCCD_Request_Data->Type;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Type:                 ");
               switch(CCCD_Type)
               {
                  case cctDatabaseChangeIncrement:
                     printf("cctDatabaseChangeIncrement.\r\n");
                     break;
                  case cctUserControlPoint:
                     printf("cctUserControlPoint.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Determine which UDS Characteristic CCCD has been*/
                     /* requested.                                      */
                     switch(CCCD_Type)
                     {
                        case cctDatabaseChangeIncrement:
                           /* Send the response.                        */
                           if((Result = UDS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, CCCD_Type, DeviceInfo->UDSServerInfo.Database_Change_Increment_Configuration)) != 0)
                              DisplayFunctionError("UDS_Read_CCCD_Request_Response", Result);
                           break;
                        case cctUserControlPoint:
                           /* Send the response.                        */
                           if((Result = UDS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, CCCD_Type, DeviceInfo->UDSServerInfo.User_Control_Point_Configuration)) != 0)
                              DisplayFunctionError("UDS_Read_CCCD_Request_Response", Result);
                           break;
                        default:
                           /* Shouldn't occur.  The event would not be  */
                           /* received.                                 */
                           break;
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, CCCD_Type, 0)) != 0)
                        DisplayFunctionError("UDS_Read_CCCD_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, CCCD_Type, 0)) != 0)
                     DisplayFunctionError("UDS_Read_CCCD_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Write_CCCD_Request:
            printf("\r\netUDS_Server_Write_CCCD_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->InstanceID;
               ConnectionID        = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->ConnectionID;
               ConnectionType      = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->ConnectionType;
               TransactionID       = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->TransactionID;
               RemoteDevice        = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->RemoteDevice;
               CCCD_Type           = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->Type;
               ClientConfiguration = UDS_Event_Data->Event_Data.UDS_Write_CCCD_Request_Data->ClientConfiguration;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Type:                 ");
               switch(CCCD_Type)
               {
                  case cctDatabaseChangeIncrement:
                     printf("cctDatabaseChangeIncrement.\r\n");
                     break;
                  case cctUserControlPoint:
                     printf("cctUserControlPoint.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }
               printf("   Configuration:        0x%04X.\r\n", ClientConfiguration);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Determine which UDS Characteristic CCCD has been*/
                     /* requested.                                      */
                     switch(CCCD_Type)
                     {
                        case cctDatabaseChangeIncrement:
                           /* Store the new configuration.              */
                           /* * NOTE * The service will verify the value*/
                           /*          before this event is received.   */
                           DeviceInfo->UDSServerInfo.Database_Change_Increment_Configuration = ClientConfiguration;

                           /* Send the response.                        */
                           if((Result = UDS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, CCCD_Type)) != 0)
                              DisplayFunctionError("UDS_Write_CCCD_Request_Response", Result);
                           break;
                        case cctUserControlPoint:
                           /* Store the new configuration.              */
                           /* * NOTE * The service will verify the value*/
                           /*          before this event is received.   */
                           DeviceInfo->UDSServerInfo.User_Control_Point_Configuration = ClientConfiguration;

                           /* Send the response.                        */
                           if((Result = UDS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, CCCD_Type)) != 0)
                              DisplayFunctionError("UDS_Write_CCCD_Request_Response", Result);
                           break;
                        default:
                           /* Shouldn't occur.  The event would not be  */
                           /* received.                                 */
                           break;
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, CCCD_Type)) != 0)
                        DisplayFunctionError("UDS_Write_CCCD_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, CCCD_Type)) != 0)
                     DisplayFunctionError("UDS_Write_CCCD_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Read_Database_Change_Increment_Request:
            printf("\r\netUDS_Server_Read_Database_Change_Increment_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Read_Database_Change_Increment_Request_Data->RemoteDevice;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure the UDS Client has obtained consent to*/
                     /* the user's data or the Database Change Increment*/
                     /* is invalid.                                     */
                     /* * NOTE * The User Index MUST be valid and       */
                     /*          consent MUST have been given.          */
                     if((User_Index != UDS_USER_INDEX_UNKNOWN_USER) &&
                        (User_Index < UDS_SA_MAXIMUM_SUPPORTED_USERS) &&
                        (DeviceInfo->Consent[User_Index] == TRUE))
                     {
                        /* Send the response.                           */
                        if((Result = UDS_Database_Change_Increment_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, UserList[User_Index].Database_Change_Increment)) != 0)
                           DisplayFunctionError("UDS_Database_Change_Increment_Read_Request_Response", Result);
                     }
                     else
                     {
                        /* Send the response.                           */
                        /* * NOTE * By passing zero we will indicate to */
                        /*          the UDS Client that the UDS client  */
                        /*          has not given consent so a user is  */
                        /*          not currently selected.             */
                        if((Result = UDS_Database_Change_Increment_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, 0)) != 0)
                           DisplayFunctionError("UDS_Database_Change_Increment_Read_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Database_Change_Increment_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, 0)) != 0)
                        DisplayFunctionError("UDS_Database_Change_Increment_Read_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Database_Change_Increment_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, 0)) != 0)
                     DisplayFunctionError("UDS_Database_Change_Increment_Read_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Write_Database_Change_Increment_Request:
            printf("\r\netUDS_Server_Write_Database_Change_Increment_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data)
            {
               /* Store event information.                              */
               InstanceID              = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->InstanceID;
               ConnectionID            = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->ConnectionID;
               ConnectionType          = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->ConnectionType;
               TransactionID           = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->TransactionID;
               RemoteDevice            = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->RemoteDevice;
               DatabaseChangeIncrement = UDS_Event_Data->Event_Data.UDS_Write_Database_Change_Increment_Request_Data->DatabaseChangeIncrement;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   DCI.                  0x%08X.\r\n", DatabaseChangeIncrement);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure the UDS Client has obtained consent to*/
                     /* the user's data or the Database Change Increment*/
                     /* is invalid.                                     */
                     /* * NOTE * The User Index MUST be valid and       */
                     /*          consent MUST have been given.          */
                     if((User_Index != UDS_USER_INDEX_UNKNOWN_USER) &&
                        (User_Index < UDS_SA_MAXIMUM_SUPPORTED_USERS) &&
                        (DeviceInfo->Consent[User_Index] == TRUE))
                     {
                        /* Store the new DCI.                           */
                        UserList[User_Index].Database_Change_Increment = DatabaseChangeIncrement;

                        /* Send the response.                           */
                        if((Result = UDS_Database_Change_Increment_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS)) != 0)
                           DisplayFunctionError("UDS_Database_Change_Increment_Write_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        if(User_Index == UDS_USER_INDEX_UNKNOWN_USER)
                        {
                           printf("\r\nUnknown user.\r\n");
                        }
                        else
                        {
                           if(User_Index >= UDS_SA_MAXIMUM_SUPPORTED_USERS)
                           {
                              printf("\r\nUser Index is greater than the number of supported users.\r\n");
                           }
                           else
                              printf("\r\nConsent not given.\r\n");
                        }

                        /* Send the error response.                     */
                        if((Result = UDS_Database_Change_Increment_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED)) != 0)
                           DisplayFunctionError("UDS_Database_Change_Increment_Write_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_Database_Change_Increment_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION)) != 0)
                        DisplayFunctionError("UDS_Database_Change_Increment_Write_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_Database_Change_Increment_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                     DisplayFunctionError("UDS_Database_Change_Increment_Write_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Read_User_Index_Request:
            printf("etUDS_Server_Read_User_Index_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Read_User_Index_Request_Data->RemoteDevice;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Send the response.                              */
                     if((Result = UDS_User_Index_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS, User_Index)) != 0)
                        DisplayFunctionError("UDS_User_Index_Read_Request_Response", Result);
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_User_Index_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, 0)) != 0)
                        DisplayFunctionError("UDS_User_Index_Read_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_User_Index_Read_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR, 0)) != 0)
                     DisplayFunctionError("UDS_User_Index_Read_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Write_User_Control_Point_Request:
            printf("\r\netUDS_Server_Write_User_Control_Point_Request with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->ConnectionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->ConnectionType;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->TransactionID;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->RemoteDevice;
               RequestData    = UDS_Event_Data->Event_Data.UDS_Write_User_Control_Point_Request_Data->UserControlPoint;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               DisplayUserControlPointRequestData(&RequestData);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Verify that the User Control Point CCCD has been*/
                     /* configured for indications.                     */
                     if(DeviceInfo->UDSServerInfo.User_Control_Point_Configuration & UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                     {
                        /* Make sure a procedure is not currently in    */
                        /* progress.                                    */
                        if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_UDS_PROCEDURE_IN_PROGRESS))
                        {
                           /* Flag that a procedure is currently in     */
                           /* progres.                                  */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_UDS_PROCEDURE_IN_PROGRESS;

                           /* Send the response to indicate we          */
                           /* successfully received and validated the   */
                           /* request.                                  */
                           if((Result = UDS_User_Control_Point_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_SUCCESS)) != 0)
                              DisplayFunctionError("UDS_User_Control_Point_Write_Request_Response", Result);

                           /* We will set the Response Code Value to    */
                           /* operation failed in case something        */
                           /* unexpected happens.                       */
                           ResponseData.Response_Code_Value = rcvUDSOperationFailed;

                           /* Determine the request based on the Op     */
                           /* Code.                                     */
                           /* * NOTE * The internal procedures will     */
                           /*          format the response data that    */
                           /*          will be indicated.               */
                           switch(RequestData.Op_Code)
                           {
                              case ucpRegisterNewUser:
                                 RegisterNewUserProcedure(&RequestData, &ResponseData);
                                 break;
                              case ucpConsent:
                                 ConsentProcedure(DeviceInfo, &RequestData, &ResponseData);
                                 break;
                              case ucpDeleteUserData:
                                 /* * NOTE * We will pass in the current*/
                                 /*          value for the selected user*/
                                 /*          (the User Index).          */
                                 DeleteUserDataProcedure(&RequestData, &ResponseData);
                                 break;
                              default:
                                 /* Format the the Request Op Code is   */
                                 /* not supported.                      */
                                 /* * NOTE * There is no parameter for  */
                                 /*          this response.             */
                                 ResponseData.Request_Op_Code     = RequestData.Op_Code;
                                 ResponseData.Response_Code_Value = rcvUDSOpCodeNotSupported;
                                 break;
                           }

                           /* Indicate the User Control Point response  */
                           /* data.                                     */
                           if((Result = UDS_Indicate_User_Control_Point_Response(BluetoothStackID, InstanceID, ConnectionID, &ResponseData)) > 0)
                           {
                              printf("\r\nUDS User Control Point Response indication sent.\r\n");
                              printf("   TransactionID:   %d\r\n", Result);
                           }
                           else
                              DisplayFunctionError("UDS_Indicate_User_Control_Point_Response", Result);
                        }
                        else
                        {
                           /* Display the error.                        */
                           printf("\r\nProcedure already in progress.\r\n");

                           /* Send the error response.                  */
                           if((Result = UDS_User_Control_Point_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)) != 0)
                              DisplayFunctionError("UDS_User_Control_Point_Write_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nUser Control Point CCCD not configured for indications.\r\n");

                        /* Send the error response.                     */
                        if((Result = UDS_User_Control_Point_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)) != 0)
                           DisplayFunctionError("UDS_User_Control_Point_Write_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = UDS_User_Control_Point_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION)) != 0)
                        DisplayFunctionError("UDS_User_Control_Point_Write_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown UDS Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = UDS_User_Control_Point_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, UDS_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                     DisplayFunctionError("UDS_User_Control_Point_Write_Request_Response", Result);
               }
            }
            break;
         case etUDS_Server_Confirmation_Data:
            printf("\r\netUDS_Server_Confirmation_Data with size %u.\r\n", UDS_Event_Data->Event_Data_Size);
            if(UDS_Event_Data->Event_Data.UDS_Confirmation_Data)
            {
               /* Print event information.                              */
               InstanceID     = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->InstanceID;
               ConnectionID   = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->ConnectionID;
               TransactionID  = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->TransactionID;
               ConnectionType = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->ConnectionType;
               RemoteDevice   = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->RemoteDevice;
               Status         = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->Status;
               BytesWritten   = UDS_Event_Data->Event_Data.UDS_Confirmation_Data->BytesWritten;

               /* Store event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Status:               0x%02X.\r\n", Status);
               printf("   Bytes Written:        %u.\r\n", BytesWritten);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that a procedure is no longer in progres.     */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_UDS_PROCEDURE_IN_PROGRESS;
               }
               else
                  printf("\r\nUnknown UDS Client.\r\n");
            }
            break;
         default:
            printf("\r\nUnknown UDS Event.\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nUDS Callback Data: Event_Data = NULL.\r\n");
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
static void BTPSAPI GATT_ClientEventCallback_UDS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   int                        Result;
   DeviceInfo_t              *DeviceInfo;
   BoardStr_t                 BoardStr;

   /* GATT UDS Client Event Callback data types.                        */
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   GATT_Request_Error_Type_t  ErrorType;
   Word_t                     MTU;
   Word_t                     AttributeHandle;
   Byte_t                    *Value;
   Word_t                     ValueOffset;
   Word_t                     ValueLength;
   Word_t                     BytesWritten;
   Word_t                     RemainingLength;

   /* UDS Response data types.                                          */
   UDS_Characteristic_t       UDS_Characteristic;
   Word_t                     ClientConfiguration;
   DWord_t                    DatabaseChangeIncrement;
   Byte_t                     UserIndex;

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
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice;
               ErrorType      = GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Error Type:       %s.\r\n", (ErrorType == retErrorResponse)? "Response Error" : "Response Timeout");

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  printf("   Request Opcode:   0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                  printf("   Request Handle:   0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                  printf("   Error Code:       0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);

                  /* Print common error codes.                          */
                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED:
                        printf("   Error Mesg:       UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED\r\n");
                        break;
                     case UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
                        printf("   Error Mesg:       UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION\r\n");
                        break;
                     case UDS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED:
                        printf("   Error Mesg:       UDS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED\r\n");
                        break;
                     case UDS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS:
                        printf("   Error Mesg:       UDS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS\r\n");
                        break;
                     case UDS_ERROR_CODE_UNLIKELY_ERROR:
                        printf("   Error Mesg:       UDS_ERROR_CODE_UNLIKELY_ERROR\r\n");
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
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", (Word_t)CallbackParameter);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* Make sure the read response is valid.                 */
               if((Value) && (ValueLength != 0) && (CallbackParameter != 0))
               {
                  /* Get the device information.                        */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* We will simply use the attribute handle type we */
                     /* stored when the request was sent to handle the  */
                     /* response.                                       */
                     switch(DeviceInfo->UDSRequestData.AttributeHandleType)
                     {
                        case ahtUDS_Characteristic:
                           /* Let's simply call the function to decode  */
                           /* the UDS Characteristic.                   */
                           /* * NOTE * We MUST have stored the UDS      */
                           /*          Characteristic type when the     */
                           /*          request was issued.              */
                           if((Result = UDS_Decode_UDS_Characteristic_Response(ValueLength, Value, DeviceInfo->UDSRequestData.Type, &UDS_Characteristic)) == 0)
                           {
                              /* Simply call the internal function to   */
                              /* display the UDS Characteristic data.   */
                              DisplayUDSCharacteristicData(DeviceInfo->UDSRequestData.Type, &UDS_Characteristic);

                              /* If this is a GATT Read request for the */
                              /* First Name, Last Name, Email Address,  */
                              /* or Language, then we need to store the */
                              /* string in a buffer so if a GATT Read   */
                              /* Long request is issued we can print the*/
                              /* entire string we have received thus far*/
                              /* when subsequent responses are received.*/
                              switch(DeviceInfo->UDSRequestData.Type)
                              {
                                 case uctFirstName:
                                 case uctLastName:
                                 case uctEmailAddress:
                                 case uctLanguage:
                                    /* Intentional fall-through since we*/
                                    /* will handle the cases similarly. */

                                    /* Simply copy the string into the  */
                                    /* buffer.                          */
                                    BTPS_MemCopy(DeviceInfo->UDSRequestData.Buffer, Value, ValueLength);

                                    /* Update the length.  Since this is*/
                                    /* the first read we will just      */
                                    /* assign the ValueLength.          */
                                    /* * NOTE * This is also the next   */
                                    /*          offset if a GATT Read   */
                                    /*          Long request is issued. */
                                    DeviceInfo->UDSRequestData.BufferLength = ValueLength;
                                 default:
                                    break;
                              }
                           }
                           else
                              DisplayFunctionError("UDS_Decode_UDS_Characteristic_Response", Result);
                           break;
                        case ahtDatabase_Change_Increment:
                           /* Verify the ValueLength.                   */
                           if(ValueLength >= NON_ALIGNED_DWORD_SIZE)
                           {
                              /* Decode the Value.                      */
                              DatabaseChangeIncrement = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(Value);

                              printf("\r\nDatabase Change Increment:\r\n");
                              printf("   Value: 0x%08X\r\n", DatabaseChangeIncrement);
                           }
                           else
                              printf("\r\nError - Invalid length.\r\n");
                           break;
                        case ahtDatabase_Change_Increment_CCCD:
                        case ahtUser_Control_Point_CCCD:
                           /* Intentional fall through since we can     */
                           /* handle these requests similarly.          */

                           /* Verify the ValueLength.                   */
                           if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                           {
                              /* Decode the Value.                      */
                              ClientConfiguration = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                              /* Display the Characteristic based on the*/
                              /* attribute handle type.                 */
                              if(DeviceInfo->UDSRequestData.AttributeHandleType == ahtDatabase_Change_Increment_CCCD)
                                 printf("\r\nDatabase Change Increment CCCD:\r\n");
                              else
                                 printf("\r\nUser Control Point CCCD:\r\n");

                              /* Display the value.                     */
                              printf("   Value: 0x%04X\r\n", ClientConfiguration);
                           }
                           else
                              printf("\r\nError - Invalid length.\r\n");
                           break;
                        case ahtUser_Index:
                           /* Verify the ValueLength.                   */
                           if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
                           {
                              /* Decode the Value.                      */
                              UserIndex = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

                              printf("\r\nUser Index:\r\n");
                              printf("   Value: 0x%02X\r\n", UserIndex);
                           }
                           else
                              printf("\r\nError - Invalid length.\r\n");
                           break;
                        case ahtUser_Control_Point:
                           printf("\r\nUser Control Point CANNOT be read.\r\n");
                           break;
                        default:
                           printf("\r\nInvalid attribute handle type.\r\n");
                           break;
                     }
                  }
                  else
                     printf("\r\nUnknown UDS Server.\r\n");
               }
               else
                  printf("\r\nValue has a zero length.\r\n");
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
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->RemoteDevice;
               ValueLength    = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->AttributeValueLength;
               Value          = GATT_Client_Event_Data->Event_Data.GATT_Read_Long_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
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
                     /* We will simply use the UDS Characteristic type  */
                     /* we stored when the request was sent to handle   */
                     /* the response.                                   */
                     switch(DeviceInfo->UDSRequestData.Type)
                     {
                        case uctFirstName:
                        case uctLastName:
                        case uctEmailAddress:
                        case uctLanguage:
                           /* Intentional fall-through so we can handle */
                           /* these requests similarly.                 */

                           /* Check to make sure we can hold the value  */
                           /* in the buffer.                            */
                           /* * NOTE * We need to make sure we account  */
                           /*          for the current size of the      */
                           /*          previous GATT Read (Standard or  */
                           /*          Long) request.                   */
                           if((DeviceInfo->UDSRequestData.BufferLength + ValueLength) <= (Word_t)UDS_SA_MAX_BUFFER_SIZE)
                           {
                              /* Simply copy the data at the next offset*/
                              /* in the buffer.  This is the current    */
                              /* length of the buffer.                  */
                              BTPS_MemCopy((DeviceInfo->UDSRequestData.Buffer + DeviceInfo->UDSRequestData.BufferLength), Value, ValueLength);

                              /* Update the offset in case we need to   */
                              /* issue another GATT Read Long request.  */
                              DeviceInfo->UDSRequestData.BufferLength += ValueLength;

                              /* Insert the NULL terminator so we do not*/
                              /* go past the end of the string.         */
                              DeviceInfo->UDSRequestData.Buffer[DeviceInfo->UDSRequestData.BufferLength] = '\0';

                              /* Display the UDS Characteristic type.   */
                              switch(DeviceInfo->UDSRequestData.Type)
                              {
                                 case uctFirstName:
                                    printf("\r\nFirst Name:\r\n");
                                    break;
                                 case uctLastName:
                                    printf("\r\nLast Name:\r\n");
                                    break;
                                 case uctEmailAddress:
                                    printf("\r\nEmail Address:\r\n");
                                    break;
                                 case uctLanguage:
                                    printf("\r\nLanguage:\r\n");
                                    break;
                                 default:
                                    /* Prevent compiler warnings.  This */
                                    /* cannot occur since we checked    */
                                    /* previously.                      */
                                    break;
                              }

                              /* Display the entire string we have      */
                              /* received thus far.                     */
                              printf("   String:  \"%s\"\r\n", (char *)DeviceInfo->UDSRequestData.Buffer);
                           }
                           else
                              printf("\r\nWarning - Buffer is full.\r\n");
                           break;
                        default:
                           printf("\r\nInvalid UDS Characteristic type for GATT Read Long response.\r\n");
                           break;
                     }
                  }
                  else
                     printf("\r\nUnknown UDS Server.\r\n");
               }
               else
                  printf("\r\nValue has a zero length.\r\n");
            }
            else
               printf("\r\nError - Null Read Long Response Data.\r\n");
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
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
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
               ConnectionType  = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->ConnectionType;
               TransactionID   = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->TransactionID;
               RemoteDevice    = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->RemoteDevice;
               BytesWritten    = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->BytesWritten;
               AttributeHandle = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeHandle;
               ValueOffset     = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValueOffset;
               ValueLength     = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValueLength;
               Value           = GATT_Client_Event_Data->Event_Data.GATT_Prepare_Write_Response_Data->AttributeValue;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   Bytes Written:     %u.\r\n", BytesWritten);
               printf("   Handle:            0x%04X.\r\n", AttributeHandle);
               printf("   Value Offset:      %u.\r\n", ValueOffset);
               printf("   Value Length:      %u.\r\n", ValueLength);

               /* * NOTE * We could verify the data that was written    */
               /*          since it should be sent back to us in this   */
               /*          response, but we will not do that here.  We  */
               /*          could cancel the request here if it is not.  */

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if((Value) && (ValueLength != 0) && (CallbackParameter != 0))
               {
                  /* Get the device information.                        */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* We will simply use the UDS Characteristic type  */
                     /* we stored when the request was sent to handle   */
                     /* the response.                                   */
                     switch(DeviceInfo->UDSRequestData.Type)
                     {
                        case uctFirstName:
                        case uctLastName:
                        case uctEmailAddress:
                        case uctLanguage:
                           /* Intentional fall-through so we can handle */
                           /* these requests similarly.                 */

                           /* Increment the offset that we have written */
                           /* thus far.                                 */
                           DeviceInfo->UDSRequestData.BufferOffset += ValueLength;

                           /* Determine if we have more data to prepare */
                           /* from the buffer.                          */
                           if(DeviceInfo->UDSRequestData.BufferOffset >= DeviceInfo->UDSRequestData.BufferLength)
                           {
                              /* If we have prepared the entired buffer.*/
                              if(DeviceInfo->UDSRequestData.BufferOffset == DeviceInfo->UDSRequestData.BufferLength)
                              {
                                 /* Send the GATT Execute Write request */
                                 /* to write the prepared data.         */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, FALSE, GATT_ClientEventCallback_UDS, DeviceInfo->UDSRequestData.AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Execute Write request sent:\r\n");
                                    printf("   TransactionID:  %d\r\n", Result);
                                    printf("   Cancel Write:   No\r\n");
                                 }
                              }
                              else
                              {
                                 printf("\r\nCurrent buffer offset is greater than the buffer length.\r\n");

                                 /* Send the GATT Execute Write request */
                                 /* to cancel the request since an error*/
                                 /* was detected.                       */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, TRUE, GATT_ClientEventCallback_UDS, DeviceInfo->UDSRequestData.AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Execute Write request sent:\r\n");
                                    printf("   TransactionID:  %d\r\n", Result);
                                    printf("   Cancel Write:   Yes\r\n");
                                 }
                              }
                           }
                           else
                           {
                              /* We have more data to prepare from the  */
                              /* buffer.                                */

                              /* Store the attribute handle to reduce   */
                              /* parameter length.                      */
                              AttributeHandle = DeviceInfo->UDSRequestData.AttributeHandle;

                              /* Try to send the remaining length.  This*/
                              /* is the buffer size minus what has been */
                              /* written.                               */
                              RemainingLength = (DeviceInfo->UDSRequestData.BufferLength - DeviceInfo->UDSRequestData.BufferOffset);

                              /* Send the GATT Prepare Write request.   */
                              if((Result = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, RemainingLength, DeviceInfo->UDSRequestData.BufferOffset, &(DeviceInfo->UDSRequestData.Buffer[DeviceInfo->UDSRequestData.BufferOffset]), GATT_ClientEventCallback_UDS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Prepare Write Request sent:\r\n");
                                 printf("   Type:              ");
                                 DisplayUDSCharacteristicType(DeviceInfo->UDSRequestData.Type);
                                 printf("   Offset:            %u\r\n", DeviceInfo->UDSRequestData.BufferOffset);
                                 printf("   TransactionID:     %d\r\n", Result);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Prepare_Write_Request", Result);
                           }
                           break;
                        default:
                           printf("\r\nInvalid UDS Characteristic type for GATT Prepare Write response.\r\n");
                           break;
                     }
                  }
                  else
                     printf("\r\nUnknown UDS Server.\r\n");
               }
               else
                  printf("\r\nInvalid event data/parameters.\r\n");
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
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   MTU:               %u.\r\n", MTU);
            }
            else
               printf("\r\nError - Null Write Response Data.\r\n");
            break;
         case etGATT_Client_Execute_Write_Response:
            printf("\r\netGATT_Client_Execute_Write_Response.\r\n");
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionID;
               ConnectionType = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->ConnectionType;
               TransactionID  = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->TransactionID;
               RemoteDevice   = GATT_Client_Event_Data->Event_Data.GATT_Execute_Write_Response_Data->RemoteDevice;

               /* Print the event data.                                 */
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
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
   int                                     Result;
   DeviceInfo_t                           *DeviceInfo;
   GAP_Encryption_Mode_t                   EncryptionMode;

   /* GATT Connection Event data types.                                 */
   BoardStr_t                              BoardStr;
   BD_ADDR_t                               RemoteBDADDR;
   unsigned int                            GATTConnectionID;
   unsigned int                            TransactionID;
   GATT_Connection_Type_t                  ConnectionType;
   Word_t                                  MTU;
   Word_t                                  AttributeHandle;
   Word_t                                  ValueLength;
   Byte_t                                 *Value;

   DWord_t                                 DatabaseChangeIncrement;
   UDS_User_Control_Point_Response_Data_t  ResponseData;

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
                     /* For each UDS Client that connects the UDS User  */
                     /* Index MUST be set to 'Unknown User'.            */
                     User_Index = UDS_USER_INDEX_UNKNOWN_USER;

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
                     /* For each UDS Client that connects the UDS User  */
                     /* Index MUST be set to 'Unknown User'.            */
                     User_Index = UDS_USER_INDEX_UNKNOWN_USER;

                     if(LocalDeviceIsMaster)
                     {
                        /* Attempt to update the MTU to the maximum     */
                        /* supported.                                   */
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_UDS, 0);
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
         case etGATT_Connection_Server_Notification:
            printf("\r\netGATT_Connection_Server_Notification with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               /* Store the event data.                                 */
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionType;
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice;
               AttributeHandle  = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle;
               ValueLength      = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength;
               Value            = GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue;

               printf("   Connection ID:     %u.\r\n", GATTConnectionID);
               printf("   Connection Type:   %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteBDADDR, BoardStr);
               printf("   Remote Device:     %s.\r\n", BoardStr);
               printf("   Attribute Handle:  0x%04X.\r\n", AttributeHandle);
               printf("   Value Length:      %d.\r\n", ValueLength);

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Make sure this is a notification for the        */
                     /* Database Change Increment.  We will simply      */
                     /* verify the attribute handle.                    */
                     if(AttributeHandle == DeviceInfo->UDSClientInfo.Database_Change_Increment)
                     {
                        /* Verify the ValueLength.                      */
                           if(ValueLength >= NON_ALIGNED_DWORD_SIZE)
                           {
                              /* Decode the Value.                      */
                              DatabaseChangeIncrement = READ_UNALIGNED_DWORD_LITTLE_ENDIAN(Value);

                              printf("\r\nDatabase Change Increment:\r\n");
                              printf("   Value: 0x%08X\r\n", DatabaseChangeIncrement);
                           }
                           else
                              printf("\r\nError - Invalid length.\r\n");
                     }
                     else
                        printf("\r\nAttribute handle is not for the UDS Database Change Increment.\r\n");
                  }
                  else
                     printf("\r\nUDS Service discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nUnknown UDS Server.\r\n");
            }
            else
               printf("\r\nError - Null Server Notification Data.\r\n");
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
               printf("   Connection Type:   %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Remote Device:     %s.\r\n", BoardStr);
               printf("   Attribute Handle:  0x%04X.\r\n", AttributeHandle);
               printf("   Attribute Length:  %d.\r\n", ValueLength);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Only send an indication response if this is a known*/
                  /* UDS Server.                                        */
                  if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, GATTConnectionID, TransactionID)) != 0)
                     DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);

                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Make sure this is an indication for the User    */
                     /* Control Point.  We will simply verify the       */
                     /* attribute handle.                               */
                     if(AttributeHandle == DeviceInfo->UDSClientInfo.User_Control_Point)
                     {
                        /* Verify that we have received the minimum     */
                        /* length for the User Control Point response.  */
                        if(ValueLength >= UDS_USER_CONTROL_POINT_RESPONSE_SIZE(0))
                        {
                           /* Simply call the function to decode the    */
                           /* value.                                    */
                           if((Result = UDS_Decode_User_Control_Point_Response(ValueLength, Value, &ResponseData)) == 0)
                           {
                              /* Display the User Control Point         */
                              /* response.                              */
                              DisplayUserControlPointResponseData(&ResponseData);
                           }
                           else
                              DisplayFunctionError("UDS_Decode_User_Control_Point_Response", Result);
                        }
                        else
                           printf("\r\nError - Invalid length.\r\n");
                     }
                     else
                        printf("\r\nAttribute handle is not for the UDS Database Change Increment.\r\n");
                  }
                  else
                     printf("\r\nUDS Service discovery has not been performed.\r\n");
               }
               else
               {
                  printf("\r\nError - Unknown remote device.\r\n");
               }
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
                     case sdUDS:
                        /* Attempt to populate the handles for the UDS  */
                        /* Service.                                     */
                        UDSPopulateHandles(&(DeviceInfo->UDSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                     case sdUDS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_UDS_SERVICE_DISCOVERY_COMPLETE;

                        printf("\r\nUDS Service Discovery Summary\r\n\r\n");
                        printf("   UDS Characteristics:\r\n");
                        printf("      First Name                        : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.First_Name                       ? "Supported" : "Not Supported"));
                        printf("      Last Name                         : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Last_Name                        ? "Supported" : "Not Supported"));
                        printf("      Email Address                     : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Email_Address                    ? "Supported" : "Not Supported"));
                        printf("      Age                               : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Age                              ? "Supported" : "Not Supported"));
                        printf("      Date of Birth                     : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Date_Of_Birth                    ? "Supported" : "Not Supported"));
                        printf("      Gender                            : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Gender                           ? "Supported" : "Not Supported"));
                        printf("      Weight                            : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Weight                           ? "Supported" : "Not Supported"));
                        printf("      Height                            : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Height                           ? "Supported" : "Not Supported"));
                        printf("      VO2 Max                           : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.VO2_Max                          ? "Supported" : "Not Supported"));
                        printf("      Heart Rate Max                    : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Heart_Rate_Max                   ? "Supported" : "Not Supported"));
                        printf("      Resting Heart Rate                : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Resting_Heart_Rate               ? "Supported" : "Not Supported"));
                        printf("      Maximum Recommended Heart Rate    : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Maximum_Recommended_Heart_Rate   ? "Supported" : "Not Supported"));
                        printf("      Aerobic Threshold                 : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Aerobic_Threshold                ? "Supported" : "Not Supported"));
                        printf("      Anaerobic Threshold               : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Anaerobic_Threshold              ? "Supported" : "Not Supported"));
                        printf("      Sport Type                        : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Sport_Type                       ? "Supported" : "Not Supported"));
                        printf("      Date of Threshold                 : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Date_Of_Threshold                ? "Supported" : "Not Supported"));
                        printf("      Waist Circumference               : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Waist_Circumference              ? "Supported" : "Not Supported"));
                        printf("      Hip Circumference                 : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Hip_Circumference                ? "Supported" : "Not Supported"));
                        printf("      Fat Burn Heart Rate Lower Limit   : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Fat_Burn_Heart_Rate_Lower_Limit  ? "Supported" : "Not Supported"));
                        printf("      Fat Burn Heart Rate Upper Limit   : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Fat_Burn_Heart_Rate_Upper_Limit  ? "Supported" : "Not Supported"));
                        printf("      Aerobic Heart Rate Lower Limit    : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Aerobic_Heart_Rate_Lower_Limit   ? "Supported" : "Not Supported"));
                        printf("      Aerobic Heart Rate Upper Limit    : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Aerobic_Heart_Rate_Upper_Limit   ? "Supported" : "Not Supported"));
                        printf("      Anaerobic Heart Rate Lower Limit  : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Anaerobic_Heart_Rate_Lower_Limit ? "Supported" : "Not Supported"));
                        printf("      Anaerobic Heart Rate Upper Limit  : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Anaerobic_Heart_Rate_Upper_Limit ? "Supported" : "Not Supported"));
                        printf("      Five Zone Heart Rate Limits       : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Five_Zone_Heart_Rate_Limits      ? "Supported" : "Not Supported"));
                        printf("      Three Zone Heart Rate Limits      : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Three_Zone_Heart_Rate_Limits     ? "Supported" : "Not Supported"));
                        printf("      Two Zone Heart Rate Limit         : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Two_Zone_Heart_Rate_Limit        ? "Supported" : "Not Supported"));
                        printf("      Language                          : %s\r\n", (DeviceInfo->UDSClientInfo.UDS_Characteristic.Language                         ? "Supported" : "Not Supported"));

                        /* Remaining UDS Characteristics.               */
                        printf("   UDS Database Change Increment        : %s\r\n", (DeviceInfo->UDSClientInfo.Database_Change_Increment      ? "Supported" : "Not Supported"));
                        printf("   UDS Database Change Increment CCCD   : %s\r\n", (DeviceInfo->UDSClientInfo.Database_Change_Increment_CCCD ? "Supported" : "Not Supported"));
                        printf("   UDS User Index                       : %s\r\n", (DeviceInfo->UDSClientInfo.User_Index                     ? "Supported" : "Not Supported"));
                        printf("   UDS User Control Point               : %s\r\n", (DeviceInfo->UDSClientInfo.User_Control_Point             ? "Supported" : "Not Supported"));
                        printf("   UDS User Control Point CCCD          : %s\r\n", (DeviceInfo->UDSClientInfo.User_Control_Point_CCCD        ? "Supported" : "Not Supported"));
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
         case UDSP_PARAMETER_VALUE:
            /* The Transport selected was UDSP, check to see if the     */
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, UDSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, UDSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}
