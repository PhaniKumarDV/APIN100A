/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< LinuxOTP.c >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXOTP - Linux Bluetooth Object Transfer Profile using GATT (LE/BREDR)  */
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
#include <errno.h>
#include <string.h>

#include "LinuxOTP.h"            /* Application Header.                       */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTOTS.h"            /* Main SS1 OTS Service Header.              */
#include "BTPSFILE.h"            /* BTPS File I/O Prototypes/Constants.       */

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

#define OTSP_PARAMETER_VALUE                        (2)  /* Denotes the value */
                                                         /* passed in on the  */
                                                         /* command line for  */
                                                         /* running with the  */
                                                         /* transport set to  */
                                                         /* OTSP.             */

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
                                                         /* create a OTS     */
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

#define DATE_TIME_STRING_LENGTH                    (14)  /* Denotes the number*/
                                                         /* of characters     */
                                                         /* required to       */
                                                         /* represent a Date  */
                                                         /* string of form    */
                                                         /* YYYYMMDDHHMMSS    */

#define OTP_USE_LE_AUTOMATIC_CREDIT_MODE           (TRUE)
                                                         /* Denotes the       */
                                                         /* LE based credit   */
                                                         /* Mode for          */
                                                         /* Object Transfer   */
                                                         /* Channel           */
                                                         /* connections over  */
                                                         /* LE.               */

#define OTP_MAXIMUM_DIRECTORY_PATH_LENGTH          (BTPS_MAXIMUM_PATH_LENGTH)
                                                         /* Denotes the max   */
                                                         /* directory path    */
                                                         /* supported         */
                                                         /* by the server.    */

#define OTP_MAXIMUM_COMBINED_PATH_LENGTH           (OTP_MAXIMUM_DIRECTORY_PATH_LENGTH + OTS_MAXIMUM_OBJECT_NAME_LENGTH)
                                                         /* Denotes the max   */
                                                         /* combined path     */
                                                         /* supported         */
                                                         /* by the server.    */

#define OTP_DEFAULT_SERVER_DIRECTORY_PATH          ("./server/")
                                                         /* The default server*/
                                                         /* directory to store*/
                                                         /* OTS Objects.      */

#define OTP_DEFAULT_NEW_FILE_NAME                  ("NewFile")
                                                         /* The default name  */
                                                         /* for a new OTS     */
                                                         /* Object that has   */
                                                         /* been created via  */
                                                         /* the OACP Create   */
                                                         /* Procedure.        */

#define OTP_MAXIMUM_GENERATED_FILE_SIZE            (500000)
                                                         /* Denotes the max   */
                                                         /* supported size    */
                                                         /* by the server for */
                                                         /* generating new    */
                                                         /* file objects.     */

#define OTP_MAXIMUM_SUPPORTED_OBJECTS              (5)
                                                         /* Denotes the max   */
                                                         /* objects supported */
                                                         /* by the server.    */

#define OTP_DEFAULT_OTS_CHARACTERISTIC_FLAGS       (OTS_CHARACTERISTIC_FLAGS_OBJECT_FIRST_CREATED |  \
                                                    OTS_CHARACTERISTIC_FLAGS_OBJECT_LAST_MODIFIED |  \
                                                    OTS_CHARACTERISTIC_FLAGS_OBJECT_LIST_FILTER   |  \
                                                    OTS_CHARACTERISTIC_FLAGS_OBJECT_CHANGED)
                                                         /* Denotes the OTS   */
                                                         /* Characteristics   */
                                                         /* (opt) supported by*/
                                                         /* the OTP Server.   */

#define OTP_DEFAULT_OTS_PROPERTY_FLAGS             (OTS_PROPERTY_FLAGS_OBJECT_NAME_ENABLE_WRITE           |  \
                                                    OTS_PROPERTY_FLAGS_OBJECT_FIRST_CREATED_ENABLE_WRITE)
                                                         /* Denotes the OTS   */
                                                         /* Characteristic    */
                                                         /* (opt) properties  */
                                                         /* supported by      */
                                                         /* the OTP Server.   */

#define OTP_DEFAULT_OACP_FEATURES                  (OTS_FEATURE_OACP_CREATE_OP_CODE_SUPPORTED             |  \
                                                    OTS_FEATURE_OACP_DELETE_OP_CODE_SUPPORTED             |  \
                                                    OTS_FEATURE_OACP_CALCULATE_CHECKSUM_OP_CODE_SUPPORTED |  \
                                                    OTS_FEATURE_OACP_EXECUTE_OP_CODE_SUPPORTED            |  \
                                                    OTS_FEATURE_OACP_READ_OP_CODE_SUPPORTED               |  \
                                                    OTS_FEATURE_OACP_WRITE_OP_CODE_SUPPORTED              |  \
                                                    OTS_FEATURE_OACP_APPENDING_SUPPORTED                  |  \
                                                    OTS_FEATURE_OACP_TRUNCATION_SUPPORTED                 |  \
                                                    OTS_FEATURE_OACP_PATCHING_SUPPORTED                   |  \
                                                    OTS_FEATURE_OACP_ABORT_OP_CODE_SUPPORTED)
                                                         /* Denotes the       */
                                                         /* default OACP      */
                                                         /* features          */
                                                         /* supported by      */
                                                         /* the OTP Server.   */

#define OTP_DEFAULT_OLCP_FEATURES                  (OTS_FEATURE_OLCP_GO_TO_OP_CODE_SUPPORTED                     |  \
                                                    OTS_FEATURE_OLCP_ORDER_OP_CODE_SUPPORTED                     |  \
                                                    OTS_FEATURE_OLCP_REQUEST_NUMBER_OF_OBJECTS_OP_CODE_SUPPORTED |  \
                                                    OTS_FEATURE_OLCP_CLEAR_MARKING_OP_CODE_SUPPORTED)
                                                         /* Denotes the       */
                                                         /* default OLCP      */
                                                         /* features          */
                                                         /* supported by      */
                                                         /* the OTP Server.   */

#define OTP_DEFAULT_OBJECT_PROPERTIES              (OTS_OBJECT_PROPERTIES_DELETE   |  \
                                                    OTS_OBJECT_PROPERTIES_EXECUTE  |  \
                                                    OTS_OBJECT_PROPERTIES_READ     |  \
                                                    OTS_OBJECT_PROPERTIES_WRITE    |  \
                                                    OTS_OBJECT_PROPERTIES_APPEND   |  \
                                                    OTS_OBJECT_PROPERTIES_TRUNCATE |  \
                                                    OTS_OBJECT_PROPERTIES_PATCH    |  \
                                                    OTS_OBJECT_PROPERTIES_MARK)
                                                         /* Denotes the       */
                                                         /* default OTS Object*/
                                                         /* properties        */
                                                         /* supported by      */
                                                         /* the OTP Server.   */

#define OTP_DEFAULT_OBJECT_FLAGS                   (OTS_OBJECT_RECORD_FLAGS_CURRENT_SIZE_PRESENT   |  \
                                                    OTS_OBJECT_RECORD_FLAGS_ALLOCATED_SIZE_PRESENT |  \
                                                    OTS_OBJECT_RECORD_FLAGS_FIRST_CREATED_PRESENT  |  \
                                                    OTS_OBJECT_RECORD_FLAGS_LAST_MODIFIED_PRESENT  |  \
                                                    OTS_OBJECT_RECORD_FLAGS_PROPERTIES_PRESENT)
                                                         /* Denotes the       */
                                                         /* default OTS Object*/
                                                         /* flags             */
                                                         /* supported by      */
                                                         /* the OTP Server.   */

#define OTP_MAXIMUM_SUPPORTED_CLIENTS             (2)    /* Denotes the max   */
                                                         /* number of         */
                                                         /* supported OTP     */
                                                         /* Clients.          */

   /* The following constants represent the application's default OTS LE*/
   /* Channel parameters for the Object Transfer Channel.               */
#define OTP_CHANNEL_PARAMETERS_FLAGS              (OTP_USE_LE_AUTOMATIC_CREDIT_MODE ? 0 : L2CA_LE_CHANNEL_PARAMETER_FLAGS_MANUAL_CREDIT_MODE)
#define OTP_CHANNEL_PARAMETERS_MAX_SDU_SIZE       ((1080) - NON_ALIGNED_WORD_SIZE)
#define OTP_CHANNEL_PARAMETERS_MAX_PDU_SIZE       (1080)
#define OTP_CHANNEL_PARAMETERS_PDU_QUEUE_DEPTH    (20)
#define OTP_CHANNEL_PARAMETERS_MAX_CREDITS        (50)

   /* The following constants represent the application's default       */
   /* queueing parameters for the Object Transfer Channel.              */
#define OTP_CHANNEL_QUEUEING_PARAMETERS_LIMIT_BY_PACKETS  (L2CA_QUEUEING_FLAG_LIMIT_BY_PACKETS)
#define OTP_CHANNEL_QUEUEING_PARAMETERS_MAX_SDU_DEPTH     (3)
#define OTP_CHANNEL_QUEUEING_PARAMETERS_SDU_THRESHOLD     (0)

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxOTP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxOTP_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxOTP"

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
   sdOTS,
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

   /* The following union holds data that needs to be stored during an  */
   /* LE/BREDR connection event that will need to be stored in the      */
   /* device information when the GATT connection event is received.    */
typedef union
{
   Link_Key_t            LinkKey;
   GAP_LE_Address_Type_t AddressType;
} Connection_Data_t;

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
#define DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE    0x0100
#define DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS    0x0200
#define DEVICE_INFO_FLAGS_OTS_OLCP_PROCEDURE_IN_PROGRESS    0x0400

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

   /* The following structure defines the data that needs to be stored  */
   /* for each OTS Object entry stored on the OTP Server.               */
typedef struct OTP_Object_Entry_t
{
   Boolean_t         Valid;
   OTS_Object_Data_t Data;
   Boolean_t         Locked;
} OTP_Object_Entry_t;

   /* Defines the bitmask values that may be set for the Flags field in */
   /* the OTP_Channel_Data_t structure.                                 */
#define OTP_CHANNEL_FLAGS_CONNECTED                      0x0001
#define OTP_CHANNEL_FLAGS_RESPONSE_REQUESTED             0x0002
#define OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL                0x0004
#define OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL             0x0008
#define OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS     0x0010
#define OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS    0x0020
#define OTP_CHANNEL_FLAGS_BUFFER_READY_READY_TO_RECEIVE  0x0040

   /* The following structure stores the data that is needed by the     */
   /* Object Transfer Channel (OTC).                                    */
   /* * NOTE * The Flags field holds state information for the OTC.     */
   /*          These are located above.                                 */
   /* * NOTE * The LE_Channel_Parameters field contains the LE Channel  */
   /*          Parameters to use for an OTP Client.                     */
   /* * NOTE * The Queueing_Parameters contain optional queueing        */
   /*          parameters that may be used when sending data over the   */
   /*          OTC.  This controls how much data can be queued by the   */
   /*          local device to be sent over the OTC.                    */
   /* * NOTE * The Channel_ID is the CID that is received by OTS Channel*/
   /*          connection events.  We will store it here since it is    */
   /*          required in order to use the OTS Channel API's.          */
   /* * NOTE * The Max_Data_Size is the MAX SDU size that is received by*/
   /*          OTS Channel connection events.  We will store it here so */
   /*          that the local device can determine how much data can be */
   /*          sent over the OTC.                                       */
   /* * NOTE * The Current and Ending Offset fields are used to         */
   /*          determine where to read/write received data.  It is also */
   /*          used by both the sending and receiving devices to        */
   /*          determine when a transfer has completed (Current_Offset =*/
   /*          Ending Offset).                                          */
   /* * NOTE * The Buffer field is used to hold the memory allocated for*/
   /*          an OTS Object's contents that will transfered/received   */
   /*          over the Object Transfer Channel.                        */
   /* * NOTE * The File_Descriptor holds the file descriptor that is    */
   /*          currently open for read/writing to a file.               */
typedef struct OTP_Channel_Data_t
{
   Word_t                        Flags;
   L2CA_LE_Channel_Parameters_t  LE_Channel_Parameters;
   L2CA_Queueing_Parameters_t    Queueing_Parameters;
   Word_t                        Channel_ID;
   Word_t                        Max_Data_Size;
   DWord_t                       Current_Offset;
   DWord_t                       Ending_Offset;
   Byte_t                       *Buffer;
   DWord_t                       Buffer_Length;
   BTPS_File_Descriptor_t        File_Descriptor;
} OTP_Channel_Data_t;

   /* The following structure defines a Filtered Object List Node.      */
   /* * NOTE * This structure is simply a wrapper for the pointer to the*/
   /*          OTS Object data so that we can use it in the linked list.*/
typedef struct _tagOTP_Filtered_Object_Node_t
{
   OTS_Object_Data_t                     *ObjectDataPtr;
   struct _tagOTP_Filtered_Object_Node_t *NextPtr;
   struct _tagOTP_Filtered_Object_Node_t *PreviousPtr;
} OTP_Filtered_Object_Node_t;

#define OTP_FILTERED_OBJECT_NODE_SIZE  (sizeof(OTP_Filtered_Object_Node_t))

   /* The following structure defines the Filtered Object List.  This   */
   /* list will be used to maintain a list of OTP_Filtered_Object_Node_t*/
   /* structures that point to a valid OTS Object's data on the OTS     */
   /* Server.                                                           */
   /* ** NOTE ** This list will be exposed for each OTP Client.  This   */
   /*            way we can expose the Current Object that has been     */
   /*            selected by the OTP Client and the current OTS Object  */
   /*            List Filters that the OTP Client has selected without  */
   /*            affecting other OTP Clients.  This list may also be    */
   /*            sorted by the OTP Client via the OLCP Order Procedure. */
   /*            Nodes in this list will directory correspond to the OTS*/
   /*            Objects List stored on the OTP Server (based on applied*/
   /*            OTS Object List Filters).  The OTP Server's Object List*/
   /*            will not be modified unless an OTS Object is           */
   /*            created/deleted.  In this case it will be added/removed*/
   /*            from the Filtered Objects List as well as the OTS      */
   /*            Objects List.                                          */
   /* * NOTE * The Current_Object field is a pointer to the Node in the */
   /*          Filtered Objects List that is the current object selected*/
   /*          by the OTP Client.  This will be NULL if the OTS Object  */
   /*          List Filters are set such that it is excluded from the   */
   /*          Filtered Objects List.                                   */
   /* * NOTE * We will store the currently applied OTS Object List      */
   /*          Filter data here since it is applied to the Filtered     */
   /*          Object List.  It is worth noting that there MUST be      */
   /*          OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS supported.     */
   /* * NOTE * We will store the list sort type here that way it can be */
   /*          applied to the Filtered Objects List every time is       */
   /*          updated.                                                 */
typedef struct _tagOTP_Filtered_Object_List_t
{
   OTP_Filtered_Object_Node_t    *Current_Object;
   OTS_Object_List_Filter_Data_t  List_Filter_Data[OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS];
   Boolean_t                      Sorted;
   OTS_List_Sort_Order_Type_t     Sort_Type;
   Byte_t                         Number_Of_Nodes;
   OTP_Filtered_Object_Node_t    *HeadPtr;
   OTP_Filtered_Object_Node_t    *TailPtr;
} OTP_Filtered_Object_List_t;

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices.  For slave we will not use this*/
   /* structure.                                                        */
   /* * NOTE * The FilteredObjectList will contain the OTS Objects List */
   /*          that is exposed for each OTP Client (This will only be   */
   /*          stored on the OTP Server).  This is PER OTP Client, which*/
   /*          is why it is stored here.                                */
   /* * NOTE * The ChannelData is stored here for the Object Transfer   */
   /*          Channel since it needs to be stored for each side of the */
   /*          connection.                                              */
typedef struct _tagDeviceInfo_t
{
   Word_t                      Flags;
   Boolean_t                   RemoteDeviceIsMaster;
   Byte_t                      EncryptionKeySize;
   GAP_LE_Address_Type_t       ConnectionAddressType;
   unsigned int                ConnectionID;
   BD_ADDR_t                   ConnectionBD_ADDR;
   Link_Key_t                  LinkKey;
   Long_Term_Key_t             LTK;
   Random_Number_t             Rand;
   Word_t                      EDIV;
   OTS_Server_Information_t    OTSServerInfo;
   OTP_Filtered_Object_List_t  FilteredObjectList;
   GATT_Client_Info_t          GATTClientInfo;
   DIS_Client_Info_t           DISClientInfo;
   GAPS_Client_Info_t          GAPSClientInfo;
   OTS_Client_Information_t    OTSClientInfo;
   OTP_Channel_Data_t          ChannelData;
   Boolean_t                   ConcurrencyLimitExceeded;
   struct _tagDeviceInfo_t    *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE  (sizeof(DeviceInfo_t))

   /* The following structure contains the data that needs to be stored */
   /* for a directory on the OTP Server.                                */
   /* * NOTE * The Path field stores the path to the specified directory*/
   /*          where the OTS Object's contents (files) are stored on the*/
   /*          file system.  This path MUST be prefixed to the file name*/
   /*          before a file is opened, closed, or deleted.             */
   /* * NOTE * The Directory_Descriptor holds the descriptor for the    */
   /*          directory that has been opened for the specified Path.   */
typedef struct OTP_Server_Directory_Data_t
{
   char                         Path[OTP_MAXIMUM_DIRECTORY_PATH_LENGTH];
   BTPS_Directory_Descriptor_t  Directory_Descriptor;
} OTP_Server_Directory_Data_t;

   /* The following structure defines the data that needs to be stored  */
   /* for the OTP Server for ALL OTP Clients.                           */
   /* * NOTE * The OTS_Feature stores the OACP and OLCP Procedures that */
   /*          are supported by the OTP Server.                         */
   /* * NOTE * The Object_List field contains the list of all OTS       */
   /*          Objects that are stored on the OTP Server.  Each OTP     */
   /*          Client will interact with the FilteredObjectList stored  */
   /*          in the OTP Client's device information stored on the OTP */
   /*          Server.  The Filtered Object List will contain pointers  */
   /*          to each OTS Object's data that is stored in the          */
   /*          Objects_List.  Only OTS Objects that are valid for the   */
   /*          currently applied OTS Object List Filters will be exposed*/
   /*          via the Filtered Objects List.                           */
   /* * NOTE * The Directory_List is used to hold an array of OTS       */
   /*          Objects that are going to be formatted for the OTS       */
   /*          Directory Listing Object's contents.                     */
   /* * NOTE * The Number_Of_Objects will contain the number of OTS     */
   /*          Objects that are stored in the Directory_List.  This     */
   /*          number will directly correspond to the number of valid   */
   /*          OTS Objects stored in the Object_List.                   */
typedef struct OTP_Server_Data_t
{
   OTS_Feature_Data_t           OTS_Feature;
   OTP_Server_Directory_Data_t  Directory_Data;
   OTP_Object_Entry_t           Object_List[OTP_MAXIMUM_SUPPORTED_OBJECTS];
   OTS_Object_Data_t            Directory_List[OTP_MAXIMUM_SUPPORTED_OBJECTS];
   unsigned int                 Number_Of_Objects;
} OTP_Server_Data_t;

   /* The following enumeration defines the attribute handle types.     */
   /* This has been included to make sending requests and receiving     */
   /* responses easier to read.                                         */
typedef enum
{
   ahtOTSFeature,
   ahtObjectMetadata,
   ahtObjectActionControlPoint,
   ahtObjectActionControlPoint_CCCD,
   ahtObjectListControlPoint,
   ahtObjectListControlPoint_CCCD,
   ahtObjectListFilter_1,
   ahtObjectListFilter_2,
   ahtObjectListFilter_3,
   ahtObjectChanged,
   ahtObjectChanged_CCCD,
   ahtServiceChanged_CCCD
} OTP_Attribute_Handle_Type_t;

   /* The following structure holds the data that is needed by the OTP  */
   /* Client for each GATT request/response.                            */
   /* * NOTE * We will add one to the maximum size of the buffer for the*/
   /*          NULL terminator so we can display the OTS Name data type */
   /*          as a c-string.                                           */
   /* * NOTE * We will add one to the maximum size of the buffer to hold*/
   /*          the OTS Object List Filter type received in a read       */
   /*          response for an OTS Object List Filter.  This way we can */
   /*          re-display the OTS Object List Filter for a GATT Read    */
   /*          Long Value response.                                     */
   /* * NOTE * The BufferOffset, BufferLength, and Buffer will be used  */
   /*          to store the data needed between requests for GATT       */
   /*          Read/Write Long procedures.                              */
   /* * NOTE * The UseReadLong field will be used to indicate that the  */
   /*          user wishes to use a GATT Read Long Value request for an */
   /*          OTS Characteristic that could not fit in the previously  */
   /*          received GATT Read Response.                             */
   /* * NOTE * The FileName field holds the file name that OTP Client   */
   /*          wishes to open in the current directory to transfer to an*/
   /*          OTP Server (OACP Write Procedure).                       */
   /* * NOTE * The File Offset that indicates the location in the file  */
   /*          (FileName) the OTP Client wishes to read from.           */
   /* * NOTE * The Data field holds all data types that may be sent in a*/
   /*          request to the OTP Server.                               */
typedef struct _tagOTP_Request_Data_t
{
   OTP_Attribute_Handle_Type_t       AttributeHandleType;
   Word_t                            AttributeHandle;
   OTS_Object_Metadata_Type_t        MetadataType;
   Word_t                            BufferOffset;
   Word_t                            BufferLength;
   Byte_t                            Buffer[OTS_MAXIMUM_OBJECT_NAME_LENGTH+2];
   Boolean_t                         UseReadLong;
   char                             *FileName;
   DWord_t                           FileOffset;
   union
   {
      Word_t                         Configuration;
      OTS_Object_Metadata_Data_t     Metadata;
      OTS_OACP_Request_Data_t        OACP_RequestData;
      OTS_OLCP_Request_Data_t        OLCP_RequestData;
      OTS_Object_List_Filter_Data_t  ListFilterData;
   } Data;
} OTP_Request_Data_t;

   /* The following structure holds the data for the currently selected */
   /* object that MUST be cached by the OTP Client.                     */
   /* * NOTE * We will add one to the maximum size of the buffer for the*/
   /*          NULL terminator so we can display the OTS Object Name as */
   /*          a c-string.                                              */
   /* * NOTE * We will read all OTS Object Metadata Characteristics,    */
   /*          after the current object has been selected or modified,  */
   /*          however we will only store the Name, CurrentSize, and    */
   /*          ObjectID.  We store the Name so that we can automatically*/
   /*          create a file with the same name as the currently        */
   /*          selected OTS Object for the OACP Read Procedure.  We     */
   /*          store the CurrentSize to allow the OTP Client to easily  */
   /*          issue requests to read/write and entire OTS Object's     */
   /*          contents.  We store ObjectID so that we can automatically*/
   /*          determine if the OTS Directory List Object has been      */
   /*          selected or another OTS Object.                          */
typedef struct _tagOTP_Current_Object_t
{
   Boolean_t          Valid;
   Byte_t             Name[OTS_MAXIMUM_OBJECT_NAME_LENGTH+1];
   DWord_t            CurrentSize;
   OTS_UINT48_Data_t  ObjectID;
} OTP_Current_Object_t;

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
static unsigned int        OTSInstanceID;
                                                    /* The following holds the OTS     */
                                                    /* Instance IDs that are returned  */
                                                    /* from OTS_Initialize_XXX().      */

static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */

static Connection_Data_t   ConnectionData;          /* Holds the connection            */
                                                    /* data that needs to be stored    */
                                                    /* during a connection so that it  */
                                                    /* can be stored later in the      */
                                                    /* device information.             */

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

static unsigned int        Connections;             /* Inicates the number of connected*/
                                                    /* remote devices.                 */

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

   /* OTP Server globals.                                               */

static OTP_Server_Data_t   ServerData;              /* The OTP Server data.            */

static DWord_t             ObjectIDCtr;             /* The OTS Object ID counter.      */

static OTS_List_Sort_Order_Type_t SortType;         /* The global sort type needed for */
                                                    /* for the comparator function.    */

   /* OTP Client globals.                                               */

static OTP_Request_Data_t  RequestData;             /* The OTP Client request data.    */

static OTP_Current_Object_t  CurrentObject;         /* The current selected OTS Object */
                                                    /* by the OTP Client.              */

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

static int DisplayRemoteDevices(ParameterList_t *TempParam);
static int SelectRemoteDevice(ParameterList_t *TempParam);

   /* Generic Attribute Profile Functions and Commands.                 */
static void GATTPopulateHandles(GATT_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static void DisplayGATTDiscoverySummary(GATT_Client_Info_t *ClientInfo);
static void DisplayKnownServiceName(GATT_UUID_t *UUID);

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
static void OTSPopulateHandles(OTS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreDescriptorHandles(OTS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle);
static void DisplayOTSDiscoverySummary(OTS_Client_Information_t *ClientInfo);

static int RegisterOTS(void);
static int UnregisterOTS(void);

static int SetupOTSServer(void);
static void CleanupOTSServer(void);
static void DisconnectionCleanup(DeviceInfo_t *DeviceInfo);

static int OpenFileContentsServer(BTPS_File_Descriptor_t *FileDescriptor, char *FileName, BTPS_Open_Mode_t OpenMode, DWord_t Offset, OTS_Object_Data_t *ObjectDataPtr);
static int OpenFileContentsClient(BTPS_File_Descriptor_t *FileDescriptor, char *FilePath, BTPS_Open_Mode_t OpenMode, DWord_t Offset, DWord_t *AllocatedSize);
static void CloseFileContents(BTPS_File_Descriptor_t *FileDescriptor);
static int DeleteFileContents(char *FileName);
static DWord_t ReadFileContents(BTPS_File_Descriptor_t FileDescriptor, DWord_t BufferLength, Byte_t *Buffer);
static DWord_t WriteFileContents(BTPS_File_Descriptor_t FileDescriptor, DWord_t BufferLength, Byte_t *Buffer);
static int ReadFileContentsInfo(char *FilePath, OTS_Date_Time_Data_t *DateTimeData, DWord_t *AllocatedSize);
static int RenameFileServer(char *OldName, char *NewName);

static int OpenDirectory(BTPS_Directory_Descriptor_t *DirectoryDescriptor, char *DirectoryPath);
static void CloseDirectory(BTPS_Directory_Descriptor_t *DirectoryDescriptor);
static int MakeDirectory(char *DirectoryPath);
static void DeleteDirectory(char *DirectoryPath);

static void CreateFileObject(char *FileName, DWord_t Properties);
static OTP_Object_Entry_t* FindOTPObjectEntry(OTS_UINT48_Data_t *ObjectID);

static void AddFilteredObjectNode(OTP_Filtered_Object_List_t *List, OTP_Filtered_Object_Node_t *Node);
static OTP_Filtered_Object_Node_t *RemoveFilteredObjectNode(OTP_Filtered_Object_List_t *List, OTP_Filtered_Object_Node_t *Node);
static void CreateFilteredObjectsList(OTP_Filtered_Object_List_t *List, OTS_UINT48_Data_t *ObjectID);
static void DeleteFilteredObjectsList(OTP_Filtered_Object_List_t *List);
static void UpdateFilteredObjectsList(OTP_Filtered_Object_List_t *List);
static void UpdateAllFilteredObjectLists(void);
static int VerifyNodeForFilteredObjectsList(OTP_Filtered_Object_List_t *List, OTS_Object_Data_t *ObjectDataPtr);
static int SortFilteredObjectsList(OTP_Filtered_Object_List_t *List);

static int StrToOTSObjectType(char *UUIDStr, GATT_UUID_t *UUID);
static int StrToOTSDateTime(char *DateTimeStr, OTS_Date_Time_Data_t *DateTimeData);

static void CreateProcedure(OTP_Filtered_Object_List_t *List, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData);
static void DeleteProcedure(OTP_Filtered_Object_List_t *List, OTS_OACP_Response_Data_t *ResponseData);
static void CalculateChecksumProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData);
static void ExecuteProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTS_OACP_Response_Data_t *ResponseData);
static void ReadProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData);
static void WriteProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData);
static void AbortProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Response_Data_t *ResponseData);

static void FirstProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);
static void LastProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);
static void PreviousProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);
static void NextProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);
static void GoToProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Request_Data_t *RequestData, OTS_OLCP_Response_Data_t *ResponseData);
static void OrderProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Request_Data_t *RequestData, OTS_OLCP_Response_Data_t *ResponseData);
static void RequestNumberOfObjectsProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);
static void ClearMarkingProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData);

static int ObjectComparator(const void *Object1, const void *Object2);
static int CurrentSizeCompare(DWord_t CurrentSize1, DWord_t CurrentSize2);
static int ObjectTypeCompare(GATT_UUID_t *UUID1, GATT_UUID_t *UUID2);
static int DateTimeCompare(OTS_Date_Time_Data_t *DateTime1, OTS_Date_Time_Data_t *DateTime2);
static int ObjectIDCompare(OTS_UINT48_Data_t *Number1, OTS_UINT48_Data_t *Number2);

static void ReverseString(Byte_t NameLength, Byte_t *Name);
static int CompareDateTimeRange(OTS_Date_Time_Data_t *Current, OTS_Date_Time_Data_t *Lower, OTS_Date_Time_Data_t *Upper);

static void ConvertObjectDataToObjectMetadata(OTS_Object_Metadata_Type_t Type, OTS_Object_Data_t *CurrentObject, OTS_Object_Metadata_Data_t *Data);
static Byte_t StoreObjectMetadataForCurrentObject(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Data, OTS_Object_Data_t *CurrentObject);

static int ReadOTSCharacteristic(void);
static int WriteOTSCharacteristic(void);

static int SendObjectMetadataWriteRequest(DeviceInfo_t *DeviceInfo);
static int SendOACPWriteRequest(DeviceInfo_t *DeviceInfo);
static int SendOLCPWriteRequest(DeviceInfo_t *DeviceInfo);
static int SendObjectListFilterWriteRequest(DeviceInfo_t *DeviceInfo, Word_t AttributeHandle);
static int SendCCCDWriteRequest(DeviceInfo_t *DeviceInfo, Word_t AttributeHandle);

static void ReadObjectMetadata(void);
static void MapAttributeHandleToObjectMetadataType(OTS_Client_Information_t *ClientInfo, Word_t AttributeHandle, OTS_Object_Metadata_Type_t *Type);
static void IndicateObjectChanged(OTS_Object_Changed_Data_t *ObjectChangedData);

static void GetConnectionMode(void);
static void SetConnectionMode(OTS_Channel_Connection_Mode_t Mode);

static void SetParameters(L2CA_LE_Channel_Parameters_t *LE_Channel_Parameters, L2CA_Queueing_Parameters_t *Queueing_Parametes);

static void OpenChannel(void);
static void CloseChannel(void);

static void SendObjectContents(OTP_Channel_Data_t *ChannelData);
static void ProcessObjectContents(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, Word_t DataLength, Byte_t *Data);
static void ProcessDirectoryListingObjectContents(OTP_Channel_Data_t *ChannelData, Word_t DataLength, Byte_t *Data);
static inline void DisplayTransferProgress(OTP_Channel_Data_t *ChannelData);

static void CleanupChannel(OTP_Channel_Data_t *ChannelData);

static void DecodeDisplayOTSFeature(unsigned int ValueLength, Byte_t *Value);
static void DecodeDisplayObjectMetadata(OTS_Client_Information_t *ClientInfo, Word_t AttributeHandle, unsigned int ValueLength, Byte_t *Value);
static void DecodeDisplayOACPResponseData(unsigned int ValueLength, Byte_t *Value, OTP_Channel_Data_t *ChannelData);
static void DecodeDisplayOLCPResponseData(unsigned int ValueLength, Byte_t *Value);
static void DecodeDisplayObjectListFilterData(unsigned int ValueLength, Byte_t *Value);
static void DecodeDisplayObjectChangedData(unsigned int ValueLength, Byte_t *Value);

static void DisplayOTSFeatureData(OTS_Feature_Data_t *OTSFeature);
static void DisplayObjectMetadataType(OTS_Object_Metadata_Type_t Type);
static void DisplayObjectMetadata(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata);
static void DisplayOACPRequestData(OTS_OACP_Request_Data_t *RequestData);
static void DisplayOACPResponseData(OTS_OACP_Response_Data_t *ResponseData);
static void DisplayOLCPRequestData(OTS_OLCP_Request_Data_t *RequestData);
static void DisplayOLCPResponseData(OTS_OLCP_Response_Data_t *ResponseData);
static void DisplayObjectListFilterData(OTS_Object_List_Filter_Data_t *ListFilterData);
static void DisplayObjectChangedData(OTS_Object_Changed_Data_t *ObjectChangedData);
static void DisplayObjectData(OTS_Object_Data_t *ObjectDataPtr);

static int RegisterOTSCommand(ParameterList_t *TempParam);
static int UnregisterOTSCommand(ParameterList_t *TempParam);
static int DiscoverOTSCommand(ParameterList_t *TempParam);
static int DiscoverAllServicesCommand(ParameterList_t *TempParam);

static int GetConnectionModeCommand(ParameterList_t *TempParam);
static int SetConnectionModeCommand(ParameterList_t *TempParam);
static int SetParametersCommand(ParameterList_t *TempParam);

static int OpenChannelCommand(ParameterList_t *TempParam);
static int CloseChannelCommand(ParameterList_t *TempParam);

static int ReadOTSCharacteristicCommand(ParameterList_t *TempParam);
static int WriteObjectMetadataCommand(ParameterList_t *TempParam);
static int WriteOACPCommand(ParameterList_t *TempParam);
static int WriteOLCPCommand(ParameterList_t *TempParam);
static int WriteObjectListFilterCommand(ParameterList_t *TempParam);
static int WriteCCCDCommand(ParameterList_t *TempParam);

static int CreateFileObjectCommand(ParameterList_t *TempParam);
static int PrintCurrentObjectCommand(ParameterList_t *TempParam);
static int PrintFilteredObjectListCommand(ParameterList_t *TempParam);
static int MarkObjectCommand(ParameterList_t *TempParam);

static void DisplaySetConnectionModeUsage(void);
static void DisplaySetParametersUsage(void);
static void DisplayReadCharacteristicCommandUsage(void);
static void DisplayWriteObjectMetadataCommandUsage(void);
static void DisplayWriteOACPCommandUsage(void);
static void DisplayWriteOLCPCommandUsage(void);
static void DisplayWriteObjectListFilterCommandUsage(void);
static void DisplayWriteCCCDCommandUsage(void);
static void DisplayCreateFileObjectCommandUsage(void);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI OTS_EventCallback(unsigned int BluetoothStackID, OTS_Event_Data_t *OTS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI OTS_Channel_EventCallback(unsigned int BluetoothStackID, OTS_Channel_Event_Data_t *OTS_Channel_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_OTS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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

         /* If we are the OTP Server (That is, this is the OTP Client   */
         /* information stored on the OTP Server).                      */
         /* * NOTE * This SHOULD NOT be called for the OTP Client.      */
         /* * NOTE * The OTS List Filter instances will be set to 'No   */
         /*          Filter' as default and no memory will be allocated */
         /*          for the Object Name filters.                       */
         if(OTSInstanceID)
         {
            /* Set the OTS Object List filters instances to the default */
            /* 'No Filter.'                                             */
            DeviceInfoPtr->FilteredObjectList.List_Filter_Data[0].Type = lftNoFilter;
            DeviceInfoPtr->FilteredObjectList.List_Filter_Data[1].Type = lftNoFilter;
            DeviceInfoPtr->FilteredObjectList.List_Filter_Data[2].Type = lftNoFilter;

            /* Initialize the Object List Filter instances that are     */
            /* exposed to the OTP Client.                               */
            UpdateFilteredObjectsList(&(DeviceInfoPtr->FilteredObjectList));
         }

         /* Set the default LE Channel Parameters.                      */
         DeviceInfoPtr->ChannelData.LE_Channel_Parameters.ChannelFlags   = OTP_CHANNEL_PARAMETERS_FLAGS;
         DeviceInfoPtr->ChannelData.LE_Channel_Parameters.MaxSDUSize     = OTP_CHANNEL_PARAMETERS_MAX_SDU_SIZE;
         DeviceInfoPtr->ChannelData.LE_Channel_Parameters.MaxPDUSize     = OTP_CHANNEL_PARAMETERS_MAX_PDU_SIZE;
         DeviceInfoPtr->ChannelData.LE_Channel_Parameters.PDUQueueDepth  = OTP_CHANNEL_PARAMETERS_PDU_QUEUE_DEPTH;
         DeviceInfoPtr->ChannelData.LE_Channel_Parameters.MaximumCredits = OTP_CHANNEL_PARAMETERS_MAX_CREDITS;

         /* Set the default L2CAP Queuing Parameters.                   */
         DeviceInfoPtr->ChannelData.Queueing_Parameters.Flags        = OTP_CHANNEL_QUEUEING_PARAMETERS_LIMIT_BY_PACKETS;
         DeviceInfoPtr->ChannelData.Queueing_Parameters.QueueLimit   = OTP_CHANNEL_QUEUEING_PARAMETERS_MAX_SDU_DEPTH;
         DeviceInfoPtr->ChannelData.Queueing_Parameters.LowThreshold = OTP_CHANNEL_QUEUEING_PARAMETERS_SDU_THRESHOLD;

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
   unsigned int Index;

   /* If memory was allocated for the OTS Channel buffer and has not    */
   /* been freed.                                                       */
   if((EntryToFree->ChannelData.Buffer) && (EntryToFree->ChannelData.Buffer_Length))
   {
      /* Free the memory.                                               */
      BTPS_FreeMemory(EntryToFree->ChannelData.Buffer);
      EntryToFree->ChannelData.Buffer        = NULL;
      EntryToFree->ChannelData.Buffer_Length = 0;
   }

   /* Close the file descriptor if it is open.                          */
   if(EntryToFree->ChannelData.File_Descriptor)
   {
      CloseFileContents(&(EntryToFree->ChannelData.File_Descriptor));
   }

   /* ** NOTE ** The following is OTP Server specific however, the      */
   /*            OTSInstanceID may be un-registered when this function  */
   /*            is called.                                             */

   /* Loop through the OTS Object List Filter data.                     */
   for(Index = 0; Index < (unsigned int)OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS; Index++)
   {
      /* We need to free memory if an OTS Object List Filter is an OTS  */
      /* Object Name type.                                              */
      if((EntryToFree->FilteredObjectList.List_Filter_Data[Index].Type >= lftNameStartsWith) && (EntryToFree->FilteredObjectList.List_Filter_Data[Index].Type <= lftNameIsExactly))
      {
         /* Make sure memory has been allocated.                        */
         if((EntryToFree->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer_Length) && (EntryToFree->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer))
         {
            /* Free the memory.                                         */
            BTPS_FreeMemory(EntryToFree->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer);
            EntryToFree->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer        = NULL;
            EntryToFree->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer_Length = 0;
         }
         else
            printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
      }
   }

   /* Delete the Filtered Objects List.                                 */
   DeleteFilteredObjectsList(&(EntryToFree->FilteredObjectList));

   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List.  Upon return of this      */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  Index;

   /* Loop through the device information list.                         */
   DeviceInfo = *ListHead;
   while(DeviceInfo)
   {
      /* If memory was allocated for the OTS Channel buffer and has not */
      /* been freed.                                                    */
      if((DeviceInfo->ChannelData.Buffer) && (DeviceInfo->ChannelData.Buffer_Length))
      {
         /* Free the memory.                                            */
         BTPS_FreeMemory(DeviceInfo->ChannelData.Buffer);
         DeviceInfo->ChannelData.Buffer        = NULL;
         DeviceInfo->ChannelData.Buffer_Length = 0;
      }

      /* Close the file descriptor if it is open.                       */
      if(DeviceInfo->ChannelData.File_Descriptor)
      {
         CloseFileContents(&(DeviceInfo->ChannelData.File_Descriptor));
      }

      /* ** NOTE ** The following is OTP Server specific however, the   */
      /*            OTSInstanceID may be un-registered when this        */
      /*            function is called.                                 */

      /* Loop through the OTS Object List Filter data.                  */
      for(Index = 0; Index < (unsigned int)OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS; Index++)
      {
         /* We need to free memory if an OTS Object List Filter is an   */
         /* OTS Object Name type.                                       */
         if((DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Type >= lftNameStartsWith) && (DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Type <= lftNameIsExactly))
         {
            /* Make sure memory has been allocated.                     */
            if((DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer_Length) && (DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer))
            {
               /* Free the memory.                                      */
               BTPS_FreeMemory(DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer);
               DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer        = NULL;
               DeviceInfo->FilteredObjectList.List_Filter_Data[Index].Data.Name.Buffer_Length = 0;
            }
            else
               printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
         }
      }

      /* Delete the Filtered Objects List.                              */
      DeleteFilteredObjectsList(&(DeviceInfo->FilteredObjectList));

      /* Get the next devices information.                              */
      DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
   }

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
   AddCommand("GETDEVICES", DisplayRemoteDevices);
   AddCommand("SETDEVICE", SelectRemoteDevice);
   AddCommand("GETMTU", GetGATTMTU);
   AddCommand("SETMTU", SetGATTMTU);
   AddCommand("REGISTEROTS", RegisterOTSCommand);
   AddCommand("UNREGISTEROTS", UnregisterOTSCommand);
   AddCommand("DISCOVEROTS", DiscoverOTSCommand);
   AddCommand("DISCOVERALL", DiscoverAllServicesCommand);
   AddCommand("GETCONNECTIONMODE", GetConnectionModeCommand);
   AddCommand("SETCONNECTIONMODE", SetConnectionModeCommand);
   AddCommand("SETPARAMETERS", SetParametersCommand);
   AddCommand("OPEN", OpenChannelCommand);
   AddCommand("CLOSE", CloseChannelCommand);
   AddCommand("READ", ReadOTSCharacteristicCommand);
   AddCommand("WRITEMETADATA", WriteObjectMetadataCommand);
   AddCommand("WRITEOACP", WriteOACPCommand);
   AddCommand("WRITEOLCP", WriteOLCPCommand);
   AddCommand("WRITELISTFILTER", WriteObjectListFilterCommand);
   AddCommand("WRITECCCD", WriteCCCDCommand);
   AddCommand("CREATE", CreateFileObjectCommand);
   AddCommand("PRINT", PrintCurrentObjectCommand);
   AddCommand("PRINTLIST", PrintFilteredObjectListCommand);
   AddCommand("MARK", MarkObjectCommand);
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

               /* We will check for the OTS Service data.  Make sure the*/
               /* mandatory UUID is present.                            */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the OTS Service data.             */
                  if(OTS_COMPARE_OTS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
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
   printf("\r\nLinuxOTP>");

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
            Connections         = 0;

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
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Check if we are currently connected to any remote devices.     */
      if(Connections)
      {
         /* Go ahead and flag that we are not connected to any remote   */
         /* devices since we are doing to send disconnect requests for  */
         /* all of them.                                                */
         Connections = 0;

         /* Loop through the device information list.                   */
         DeviceInfo = DeviceInfoList;
         while(DeviceInfo)
         {
            /* If we find a remote device with a valid connection ID,   */
            /* then we need to disconnect it.                           */
            if(DeviceInfo->ConnectionID)
            {
               /* Determine the transport to disconnect.                */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED)
               {
                  /* Disconnect over BR/EDR                             */
                  GATT_Disconnect(BluetoothStackID, DeviceInfo->ConnectionID);
               }
               else
               {
                  /* Disconnect over LE.                                */
                  GAP_LE_Disconnect(BluetoothStackID, DeviceInfo->ConnectionBD_ADDR);
               }
            }

            /* Get the next devices information.                        */
            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }
      }

      /* Cleanup OTS Service.                                           */
      if(OTSInstanceID)
      {
         /* Call the internal function to cleanup the OTS server.       */
         UnregisterOTS();
      }

      /* Cleanup GAP Service Module.                                    */
      if(GAPSInstanceID)
      {
         GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

         GAPSInstanceID = 0;
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

            if(!ret_val)
            {
               printf("\r\nExtended pairing request sent successfully.\r\n");
               printf("   Bluetooth Address:  %s.\r\n", BoardStr);
            }
            else
               DisplayFunctionError("GAP_LE_Extended_Pair_Remote_Device", ret_val);
         }
         else
         {
            /* As a slave we can only request that the Master start     */
            /* the pairing process.                                     */
            ret_val = GAP_LE_Extended_Request_Security(BluetoothStackID, BD_ADDR, &ExtendedCapabilities, GAP_LE_Event_Callback, 0);

            if(!ret_val)
            {
               printf("\r\nExtended security request sent successfully.\r\n");
               printf("   Bluetooth Address:  %s.\r\n", BoardStr);
            }
            else
               DisplayFunctionError("GAP_LE_Extended_Request_Security", ret_val);
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
   printf("* OTP Sample Application                                         *\r\n");
   printf("******************************************************************\r\n");
   printf("* Command Options General: Help, Quit, GetLocalAddress,          *\r\n");
   printf("*                          EnableDebug, GetMTU, SetMTU           *\r\n");
   printf("*                          GetDevices, SetDevice,                *\r\n");
   printf("* Command Options GAPLE:   SetDiscoverabilityMode,               *\r\n");
   printf("*                          SetConnectabilityMode,                *\r\n");
   printf("*                          SetPairabilityMode,                   *\r\n");
   printf("*                          ChangePairingParameters,              *\r\n");
   printf("*                          AdvertiseLE, StartScanning,           *\r\n");
   printf("*                          StopScanning, Connect,                *\r\n");
   printf("*                          Disconnect, PairLE,                   *\r\n");
   printf("*                          LEPasskeyResponse,                    *\r\n");
   printf("*                          SetPasskey,                           *\r\n");
   printf("* Command Options DIS:     DiscoverDIS, ReadDIS,                 *\r\n");
   printf("* Command Options GAPS:    DiscoverGAPS, GetLocalName,           *\r\n");
   printf("*                          SetLocalName, GetRemoteName,          *\r\n");
   printf("*                          SetLocalAppearance,                   *\r\n");
   printf("*                          GetLocalAppearance,                   *\r\n");
   printf("*                          GetRemoteAppearance,                  *\r\n");
   printf("* Command Options OTC:                                           *\r\n");
   printf("*                Server:   GetConnectionMode,                    *\r\n");
   printf("*                          SetConnectionMode,                    *\r\n");
   printf("*                Client:   Open, Close,                          *\r\n");
   printf("*                Both:     SetParameters,                        *\r\n");
   printf("* Command Options OTS:                                           *\r\n");
   printf("*                Server:   RegisterOTS                           *\r\n");
   printf("*                          UnRegisterOTS,                        *\r\n");
   printf("*                          Create,                               *\r\n");
   printf("*                          Print,                                *\r\n");
   printf("*                          PrintList,                            *\r\n");
   printf("*                          Mark,                                 *\r\n");
   printf("*                Client:   DiscoverALL, DiscoverOTS,             *\r\n");
   printf("*                          WriteCCCD,                            *\r\n");
   printf("*                          WriteMetadata,                        *\r\n");
   printf("*                          WriteOACP, WriteOLCP,                 *\r\n");
   printf("*                          WriteListFilter,                      *\r\n");
   printf("*                Both:     Read                                  *\r\n");
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
   union
   {
      Advertising_Data_t               AdvertisingData;
      Scan_Response_Data_t             ScanResponseData;
   } Advertisement_Data_Buffer;

   /* First, check that valid Bluetooth Stack ID exists. And that we are*/
   /* not already connected.                                            */
   if((BluetoothStackID) && (!Connections))
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

            /* Configure the flags field otsed on the Discoverability   */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            if(OTSInstanceID)
            {
               /* Advertise the Battery Server(1 byte type and 2 bytes  */
               /* UUID)                                                 */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               OTS_ASSIGN_OTS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));
            }

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] + 2), &(Advertisement_Data_Buffer.AdvertisingData));
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
   int                            ret_val = 0;
   BoardStr_t                     BoardStr;
   BD_ADDR_t                      BD_ADDR;
   Boolean_t                      UseWhiteList;
   unsigned int                   WhiteListChanged;
   GAP_LE_White_List_Entry_t      WhiteListEntry;
   GAP_LE_Connection_Parameters_t ConnectionParameters;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Next, make sure that the parameters are valid.                 */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->NumberofParameters < 4) && (TempParam->Params[0].strParam) && (BTPS_StringLength(TempParam->Params[0].strParam) == (sizeof(BD_ADDR_t)*2)))
      {
         /* Convert the parameter to a Bluetooth Device Address.        */
         StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);

         /* If there are two parameters then the user has specified an  */
         /* address type and we will connect over LE.                   */
         if(TempParam->NumberofParameters == 3)
         {
            /* Remove any previous entries for this device from the     */
            /* White List.                                              */
            WhiteListEntry.Address_Type = (GAP_LE_Address_Type_t)TempParam->Params[1].intParam;
            WhiteListEntry.Address      = BD_ADDR;

            /* Simply call the API to remove the device from the white  */
            /* list.                                                    */
            GAP_LE_Remove_Device_From_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);

            /* Store the use white list parameter.                      */
            UseWhiteList = (Boolean_t)TempParam->Params[2].intParam;

            /* Determine if we are going to use the white list to       */
            /* connect.                                                 */
            if(UseWhiteList)
               ret_val = GAP_LE_Add_Device_To_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);

            /* If everything has been successful, up until this point,  */
            /* then go ahead and attempt the connection.                */
            if(!ret_val)
            {
               /* Initialize the connection parameters.                 */
               ConnectionParameters.Connection_Interval_Min   = 50;
               ConnectionParameters.Connection_Interval_Max   = 200;
               ConnectionParameters.Minimum_Connection_Length = 0;
               ConnectionParameters.Maximum_Connection_Length = 10000;
               ConnectionParameters.Slave_Latency             = 0;
               ConnectionParameters.Supervision_Timeout       = 20000;

               /* Everything appears correct, go ahead and attempt to   */
               /* make the connection.                                  */
               /* * NOTE * We will re-use the white list address type   */
               /*          for the remote address type.                 */
               if((ret_val = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, ((UseWhiteList) ? fpWhiteList : fpNoFilter), WhiteListEntry.Address_Type, &BD_ADDR, latPublic, &ConnectionParameters, GAP_LE_Event_Callback, 0)) == 0)
               {
                  BD_ADDRToStr(BD_ADDR, BoardStr);
                  printf("\r\nConnection Request successfully sent.\r\n");
                  printf("   Bluetooth Address:  %s.\r\n", BoardStr);
                  printf("   Using White List:   %s.\r\n", ((UseWhiteList) ? "Yes" : "No"));
               }
               else
               {
                  DisplayFunctionError("GAP_LE_Create_Connection", ret_val);
               }
            }
            else
               DisplayFunctionError("GAP_LE_Add_Device_To_White_List", ret_val);
         }
         else
         {
            /* If there is only one parameter then this must be a BR/EDR*/
            /* connection.                                              */
            if(TempParam->NumberofParameters == 1)
            {
               /* Call GATT_Connect() to connect BR/EDR over GATT.      */
               if((ret_val = GATT_Connect(BluetoothStackID, BD_ADDR, GATT_Connection_Event_Callback, 0)) <= 0)
                  DisplayFunctionError("GATT_Connect", ret_val);
            }
            else
            {
               /* Invalid parameters specified.                         */
               printf("Usage: Connect [LE BD_ADDR] [Local Address Type(0=PUBLIC, 1=RANDOM)] [Use White List(0=FALSE, 1=TRUE)].\r\n");
               printf("       Connect [BR/EDR BD_ADDR].\r\n");
            }
         }
      }
      else
      {
         /* Invalid parameters specified.                               */
         printf("Usage: Connect [LE BD_ADDR] [Local Address Type(0=PUBLIC, 1=RANDOM)] [Use White List(0=FALSE, 1=TRUE)].\r\n");
         printf("       Connect [BR/EDR BD_ADDR].\r\n");
      }
   }

   return(0);
}

   /* The following function is responsible for disconnecting a remote  */
   /* device device.  This function returns zero if successful and a    */
   /* negative value if an error occurred.                              */
static int Disconnect(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device information so we can determine what         */
         /* transport we are connected over.                            */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Determine the transport the remote device is using.      */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED))
            {
               /* Disconnect over the LE transport.                     */
               if((ret_val = GAP_LE_Disconnect(BluetoothStackID, ConnectionBD_ADDR)) == 0)
               {
                  BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
                  printf("\r\nDisconnection request successfully sent.\r\n");
                  printf("   Bluetooth Address:  %s.\r\n", BoardStr);
               }
               else
                  DisplayFunctionError("GAP_LE_Disconnect", ret_val);
            }
            else
            {
               /* Disconnect over the BR/EDR transport.                 */
               if((ret_val = GATT_Disconnect(BluetoothStackID, DeviceInfo->ConnectionID)) == 0)
               {
                  BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
                  printf("\r\nDisconnection request successfully sent.\r\n");
                  printf("   Bluetooth Address:  %s.\r\n", BoardStr);
               }
               else
                  DisplayFunctionError("GATT_Disconnect", ret_val);
            }

            /* Go ahead and assume that we are no longer connected to a */
            /* remote device.                                           */
            Connections = FALSE;
            ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);

            /* Loop through the device information list.                */
            DeviceInfo = DeviceInfoList;
            while(DeviceInfo)
            {
               /* If we find a remote device with a valid connection ID,*/
               /* then we are still connected to at least one remote    */
               /* device.                                               */
               /* * NOTE * We previously cleared the Connection ID for  */
               /*          the remote device that disconnected.         */
               if(DeviceInfo->ConnectionID)
               {
                  /* Flag that we are connected to at least one remote  */
                  /* device and make the first remote device found the  */
                  /* currently selected remote device.                  */
                  Connections         = TRUE;
                  ConnectionBD_ADDR = DeviceInfo->ConnectionBD_ADDR;
                  break;
               }

               /* Get the next devices information.                     */
               DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
            }

            /* Let the user know if there is a new selected remote      */
            /* device.                                                  */
            BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
            printf("\r\nSelected remote device: %s\r\n", BoardStr);
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nNo active connection.\r\n");
   }
   else
      printf("\r\nInvalid Bluetooth StackID.\r\n");

   return(0);
}

   /* The following function is provided to allow a mechanism of        */
   /* Pairing (or requesting security if a slave) to the connected      */
   /* device.                                                           */
static int PairLE(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device information so we can determine what         */
         /* transport we are connected over.                            */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Determine the transport the remote device is using.      */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED))
            {
               /* Send the pairing request.                             */
               SendPairingRequest(ConnectionBD_ADDR, ((DeviceInfo->RemoteDeviceIsMaster) ? FALSE : TRUE));
            }
            else
               printf("\r\nOnly an LE connection can send a pairing request.\r\n");
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nNo active connection.\r\n");
   }
   else
      printf("\r\nInvalid Bluetooth StackID.\r\n");

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

   /* The following function is responsible for displaying a list of    */
   /* currently connected remote devices.                               */
static int DisplayRemoteDevices(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;

   printf("\r\nRemote device information:\r\n");

   /* Loop through the device information list.                         */
   DeviceInfo = DeviceInfoList;
   while(DeviceInfo)
   {
      /* Display the remote device information.                         */
      BD_ADDRToStr(DeviceInfo->ConnectionBD_ADDR, BoardStr);
      printf("\r\nBluetooth Address:  %s\r\n", BoardStr);
      printf("Connection Info:\r\n");
      printf("   Connections:       %s\r\n", (DeviceInfo->ConnectionID) ? "Yes" : "No");
      printf("   Type:            %s\r\n", (DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED) ? "BR/EDR" : "LE");
      printf("   ID:              %u\r\n", DeviceInfo->ConnectionID);

      /* Get the next devices information.                              */
      DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
   }

   return(0);
}

   /* The following function is responsible for selecting the remote    */
   /* device from a list of currently connected remote devices.  This   */
   /* allows the local device to access the remote device's information,*/
   /* which is needed by many local functions (service discovery,       */
   /* disconnecting, etc).                                              */
   /* * NOTE * Once a remote device has successfully connected it will  */
   /*          automatically become the selected remote device.         */
static int SelectRemoteDevice(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  ConnectionID;
   BoardStr_t    BoardStr;

   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the Connection ID.                                       */
      ConnectionID = (unsigned int)TempParam->Params[0].intParam;

      /* Make sure the Connection ID is valid.                          */
      if(ConnectionID)
      {
         /* Loop through the device information list.                   */
         DeviceInfo = DeviceInfoList;
         while(DeviceInfo)
         {
            /* If we find the matching Connection ID.                   */
            if(DeviceInfo->ConnectionID == ConnectionID)
            {
               /* Update the currently selected remote device.          */
               ConnectionBD_ADDR = DeviceInfo->ConnectionBD_ADDR;

               /* Let the user know the new selected remote device.     */
               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
               printf("\r\nSelected remote device: %s\r\n", BoardStr);
            }

            /* Get the next devices information.                        */
            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }
      }
      else
         printf("\r\nThe Connection ID MUST be positive non-zero.\r\n");
   }
   else
   {
      printf("\r\nUsage: SetDevice [Connection ID (UINT16)]\r\n");
   }

   return(0);
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

   /* Print the service name if it is OTS.                              */
   if(OTS_COMPARE_OTS_SERVICE_UUID_TO_UUID_16(UUID->UUID.UUID_16))
   {
      printf("   Name:             OTS\r\n");
   }
}

   /* The following function is responsible for performing a DIS Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverDIS(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID;
   DeviceInfo_t *DeviceInfo;

   /* Make sure that we are connected and a remote device is selected.  */
   if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Get the device info for the remote device.                     */
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

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, DeviceInfo->ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, sdDIS);
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
         /* Make sure that we are connected and a remote device is      */
         /* selected.                                                   */
         if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
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
                     if((ret_val = GATT_Read_Value_Request(BluetoothStackID, DeviceInfo->ConnectionID, AttributeHandle, GATT_ClientEventCallback_DIS, AttributeHandle)) > 0)
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

   /* Make sure that we are connected and a remote device is selected.  */
   if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Get the device info for the connected device.                  */
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

               ret_val = GATT_Start_Service_Discovery_Handle_Range(BluetoothStackID, DeviceInfo->ConnectionID, &DiscoveryHandleRange, (sizeof(UUID) / sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdGAPS);
            }
            else
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, DeviceInfo->ConnectionID, (sizeof(UUID) / sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdGAPS);

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

   /* Make sure that we are connected and a remote device is selected.  */
   if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Get the device info for the connected device.                  */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceNameHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, DeviceInfo->ConnectionID, DeviceInfo->GAPSClientInfo.DeviceNameHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceNameHandle);
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

   /* Make sure that we are connected and a remote device is selected.  */
   if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Get the device info for the connected device.                  */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, DeviceInfo->ConnectionID, DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle);
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
   /* mechanism of populating an OTP Client Information structure with  */
   /* the information discovered from a GATT Service Discovery          */
   /* operation.                                                        */
static void OTSPopulateHandles(OTS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;
   unsigned int                       ListFilterIndex = 0;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (OTS_COMPARE_OTS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
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
            printf("\r\nOTS Characteristic:\r\n");
            printf("   Handle:       0x%04X\r\n", CharacteristicInfoPtr->Characteristic_Handle);
            printf("   Properties:   0x%02X\r\n", CharacteristicInfoPtr->Characteristic_Properties);
            printf("   UUID:         0x");
            DisplayUUID(&(CharacteristicInfoPtr->Characteristic_UUID));
            printf("   Descriptors:  %u\r\n", CharacteristicInfoPtr->NumberOfDescriptors);
         }

         /* All OTS UUIDs are defined to be 16 bit UUIDs.               */
         if(CharacteristicInfoPtr->Characteristic_UUID.UUID_Type == guUUID_16)
         {
            /* OTS Feature.                                             */
            if(OTS_COMPARE_OTS_FEATURE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->OTS_Feature = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Name.                                         */
            if(OTS_COMPARE_OBJECT_NAME_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Name = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Type.                                         */
            if(OTS_COMPARE_OBJECT_TYPE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Type = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Size.                                         */
            if(OTS_COMPARE_OBJECT_SIZE_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Size = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object First Created.                                */
            if(OTS_COMPARE_OBJECT_FIRST_CREATED_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_First_Created = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Last Modified.                                */
            if(OTS_COMPARE_OBJECT_LAST_MODIFIED_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Last_Modified = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object ID.                                           */
            if(OTS_COMPARE_OBJECT_ID_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_ID = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Properties.                                   */
            if(OTS_COMPARE_OBJECT_PROPERTIES_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                  printf("Warning - Mandatory write property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Properties = CharacteristicInfoPtr->Characteristic_Handle;

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Action Control Point (OACP).                  */
            if(OTS_COMPARE_OBJECT_ACTION_CONTROL_POINT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                  printf("Warning - Mandatory write property not supported!\r\n");

               /* Verify that indicate is supported.                    */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                  printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Action_Control_Point = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Object_Action_Control_Point_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object List Control Point (OLCP).                    */
            if(OTS_COMPARE_OBJECT_LIST_CONTROL_POINT_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                  printf("Warning - Mandatory write property not supported!\r\n");

               /* Verify that indicate is supported.                    */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                  printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_List_Control_Point = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Object_List_Control_Point_CCCD));

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object List Filter.                                  */
            if(OTS_COMPARE_OBJECT_LIST_FILTER_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that read is supported.                        */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                  printf("Warning - Mandatory read property not supported!\r\n");

               /* Verify that write is supported.                       */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                  printf("Warning - Mandatory write property not supported!\r\n");

               /* We can only store three instances for the OTS Object  */
               /* List Filter                                           */
               if(ListFilterIndex < 3)
               {
                  /* Store the handle.                                  */
                  ClientInfo->Object_List_Filter[ListFilterIndex++] = CharacteristicInfoPtr->Characteristic_Handle;
               }
               else
                  printf("Warning - Too many OTS Object List Filter Characteristic instances discovered!\r\n");

               /* Get the next Characteristic.                          */
               continue;
            }

            /* OTS Object Changed.                                      */
            if(OTS_COMPARE_OBJECT_CHANGED_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
            {
               /* Verify that indicate is supported.                    */
               if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
                  printf("Warning - Mandatory indicate property not supported!\r\n");

               /* Store the handle.                                     */
               ClientInfo->Object_Changed = CharacteristicInfoPtr->Characteristic_Handle;

               /* Store the CCCD descriptor handle.                     */
               StoreDescriptorHandles(ClientInfo, CharacteristicInfoPtr, &(ClientInfo->Object_Changed_CCCD));

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

   /* Make sure we discovered three instances of the OTS Object List    */
   /* Filter Characteristic if it is supported.                         */
   if((ListFilterIndex) && (ListFilterIndex != 3))
   {
      printf("Warning - Three OTS Object List Filter Characteristic instances were not discovered!\r\n");
   }
}

   /* The following function is responsible for populating descriptor   */
   /* handles for an OTS Characteristic.                                */
static void StoreDescriptorHandles(OTS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Word_t *Handle)
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

   /* The following function is responsible for displaying the OTS      */
   /* service discovery summary.                                        */
static void DisplayOTSDiscoverySummary(OTS_Client_Information_t *ClientInfo)
{
   printf("\r\nOTS Service Discovery Summary\r\n\r\n");
   printf("   Object Feature                    : %s\r\n", (ClientInfo->OTS_Feature                       ? "Supported" : "Not Supported"));
   printf("   Object Name                       : %s\r\n", (ClientInfo->Object_Name                       ? "Supported" : "Not Supported"));
   printf("   Object Type                       : %s\r\n", (ClientInfo->Object_Type                       ? "Supported" : "Not Supported"));
   printf("   Object Size                       : %s\r\n", (ClientInfo->Object_Size                       ? "Supported" : "Not Supported"));
   printf("   Object First Created              : %s\r\n", (ClientInfo->Object_First_Created              ? "Supported" : "Not Supported"));
   printf("   Object Last Modified              : %s\r\n", (ClientInfo->Object_Last_Modified              ? "Supported" : "Not Supported"));
   printf("   Object ID                         : %s\r\n", (ClientInfo->Object_ID                         ? "Supported" : "Not Supported"));
   printf("   Object Properties                 : %s\r\n", (ClientInfo->Object_Properties                 ? "Supported" : "Not Supported"));
   printf("   Object Action Control Point       : %s\r\n", (ClientInfo->Object_Action_Control_Point       ? "Supported" : "Not Supported"));
   printf("   Object Action Control Point CCCD  : %s\r\n", (ClientInfo->Object_Action_Control_Point_CCCD  ? "Supported" : "Not Supported"));
   printf("   Object List Control Point         : %s\r\n", (ClientInfo->Object_List_Control_Point         ? "Supported" : "Not Supported"));
   printf("   Object List Control Point CCCD    : %s\r\n", (ClientInfo->Object_List_Control_Point_CCCD    ? "Supported" : "Not Supported"));
   printf("   Object List Filter 1              : %s\r\n", (ClientInfo->Object_List_Filter[0]             ? "Supported" : "Not Supported"));
   printf("   Object List Filter 2              : %s\r\n", (ClientInfo->Object_List_Filter[1]             ? "Supported" : "Not Supported"));
   printf("   Object List Filter 3              : %s\r\n", (ClientInfo->Object_List_Filter[2]             ? "Supported" : "Not Supported"));
   printf("   Object Changed                    : %s\r\n", (ClientInfo->Object_Changed                    ? "Supported" : "Not Supported"));
   printf("   Object Changed CCCD               : %s\r\n", (ClientInfo->Object_Changed_CCCD               ? "Supported" : "Not Supported"));
}

   /* The following function is responsible for registering OTS.        */
static int RegisterOTS(void)
{
   int                   ret_val = 0;
   OTS_Initialize_Data_t InitializeData;

   /* Make sure that we are not connected.                              */
   if(!Connections)
   {
      /* Verify that the Service is not already registered.             */
      if(!OTSInstanceID)
      {
         /* We will initialize all features for the Object Transfer     */
         /* Service (OTS).                                              */
         InitializeData.OTS_Characteristic_Flags        = OTP_DEFAULT_OTS_CHARACTERISTIC_FLAGS;
         InitializeData.OTS_Property_Flags              = OTP_DEFAULT_OTS_PROPERTY_FLAGS;
         InitializeData.Multiple_Objects_Supported      = TRUE;
         InitializeData.OACP_Create_Procedure_Supported = TRUE;
         InitializeData.Real_Time_Clock_Supported       = TRUE;
         InitializeData.Connection_Mode                 = ocmAutomaticAccept;
         InitializeData.EventCallback                   = OTS_Channel_EventCallback;
         InitializeData.CallbackParameter               = BluetoothStackID;

         /* We will also set the default LE Channel Parameters that need*/
         /* to be stored for the OTP Server in the service until an OTP */
         /* Client sends a request to open the Object Transfer Channel  */
         /* (OTC) over LE.                                              */
         /* * NOTE * All configurations are default and may be changed  */
         /*          at the top of the file.                            */
         InitializeData.Default_LE_Channel_Parameters.ChannelFlags   = OTP_CHANNEL_PARAMETERS_FLAGS;
         InitializeData.Default_LE_Channel_Parameters.MaxSDUSize     = OTP_CHANNEL_PARAMETERS_MAX_SDU_SIZE;
         InitializeData.Default_LE_Channel_Parameters.MaxPDUSize     = OTP_CHANNEL_PARAMETERS_MAX_PDU_SIZE;
         InitializeData.Default_LE_Channel_Parameters.PDUQueueDepth  = OTP_CHANNEL_PARAMETERS_PDU_QUEUE_DEPTH;
         InitializeData.Default_LE_Channel_Parameters.MaximumCredits = OTP_CHANNEL_PARAMETERS_MAX_CREDITS;

         /* Initialize the service.                                     */
         ret_val = OTS_Initialize_Service(BluetoothStackID, (unsigned int)OTS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, OTS_EventCallback, BluetoothStackID, &OTSInstanceID);
         if((ret_val > 0) && (OTSInstanceID > 0))
         {
            /* Display succots message.                                 */
            printf("Successfully registered OTS Service, OTSInstanceID = %u.\r\n", ret_val);

            /* Save the ServiceID of the registered service.            */
            OTSInstanceID = (unsigned int)ret_val;

            /* Let's go ahead and finish any remaining setup needed for */
            /* the OTP Server.                                          */
            if((ret_val = SetupOTSServer()) != 0)
            {
               /* Unregister the service since we failed to setup the   */
               /* OTP Server.                                           */
               ret_val = UnregisterOTS();
            }
         }
         else
            DisplayFunctionError("OTS_Initialize_Service", ret_val);
      }
      else
      {
         printf("\r\nOTS is already registered.\r\n");
      }
   }
   else
   {
      printf("\r\nConnection currently active.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for un-registering OTS.     */
static int UnregisterOTS(void)
{
   int ret_val = 0;

   /* Verify that a service is registered.                              */
   if(OTSInstanceID)
   {
      /* Make sure we are not connected to any remote devices.          */
      if(!Connections)
      {
         /* Unregister the OTS Service with GATT.                       */
         ret_val = OTS_Cleanup_Service(BluetoothStackID, OTSInstanceID);
         if(ret_val == 0)
         {
            /* Display success message.                                 */
            printf("\r\nSuccessfully unregistered OTS Service InstanceID %u.\r\n", OTSInstanceID);

            /* Clear the InstanceID.                                    */
            OTSInstanceID = 0;

            /* Cleanup the OTP Server.                                  */
            CleanupOTSServer();
         }
         else
            DisplayFunctionError("OTS_Cleanup_Service", ret_val);
      }
      else
         printf("\r\nCannot un-register OTS while a connection is active.\r\n");
   }
   else
      printf("\r\nOTS Service not registered.\r\n");

   return(ret_val);
}

   /* The following is responsible for performing any setup needed by   */
   /* the OTP Server after the service has been registered.             */
static int SetupOTSServer(void)
{
   int                ret_val = 0;
   unsigned int       Index;
   OTS_Object_Data_t *ObjectDataPtr;

   /* Generate random seed.                                             */
   srand(rand() % 100);

   /* Set the default OTP Server OACP and OLCP supported features.      */
   ServerData.OTS_Feature.OACP_Features = OTP_DEFAULT_OACP_FEATURES;
   ServerData.OTS_Feature.OLCP_Features = OTP_DEFAULT_OLCP_FEATURES;

   /* Initialize the OTS Object ID counter.                             */
   /* * NOTE * The OTS Directory Listing Object will use the (ID = 0).  */
   /*          All other OTS Objects MUST be in the range               */
   /*          (0x000000000100 to 0xFFFFFFFFFFFF).                      */
   ObjectIDCtr = 0x00000100;

   /* Set the default directory to use to hold the Object's contents.   */
   /* * NOTE * We will add the forward slash that is needed for when we */
   /*          append the file name.                                    */
   BTPS_StringCopy(ServerData.Directory_Data.Path, OTP_DEFAULT_SERVER_DIRECTORY_PATH);

   /* Loop through the Objects List.                                    */
   for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
   {
      /* We need to allocate memory for each OTS Object's Name field.   */
      /* * NOTE * We will add one to the length for the NULL terminator */
      /*          so we can display the OTS Object Name as a c-string.  */
      /* * NOTE * The buffer length will already be set to zero and will*/
      /*          be updated when a name is assigned.                   */
      if((ServerData.Object_List[Index].Data.Name.Buffer = (Byte_t *)BTPS_AllocateMemory(NON_ALIGNED_BYTE_SIZE * (OTS_MAXIMUM_OBJECT_NAME_LENGTH+1))) == NULL)
      {
         /* Return an error occured since we failed to allocate the     */
         /* memory.                                                     */
         /* * NOTE * The CleanupOTSServer() will free any memory that   */
         /*          has been allocated.                                */
         ret_val = FUNCTION_ERROR;
         break;
      }
   }

   /* If an error has not occured.                                      */
   if(!ret_val)
   {
      /* Attempt to create the default directory.                       */
      /* * NOTE * We will open it if make it or it already exists.      */
      ret_val = MakeDirectory(ServerData.Directory_Data.Path);
      if((ret_val == BTPS_FILE_MAKE_DIRECTORY_SUCCESS) || (ret_val == BTPS_FILE_MAKE_DIRECTORY_ERROR_ALREADY_EXISTS))
      {
         /* Attempt to open the directory.                              */
         if(!OpenDirectory(&(ServerData.Directory_Data.Directory_Descriptor), ServerData.Directory_Data.Path))
         {
            /* Store a pointer to the OTS Object data.                  */
            ObjectDataPtr = &(ServerData.Object_List[0].Data);

            /* We will go ahead and setup the OTS Directory Listing     */
            /* Object.                                                  */
            /* * NOTE * This will always be the first OTS Object.       */
            ServerData.Object_List[0].Valid = TRUE;

            /* Set the Flags field to control what OTS Object fields are*/
            /* included for this object in the OTS Directory Listing    */
            /* Object's contents.                                       */
            ObjectDataPtr->Flags = OTP_DEFAULT_OBJECT_FLAGS;

            /* Set the OTS Object Name.                                 */
            BTPS_StringCopy((char *)(ObjectDataPtr->Name.Buffer), "OTS Directory Listing Object");
            ObjectDataPtr->Name.Buffer_Length = BTPS_StringLength((char *)ObjectDataPtr->Name.Buffer);

            /* Set the OTS Object Type.                                 */
            ObjectDataPtr->Type.UUID_Type = guUUID_16;
            OTS_ASSIGN_DIRECTORY_LISTING_OBJECT_TYPE_UUID16(ObjectDataPtr->Type.UUID.UUID_16);

            /* Set the Size fields to zero.                             */
            /* * NOTE * These will be updated later when we format the  */
            /*          OTS Directory Listing Object.                   */
            ObjectDataPtr->Size.Allocated_Size = 0;
            ObjectDataPtr->Size.Current_Size   = 0;

            /* Set the OTS Object ID.                                   */
            /* * NOTE * This is REQUIRED to be zero.                    */
            ObjectDataPtr->ID.Lower = 0;
            ObjectDataPtr->ID.Upper = 0;

            /* Set the OTS Object First Created.                        */
            /* * NOTE * We will set the First Created date to zero since*/
            /*          it has no meaning for the OTS Directory Listing */
            /*          Object.                                         */
            ObjectDataPtr->First_Created.Year    = 0;
            ObjectDataPtr->First_Created.Month   = 0;
            ObjectDataPtr->First_Created.Day     = 0;
            ObjectDataPtr->First_Created.Hours   = 0;
            ObjectDataPtr->First_Created.Minutes = 0;
            ObjectDataPtr->First_Created.Seconds = 0;

            /* Set the OTS Object Last Modified.                        */
            /* * NOTE * We will set the Last Modified date to zero since*/
            /*          it has no meaning for the OTS Directory Listing */
            /*          Object.                                         */
            ObjectDataPtr->Last_Modified.Year    = 0;
            ObjectDataPtr->Last_Modified.Month   = 0;
            ObjectDataPtr->Last_Modified.Day     = 0;
            ObjectDataPtr->Last_Modified.Hours   = 0;
            ObjectDataPtr->Last_Modified.Minutes = 0;
            ObjectDataPtr->Last_Modified.Seconds = 0;

            /* Set the OTS Object properties.                           */
            /* * NOTE * We will only permit reading.  This is REQUIRED. */
            ObjectDataPtr->Properties = (DWord_t)OTS_OBJECT_PROPERTIES_READ;

            /* Simply return success to the caller.                     */
            ret_val = 0;
         }
         else
         {
            /* Return an error since we were unable to open the         */
            /* directory                                                */
            ret_val = FUNCTION_ERROR;
         }
      }
   }

   return(ret_val);
}

   /* The following is responsible for performing any cleanup needed by */
   /* the OTP Server after the service has been un-registered.          */
static void CleanupOTSServer(void)
{
   unsigned int Index;

   /* Let the user know we are deleting the files in the directory.     */
   printf("\r\nDeleting files in the directory: \"%s\".\r\n", ServerData.Directory_Data.Path);

   /* Loop through the objects list and remove any associated files on  */
   /* the file system.                                                  */
   for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
   {
      /* If the OTS Object is valid.                                    */
      /* * NOTE * The file CANNOT exist if the OTS Object is not valid. */
      if(ServerData.Object_List[Index].Valid)
      {
         /* Make sure the OTS Object is not the OTS Directory Listing   */
         /* Object, since it has no file to delete.                     */
         /* * NOTE * This sample application does not use the upper     */
         /*          portion of the OTS Object ID.                      */
         if(ServerData.Object_List[Index].Data.ID.Lower != 0)
         {
            /* Remove the file.                                         */
            DeleteFileContents((char *)ServerData.Object_List[Index].Data.Name.Buffer);
         }

         /* Mark that this OTS Object is no longer valid.               */
         ServerData.Object_List[Index].Valid = FALSE;
      }
   }

   /* Loop through the objects list and free any memory allocated for   */
   /* each OTS Object Name.                                             */
   for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
   {
      /* Make sure the buffer for the OTS Object name has been          */
      /* allocated.                                                     */
      /* * NOTE * We allocated memory at start up for the OTS Object    */
      /*          name so we need to ALWAYS free it.  May not be        */
      /*          allocated if an error occured at startup and we are   */
      /*          cleaning up the OTP Server as a result.               */
      if(ServerData.Object_List[Index].Data.Name.Buffer)
      {
         BTPS_FreeMemory((ServerData.Object_List[Index].Data.Name.Buffer));
         ServerData.Object_List[Index].Data.Name.Buffer        = NULL;
         ServerData.Object_List[Index].Data.Name.Buffer_Length = 0;
      }
   }

   /* Close the directory if it is open.                                */
   if(ServerData.Directory_Data.Directory_Descriptor)
   {
      /* Simply call the internal function to close the directory.      */
      CloseDirectory(ServerData.Directory_Data.Directory_Descriptor);
   }

   /* Let the user know we are deleting the directory.                  */
   printf("\r\nDeleting the directory: \"%s\".\r\n", ServerData.Directory_Data.Path);

   /* Attempt to delete the directory.                                  */
   DeleteDirectory(ServerData.Directory_Data.Path);
}

   /* The following function is responsible for the cleaning up the OTS */
   /* Server after a disconnect event is received from an OTP Client.   */
   /* ** NOTE ** This function MUST be called when a disconnection event*/
   /*            is received.                                           */
   /* * NOTE * This function will reset all Object List Filter instances*/
   /*          to 'No filter' and free any memory currently allocated   */
   /*          for an Object Name filter type.                          */
   /* * NOTE * This function will remove any malformatted objects.  That*/
   /*          is an OTS Object that has been created via the OACP      */
   /*          Create Procedure by the OTP Client, but has not yet been */
   /*          given an OTS Object Name.  If this new object has a file */
   /*          associated with it on the file system, it will be        */
   /*          deleted.  This may occur of the OTP Client has written   */
   /*          data to the OTS Object before giving it a valid name.    */
   /* * NOTE * The Filtered Objects List will be updated in case a      */
   /*          malformatted OTS Object has been removed and the current */
   /*          sort order will remain applied if the OTP Client         */
   /*          specified an order via the OLCP Order Procedure.  The    */
   /*          Filtered Objects List will be deleted if the OTP Server  */
   /*          is not bonded to the OTP Client (The device information  */
   /*          will be deleted).  However, if we are bonded then the    */
   /*          Filtered Objects List will be set for when the OTP Client*/
   /*          reconnects.                                              */
static void DisconnectionCleanup(DeviceInfo_t *DeviceInfo)
{
   unsigned int                   Index;
   OTS_Object_Data_t             *ObjectDataPtr;
   OTS_Object_List_Filter_Data_t *ListFilterData;

   /* Loop through the OTS Object List Filter data.                     */
   for(Index = 0; Index < (unsigned int)OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS; Index++)
   {
      /* Store a pointer to the Object List Filter instance data.       */
      ListFilterData = &(DeviceInfo->FilteredObjectList.List_Filter_Data[Index]);

      /* We need to free memory if an OTS Object List Filter is an OTS  */
      /* Object Name type.                                              */
      if((ListFilterData->Type >= lftNameStartsWith) && (ListFilterData->Type <= lftNameIsExactly))
      {
         /* Make sure memory has been allocated.                        */
         if((ListFilterData->Data.Name.Buffer_Length) && (ListFilterData->Data.Name.Buffer))
         {
            /* Free the memory.                                         */
            BTPS_FreeMemory(ListFilterData->Data.Name.Buffer);
            ListFilterData->Data.Name.Buffer        = NULL;
            ListFilterData->Data.Name.Buffer_Length = 0;
         }
         else
            printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
      }

      /* Set the Object List Filter instance type to 'No Filter'.       */
      ListFilterData->Type = lftNoFilter;
   }

   /* Loop through the objects list and make sure a malformmated object */
   /* is removed.                                                       */
   /* * NOTE * This may occur if the OTP Client has recently used the   */
   /*          OACP Create Procedure and has not given the new OTS      */
   /*          Object a valid name.  There can only be one new file     */
   /*          object for this sample application since the OACP Create */
   /*          Procedure will fail if an OTS Object exists with the     */
   /*          temporary name.                                          */
   for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
   {
      /* If the OTS Object is valid.                                    */
      if(ServerData.Object_List[Index].Valid)
      {
         /* Store a pointer to the OTS Object data.                     */
         ObjectDataPtr = &(ServerData.Object_List[Index].Data);

         /* Make sure the OTS Object Name is the default new OTS Object */
         /* file name.                                                  */
         if(!BTPS_MemCompare(ObjectDataPtr->Name.Buffer, OTP_DEFAULT_NEW_FILE_NAME, BTPS_StringLength(OTP_DEFAULT_NEW_FILE_NAME)))
         {
            /* If the current size is valid, then a file exists on the  */
            /* file system and it needs to be deleted.                  */
            /* * NOTE * This may occur if the OTP Client has written    */
            /*          data to the new file before a valid file name   */
            /*          has been given.  This is why we use a temporary */
            /*          name.  We still return empty string when        */
            /*          necessary.                                      */
            if(ObjectDataPtr->Size.Current_Size)
            {
               /* Remove the file.                                      */
               DeleteFileContents((char *)ObjectDataPtr->Name.Buffer);
            }

            /* Make sure the OTS Object is unlocked.                    */
            ServerData.Object_List[Index].Locked = FALSE;

            /* Mark the OTS Object as invalid.                          */
            ServerData.Object_List[Index].Valid  = FALSE;

            /* There can only be one OTS Object with the Default New    */
            /* File Name.                                               */
            break;
         }
      }
   }

   /* We need to update the Filtered Objects List in case the device    */
   /* information is not going to be deleted.  This way when the OTP    */
   /* Client reconnects the Filtered Object List is valid.              */
   /* * NOTE * If a sort order is applied, it will remain applied.      */
   UpdateFilteredObjectsList(&(DeviceInfo->FilteredObjectList));

   /* If the current object is still valid.                             */
   /* * NOTE * If the current object was the malformatted object that   */
   /*          was previously deleted, then it has already been         */
   /*          unlocked.  If this is the case then the current object   */
   /*          will not be valid when the Filtered Objects List is      */
   /*          updated.                                                 */
   if(DeviceInfo->FilteredObjectList.Current_Object)
   {
      /* Loop through the objects list and unlock the current object.   */
      for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
      {
         /* Make sure the OTS Object is valid.                          */
         if(ServerData.Object_List[Index].Valid)
         {
            /* If we find the current object.                           */
            /* * NOTE * We are guaranteed to find it based on previous  */
            /*          checks.                                         */
            if(ServerData.Object_List[Index].Data.ID.Lower == DeviceInfo->FilteredObjectList.Current_Object->ObjectDataPtr->ID.Lower)
            {
               /* Unlock the object.                                    */
               ServerData.Object_List[Index].Locked = FALSE;

               /* We are done.                                          */
               break;
            }
         }
      }
   }

   /* We will ALWAYS make the current object invalid.                   */
   /* * NOTE * We MUST do this after the Filtered Objects List has been */
   /*          regenerated since it will select the current object.     */
   DeviceInfo->FilteredObjectList.Current_Object = NULL;
}

   /* The following function is only for the OTP Server and is          */
   /* responsible for opening a file's contents (descriptor to a file on*/
   /* the file system).                                                 */
   /* * NOTE * This function will automatically append the FileName to  */
   /*          the directory path before opening the file.              */
   /* * NOTE * This function will auto-generate the OTS Object First    */
   /*          Created time for the current object that is selected.    */
   /* * NOTE * If the file is created then the Offset MUST be zero.     */
static int OpenFileContentsServer(BTPS_File_Descriptor_t *FileDescriptor, char *FileName, BTPS_Open_Mode_t OpenMode, DWord_t Offset, OTS_Object_Data_t *ObjectDataPtr)
{
   int   ret_val  = 0;
   char  FilePath[OTP_MAXIMUM_COMBINED_PATH_LENGTH];

   /* Make sure the parameters are semi-valid.                          */
   if((FileDescriptor) && (FileName) && (ObjectDataPtr))
   {
      /* Prefix the directory path before assigning the file name.      */
      BTPS_StringCopy(FilePath, ServerData.Directory_Data.Path);
      strcat(FilePath, FileName);

      /* Try to open the existing file for the specified mode.          */
      if((*FileDescriptor = BTPS_Open_File(FilePath, OpenMode)) != NULL)
      {
         /* Position the stream location to the specified offset.       */
         if(BTPS_Seek_File(*FileDescriptor, smBeginning, Offset))
         {
            /* We will go ahead and format the OTS Object First Created */
            /* time.                                                    */
            /* * NOTE * We will randomize the time.                     */
            ObjectDataPtr->First_Created.Year    = 2016;
            ObjectDataPtr->First_Created.Month   = 3;
            ObjectDataPtr->First_Created.Day     = 18;
            ObjectDataPtr->First_Created.Hours   = (rand() % 24);
            ObjectDataPtr->First_Created.Minutes = (rand() % 60);
            ObjectDataPtr->First_Created.Seconds = (rand() % 60);

            /* Simply call the internal function to read the file       */
            /* information.                                             */
            /* * NOTE * This function will set the OTS Object Last      */
            /*          Modified and Allocated Size fields for the      */
            /*          current object.                                 */
            if(!(ret_val = ReadFileContentsInfo(FilePath, &(ObjectDataPtr->Last_Modified), &(ObjectDataPtr->Size.Allocated_Size))))
            {
               /* We will set the current size to the allocated size.   */
               ObjectDataPtr->Size.Current_Size = ObjectDataPtr->Size.Allocated_Size;
            }
            else
            {
               /* Close the file descriptor.                            */
               CloseFileContents(FileDescriptor);
            }
         }
         else
         {
            /* Close the file descriptor.                               */
            CloseFileContents(FileDescriptor);

            printf("\r\nCould not seek to the position of the specified offset.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Determine the error message to display.                     */
         if(OpenMode == omCreate)
         {
            /* Print the error message.                                 */
            printf("\r\nThe file: \"%s\", could not be created.\r\n", FileName);
            printf("   Reason: \"%s\".\r\n", strerror(errno));
            ret_val = FUNCTION_ERROR;
         }
         else
         {
            /* Print the error message.                                 */
            printf("\r\nThe file: \"%s\", could not be opened.\r\n", FileName);
            printf("   Reason: \"%s\".\r\n", strerror(errno));
            ret_val = FUNCTION_ERROR;
         }
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

  /* The following function is only for the OTP Client and is           */
  /* responsible for opening a file's contents (a descriptor to a file  */
  /* on the file system).  This function will seek to the specified     */
  /* offset if the file descriptor is opened.                           */
static int OpenFileContentsClient(BTPS_File_Descriptor_t *FileDescriptor, char *FilePath, BTPS_Open_Mode_t OpenMode, DWord_t Offset, DWord_t *AllocatedSize)
{
   int  ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((FileDescriptor) && (FilePath))
   {
      /* Try to open the existing file for the specified mode.          */
      if((*FileDescriptor = BTPS_Open_File(FilePath, OpenMode)) != NULL)
      {
         /* Position the stream location to the specified offset.       */
         if(BTPS_Seek_File(*FileDescriptor, smBeginning, Offset))
         {
            /* Simply call the internal function to read the file       */
            /* information.                                             */
            /* * NOTE * This function will set the allocated size of the*/
            /*          file.                                           */
            if((ret_val = ReadFileContentsInfo(FilePath, NULL, AllocatedSize)) != 0)
            {
               /* Close the file descriptor.                            */
               CloseFileContents(FileDescriptor);

               /* Print the error message.                              */
               printf("\r\nThe contents of file: \"%s\", could not be read.\r\n", FilePath);
            }
         }
         else
         {
            /* Close the file descriptor.                               */
            CloseFileContents(FileDescriptor);

            printf("\r\nCould not seek to the position of the specified by the offset.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* Print the error message.                                    */
         printf("\r\nThe file: \"%s\", could not be opened.\r\n", FilePath);
         printf("   Reason: \"%s\".\r\n", strerror(errno));
         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is responsible for closing an open file.   */
   /* * NOTE * The file descriptor will be set to NULL if this function */
   /*          is successful.                                           */
static void CloseFileContents(BTPS_File_Descriptor_t *FileDescriptor)
{
   /* Try to close the file.                                            */
   BTPS_Close_File(*FileDescriptor);
   *FileDescriptor = NULL;
}

   /* The following function is responsible for deleting a file from the*/
   /* filesystem.                                                       */
static int DeleteFileContents(char *FileName)
{
   int   ret_val = 0;
   char  FilePath[OTP_MAXIMUM_COMBINED_PATH_LENGTH];

   /* Prefix the directory path before assigning the file name.         */
   BTPS_StringCopy(FilePath, ServerData.Directory_Data.Path);
   strcat(FilePath, FileName);

   /* Remove the file.                                                  */
   if(!BTPS_Delete_File(FilePath))
   {
      printf("\r\nCannot remove file: \"%s\".\r\n", FileName);
      printf("   Reason: \"%s\".\r\n", strerror(errno));
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading a file on the   */
   /* filesystem.                                                       */
   /* * NOTE * This function returns the number of bytes successfully   */
   /*          written or zero.                                         */
static DWord_t ReadFileContents(BTPS_File_Descriptor_t FileDescriptor, DWord_t BufferLength, Byte_t *Buffer)
{
   DWord_t  ret_val = 0;

   /* Read the file.                                                    */
   if(!(BTPS_Read_File(FileDescriptor, BufferLength, Buffer, &ret_val)))
   {
      printf("\r\nCannot read file.\r\n");
      printf("   Reason: \"%s\".\r\n", strerror(errno));
   }

   return(ret_val);
}

   /* The following function is responsible for writing a file on the   */
   /* filesystem.                                                       */
   /* * NOTE * This function returns the number of bytes successfully   */
   /*          written or zero.                                         */
static DWord_t WriteFileContents(BTPS_File_Descriptor_t FileDescriptor, DWord_t BufferLength, Byte_t *Buffer)
{
   DWord_t  ret_val = 0;

   /* Write the file.                                                   */
   if(!BTPS_Write_File(FileDescriptor, BufferLength, Buffer, &ret_val))
   {
      printf("\r\nCannot write file.\r\n");
      printf("   Reason: \"%s\".\r\n", strerror(errno));
   }

   return(ret_val);
}

   /* The following function is responsible for reading a file's        */
   /* information on the filesystem.                                    */
   /* * NOTE * This function may be used to read information about a    */
   /*          directory.                                               */
   /* * NOTE * This function will only set the OTS Date Time data type  */
   /*          and Allocated size if the parameters are included.  If   */
   /*          both are excluded (NULL), this function will do nothing. */
static int ReadFileContentsInfo(char *FilePath, OTS_Date_Time_Data_t *DateTimeData, DWord_t *AllocatedSize)
{
   int                      ret_val = 0;
   BTPS_File_Information_t  FileInformation;

   /* Read the file information.                                        */
   if(BTPS_Query_File_Information(FilePath, &FileInformation))
   {
      /* Set the OTS Date Time data.                                    */
      if(DateTimeData)
      {
         DateTimeData->Year    = FileInformation.FileTime.Year;
         DateTimeData->Month   = FileInformation.FileTime.Month;
         DateTimeData->Day     = FileInformation.FileTime.DayOfMonth;
         DateTimeData->Hours   = FileInformation.FileTime.Hour;
         DateTimeData->Minutes = FileInformation.FileTime.Minute;
         DateTimeData->Seconds = FileInformation.FileTime.Seconds;
      }

      /* Set the Allocated Size.                                        */
      if(AllocatedSize)
      {
         *AllocatedSize = FileInformation.FileSize;
      }
   }
   else
   {
      printf("\r\nCannot read file information for file: \"%s\".\r\n", FilePath);
      printf("   Reason: \"%s\".\r\n", strerror(errno));
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for renaming a file on the  */
   /* file system.                                                      */
static int RenameFileServer(char *OldName, char *NewName)
{
   int   ret_val = 0;
   char  OldPath[OTP_MAXIMUM_COMBINED_PATH_LENGTH];
   char  NewPath[OTP_MAXIMUM_COMBINED_PATH_LENGTH];

   /* Prefix the directory path before assigning the old file name.     */
   BTPS_StringCopy(OldPath, ServerData.Directory_Data.Path);
   strcat(OldPath, OldName);

   /* Prefix the directory path before assigning the new file name.     */
   BTPS_StringCopy(NewPath, ServerData.Directory_Data.Path);
   strcat(NewPath, NewName);

   /* Rename the file.                                                  */
   if(rename(OldPath, NewPath) != 0)
   {
      printf("\r\nThe file could not be renamed.\r\n");
      printf("   Reason: %s\r\n", strerror(errno));
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for opening a directory on  */
   /* the filesystem.                                                   */
static int OpenDirectory(BTPS_Directory_Descriptor_t *DirectoryDescriptor, char *DirectoryPath)
{
   int  ret_val = 0;

   /* Open the directory.                                               */
   if((*DirectoryDescriptor = BTPS_Open_Directory(DirectoryPath)) == NULL)
   {
      printf("\r\nCannot open the directory: \"%s\".\r\n", DirectoryPath);
      printf("   Reason: \"%s\".\r\n", strerror(errno));
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for closing a directory on  */
   /* the filesystem.                                                   */
static void CloseDirectory(BTPS_Directory_Descriptor_t *DirectoryDescriptor)
{
   /* Simply call the function to close the directory.                  */
   BTPS_Close_Directory(DirectoryDescriptor);
   DirectoryDescriptor = NULL;
}

   /* The following function is responsible for making a directory on   */
   /* the filesystem.                                                   */
static int MakeDirectory(char *DirectoryPath)
{
   int ret_val = 0;

   /* Make the directory.                                               */
   ret_val = BTPS_Make_Directory(DirectoryPath);
   switch(ret_val)
   {
      case BTPS_FILE_MAKE_DIRECTORY_SUCCESS:
         printf("\r\nSuccessfully created the directory: \"%s\".\r\n", DirectoryPath);
         break;
      case BTPS_FILE_MAKE_DIRECTORY_ERROR_ALREADY_EXISTS:
         printf("\r\nThe directory: \"%s\", already exists.\r\n", DirectoryPath);
         break;
      default:
         /* If we were unable to create the directory.                  */
         printf("\r\nCannot create the directory: \"%s\".\r\n", DirectoryPath);
         printf("   Reason: \"%s\".\r\n", strerror(errno));
   }

   return(ret_val);
}

   /* The following function is responsible for deleting a directory on */
   /* the filesystem.                                                   */
static void DeleteDirectory(char *DirectoryPath)
{
   /* Delete the directory.                                             */
   if(!BTPS_Delete_Directory(DirectoryPath))
   {
      printf("\r\nCannot remove the directory: \"%s\".\r\n", DirectoryPath);
      printf("   Reason: \"%s\".\r\n", strerror(errno));
   }
}

   /* The following function is responsible for adding a file object to */
   /* the OTS Objects list that does not have an existing file on the   */
   /* file system.                                                      */
   /* * NOTE * This function will reset all OTS Object List Filters if  */
   /*          we are connected to an OTP Client.                       */
static void CreateFileObject(char *FileName, DWord_t Properties)
{
   unsigned int                Index;
   unsigned int                Index2;
   Boolean_t                   ObjectAvailable = FALSE;
   BTPS_File_Descriptor_t      FileDescriptor;
   OTS_Object_Data_t          *ObjectDataPtr;
   Byte_t                      Buffer[OTP_MAXIMUM_GENERATED_FILE_SIZE];
   DeviceInfo_t               *DeviceInfo;
   OTP_Filtered_Object_List_t *List;
   OTS_Object_Changed_Data_t   ObjectChangedData;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Loop through the OTS Object List to determine if there is an   */
      /* OTS Object available.                                          */
      for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
      {
         /* Store a pointer to the OTS Object data.                     */
         ObjectDataPtr = &(ServerData.Object_List[Index].Data);

         /* If we find an OTS Object that is not valid.                 */
         if(ServerData.Object_List[Index].Valid == FALSE)
         {
            /* Make sure we can create the new file on the file system. */
            if(!OpenFileContentsServer(&FileDescriptor, FileName, omCreate, 0, ObjectDataPtr))
            {
               /* Simply flag that an OTS Object is available.          */
               ObjectAvailable = TRUE;

               /* Flag that the OTS Object is valid.                    */
               ServerData.Object_List[Index].Valid = TRUE;

               /* Set the Flags field to control what OTS Object fields */
               /* are included for this object in the OTS Directory     */
               /* Listing Object's contents.                            */
               ObjectDataPtr->Flags = OTP_DEFAULT_OBJECT_FLAGS;

               /* Store the OTS Object Name.                            */
               BTPS_StringCopy((char *)(ObjectDataPtr->Name.Buffer), FileName);
               ObjectDataPtr->Name.Buffer_Length = BTPS_StringLength(FileName);

               /* Store the OTS Object Type.                            */
               /* * NOTE * We will randomly generate a 16-bit UUID or a */
               /*          128-bit UUID depending on whether the        */
               /*          randomly generated number is even/odd.       */
               if(rand() % 2)
               {
                  /* Randomize the 16-bit UUID.                         */
                  ObjectDataPtr->Type.UUID_Type               = guUUID_16;
                  ObjectDataPtr->Type.UUID.UUID_16.UUID_Byte0 = rand() % 256;
                  ObjectDataPtr->Type.UUID.UUID_16.UUID_Byte1 = rand() % 256;
               }
               else
               {
                  /* Randomize the 128-bit UUID.                        */
                  ObjectDataPtr->Type.UUID_Type = guUUID_128;
                  for(Index2 = 0; Index2 < UUID_128_SIZE; Index2++)
                  {
                     ((Byte_t *)&(ObjectDataPtr->Type.UUID.UUID_128))[Index2] = rand() % 256;
                  }

                  /* Update the OTS Object Flags to reflect the change. */
                  ObjectDataPtr->Flags |= OTS_OBJECT_RECORD_FLAGS_TYPE_UUID_SIZE_128;
               }

               /* Store the OTS Object Size.                            */
               /* * NOTE * We will randomize the allocated size.        */
               /*          However, the current size MUST be less than  */
               /*          or equal to the allocated size so we will    */
               /*          simply set the current size to the allocated */
               /*          size.                                        */
               ObjectDataPtr->Size.Allocated_Size = (rand() % OTP_MAXIMUM_GENERATED_FILE_SIZE);
               ObjectDataPtr->Size.Current_Size   = ObjectDataPtr->Size.Allocated_Size;

               /* * NOTE * The OTS Object First Created date will be set*/
               /*          when the file is created on the file system. */

               /* * NOTE * The OTS Object Last Modified date will be set*/
               /*          when the file is created on the file system. */

               /* Store the OTS Object ID.                              */
               /* * NOTE * We will always increment the Object ID       */
               /*          Counter, however we should never reach the   */
               /*          upper portion of the UINT48 size for this    */
               /*          sample application.                          */
               ObjectDataPtr->ID.Lower = ObjectIDCtr++;
               ObjectDataPtr->ID.Upper = 0;

               /* Store the OTS Object Properties.                      */
               ObjectDataPtr->Properties = Properties;

               /* Fill the buffer with random data based on the current */
               /* size.                                                 */
               for(Index2 = 0; Index2 < (unsigned int)ObjectDataPtr->Size.Current_Size; Index2++)
               {
                  Buffer[Index2] = rand() % 256;
               }

               /* Write the buffer to the file.                         */
               /* * NOTE * This should not fail, however an error will  */
               /*          be printed if it does.                       */
               WriteFileContents(FileDescriptor, ObjectDataPtr->Size.Current_Size, Buffer);

               /* Close the file descriptor.                            */
               CloseFileContents(&FileDescriptor);

               /* Display the created OTS Object.                       */
               printf("\r\nCreated Object:\r\n");
               DisplayObjectData(ObjectDataPtr);

               /* Make sure that we are connected and a remote device is*/
               /* selected.                                             */
               if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
               {
                  /* Get the device information for the OTP Client.     */
                  /* * NOTE * This should not FAIL.                     */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                  {
                     /* Store a pointer to the Filtered Objects List    */
                     /* data.                                           */
                     List = &(DeviceInfo->FilteredObjectList);

                     /* Create the Filtered Objects List based on the   */
                     /* Objects List, which has been updated with the   */
                     /* new OTS Object.                                 */
                     /* * NOTE * We will pass in the OTS Object ID to   */
                     /*          set the new OTS Object as the current  */
                     /*          object.                                */
                     CreateFilteredObjectsList(List, &(ObjectDataPtr->ID));

                     /* Loop through the OTS Object List Filter data.   */
                     for(Index = 0; Index < (unsigned int)OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS; Index++)
                     {
                        /* We need to free memory if an OTS Object List */
                        /* Filter is an OTS Object Name type.           */
                        if((List->List_Filter_Data[Index].Type >= lftNameStartsWith) && (List->List_Filter_Data[Index].Type <= lftNameIsExactly))
                        {
                           /* Make sure memory has been allocated.      */
                           if((List->List_Filter_Data[Index].Data.Name.Buffer_Length) && (List->List_Filter_Data[Index].Data.Name.Buffer))
                           {
                              /* Free the memory.                       */
                              BTPS_FreeMemory(List->List_Filter_Data[Index].Data.Name.Buffer);
                              List->List_Filter_Data[Index].Data.Name.Buffer        = NULL;
                              List->List_Filter_Data[Index].Data.Name.Buffer_Length = 0;
                           }
                           else
                              printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
                        }

                        /* Set the filter type to 'No Filter'.          */
                        /* * NOTE * We need to to do this so that the   */
                        /*          new OTS Object is ALWAYS exposed to */
                        /*          the OTP Client when it is created.  */
                        List->List_Filter_Data[Index].Type = lftNoFilter;
                     }

                     /* Let the user know we have reset all the list    */
                     /* filter instances.                               */
                     printf("\r\nAll Object List Filter instances have been reset to 'No Filter'.\r\n");

                     /* We need to send an indication to the OTP        */
                     /* Clientthat the current object has changed to the*/
                     /* new Object.                                     */
                     ObjectChangedData.Flags     = (OTS_OBJECT_CHANGED_FLAGS_OBJECT_CONTENTS_CHANGED | \
                                                    OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED | \
                                                    OTS_OBJECT_CHANGED_FLAGS_OBJECT_CREATION);
                     ObjectChangedData.Object_ID = ObjectDataPtr->ID;

                     /* Simply call the internal function to send the   */
                     /* indication.                                     */
                     IndicateObjectChanged(&ObjectChangedData);
                  }
                  else
                     printf("\r\nNo device information for the OTP Client.\r\n");
               }

               /* Terminate the loop since we are done.                 */
               break;
            }
         }
      }

      /* If an OTS Object was not available.                            */
      if(!ObjectAvailable)
         printf("\r\nMaximum number of supported OTS Objects has been reached.\r\n");
   }
   else
      printf("\r\nOnly the OTP Server may create an OTS Object.\r\n");
}

   /* The following function is responsible for finding an OTP Object   */
   /* Entry based on an OTS Object ID.                                  */
static OTP_Object_Entry_t* FindOTPObjectEntry(OTS_UINT48_Data_t *ObjectID)
{
   OTP_Object_Entry_t *ret_val = NULL;
   unsigned int        Index;

   if(ObjectID)
   {
      /* Loop through the objects list and find the current object.     */
      for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
      {
         /* Make sure the OTS Object is valid.                          */
         if(ServerData.Object_List[Index].Valid)
         {
            /* If we find the OTS Object.                               */
            /* * NOTE * This sample application only uses the lower     */
            /*          portion of the Object ID.                       */
            if(ServerData.Object_List[Index].Data.ID.Lower == ObjectID->Lower)
            {
               /* Simply return a pointer to it.                        */
               ret_val = &(ServerData.Object_List[Index]);

               /* We are done.                                          */
               break;
            }
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for adding a Filtered Object*/
   /* Node to the Filter Objects List.                                  */
   /* ** NOTE ** This Node's fields MUST be initialized before callings */
   /*            this function.  This function WILL NOT CHECK OR SET    */
   /*            THEM.                                                  */
   /* * NOTE * This function is always add Nodes at the end of the list.*/
static void AddFilteredObjectNode(OTP_Filtered_Object_List_t *List, OTP_Filtered_Object_Node_t *Node)
{
   /* If the list exists and has Nodes.                                 */
   if((List->HeadPtr != NULL) && (List->Number_Of_Nodes))
   {
      /* Update the new Node's previous pointer to the current tail of  */
      /* the list.                                                      */
      /* * NOTE * The next pointer will remain NULL since it is at the  */
      /*          end of the list.                                      */
      Node->PreviousPtr      = List->TailPtr;

      /* Insert the new Node at the end of the list.                    */
      List->TailPtr->NextPtr = Node;

      /* Update the tail pointer to the new Node.                       */
      List->TailPtr          = Node;
   }
   else
   {
      /* Otherwise this is the first Node in the list and we need to    */
      /* point the head and tail pointers to the Node.                  */
      List->HeadPtr = Node;
      List->TailPtr = Node;
   }

   /* Increment the number of Nodes in the list.                        */
   List->Number_Of_Nodes++;
}

   /* The following function is responsible for removing an Node from   */
   /* the Filtered Objects List.                                        */
   /* ** NOTE ** This function SHOULD NOT be called if the Node is not  */
   /*            in the List.                                           */
   /* * NOTE * It is the caller's responsibility to set the Node to NULL*/
   /*          after this function is called.                           */
static OTP_Filtered_Object_Node_t *RemoveFilteredObjectNode(OTP_Filtered_Object_List_t *List, OTP_Filtered_Object_Node_t *Node)
{
   OTP_Filtered_Object_Node_t *ret_val;

   /* If the Node is at the head of the list.                           */
   if(Node == List->HeadPtr)
   {
      /* Set the new head of the list to the Nodes next pointer.        */
      /* * NOTE * This may be NULL if there is only one Node in the     */
      /*          list.  If this is the case we will need to update the */
      /*          tail pointer to NULL.  Otherwise we will leave the    */
      /*          tail pointer alone.                                   */
      List->HeadPtr = Node->NextPtr;

      /* Update the head pointer's previous pointer to NULL since it no */
      /* longer points to the old head (Node to be removed).            */
      /* * NOTE * Only do this if the Head pointer is not NULL.  This   */
      /*          may be the case if the Node's next pointer is NULL    */
      /*          (Only one Node in the list).                          */
      if(List->HeadPtr != NULL)
      {
         List->HeadPtr->PreviousPtr = NULL;
      }
      else
      {
         /* The list MUST be empty so we will update the tail pointer to*/
         /* NULL.                                                       */
         List->TailPtr = NULL;
      }

      /* Return the new head pointer of the List, which is the next     */
      /* object.                                                        */
      ret_val = List->HeadPtr;

      /* Free the Node.                                                 */
      BTPS_FreeMemory(Node);

      /* Decrement the number of Nodes in the list.                     */
      List->Number_Of_Nodes--;
   }
   else
   {
      /* The Node MUST be in the middle or at the end of the list.      */
      /* * NOTE * The Node's previous pointer MUST be valid for this    */
      /*          case.                                                 */

      /* Update the Node's previous pointer's next pointer to point to  */
      /* the Node's Next pointer.  Effectively skipping over the Node.  */
      /* * NOTE * This may be NULL if the Node is the last Node in the  */
      /*          list.  If this is the case we need to update the tail */
      /*          pointer.                                              */
      Node->PreviousPtr->NextPtr = Node->NextPtr;

      /* If the Node is the last Node in the list.                      */
      if(Node->NextPtr == NULL)
      {
         /* The new tail of the list is the Node's previous pointer.    */
         List->TailPtr = Node->PreviousPtr;
      }

      /* Return the Node's Next pointer so we can correctly increment to*/
      /* the new Next Node.                                             */
      ret_val = Node->NextPtr;

      /* Free the Node.                                                 */
      BTPS_FreeMemory(Node);

      /* Decrement the number of Nodes in the list.                     */
      List->Number_Of_Nodes--;
   }

   return(ret_val);
}

   /* The following function is responsible for creating the Filtered   */
   /* Objects List.                                                     */
   /* * NOTE * The resulting Filtered Objects List will contain a Node  */
   /*          for each OTS Object stored on the OTP Server.  This      */
   /*          implies that all OTS Object List Filter instances are set*/
   /*          to 'No Filter'.                                          */
   /* * NOTE * If the Filtered Objets List was previously sorted we will*/
   /*          re-sort it.                                              */
static void CreateFilteredObjectsList(OTP_Filtered_Object_List_t *List, OTS_UINT48_Data_t *ObjectID)
{
   unsigned int                Index;
   OTP_Filtered_Object_Node_t *Node;
   Boolean_t                   Error = FALSE;

   /* Delete the current Filtered Objects List.                         */
   DeleteFilteredObjectsList(List);

   /* Loop through the OTP Server Objects List.                         */
   for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
   {
      /* If the OTS Object is valid.                                    */
      if(ServerData.Object_List[Index].Valid)
      {
         /* Allocate memory for the new Node.                           */
         if((Node = (OTP_Filtered_Object_Node_t *)BTPS_AllocateMemory(OTP_FILTERED_OBJECT_NODE_SIZE)) != NULL)
         {
            /* Initialize the Node.                                     */
            BTPS_MemInitialize(Node, 0, OTP_FILTERED_OBJECT_NODE_SIZE);

            /* Point the Node to its associated OTS Object.             */
            Node->ObjectDataPtr = &(ServerData.Object_List[Index].Data);

            /* Add the Node to the Filtered Objects List.               */
            AddFilteredObjectNode(List, Node);

            /* Set the pointer for the Node that is current object.     */
            /* * NOTE * This can only happen once.                      */
            /* * NOTE * This sample application only uses the lower     */
            /*          portion of the OTS Object ID.                   */
            if(Node->ObjectDataPtr->ID.Lower == ObjectID->Lower)
            {
               /* We found the Node that is the current object.  Store  */
               /* the pointer.                                          */
               List->Current_Object = Node;
            }
         }
         else
         {
            /* Print an error, cleanup the Filtered Objects List, Mark  */
            /* the current object as not included in the Filtered       */
            /* Objects List and exit.                                   */
            printf("\r\nCould not allocate memory for the Node.\r\n");
            DeleteFilteredObjectsList(List);
            Error = TRUE;
            break;
         }
      }
   }

   /* If an error did not occur then the list was created.              */
   if(!Error)
   {
      /* Go ahead and re-sort the Filtered Objects List if the OTP      */
      /* Client has previously sorted it.                               */
      /* * NOTE * We don't have to do this, but when a new OTS Object is*/
      /*          created or an OTS Object List Filter is applied, we   */
      /*          want the previous sort to be applied.                 */
      if(List->Sorted)
      {
         /* Make sure the list is not alrady sorted.                    */
         if((List->Number_Of_Nodes) && (List->Number_Of_Nodes != 1))
         {
            /* Simply call the internal function to perform the sort.   */
            /* * NOTE * This should NOT fail.  If it does an error will */
            /*          be printed.                                     */
            SortFilteredObjectsList(List);
         }
      }
   }
}

   /* The following function is responsible for deleting the Filtered   */
   /* Objects List.                                                     */
static void DeleteFilteredObjectsList(OTP_Filtered_Object_List_t *List)
{
   OTP_Filtered_Object_Node_t *Node;
   OTP_Filtered_Object_Node_t *NextNode;

   /* Set the current Node to the head of the list.                     */
   Node = List->HeadPtr;

   /* While there is a Node to delete.                                  */
   while(Node)
   {
      /* Save a pointer to the next Node so we can delete it next.      */
      /* * NOTE * This may be NULL if we reached the end of the list.   */
      NextNode = Node->NextPtr;

      /* Delete the Node.                                               */
      BTPS_FreeMemory(Node);

      /* Decrement the number of Nodes.                                 */
      List->Number_Of_Nodes--;

      /* Set the next Node to delete.                                   */
      Node = NextNode;
   }

   /* Now that the list is empty set the List head and tail pointers to */
   /* NULL.                                                             */
   List->HeadPtr = NULL;
   List->TailPtr = NULL;

   /* Make sure the current object is set to NULL.                      */
   List->Current_Object = NULL;
}

   /* The following function is responsible for updating the Filtered   */
   /* Objects List based on the current OTS Object List filters         */
   /* currently being applied.                                          */
static void UpdateFilteredObjectsList(OTP_Filtered_Object_List_t *List)
{
   OTP_Filtered_Object_Node_t *Node;
   OTS_UINT48_Data_t           ObjectID;

   /* Initialize the ObjectID structure to zero.                        */
   /* * NOTE * This will force the current object to ALWAYS be reset to */
   /*          the OTS Directory Listing Object, which CANNOT be deleted*/
   /*          and is ALWAYS on the OTS Server.                         */
   BTPS_MemInitialize(&ObjectID, 0, OTS_UINT48_DATA_SIZE);

   /* If the current object is valid.                                   */
   if(List->Current_Object)
   {
      /* Store the OTS Object ID that is REQUIRED for re-select the     */
      /* current object in the Filtered Objects List when it is rebuilt.*/
      /* * NOTE * This MUST be done since the Node for the current      */
      /*          object will have a new address.                       */
      ObjectID.Lower = List->Current_Object->ObjectDataPtr->ID.Lower;
      ObjectID.Upper = List->Current_Object->ObjectDataPtr->ID.Upper;
   }

   /* Create a fresh copy of the Filtered Objects List.                 */
   CreateFilteredObjectsList(List, &ObjectID);

   /* Loop through the Filtered Objects List.                           */
   Node = List->HeadPtr;
   while(Node)
   {
      /* Determine if the Node needs to be removed from the Filtered    */
      /* Objects List based on the OTS Object List Filter instances     */
      /* currently being applied.                                       */
      if(!VerifyNodeForFilteredObjectsList(List, Node->ObjectDataPtr))
      {
         /* Lets get the next Node to verify.                           */
         Node = Node->NextPtr;
      }
      else
      {
         /* Determine if the Node for the current object is going to be */
         /* removed.                                                    */
         /* * NOTE * Do this first since the Node pointer will be       */
         /*          invalid after we remove it.                        */
         if(List->Current_Object == Node)
         {
            /* We will simply mark that it is no longer valid.          */
            List->Current_Object = NULL;
         }

         /* Remove the Node from the Filtered Objects List.             */
         /* * NOTE * This function will set the new Next Node in the    */
         /*          list.                                              */
         Node = RemoveFilteredObjectNode(List, Node);
      }
   }
}

   /* The following function is responsible for updating the Filtered   */
   /* Objects List for every remote device that has device information  */
   /* stored on the OTS Server.                                         */
static void UpdateAllFilteredObjectLists(void)
{
   DeviceInfo_t *DeviceInfo;

   /* Loop through the device information.                              */
   DeviceInfo = DeviceInfoList;
   while(DeviceInfo)
   {
      /* Simply call the internal function to do the work.              */
      UpdateFilteredObjectsList(&(DeviceInfo->FilteredObjectList));

      /* Get the next devices information.                              */
      DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
   }
}

   /* The following function is a helper function that will determine if*/
   /* a Node should remain in the Filtered Objects List based on the OTS*/
   /* Object List Filter instances currently being applied.             */
static int VerifyNodeForFilteredObjectsList(OTP_Filtered_Object_List_t *List, OTS_Object_Data_t *ObjectDataPtr)
{
   int                            ret_val = 0;
   unsigned int                   Index;
   Byte_t                         ReversedName[OTS_MAXIMUM_OBJECT_NAME_LENGTH];
   Byte_t                         ReversedFilter[OTS_MAXIMUM_OBJECT_NAME_LENGTH];
   OTS_Object_List_Filter_Data_t *ObjectListFilter;

   /* Loop through the Filtered Object List filters currently being     */
   /* applied.                                                          */
   for(Index = 0; (!ret_val) && (Index < OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS); Index++)
   {
      /* Store a pointer to the OTS Object List Filter we are currently */
      /* going to verify for the Node's OTS Object data.                */
      ObjectListFilter = &(List->List_Filter_Data[Index]);

      /* Peform the comparison based on the OTS Object Filter type.     */
      switch(ObjectListFilter->Type)
      {
         case lftNoFilter:
            /* Do nothing, since this means the Node is always included */
            /* in the Filtered Objects List.                            */
            break;
         case lftNameStartsWith:
            /* Compare the OTS Object Name with the starting string.    */
            if(BTPS_MemCompareI((char *)ObjectListFilter->Data.Name.Buffer, (char *)ObjectDataPtr->Name.Buffer, ObjectListFilter->Data.Name.Buffer_Length) != 0)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftNameEndsWith:
            /* Store a copy of the OTS Object Name so that we do not    */
            /* modify its actual value.                                 */
            BTPS_MemCopy(ReversedName, ObjectDataPtr->Name.Buffer, ObjectDataPtr->Name.Buffer_Length);

            /* Store a copy of the OTS Object Filter Data Name so that  */
            /* we do not modify its actual value for the next OTS Object*/
            /* List Filter's data that will verified.                   */
            BTPS_MemCopy(ReversedFilter, ObjectListFilter->Data.Name.Buffer, ObjectListFilter->Data.Name.Buffer_Length);

            /* Reverse the strings for comparison.                      */
            ReverseString(ObjectDataPtr->Name.Buffer_Length,         ReversedName);
            ReverseString(ObjectListFilter->Data.Name.Buffer_Length, ReversedFilter);

            /* Compare the OTS Object Name with the starting string     */
            /* (This is the ending string).                             */
            if(BTPS_MemCompareI(ReversedFilter, ReversedName, ObjectListFilter->Data.Name.Buffer_Length) != 0)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftNameContains:
            /* Locate the substring.                                    */
            if(strstr((char *)ObjectDataPtr->Name.Buffer, ((char *)ObjectListFilter->Data.Name.Buffer)) == NULL)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftNameIsExactly:
            /* Make sure the OTS Object Name matches the filtered Name  */
            /* length.                                                  */
            if(ObjectListFilter->Data.Name.Buffer_Length == ObjectListFilter->Data.Name.Buffer_Length)
            {
               /* Compare the OTS Object Name with the string.          */
               if(strcasecmp((char *)ObjectListFilter->Data.Name.Buffer, (char *)ObjectDataPtr->Name.Buffer) != 0)
               {
                  /* Return that we did not match.                      */
                  ret_val = FUNCTION_ERROR;
               }
               break;
            }
            else
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftObjectType:
            /* Determine the OTS Object Type UUID type.                 */
            if(ObjectListFilter->Data.Type.UUID_Type == guUUID_16)
            {
               /* Compare the 16-bit UUID.                              */
               if(!COMPARE_UUID_16(ObjectListFilter->Data.Type.UUID.UUID_16, ObjectDataPtr->Type.UUID.UUID_16))
               {
                  /* Return that we did not match.                      */
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               /* Compare the 128-bit UUID.                             */
               if(!COMPARE_UUID_128(ObjectListFilter->Data.Type.UUID.UUID_128, ObjectDataPtr->Type.UUID.UUID_128))
               {
                  /* Return that we did not match.                      */
                  ret_val = FUNCTION_ERROR;
               }
            }
            break;
         case lftCreatedBetween:
            /* Check to make sure the OTS First Created Date Time is    */
            /* withing the filtered range .                             */
            if(CompareDateTimeRange(&(ObjectDataPtr->First_Created), &(ObjectListFilter->Data.Time_Range.Minimum), &(ObjectListFilter->Data.Time_Range.Maximum)) != 0)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftModifiedBetween:
            /* Check to make sure the OTS Last Modified Date Time is    */
            /* withing the filtered range .                             */
            if(CompareDateTimeRange(&(ObjectDataPtr->Last_Modified), &(ObjectListFilter->Data.Time_Range.Minimum), &(ObjectListFilter->Data.Time_Range.Maximum)) != 0)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftCurrentSizeBetween:
            /* Check to make sure the OTS Object Current Size is withing*/
            /* the filtered range.                                      */
            if(!((ObjectDataPtr->Size.Current_Size >= ObjectListFilter->Data.Size_Range.Minimum) && (ObjectDataPtr->Size.Current_Size <= ObjectListFilter->Data.Size_Range.Maximum)))
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftAllocatedSizeBetween:
            /* Check to make sure the OTS Object Current Size is withing*/
            /* the filtered range.                                      */
            if(!((ObjectDataPtr->Size.Allocated_Size >= ObjectListFilter->Data.Size_Range.Minimum) && (ObjectDataPtr->Size.Allocated_Size <= ObjectListFilter->Data.Size_Range.Maximum)))
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         case lftMarkedObjects:
            /* Simply check if the OTS Object is marked.                */
            if(ObjectDataPtr->Marked == FALSE)
            {
               /* Return that we did not match.                         */
               ret_val = FUNCTION_ERROR;
            }
            break;
         default:
            printf("\r\nWarning - Invalid OTS Object List Filter type.\r\n");
            break;
      }
   }

   return ret_val;
}

   /* The following function is responsible for sorting the Filtered    */
   /* Objects List.                                                     */
   /* * NOTE * This function should not be called if the List only      */
   /*          contains one Filtered OTS Object.  This means that the   */
   /*          list is already sorted.                                  */
static int SortFilteredObjectsList(OTP_Filtered_Object_List_t *List)
{
   int                         ret_val = 0;
   unsigned int                Index;
   OTS_Object_Data_t         **ObjectDataPtrList;
   OTP_Filtered_Object_Node_t *Node;
   OTS_UINT48_Data_t           ObjectID;

   /* Initialize the current object ID structure to zero.               */
   /* * NOTE * Included to prevent a compiler warning that CANNOT occur.*/
   BTPS_MemInitialize(&ObjectID, 0, OTS_UINT48_DATA_SIZE);

   /* If the current object is selected in the Filtered Objects List,   */
   /* then we need to save it since the Node's OTS Object data pointer  */
   /* for the current object may change after the sort.                 */
   if(List->Current_Object)
   {
      /* * NOTE * This sample application only uses the lower portion of*/
      /*          the OTS Object ID.                                    */
      ObjectID.Lower = List->Current_Object->ObjectDataPtr->ID.Lower;
      ObjectID.Upper = 0;
   }

   /* Create a list of OTS Object data pointers.                        */
   if((ObjectDataPtrList = (OTS_Object_Data_t **)BTPS_AllocateMemory((sizeof(OTS_Object_Data_t *) * List->Number_Of_Nodes))) != NULL)
   {
      /* Loop through the Filtered Objects List.                        */
      for(Index = 0, Node = List->HeadPtr; (Node) && (Index < (unsigned int)List->Number_Of_Nodes); Index++, Node = Node->NextPtr)
      {
         /* We will simply store each Nodes OTS Object data pointer.    */
         ObjectDataPtrList[Index] = Node->ObjectDataPtr;
      }

      /* Set the sort type so that our comparator function will compare */
      /* the specified OTS Object data field.                           */
      /* * NOTE * We need to do this because we can't pass in the sort  */
      /*          type to the comparator.                               */
      /* * NOTE * We will use a global and since no other OTP can be    */
      /*          executing a procedure we can guarantee that it won't  */
      /*          be modified.                                          */
      SortType = List->Sorted;

      /* Perform the sort using quick sort.                             */
      /* ** NOTE ** The SortOrder MUST be set for the ObjectComparator()*/
      /*            function before calling qsort().                    */
      /* * NOTE * The sort is made stable in the comparator by using the*/
      /*          unique ID.                                            */
      /* * NOTE * The comparator will handle putting the list in the    */
      /*          correct order (Ascending or Descending).              */
      qsort(ObjectDataPtrList, (unsigned int)List->Number_Of_Nodes, sizeof(OTS_Object_Data_t *), ObjectComparator);

      /* Update the Filtered Objects List with the sorted OTS Object    */
      /* data pointers list.                                            */
      for(Index = 0, Node = List->HeadPtr; (Node) && (Index < (unsigned int)List->Number_Of_Nodes); Index++, Node = Node->NextPtr)
      {
         /* We will simply update each Nodes OTS Object data ptr to give*/
         /* the illusion that we sorted the Filtered Objects List.      */
         Node->ObjectDataPtr = ObjectDataPtrList[Index];

         /* If we need to re-assign the current object.                 */
         if(List->Current_Object)
         {
            /* If we find a match based on the unique OTS Object ID.    */
            /* * NOTE * This can only possibly match once.              */
            if(Node->ObjectDataPtr->ID.Lower == ObjectID.Lower)
            {
               /* Update the current object.                            */
               List->Current_Object = Node;
            }
         }
      }

      /* Free the memory used for the OTS Object pointers list.         */
      BTPS_FreeMemory(ObjectDataPtrList);
      ObjectDataPtrList = NULL;
   }
   else
   {
      printf("Memory could not be allocated for the OTS Object pointers list.\r\n");
      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is a helper function that is responsible   */
   /* for converting the specified string into data of type GATT_UUID_t.*/
static int StrToOTSObjectType(char *UUIDStr, GATT_UUID_t *UUID)
{
   int           ret_val      = 0;
   char          TempString[512];
   Byte_t       *Value        = NULL;
   unsigned int  Index;
   unsigned int  HexValue;
   unsigned int  StringLength;

   if((UUIDStr) && (StringLength = BTPS_StringLength(UUIDStr)) && (StringLength >= 2))
   {
      if((UUIDStr[0] != '0') || (UUIDStr[1] != 'x'))
      {
         strcpy(TempString, "0x");
         strncpy(&TempString[2], UUIDStr, (sizeof(TempString) - 2));

         StringLength += 2;
      }
      else
         strncpy(TempString, UUIDStr, 256);

      if(StringLength == UUID_16_HEX_STRING_LENGTH)
      {
         UUID->UUID_Type = guUUID_16;
         Value           = &(UUID->UUID.UUID_16.UUID_Byte0);
      }
      else
      {
         if(StringLength == UUID_128_HEX_STRING_LENGTH)
         {
            UUID->UUID_Type = guUUID_128;
            Value           = &(UUID->UUID.UUID_128.UUID_Byte0);
         }
         else
         {
            if(StringLength == UUID_32_HEX_STRING_LENGTH)
            {
               UUID->UUID_Type = guUUID_32;
               Value           = &(UUID->UUID.UUID_32.UUID_Byte0);
            }
         }
      }

      if(Value)
      {
         /* Scan two characters at a time from last to first excluding  */
         /* 0x.                                                         */
         for(Index = StringLength - 2; Index > 0; Index -= 2, ++Value)
         {
            sscanf(&TempString[Index], "%02X", &HexValue);
            *Value = (Byte_t)HexValue;
         }
      }
      else
         ret_val = FUNCTION_ERROR;
   }
   else
      ret_val = FUNCTION_ERROR;

   return(ret_val);
}

   /* The following function is a helper function that is responsible   */
   /* for converting the specified string into data of type             */
   /* OTS_Date_Time_Data_t.  The first parameter of this function is the*/
   /* Date string in YYYYMMDDHHMMSS format to be converted to a         */
   /* OTS_Date_Time_Data_t.                                             */
static int StrToOTSDateTime(char *DateTimeStr, OTS_Date_Time_Data_t *DateTimeData)
{
   int ret_val = 0;

   /* Verify the length.                                                */
   if(BTPS_StringLength(DateTimeStr) == DATE_TIME_STRING_LENGTH)
   {
      /* Parse the Date Time string.                                    */
      if(sscanf(DateTimeStr, "%04u%02u%02u%02u%02u%02u", (unsigned int *)&(DateTimeData->Year),
                                                          (unsigned int *)&(DateTimeData->Month),
                                                          (unsigned int *)&(DateTimeData->Day),
                                                          (unsigned int *)&(DateTimeData->Hours),
                                                          (unsigned int *)&(DateTimeData->Minutes),
                                                          (unsigned int *)&(DateTimeData->Seconds)) != 6)
      {
         /* We did not parrse the date time correctly.                  */
         ret_val = FUNCTION_ERROR;
      }
   }

   return(ret_val);
}

   /* The following function is responsible for executing the OACP      */
   /* Create Procedure.                                                 */
   /* * NOTE * If this function fails the Current Object will remain set*/
   /*          to it's previous value.                                  */
static void CreateProcedure(OTP_Filtered_Object_List_t *List, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData)
{
   unsigned int       Index;
   Boolean_t          ObjectAvailable = FALSE;
   OTS_Object_Data_t *ObjectDataPtr;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_CREATE_OP_CODE_SUPPORTED)
   {
      /* * NOTE * We could reject the request if the UUID type in not   */
      /*          supported, however we will allow all UUIDS to be      */
      /*          accepted by this sample application.                  */

      /* Loop through the OTS Object List to determine if there is an   */
      /* OTS Object available.                                          */
      for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
      {
         /* Store a pointer to the OTS Object data.                     */
         ObjectDataPtr = &(ServerData.Object_List[Index].Data);

         /* If we find an OTS Object that is not valid.                 */
         if(ServerData.Object_List[Index].Valid == FALSE)
         {
            /* Simply flag that an OTS Object is available.             */
            ObjectAvailable = TRUE;

            /* Flag that the OTS Object is valid.                       */
            ServerData.Object_List[Index].Valid = TRUE;

            /* Set the Flags field to control what OTS Object fields are*/
            /* included for this object in the OTS Directory Listing    */
            /* Object's contents.                                       */
            ObjectDataPtr->Flags = OTP_DEFAULT_OBJECT_FLAGS;

            /* We will assign a temporary name for the file until the   */
            /* OTP Client writes the OTS Object Name.                   */
            /* ** NOTE ** Each OTS Object Data Entry contains a Name    */
            /*            buffer we will simply assign to the Buffer    */
            /*            field so that we do not need to allocate      */
            /*            memory for each OTS Object Name.              */
            /* * NOTE * The OTS Object Name MUST be writeable if the OTS*/
            /*          Server supports the OACP Create Procedure.      */
            /* * NOTE * If the OTP Client uses the OACP Create Procedure*/
            /*          twice we will overwrite the temporary file if it*/
            /*          has not been renamed.                           */
            BTPS_StringCopy((char *)(ObjectDataPtr->Name.Buffer), OTP_DEFAULT_NEW_FILE_NAME);
            ObjectDataPtr->Name.Buffer_Length = BTPS_StringLength(OTP_DEFAULT_NEW_FILE_NAME);

            /* Store the OTS Object Type included as a parameter in the */
            /* OACP request.                                            */
            ObjectDataPtr->Type = RequestData->Parameter.Create_Data.UUID;

            /* Store the OTS Object Size included as a parameter in the */
            /* OACP request.                                            */
            /* * NOTE * The current size will be zero since the new OTS */
            /*          Object has not contents.                        */
            ObjectDataPtr->Size.Allocated_Size = RequestData->Parameter.Create_Data.Size;
            ObjectDataPtr->Size.Current_Size   = 0;

            /* Store the OTS First Created.                             */
            /* * NOTE * This will be zero since the OTS First Created is*/
            /*          not valid (The contents have not been written). */
            BTPS_MemInitialize(&(ObjectDataPtr->First_Created), 0, GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE);

            /* Store the OTS Last Modified.                             */
            /* * NOTE * This will be zero since the OTS Last Modified is*/
            /*          not valid (The contents have not been written). */
            BTPS_MemInitialize(&(ObjectDataPtr->Last_Modified), 0, GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE);

            /* Store the OTS Object ID.                                 */
            /* * NOTE * We will always increment the Object ID Counter, */
            /*          however we should never reach the upper portion */
            /*          of the UINT48 size for this sample application. */
            ObjectDataPtr->ID.Lower = ObjectIDCtr++;
            ObjectDataPtr->ID.Upper = 0;

            /* Store the OTS Object Properties.                         */
            ObjectDataPtr->Properties = OTP_DEFAULT_OBJECT_PROPERTIES;

            /* Assign the new current object for the Filtered Objects   */
            /* List.                                                    */
            /* * NOTE * We are going to rebuild the Filtered Objects    */
            /*          List for every remote device that has           */
            /*          information stored on the OTS Server.           */
            List->Current_Object->ObjectDataPtr = ObjectDataPtr;

            /* Loop through the OTS Object List Filters and make sure   */
            /* they are set to 'No Filter' so the new object is exposed */
            /* to the OTP Client that made the request.                 */
            for(Index = 0; Index < (unsigned int)OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS; Index++)
            {
               /* We need to free memory if an OTS Object List Filter is*/
               /* an OTS Object Name type.                              */
               if((List->List_Filter_Data[Index].Type >= lftNameStartsWith) && (List->List_Filter_Data[Index].Type <= lftNameIsExactly))
               {
                  /* Make sure memory has been allocated.               */
                  if((List->List_Filter_Data[Index].Data.Name.Buffer_Length) && (List->List_Filter_Data[Index].Data.Name.Buffer))
                  {
                     /* Free the memory.                                */
                     BTPS_FreeMemory(List->List_Filter_Data[Index].Data.Name.Buffer);
                     List->List_Filter_Data[Index].Data.Name.Buffer        = NULL;
                     List->List_Filter_Data[Index].Data.Name.Buffer_Length = 0;
                  }
                  else
                     printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
               }

               /* Set the filter type to 'No Filter'.                   */
               /* * NOTE * We need to to do this so that the new OTS    */
               /*          Object is ALWAYS exposed to the OTP Client   */
               /*          when it is created.                          */
               List->List_Filter_Data[Index].Type = lftNoFilter;
            }

            /* Let the user know we have reset all the list filter      */
            /* instances for this client.                               */
            printf("\r\nAll Object List Filter instances have been reset to 'No Filter'.\r\n");

            /* Update the Filtered Objects List for every remote device */
            /* that has information stored by the OTS Server.           */
            /* * NOTE * This function will destroy every Filtered Object*/
            /*          List and rebuild it.  As such in order to       */
            /*          include the new OTS Object the OTS Object List  */
            /*          Filter instances MUST be set to expose the new  */
            /*          OTS Object.  It will ALWAYS be exposed for the  */
            /*          OTP client that sent the OACP Create Procedure  */
            /*          request.                                        */
            UpdateAllFilteredObjectLists();

            /* Set the Result code for the OACP response indication.    */
            ResponseData->Result_Code = oarSuccess;

            /* Terminate the loop since we are done.                    */
            break;
         }
      }

      /* If an OTS Object was not available.                            */
      if(!ObjectAvailable)
      {
         printf("\r\nMaximum number of supported OTS Objects has been reached.\r\n");
         ResponseData->Result_Code = oarOperationFailed;
      }
   }
   else
   {
      printf("\r\nThe Create Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP      */
   /* Delete Procedure.                                                 */
static void DeleteProcedure(OTP_Filtered_Object_List_t *List, OTS_OACP_Response_Data_t *ResponseData)
{
   OTS_Object_Data_t  *CurrentObject;
   OTP_Object_Entry_t *ObjectEntryPtr = NULL;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_DELETE_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(List->Current_Object)
      {
         /* Store a pointer to the current object's data.               */
         CurrentObject = List->Current_Object->ObjectDataPtr;

         /* Check to make sure that the current object's properties     */
         /* permit deletion.                                            */
         if(CurrentObject->Properties & OTS_OBJECT_PROPERTIES_DELETE)
         {
            /* Find the OTS Object Entry for the current object.        */
            ObjectEntryPtr = FindOTPObjectEntry(&(CurrentObject->ID));

            /* Make sure that the currently selected object has not been*/
            /* locked by another client for writing.                    */
            if((ObjectEntryPtr) && (ObjectEntryPtr->Locked == FALSE))
            {
               /* Let's go ahead and delete the file on the file system.*/
               DeleteFileContents((char *)CurrentObject->Name.Buffer);

               /* Mark that the OTS Object Entry is invalid.            */
               ObjectEntryPtr->Valid = FALSE;

               /* Set the Result code that the procedure completed      */
               /* successfully.                                         */
               ResponseData->Result_Code = oarSuccess;

               /* Update the Filtered Objects List for every remote     */
               /* device that has information stored by the OTS Server. */
               /* * NOTE * This function will destroy every Filtered    */
               /*          Object List and rebuild it.  As such the     */
               /*          deleted OTS Object will be excluded if it is */
               /*          present.  Also if the deleted OTS Object is  */
               /*          the current object it will also be           */
               /*          re-assigned to NULL.                         */
               UpdateAllFilteredObjectLists();
            }
            else
            {
               printf("\r\nObject is locked.\r\n");
               ResponseData->Result_Code = oarObjectLocked;
            }
         }
         else
         {
            printf("\r\nProcedure not permitted.\r\n");
            ResponseData->Result_Code = oarProcedureNotPermitted;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Delete Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP      */
   /* Calculate Checksum Procedure.                                     */
static void CalculateChecksumProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData)
{
   int                     Result;
   BTPS_File_Descriptor_t  File_Descriptor;
   DWord_t                 Length;
   DWord_t                 Offset;
   Byte_t                 *Buffer;
   OTS_Object_Data_t      *ObjectDataPtr;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_CALCULATE_CHECKSUM_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(CurrentObject)
      {
         /* Store a pointer to the current object's data.               */
         ObjectDataPtr = CurrentObject->ObjectDataPtr;

         /* Store the parameters to make the code more readable.        */
         Length = RequestData->Parameter.Calculate_Checksum_Data.Length;
         Offset = RequestData->Parameter.Calculate_Checksum_Data.Offset;

         /* Check to make sure the parameters included in the OACP      */
         /* Request are valid.                                          */
         /* * NOTE * We will simply make sure the specified offset plus */
         /*          length does not exceed the allocate size for the   */
         /*          file.                                              */
         if((Offset + Length) <= ObjectDataPtr->Size.Allocated_Size)
         {
            /* Open the file descriptor to the existing file at the     */
            /* specified offset.                                        */
            if(!OpenFileContentsServer(&File_Descriptor, (char *)ObjectDataPtr->Name.Buffer, omReadOnly, Offset, ObjectDataPtr))
            {
               /* Create a buffer large enough to hold data read from   */
               /* the file that the checksum will be calculated over.   */
               if((Buffer = (Byte_t *)BTPS_AllocateMemory(sizeof(Byte_t) * Length)) != NULL)
               {
                  /* Initialize the buffer so we don't have any         */
                  /* unexpected behaviour.                              */
                  BTPS_MemInitialize(Buffer, 0, Length);

                  /* Store the data from the file in the buffer.        */
                  /* * NOTE * We will make sure that the number of bytes*/
                  /*          read from the file matches the expected   */
                  /*          length.  Otherwise an error has occured.  */
                  if((ReadFileContents(File_Descriptor, Length, Buffer)) == Length)
                  {
                     /* Simply call the internal function to calculate  */
                     /* the 32-bit CRC.                                 */
                     if((Result = OTS_Calculate_CRC_32(BluetoothStackID, Length, Buffer, &(ResponseData->Parameter.Checksum))) == 0)
                     {
                        /* Set the Result code that the procedure       */
                        /* completed successfully.                      */
                        ResponseData->Result_Code = oarSuccess;
                     }
                     else
                     {
                        DisplayFunctionError("OTS_Calculate_CRC_32", Result);
                        ResponseData->Result_Code = oarOperationFailed;
                     }
                  }
                  else
                  {
                     printf("\r\nFailed to read the data from the file to store in the buffer.\r\n");
                     ResponseData->Result_Code = oarOperationFailed;
                  }

                  /* Free the allocated memory.                         */
                  BTPS_FreeMemory(Buffer);
                  Buffer = NULL;
               }
               else
               {
                  printf("\r\nCould not allocate memory for the buffer.\r\n");
                  ResponseData->Result_Code = oarOperationFailed;
               }

               /* Close the file.                                       */
               CloseFileContents(&File_Descriptor);
            }
            else
            {
               /* OpenExistingFile() already printed an error so just   */
               /* set the Result Code.                                  */
               ResponseData->Result_Code = oarOperationFailed;
            }
         }
         else
         {
            printf("\r\nObject allocated size will be exceeded.\r\n");
            ResponseData->Result_Code = oarInvalidParameter;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Calculate Checksum Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP      */
   /* Execute Procedure.                                                */
static void ExecuteProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTS_OACP_Response_Data_t *ResponseData)
{
   OTS_Object_Data_t *ObjectDataPtr;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_EXECUTE_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(CurrentObject)
      {
         /* Store a pointer to the current object's data.               */
         ObjectDataPtr = CurrentObject->ObjectDataPtr;

         /* Check to make sure that the current object's properties     */
         /* permit execution.                                           */
         if(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_EXECUTE)
         {
            /* We will simply print that we executed and action on the  */
            /* OTP Server for the current object.                       */
            printf("\r\nExecuting an action on the server.\r\n");

            /* Set the Result code that the procedure completed         */
            /* successfully.                                            */
            ResponseData->Result_Code = oarSuccess;
         }
         else
         {
            printf("\r\nProcedure not permitted.\r\n");
            ResponseData->Result_Code = oarProcedureNotPermitted;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Execute Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP Read */
   /* Procedure.                                                        */
static void ReadProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData)
{
   int                 Result;
   DWord_t             Length;
   DWord_t             Offset;
   OTS_Object_Data_t  *ObjectDataPtr;
   OTP_Object_Entry_t *ObjectEntryPtr = NULL;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_READ_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(CurrentObject)
      {
         /* Store a pointer to the current object's data.               */
         ObjectDataPtr = CurrentObject->ObjectDataPtr;

         /* Check to make sure that the current object's properties     */
         /* permit reading.                                             */
         if(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_READ)
         {
            /* Make sure we are connected over the Object Transfer      */
            /* Channel (OTC).                                           */
            if(ChannelData->Flags & OTP_CHANNEL_FLAGS_CONNECTED)
            {
               /* Find the OTS Object Entry for the current object.     */
               ObjectEntryPtr = FindOTPObjectEntry(&(ObjectDataPtr->ID));

               /* Make sure that the currently selected object has not  */
               /* been locked by another client for writing.            */
               if((ObjectEntryPtr) && (ObjectEntryPtr->Locked == FALSE))
               {
                  /* Store the parameters to make the code more         */
                  /* readable.                                          */
                  Length = RequestData->Parameter.Read_Data.Length;
                  Offset = RequestData->Parameter.Read_Data.Offset;

                  /* Check to make sure the parameters included in the  */
                  /* OACP Request are valid.                            */
                  /* * NOTE * We will simply make sure the specified    */
                  /*          offset plus length does not exceed the    */
                  /*          allocated size for the file.              */
                  /* * NOTE * This may not be current for the OTS       */
                  /*          Directory Listing Object.  The OTP Client */
                  /*          should always read the OTS Object Size    */
                  /*          before performing an OACP Read Procedure. */
                  if((Offset + Length) <= ObjectDataPtr->Size.Current_Size)
                  {
                     /* Determine if we are reading the OTS Directory   */
                     /* Listing Object or another OTS Object.  We will  */
                     /* simply check the OTS Object ID for the current  */
                     /* object since the OTS Directory Listing Object   */
                     /* will have an OTS Object ID of zero.             */
                     /* * NOTE * We need to do this since the OTS       */
                     /*          Directory Listing Content's is made up */
                     /*          of OTS Object Records and not data from*/
                     /*          a file.                                */
                     /* * NOTE * This sample application does not use   */
                     /*          the upper portion of the ID field.     */
                     if((ObjectDataPtr->ID.Lower == 0) && (ObjectDataPtr->ID.Upper == 0))
                     {
                        /* Make sure the current size is semi-valid.    */
                        if((Offset + Length) == ObjectDataPtr->Size.Current_Size)
                        {
                           /* Store the transfer buffer length for the  */
                           /* OTS Directory Listing Object.             */
                           ChannelData->Buffer_Length = ObjectDataPtr->Size.Current_Size;

                           /* Allocate memory for the transfer buffer to*/
                           /* hold the OTS Directory Listing Object.    */
                           /* * NOTE * The OTP Client should read the   */
                           /*          OTS Object Size before requesting*/
                           /*          the OTS Directory Listing        */
                           /*          Object's contents since the size */
                           /*          will vary depending on the number*/
                           /*          of OTS Objects on the OTP Server */
                           /*          and there current Flags field    */
                           /*          that is set for each OTS Object. */
                           if((ChannelData->Buffer = (Byte_t *)BTPS_AllocateMemory(ChannelData->Buffer_Length)) != NULL)
                           {
                              /* Format the OTS Directory Listing       */
                              /* Object's contents into the buffer.     */
                              if((Result = OTS_Format_Directory_Listing_Object_Contents(ServerData.Number_Of_Objects, ServerData.Directory_List, ChannelData->Buffer_Length, ChannelData->Buffer)) == 0)
                              {
                                 /* Set the OTS Channel data on the OTS */
                                 /* Server for when the OTP Client reads*/
                                 /* the contents over the Object        */
                                 /* Transfer Channel (OTC).             */
                                 ChannelData->Current_Offset =  Offset;
                                 ChannelData->Ending_Offset  = (Offset + Length);

                                 /* Set the Result code that the        */
                                 /* procedure completed successfully.   */
                                 ResponseData->Result_Code = oarSuccess;
                              }
                              else
                              {
                                 DisplayFunctionError("OTS_Format_Directory_Listing_Object_Contents", Result);
                                 ResponseData->Result_Code = oarOperationFailed;

                                 /* Simply call the internal function to*/
                                 /* cleanup the channel data.           */
                                 CleanupChannel(ChannelData);
                              }
                           }
                           else
                           {
                              printf("\r\nThe OTS Directory Listing Object's contents has not been formatted.");
                              printf("\r\nReading the OTS Directory Listing Object's size will format the contents.\r\n");
                              ResponseData->Result_Code = oarOperationFailed;
                           }
                        }
                        else
                        {
                           printf("\r\nOffset plus length MUST equal the OTS Directory Listing Object's current size.\r\n");
                           ResponseData->Result_Code = oarOperationFailed;
                        }
                     }
                     else
                     {
                        /* The OTS Object's contents are in a file on   */
                        /* the file system so we will open a file stream*/
                        /* to the existing file at the specified offset.*/
                        if(!OpenFileContentsServer(&(ChannelData->File_Descriptor), (char *)ObjectDataPtr->Name.Buffer, omReadOnly, Offset, ObjectDataPtr))
                        {
                           /* Store the buffer length.                  */
                           ChannelData->Buffer_Length = Length;

                           /* Allocate memory for the buffer.           */
                           if((ChannelData->Buffer = (Byte_t *)BTPS_AllocateMemory(ChannelData->Buffer_Length)) != NULL)
                           {
                              /* Store the data from the file in the    */
                              /* buffer.                                */
                              /* * NOTE * We will make sure that the    */
                              /*          number of bytes read from the */
                              /*          file matches the expected     */
                              /*          length.  Otherwise an error   */
                              /*          has occured.                  */
                              if(ReadFileContents(ChannelData->File_Descriptor, ChannelData->Buffer_Length, ChannelData->Buffer) == Length)
                              {
                                 /* Set the OTS Channel data on the OTS */
                                 /* Server for when the OTP Client reads*/
                                 /* the contents over the Object        */
                                 /* Transfer Channel (OTC).             */
                                 ChannelData->Current_Offset = 0;
                                 ChannelData->Ending_Offset  = Length;

                                 /* Set the Result code that the        */
                                 /* procedure completed successfully.   */
                                 ResponseData->Result_Code = oarSuccess;
                              }
                              else
                              {
                                 /* ReadFileContents() already printed  */
                                 /* an error so just set the Result     */
                                 /* Code.                               */
                                 ResponseData->Result_Code = oarOperationFailed;

                                 /* Free the memory previous allocated. */
                                 BTPS_FreeMemory(ChannelData->Buffer);
                                 ChannelData->Buffer        = NULL;
                                 ChannelData->Buffer_Length = 0;
                              }
                           }
                           else
                           {
                              printf("\r\nCould not allocate memory to hold the OTS Directory Listing Object's contents.\r\n");
                              ResponseData->Result_Code = oarInsufficientResources;
                           }
                        }
                        else
                        {
                           /* OpenExistingFile() already printed an     */
                           /* error so just set the Result Code.        */
                           ResponseData->Result_Code = oarOperationFailed;
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nObject's current size will be exceeded.\r\n");
                     ResponseData->Result_Code = oarInvalidParameter;
                  }
               }
               else
               {
                  printf("\r\nObject is locked.\r\n");
                  ResponseData->Result_Code = oarObjectLocked;
               }
            }
            else
            {
               printf("\r\nObject Transfer Channel is not connected.\r\n");
               ResponseData->Result_Code = oarChannelUnavailable;
            }
         }
         else
         {
            printf("\r\nProcedure not permitted.\r\n");
            ResponseData->Result_Code = oarProcedureNotPermitted;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Read Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP Write*/
   /* Procedure.                                                        */
static void WriteProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Request_Data_t *RequestData, OTS_OACP_Response_Data_t *ResponseData)
{
   Boolean_t              Error = FALSE;
   DWord_t                Length;
   DWord_t                Offset;
   OTS_Write_Mode_Type_t  Type;
   OTS_Object_Data_t     *ObjectDataPtr;
   OTP_Object_Entry_t    *ObjectEntryPtr = NULL;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_WRITE_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(CurrentObject)
      {
         /* Store a pointer to the current object's data.               */
         ObjectDataPtr = CurrentObject->ObjectDataPtr;

         /* Check to make sure that the current object's properties     */
         /* permit writing.                                             */
         if(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_WRITE)
         {
            /* Make sure we are connected over the Object Transfer      */
            /* Channel (OTC).                                           */
            if(ChannelData->Flags & OTP_CHANNEL_FLAGS_CONNECTED)
            {
               /* Find the OTS Object Entry for the current object.     */
               ObjectEntryPtr = FindOTPObjectEntry(&(ObjectDataPtr->ID));

               /* Make sure that the currently selected object has not  */
               /* been locked by another client for writing.            */
               if((ObjectEntryPtr) && (ObjectEntryPtr->Locked == FALSE))
               {
                  /* Store the parameters to make the code more         */
                  /* readable.                                          */
                  Length = RequestData->Parameter.Write_Data.Length;
                  Offset = RequestData->Parameter.Write_Data.Offset;
                  Type   = RequestData->Parameter.Write_Data.Mode;

                  /* Check to make sure the offset does not exceed the  */
                  /* current object's current size.                     */
                  /* * NOTE * The current size will become the offset if*/
                  /*          the Mode is set to truncate.  However, we */
                  /*          CANNOT truncate a file past its current   */
                  /*          size.                                     */
                  if(Offset <= ObjectDataPtr->Size.Current_Size)
                  {
                     /* If the offset is less than or equal to the      */
                     /* current size of the current object then patching*/
                     /* MUST be supported to overwrite the old contents.*/
                     if(Offset < ObjectDataPtr->Size.Current_Size)
                     {
                        /* Check if the OTP Server supports patching.   */
                        if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_PATCHING_SUPPORTED)
                        {
                           /* Make sure the current object's properties */
                           /* support pathing.                          */
                           if(!(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_PATCH))
                           {
                              printf("\r\nPatching not permitted.\r\n");
                              ResponseData->Result_Code = oarProcedureNotPermitted;
                              Error = TRUE;
                           }
                        }
                        else
                        {
                           printf("\r\nPatching not supported by the OTP Server.\r\n");
                           ResponseData->Result_Code = oarProcedureNotPermitted;
                           Error = TRUE;
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nObject's current size will be exceeded.\r\n");
                     ResponseData->Result_Code = oarInvalidParameter;
                     Error = TRUE;
                  }

                  /* If an error has not occured.                       */
                  if(!Error)
                  {
                     /* If the offset plus length is greater than the   */
                     /* allocated size of the current object then       */
                     /* appending MUST be supported to write past the   */
                     /* alloated size.                                  */
                     if((Offset + Length) > ObjectDataPtr->Size.Allocated_Size)
                     {
                        /* Check if the OTP Server supports patching.   */
                        if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_APPENDING_SUPPORTED)
                        {
                           /* Make sure the current object's properties */
                           /* support pathing.                          */
                           if(!(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_APPEND))
                           {
                              printf("\r\nAppending not permitted.\r\n");
                              ResponseData->Result_Code = oarProcedureNotPermitted;
                              Error = TRUE;
                           }
                        }
                        else
                        {
                           printf("\r\nAppending not supported by the OTP Server.\r\n");
                           ResponseData->Result_Code = oarProcedureNotPermitted;
                           Error = TRUE;
                        }
                     }
                  }

                  /* If an error has not occured.                       */
                  if(!Error)
                  {
                     /* Determine if truncation is requested.           */
                     switch(Type)
                     {
                        case wmtNone:
                           /* Do nothing for this case.                 */
                           break;
                        case wmtTruncate:
                           /* Make sure the OTP Server supports this    */
                           /* feature.                                  */
                           if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_TRUNCATION_SUPPORTED)
                           {
                              /* Make sure the current object's         */
                              /* properties support truncation.         */
                              /* * NOTE * We will update the current    */
                              /*          size to the specified offset  */
                              /*          once the file has been opened.*/
                              if(!(ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_TRUNCATE))
                              {
                                 printf("\r\nTruncate not permitted.\r\n");
                                 ResponseData->Result_Code = oarProcedureNotPermitted;
                                 Error = TRUE;
                              }
                           }
                           else
                           {
                              printf("\r\nTruncate not supported by the OTP Server.\r\n");
                              ResponseData->Result_Code = oarProcedureNotPermitted;
                              Error = TRUE;
                           }
                           break;
                        default:
                           printf("\r\nInvalid Write Mode type.\r\n");
                           ResponseData->Result_Code = oarInvalidParameter;
                           Error = TRUE;
                           break;
                     }
                  }

                  /* If an error has not occured.                       */
                  if(!Error)
                  {
                     /* If the current size is not zero then, the OTS   */
                     /* Object has been written before and the file MUST*/
                     /* exist, even if it has not been given a Name yet.*/
                     if(ObjectDataPtr->Size.Current_Size)
                     {
                        /* Open the existing file.                      */
                        if(OpenFileContentsServer(&(ChannelData->File_Descriptor), (char *)ObjectDataPtr->Name.Buffer, omWriteOnly, Offset, ObjectDataPtr) == 0)
                        {
                           /* Check if we are going to truncate the     */
                           /* file.                                     */
                           if(Type == wmtTruncate)
                           {
                              /* Set the current size to the offset to  */
                              /* truncate.  This way the data will be   */
                              /* overwritten.                           */
                              /* * NOTE * This MUST be done here since  */
                              /*          OpenFileContentsServer() will */
                              /*          set the current size to the   */
                              /*          allocated size by default.    */
                              ObjectDataPtr->Size.Current_Size = Offset;
                           }
                        }
                        else
                        {
                           /* OpenFileContentsServer() already printed  */
                           /* an error so just set the Result Code.     */
                           ResponseData->Result_Code = oarOperationFailed;
                           Error = TRUE;
                        }
                     }
                     else
                     {
                        /* Create the new file since the new OTS Object */
                        /* has not been written before.                 */
                        /* * NOTE * We can guarantee at this point that */
                        /*          the offset is also zero since it was*/
                        /*          checked against OTS Object Current  */
                        /*          Size, which is zero.                */
                        /* * NOTE * If the file is created then it will */
                        /*          ALWAYS be truncated regardless of   */
                        /*          the option so we will ignore it.    */
                        if(OpenFileContentsServer(&(ChannelData->File_Descriptor), (char *)ObjectDataPtr->Name.Buffer, omCreate, Offset, ObjectDataPtr) != 0)
                        {
                           /* OpenFileContentsServer() already printed  */
                           /* an error so just set the Result Code.     */
                           ResponseData->Result_Code = oarOperationFailed;
                           Error = TRUE;
                        }
                     }

                     /* If an error has not occured.                    */
                     if(!Error)
                     {
                        /* Set the OTS Channel data on the OTP Server   */
                        /* for when the OTP Client reads the contents   */
                        /* over the Object Transfer Channel (OTC).      */
                        ChannelData->Current_Offset =  Offset;
                        ChannelData->Ending_Offset  = (Offset + Length);

                        /* Set the Result code that the procedure       */
                        /* completed successfully.                      */
                        ResponseData->Result_Code   = oarSuccess;

                        /* Lock the object.                             */
                        ObjectEntryPtr->Locked      = TRUE;

                        /* Flag that an OACP Write Procedure is in      */
                        /* progress.                                    */
                        ChannelData->Flags         |= OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS;
                     }
                  }
               }
               else
               {
                  printf("\r\nThe current object has been locked by another client.\r\n");
                  ResponseData->Result_Code = oarObjectLocked;
               }
            }
            else
            {
               printf("\r\nObject Transfer Channel is not connected.\r\n");
               ResponseData->Result_Code = oarChannelUnavailable;
            }
         }
         else
         {
            printf("\r\nProcedure not permitted.\r\n");
            ResponseData->Result_Code = oarProcedureNotPermitted;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Write Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for executing the OACP Write*/
   /* Procedure.                                                        */
static void AbortProcedure(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, OTS_OACP_Response_Data_t *ResponseData)
{
   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OACP_Features & OTS_FEATURE_OACP_ABORT_OP_CODE_SUPPORTED)
   {
      /* Make sure that the current object is selected.                 */
      if(CurrentObject)
      {
         /* Make sure an OACP Read procedure is currently in progress.  */
         if(ChannelData->Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)
         {
            /* Simply free the buffer previously formatted for the OACP */
            /* Read Procedure.                                          */
            /* * NOTE * This will flag we no longer will send data from */
            /*          the OTP Server to the OTP Client since the OTP  */
            /*          Client has requested to abort the OACP Read     */
            /*          Procedure.                                      */
            if((ChannelData->Buffer_Length) && (ChannelData->Buffer))
            {
               /* Flag that the OACP Read Procedure is no longer in     */
               /* progress.                                             */
               ChannelData->Flags &= ~(OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS);

               /* Flag that we need to cleanup the channel.             */
               ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;

               /* Set the Result code that the procedure completed      */
               /* successfully.                                         */
               ResponseData->Result_Code = oarSuccess;
            }
            else
            {
               printf("\r\nMemory is not allocated for the read procedure buffer.\r\n");
               ResponseData->Result_Code = oarOperationFailed;
            }
         }
         else
         {
            printf("\r\nA Read Procedure is not in progress.\r\n");
            ResponseData->Result_Code = oarOperationFailed;
         }
      }
      else
      {
         printf("\r\nCurrent object not selected.\r\n");
         ResponseData->Result_Code = oarInvalidObject;
      }
   }
   else
   {
      printf("\r\nThe Abort Procedure is not supported.\r\n");
      ResponseData->Result_Code = oarOpcodeNotSupported;
   }
}

   /* The following function is responsible for selecting the first OTS */
   /* Object in the objects list as the current object.                 */
static void FirstProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Make sure the first object is valid.                              */
   if(List->HeadPtr != NULL)
   {
      /* We will simply store the pointer to the head Node as the       */
      /* current object.                                                */
      List->Current_Object = List->HeadPtr;

      /* Set the Result code that the procedure completed successfully. */
      ResponseData->Result_Code = olrSuccess;
   }
   else
   {
      printf("\r\nNo object.\r\n");
      ResponseData->Result_Code = olrNoObject;
   }
}

   /* The following function is responsible for selecting the last OTS  */
   /* Object in the objects list as the current object.                 */
static void LastProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Make sure the last object is valid.  Otherwise the list is empty  */
   if(List->TailPtr != NULL)
   {
      /* We will simply store the pointer to the tail Node as the       */
      /* current object.                                                */
      List->Current_Object = List->TailPtr;

      /* Set the Result code that the procedure completed successfully. */
      ResponseData->Result_Code = olrSuccess;
   }
   else
   {
      printf("\r\nNo object.\r\n");
      ResponseData->Result_Code = olrNoObject;
   }
}

   /* The following function is responsible for selecting the object    */
   /* before the current object as the new current object.              */
static void PreviousProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Make sure the current object is selected.  Otherwise the list is  */
   /* empty.                                                            */
   if(List->Current_Object)
   {
      /* Make sure the current object is not at the head of the list.   */
      /* Otherwise the current object will remain the same since we will*/
      /* go out of bounds.                                              */
      if(List->Current_Object->PreviousPtr != NULL)
      {
         /* We will simply store the pointer to the Node as the current */
         /* object.                                                     */
         List->Current_Object = List->Current_Object->PreviousPtr;

         /* Set the Result code that the procedure completed            */
         /* successfully.                                               */
         ResponseData->Result_Code = olrSuccess;
      }
      else
      {
         printf("\r\nOut of Bounds.\r\n");
         ResponseData->Result_Code = olrOutOfBounds;
      }
   }
   else
   {
      printf("\r\nNo object.\r\n");
      ResponseData->Result_Code = olrNoObject;
   }
}

   /* The following function is responsible for selecting the object    */
   /* after the current object as the new current object.               */
static void NextProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Make sure the list is not empty.                                  */
   if(List->Number_Of_Nodes)
   {
      /* Make sure the current object is selected.                      */
      if(List->Current_Object)
      {
         /* Make sure the current object is not at the tail of the list.*/
         /* Otherwise the current object will remain the same since we  */
         /* will go out of bounds.                                      */
         if(List->Current_Object->NextPtr != NULL)
         {
            /* We will simply store the pointer to the Node as the      */
            /* current object.                                          */
            List->Current_Object = List->Current_Object->NextPtr;

            /* Set the Result code that the procedure completed         */
            /* successfully.                                            */
            ResponseData->Result_Code = olrSuccess;
         }
         else
         {
            printf("\r\nOut of Bounds.\r\n");
            ResponseData->Result_Code = olrOutOfBounds;
         }
      }
      else
      {
         printf("\r\nNo object.\r\n");
         ResponseData->Result_Code = olrOperationFailed;
      }
   }
   else
   {
      printf("\r\nFiltered Objects List is empty.\r\n");
      ResponseData->Result_Code = olrNoObject;
   }
}

   /* The following function is responsible for selecting new current   */
   /* object based on the OTS Object ID.                                */
static void GoToProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Request_Data_t *RequestData, OTS_OLCP_Response_Data_t *ResponseData)
{
   unsigned int                Index;
   OTP_Filtered_Object_Node_t *Node;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OLCP_Features & OTS_FEATURE_OLCP_GO_TO_OP_CODE_SUPPORTED)
   {
      /* Make sure the list is not empty.                               */
      if(List->Number_Of_Nodes)
      {
         /* Set the Result code that the matching Object ID could not be*/
         /* found in case an error occurs.                              */
         ResponseData->Result_Code = olrObjectIDNotFound;

         /* Loop through the Filtered Objects List.                     */
         Node = List->HeadPtr;
         for(Index = 0; (Node) && (Index < (unsigned int)List->Number_Of_Nodes); Index++)
         {
            /* If we match the Node's Object ID then we found the new   */
            /* current object.                                          */
            /* * NOTE * This sample application will not use the upper  */
            /*          portion of the ID.                              */
            if(Node->ObjectDataPtr->ID.Lower == RequestData->Parameter.Object_ID.Lower)
            {
               /* Set the Node as the current object.                   */
               List->Current_Object = Node;

               /* Set the Result code that the procedure completed      */
               /* successfully.                                         */
               ResponseData->Result_Code = olrSuccess;
               break;
            }

            /* Go to the next Node.                                     */
            Node = Node->NextPtr;
         }
      }
      else
      {
         printf("\r\nFiltered Objects List is empty.\r\n");
         ResponseData->Result_Code = olrNoObject;
      }
   }
   else
   {
      printf("\r\nThe GoTo Procedure is not supported.\r\n");
      ResponseData->Result_Code = olrOpcodeNotSupported;
   }
}

   /* The following function is responsible for sorting the Filtered    */
   /* Objects List.                                                     */
   /* * NOTE * We will simply store the OTS Object data pointers for    */
   /*          each Node in a list, then reassign them to the Node's in */
   /*          the sorted order.  So basically we will sort the OTS     */
   /*          Object data pointers based on the specified field of     */
   /*          data.  This way we can sort the Filtered Objects List    */
   /*          without modifying each Node.                             */
static void OrderProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Request_Data_t *RequestData, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OLCP_Features & OTS_FEATURE_OLCP_ORDER_OP_CODE_SUPPORTED)
   {
      /* Make sure the list is not empty.                               */
      if(List->Number_Of_Nodes)
      {
         /* If there is only one Node then the list is already sorted.  */
         if(List->Number_Of_Nodes != 1)
         {
            /* Make sure the List Sort Order parameter is valid.        */
            switch(RequestData->Parameter.List_Sort_Order)
            {
               case lstOrderAscendingByName:
               case lstOrderAscendingByObjectType:
               case lstOrderAscendingByObjectCurrentSize:
               case lstOrderAscendingByObjectFirstCreated:
               case lstOrderAscendingByObjectLastModified:
               case lstOrderDescendingByName:
               case lstOrderDescendingByObjectType:
               case lstOrderDescendingByObjectCurrentSize:
               case lstOrderDescendingByObjectFirstCreated:
               case lstOrderDescendingByObjectLastModified:
                  /* Intentional fall-through for valid List Sort Order */
                  /* types.                                             */

                  /* Store the sort type for future sorts.              */
                  List->Sort_Type = RequestData->Parameter.List_Sort_Order;

                  /* Simply call the internal function to perform the   */
                  /* sort.                                              */
                  if(!SortFilteredObjectsList(List))
                  {
                     /* Flag that a sort has been applied.              */
                     /* * NOTE * We will do this so we can re-sort the  */
                     /*          list if we need to re-sync the Filtered*/
                     /*          Objects List.                          */
                     List->Sorted = TRUE;

                     /* Set the Result code that the procedure completed*/
                     /* successfully since the list has been sorted.    */
                     ResponseData->Result_Code = olrSuccess;
                  }
                  else
                  {
                     /* Set the Result code that the procedure failed.  */
                     ResponseData->Result_Code = olrOperationFailed;
                  }
                  break;
               default:
                  /* Set the Result code that the sort type is invalid. */
                  ResponseData->Result_Code = olrInvalidParameter;
                  break;
            }
         }
         else
         {
            /* Set the Result code that the procedure completed         */
            /* successfully since the list is already sorted.           */
            ResponseData->Result_Code = olrSuccess;
         }
      }
      else
      {
         printf("\r\nFiltered Objects List is empty.\r\n");
         ResponseData->Result_Code = olrNoObject;
      }
   }
   else
   {
      printf("\r\nThe Order Procedure is not supported.\r\n");
      ResponseData->Result_Code = olrOpcodeNotSupported;
   }
}

   /* The following function is responsible for returning the requested */
   /* number of Filtered Objects in the Filtered Objects List.          */
static void RequestNumberOfObjectsProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OLCP_Features & OTS_FEATURE_OLCP_CLEAR_MARKING_OP_CODE_SUPPORTED)
   {
      /* Make sure the list is not empty.                               */
      if(List->Number_Of_Nodes)
      {
         /* Simply set the response parameter for the total number of   */
         /* objects.                                                    */
         ResponseData->Parameter.Total_Number_Of_Objects = (DWord_t)List->Number_Of_Nodes;

         /* Set the Result code that the procedure completed            */
         /* successfully since the list is already sorted.              */
         ResponseData->Result_Code = olrSuccess;
      }
      else
      {
         printf("\r\nFiltered Objects List is empty.\r\n");
         ResponseData->Result_Code = olrNoObject;
      }
   }
   else
   {
      printf("\r\nThe Clear Marking Procedure is not supported.\r\n");
      ResponseData->Result_Code = olrOpcodeNotSupported;
   }
}

   /* The following function is responsible for clearing marked OTS     */
   /* Objects in the Filtered Objects List.                             */
static void ClearMarkingProcedure(OTP_Filtered_Object_List_t *List, OTS_OLCP_Response_Data_t *ResponseData)
{
   unsigned int                Index;
   OTP_Filtered_Object_Node_t *Node;

   /* Check to make sure that the procedure is supported by the OTS     */
   /* Server.                                                           */
   if(ServerData.OTS_Feature.OLCP_Features & OTS_FEATURE_OLCP_CLEAR_MARKING_OP_CODE_SUPPORTED)
   {
      /* Make sure the list is not empty.                               */
      if(List->Number_Of_Nodes)
      {
         /* Go through the Filtered Objects List and clear the marked   */
         /* objects.                                                    */
         for(Index = 0, Node = List->HeadPtr; (Node) && (Index < (unsigned int)List->Number_Of_Nodes); Index++, Node = Node->NextPtr)
         {
            /* Simply mark the OTS Object data as no longer marked.     */
            Node->ObjectDataPtr->Marked = FALSE;
         }

         /* Simply call the internal function to update the Filtered    */
         /* Objects List.                                               */
         /* * NOTE * If a filter of 'Marked' is applied, then there will*/
         /*          be no Nodes (or no OTS Objects) in the Filtered    */
         /*          Objects List.                                      */
         UpdateFilteredObjectsList(List);

         /* Set the Result code that the procedure completed            */
         /* successfully since the list is already sorted.              */
         ResponseData->Result_Code = olrSuccess;
      }
      else
      {
         printf("\r\nFiltered Objects List is empty.\r\n");
         ResponseData->Result_Code = olrNoObject;
      }
   }
   else
   {
      printf("\r\nThe Clear Marking Procedure is not supported.\r\n");
      ResponseData->Result_Code = olrOpcodeNotSupported;
   }
}

   /* The following function is a helper function, which will be passed */
   /* as the final parameter to qsort() in the OrderProcedure().  This  */
   /* function will call the required comparison function for the       */
   /* specified sort order type (This is global since it cannot be      */
   /* passed into qsort()).  This function returns -1 if the first OTS  */
   /* Object is less than the second OTS Object based on the comparison.*/
   /* Otherwise, 1 is returned.                                         */
   /* * NOTE * If the the sort order is descending then we will reverse */
   /*          the result so the correct order is produced.  By default */
   /*          qsort sorts in ascending order.                          */
   /* * NOTE * Zero can never be returned since a second comparison will*/
   /*          be performed if the first comparison determines that the */
   /*          object member being compared is equal.  The second       */
   /*          comparison is for the objectID member, which is unique   */
   /*          and allows for a stable sort.                            */
static int ObjectComparator(const void *Object1, const void *Object2)
{
   int                ret_val        = 0;
   OTS_Object_Data_t *ObjectDataPtr1 = *(OTS_Object_Data_t **)Object1;
   OTS_Object_Data_t *ObjectDataPtr2 = *(OTS_Object_Data_t **)Object2;

   switch(SortType)
   {
      case lstOrderAscendingByName:
      case lstOrderDescendingByName:
         /* Intentional fall-through.                                   */

         /* Compare the OTS Object Name Length.                         */
         if(ObjectDataPtr1->Name.Buffer_Length == ObjectDataPtr2->Name.Buffer_Length)
         {
            /* Compare the OTS Object Name.                             */
            /* * NOTE * This is a Unix call.                            */
            if((ret_val = strcasecmp((char *)ObjectDataPtr1->Name.Buffer, (char *)ObjectDataPtr2->Name.Buffer)) == 0)
            {
               /* If there are equal compare the OTS Object ID, which is*/
               /* guaranteed to be unique.                              */
               ret_val = ObjectIDCompare(&ObjectDataPtr1->ID, &ObjectDataPtr2->ID);
            }
            else
            {
               /* Determine if we need to reverse the result.           */
               if(SortType == lstOrderDescendingByName)
               {
                  ret_val = (ret_val) ? -1 : 1;
               }
            }
         }
         else
         {
            /* Compare the Lengths since they are not equal.            */
            ret_val = (ObjectDataPtr1->Name.Buffer_Length < ObjectDataPtr2->Name.Buffer_Length) ? -1 : 1;

            /* Determine if we need to reverse the result.              */
            if(SortType == lstOrderDescendingByName)
            {
               ret_val = (ret_val) ? -1 : 1;
            }
         }
         break;
      case lstOrderAscendingByObjectType:
      case lstOrderDescendingByObjectType:
         /* Intentional fall-through.                                   */

         /* Compare the OTS Object Type.                                */
         if((ret_val = ObjectTypeCompare(&(ObjectDataPtr1->Type), &(ObjectDataPtr2->Type))) == 0)
         {
            /* If there are equal compare the OTS Object ID, which is   */
            /* guaranteed to be unique.                                 */
            ret_val = ObjectIDCompare(&ObjectDataPtr1->ID, &ObjectDataPtr2->ID);
         }
         else
         {
            /* Determine if we need to reverse the result.              */
            if(SortType == lstOrderDescendingByObjectType)
            {
               ret_val = (ret_val) ? -1 : 1;
            }
         }
         break;
      case lstOrderAscendingByObjectCurrentSize:
      case lstOrderDescendingByObjectCurrentSize:
         /* Intentional fall-through.                                   */

         /* Compare the OTS Object Current Size.                        */
         if((ret_val = CurrentSizeCompare(ObjectDataPtr1->Size.Current_Size, ObjectDataPtr2->Size.Current_Size)) == 0)
         {
            /* If there are equal compare the OTS Object ID, which is   */
            /* guaranteed to be unique.                                 */
            ret_val = ObjectIDCompare(&ObjectDataPtr1->ID, &ObjectDataPtr2->ID);
         }
         else
         {
            /* Determine if we need to reverse the result.              */
            if(SortType == lstOrderDescendingByObjectCurrentSize)
            {
               ret_val = (ret_val) ? -1 : 1;
            }
         }
         break;
      case lstOrderAscendingByObjectFirstCreated:
      case lstOrderDescendingByObjectFirstCreated:
         /* Intentional fall-through.                                   */

         /* Compare the OTS Object First Created date.                  */
         if((ret_val = DateTimeCompare(&(ObjectDataPtr1->First_Created), &(ObjectDataPtr2->First_Created))) == 0)
         {
            /* If there are equal compare the OTS Object ID, which is   */
            /* guaranteed to be unique.                                 */
            ret_val = ObjectIDCompare(&ObjectDataPtr1->ID, &ObjectDataPtr2->ID);
         }
         else
         {
            /* Determine if we need to reverse the result.              */
            if(SortType == lstOrderDescendingByObjectFirstCreated)
            {
               ret_val = (ret_val) ? -1 : 1;
            }
         }
         break;
      case lstOrderAscendingByObjectLastModified:
      case lstOrderDescendingByObjectLastModified:
         /* Intentional fall-through.                                   */

         /* Compare the OTS Object Last Modified date.                  */
         if((ret_val = DateTimeCompare(&(ObjectDataPtr1->Last_Modified), &(ObjectDataPtr2->Last_Modified))) == 0)
         {
            /* If there are equal compare the OTS Object ID, which is   */
            /* guaranteed to be unique.                                 */
            ret_val = ObjectIDCompare(&ObjectDataPtr1->ID, &ObjectDataPtr2->ID);
         }
         else
         {
            /* Determine if we need to reverse the result.              */
            if(SortType == lstOrderDescendingByObjectLastModified)
            {
               ret_val = (ret_val) ? -1 : 1;
            }
         }
         break;
      default:
         printf("\r\nWarning - ObjectComparator(): Unknown Comparison Type.\r\n");
         break;
   }

   return ret_val;
}

   /* The following function is responsible for comparing two OTS       */
   /* Objects based on their Current Size.                              */
static int CurrentSizeCompare(DWord_t CurrentSize1, DWord_t CurrentSize2)
{
   int ret_val = 0;

   if(CurrentSize1 != CurrentSize2)
      ret_val = (CurrentSize1 > CurrentSize2) ? 1 : -1;

   return ret_val;
}

   /* The following function is responsible for comparing two OTS       */
   /* Objects based on their Object Type.                               */
   /* * NOTE * This function expects the UUIDs being compared are in    */
   /*          Little-Endian form.                                      */
static int ObjectTypeCompare(GATT_UUID_t *UUID1, GATT_UUID_t *UUID2)
{
   int          ret_val = 0;
   unsigned int Index;

   /* If the UUID types are not equal.                                  */
   if(UUID1->UUID_Type != UUID2->UUID_Type)
   {
     /* If UUID1's type is less than UUID2's type so return -1.         */
     /* Otherwise it must be greater than so return 1.                  */
      ret_val = ((UUID1->UUID_Type == guUUID_16) && (UUID2->UUID_Type == guUUID_128)) ? -1 : 1;
   }
   else
   {
      /* * NOTE * We will assume that the UUID's are equal.             */

      /* If this is a 16-bit UUID comparison.  Otherwise MUST be a      */
      /* 128-bit UUID.                                                  */
      if(UUID1->UUID_Type == guUUID_16)
      {
         /* Loop over the UUID.                                         */
         for(Index = 0; Index < UUID_16_SIZE; Index++)
         {
            /* If the bytes are not equal.                              */
            if((((Byte_t *)&(UUID1->UUID.UUID_16))[Index]) != (((Byte_t *)&(UUID2->UUID.UUID_16))[Index]))
            {
               /* Compare them and stop since we are finished.          */
               ret_val = ((((Byte_t *)&(UUID1->UUID.UUID_16))[Index]) > (((Byte_t *)&(UUID2->UUID.UUID_16))[Index])) ? 1 : -1;
               break;
            }
         }
      }
      else
      {
         /* Loop over the UUID.                                         */
         for(Index = 0; Index < UUID_128_SIZE; Index++)
         {
            /* If the bytes are not equal.                              */
            if((((Byte_t *)&(UUID1->UUID.UUID_128))[Index]) != (((Byte_t *)&(UUID2->UUID.UUID_128))[Index]))
            {
               /* Compare them and stop since we are finished.          */
               ret_val = ((((Byte_t *)&(UUID1->UUID.UUID_128))[Index]) > (((Byte_t *)&(UUID2->UUID.UUID_128))[Index])) ? 1 : -1;
               break;
            }
         }
      }
   }

   return ret_val;
}

   /* The following function is responsible for comparing two OTS       */
   /* Objects based on their First Created or Last Modified fields.     */
static int DateTimeCompare(OTS_Date_Time_Data_t *DateTime1, OTS_Date_Time_Data_t *DateTime2)
{
   int ret_val = 0;

   /* If the DateTime1 year is equal to DateTime2 year we need to check */
   /* the month of the timestamp.                                       */
   if(DateTime1->Year == DateTime2->Year)
   {
      /* If the DateTime1 month is equal to DateTime2 month we need to  */
      /* check the day of the timestamp.                                */
      if(DateTime1->Month == DateTime2->Month)
      {
         /* If the DateTime1 day is equal to DateTime2 day we need to   */
         /* check the time of the timestamp.                            */
         if(DateTime1->Day == DateTime2->Day)
         {
            /* If the DateTime1 hour is equal to DateTime2 hour we need */
            /* to check the minute of the timestamp.                    */
            if(DateTime1->Hours == DateTime2->Hours)
            {
               /* If the DateTime1 minute is equal to DateTime2 minute  */
               /* we need to check the second of the timestamp.         */
               if(DateTime1->Minutes == DateTime2->Minutes)
               {
                  /* If the DateTime1 second is equal to DateTime2      */
                  /* second then they must be equal.                    */
                  if(DateTime1->Seconds != DateTime2->Seconds)
                  {
                     /* If the DateTime1 second is greater than         */
                     /* DateTime2 second return greater than otherwise  */
                     /* is must be less than.                           */
                     ret_val = (DateTime1->Seconds > DateTime2->Seconds) ? 1 : -1;
                  }
               }
               else
               {
                  /* If the DateTime1 minute is greater than DateTime2  */
                  /* minute return greater than otherwise it must be    */
                  /* less than.                                         */
                  ret_val = (DateTime1->Minutes > DateTime2->Minutes) ? 1 : -1;
               }
            }
            else
            {
               /* If the DateTime1 hour is greater than DateTime2 hour  */
               /* return greater than otherwise is must be less than.   */
               ret_val = (DateTime1->Hours > DateTime2->Hours) ? 1 : -1;
            }
         }
         else
         {
            /* If the DateTime1 day is greater than DateTime2 day return*/
            /* greater than otherwise it must be less than.             */
            ret_val = (DateTime1->Day > DateTime2->Day) ? 1 : -1;
         }
      }
      else
      {
         /* If the DateTime1 month is greater than DateTime2 month      */
         /* return greater than otherwise it must be less than.         */
         ret_val = (DateTime1->Month > DateTime2->Month) ? 1 : -1;
      }
   }
   else
   {
      /* If the DateTime1 year is greater than DateTime2 year return    */
      /* greater than otherwise it must be less than.                   */
      ret_val = (DateTime1->Year > DateTime2->Year) ? 1 : -1;
   }

   return ret_val;
}

   /* The following function is responsible for comparing two OTS       */
   /* Objects based on their Object ID.                                 */
   /* * NOTE * This function CANNOT return zero since they are unique   */
   /*          and CANNOT be equal.  This allows for the sort to be     */
   /*          stable.                                                  */
static int ObjectIDCompare(OTS_UINT48_Data_t *Number1, OTS_UINT48_Data_t *Number2)
{
   int ret_val = 0;

   /* Check the upper portion first.                                    */
   if(Number1->Upper > Number2->Upper || Number1->Upper < Number2->Upper)
   {
      ret_val = (Number1->Upper > Number2->Upper) ? 1 : -1;
   }
   else
   {
      /* The upper portion must be equal so we need to compare the lower*/
      /* portion                                                        */
      ret_val = (Number1->Lower > Number2->Lower) ? 1 : -1;
   }

   return ret_val;
}

   /* The following function is a helper function to reverse a string.  */
static void ReverseString(Byte_t NameLength, Byte_t *Name)
{
   Byte_t  StartIndex = 0;
   Byte_t  EndIndex   = NameLength - 1;
   Byte_t  temp;

   /* Reverse the string.                                               */
   /* * NOTE * We are not included the NULL terminator in the reversal  */
   /*          since it MUST remain at the end.                         */
   while((StartIndex < EndIndex) && (StartIndex < (OTS_MAXIMUM_OBJECT_NAME_LENGTH - 1)))
   {
      /* Swap the two indexes                                           */
      temp             = Name[EndIndex];
      Name[EndIndex]   = Name[StartIndex];
      Name[StartIndex] = temp;

      /* Update the indexes.                                            */
      StartIndex++;
      EndIndex--;
   }
}

   /* The following function is a helper function to compare an OTS Date*/
   /* Time to make sure it is within the specified range.               */
static int CompareDateTimeRange(OTS_Date_Time_Data_t *Current, OTS_Date_Time_Data_t *Lower, OTS_Date_Time_Data_t *Upper)
{
   int       ret_val          = 0;
   Boolean_t WithinLowerBound = FALSE;
   Boolean_t WithinUpperBound = FALSE;

   /* Check the year to make sure it falls within the range.            */
   if((Current->Year >= Lower->Year) && (Current->Year <= Upper->Year))
   {
      /* Check the lower bound.                                         */

      /* If the year is equal to the lower bound we need to check the   */
      /* month.                                                         */
      if(Current->Year == Lower->Year)
      {
         /* Check the lower bound month to make sure it falls within the*/
         /* range.                                                      */
         if(Current->Month >= Lower->Month)
         {
            /* If the month is equal to the lower bound we need to check*/
            /* the day.                                                 */
            if(Current->Month == Lower->Month)
            {
               /* Check the lower bound day to make sure it falls       */
               /* between the range.                                    */
               if(Current->Day >= Lower->Day)
               {
                  /* If the day is equal to the lower bound we need to  */
                  /* check the time.                                    */
                  if(Current->Day == Lower->Day)
                  {
                     /* Check the lower bound hour to make sure it falls*/
                     /* between the range.                              */
                     if(Current->Hours >= Lower->Hours)
                     {
                        /* If the hour is equal to the lower bound we   */
                        /* need to check the minute.                    */
                        if(Current->Hours == Lower->Hours)
                        {
                            /* Check the lower bound minute to make sure*/
                            /* it falls between the range.              */
                           if(Current->Minutes >= Lower->Minutes)
                           {
                              /* If the minute is equal to the lower    */
                              /* bound we need to check the second.     */
                              if(Current->Minutes == Lower->Minutes)
                              {
                                 /* Check the lower bound second to make*/
                                 /* sure it falls between the range.    */
                                 if(Current->Seconds >= Lower->Seconds)
                                 {
                                    /* The timestamp must be greater    */
                                    /* than or equal to the lower bound.*/
                                    WithinLowerBound = TRUE;
                                 }
                              }
                              else
                              {
                                 /* Otherwise the timestamp must be     */
                                 /* greater than or equal to the lower  */
                                 /* bound.                              */
                                 WithinLowerBound = TRUE;
                              }
                           }
                        }
                        else
                        {
                           /* Otherwise the timestamp must be greater   */
                           /* than or equal to the lower bound.         */
                           WithinLowerBound = TRUE;
                        }
                     }
                  }
                  else
                  {
                     /* Otherwise the timestamp must be greater than or */
                     /* equal to the lower bound.                       */
                     WithinLowerBound = TRUE;
                  }
               }
            }
            else
            {
               /* Otherwise the timestamp must be greater than or equal */
               /* to the lower bound.                                   */
               WithinLowerBound = TRUE;
            }
         }

      }
      else{
         /* Otherwise the timestamp must be greater than or equal to the*/
         /* lower bound.                                                */
         WithinLowerBound = TRUE;
      }

      /* Check the upper bound.                                         */

      /* If the year is equal to the upper bound we need to check the   */
      /* month.                                                         */
      if(Current->Year == Upper->Year)
      {
         /* Check the upper bound month to make sure it falls within the*/
         /* range.                                                      */
         if(Current->Month <= Upper->Month)
         {
            /* If the month is equal to the upper bound we need to check*/
            /* the day.                                                 */
            if(Current->Month == Upper->Month)
            {
               /* Check the upper bound day to make sure it falls       */
               /* between the range.                                    */
               if(Current->Day <= Upper->Day)
               {
                  /* If the day is equal to the upper bound we need to  */
                  /* check the time.                                    */
                  if(Current->Day == Upper->Day)
                  {
                     /* Check the uppper bound hour to make sure it     */
                     /* falls between the range.                        */
                     if(Current->Hours <= Upper->Hours)
                     {
                        /* If the hour is equal to the upper bound we   */
                        /* need to check the minute.                    */
                        if(Current->Hours == Upper->Hours)
                        {
                            /* Check the upper bound minute to make sure*/
                            /* it falls between the range.              */
                           if(Current->Minutes <= Upper->Minutes)
                           {
                              /* If the minute is equal to the upper    */
                              /* bound we need to check the second.     */
                              if(Current->Minutes == Upper->Minutes)
                              {
                                 /* Check the upper bound second to make*/
                                 /* sure it falls between the range.    */
                                 if(Current->Seconds <= Upper->Seconds)
                                 {
                                    /* The timestamp must be less than  */
                                    /* or equal to the upper bound.     */
                                    WithinUpperBound = TRUE;
                                 }
                              }
                              else
                              {
                                 /* Otherwise the timestamp must be less*/
                                 /* than or equal to the upper bound.   */
                                 WithinUpperBound = TRUE;
                              }
                           }
                        }
                        else
                        {
                           /* Otherwise the timestamp must be less than */
                           /* or equal to the upper bound.              */
                           WithinUpperBound = TRUE;
                        }
                     }
                  }
                  else
                  {
                     /* Otherwise the timestamp must be less than or    */
                     /* equal to the upper bound.                       */
                     WithinUpperBound = TRUE;
                  }
               }
            }
            else
            {
               /* Otherwise the timestamp must be less than or equal to */
               /* the upper bound.                                      */
               WithinUpperBound = TRUE;
            }
         }

      }
      else{
         /* Otherwise the timestamp must be less than or equal to the   */
         /* upper bound.                                                */
         WithinUpperBound = TRUE;
      }

      /* Determine if the OTS Date Time is within the lower and upper   */
      /* OTS Date Time range.                                           */
      if(!((WithinLowerBound) && (WithinUpperBound)))
      {
         /* Return that we did not match.                               */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* Return that we did not match.                                  */
      ret_val = FUNCTION_ERROR;
   }

   return ret_val;
}

   /* The following function is responsible for copying OTS Object data */
   /* to the OTS Object Metadata structure.                             */
   /* * NOTE * This function SHOULD ONLY BE USED BY THE                 */
   /*          OTS_EVENTCALLBACK().                                     */
static void ConvertObjectDataToObjectMetadata(OTS_Object_Metadata_Type_t Type, OTS_Object_Data_t *CurrentObject, OTS_Object_Metadata_Data_t *Data)
{
   if((CurrentObject) && (Data))
   {
      /* Copy the OTS Object Data based on the OTS Object Metadata type.*/
      switch(Type)
      {
         case omtObjectName:
            /* Simply copy the pointer an the length of the OTS Object  */
            /* Name.                                                    */
            Data->Name          = CurrentObject->Name;

            /* Make sure the Object Name is not the New File name for an*/
            /* OTS Object that has been created via the OACP Create     */
            /* Procedure.  If it is we need to return an empty string   */
            /* since the specification says it MUST be an empty string. */
            /* * NOTE * We use the name so that we can create a temp    */
            /*          file on the file system to hold data if the OTP */
            /*          Client writes the new OTS Object contents before*/
            /*          giving it an OTS Object Name.                   */
            if(!strncmp((char *)Data->Name.Buffer, OTP_DEFAULT_NEW_FILE_NAME, BTPS_StringLength(OTP_DEFAULT_NEW_FILE_NAME)))
            {
               /* We will simply set the length of the buffer for the   */
               /* name to zero.                                         */
               Data->Name.Buffer_Length = 0;
            }
            break;
         case omtObjectType:
            Data->Type          = CurrentObject->Type;
            break;
         case omtObjectSize:
            Data->Size          = CurrentObject->Size;
            break;
         case omtObjectFirstCreated:
            Data->First_Created = CurrentObject->First_Created;
            break;
         case omtObjectLastModified:
            Data->Last_Modified = CurrentObject->Last_Modified;
            break;
         case omtObjectID:
            Data->ID            = CurrentObject->ID;
            break;
         case omtObjectProperties:
            Data->Properties    = CurrentObject->Properties;
            break;
         default:
            printf("\r\nInvalid OTS Object Metadata type.\r\n");
            break;
      }
   }
}

   /* The following function is responsible for copying OTS Object      */
   /* Metadata to the OTS Object data structure.                        */
   /* * NOTE * This function SHOULD ONLY BE USED BY THE                 */
   /*          OTS_EVENTCALLBACK().                                     */
static Byte_t StoreObjectMetadataForCurrentObject(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Data, OTS_Object_Data_t *CurrentObject)
{
   Byte_t        ret_val = OTS_ERROR_CODE_SUCCESS;
   unsigned int  Index;
   char          NewName[OTS_MAXIMUM_OBJECT_NAME_LENGTH+1];

   if((Data) && (CurrentObject))
   {
      /* Copy the OTS Object Metadata based on the OTS Object Metadata  */
      /* type.                                                          */
      switch(Type)
      {
         case omtObjectName:
            /* If this is not the OTS Directory Listing Object.         */
            if(CurrentObject->ID.Lower != 0)
            {
               /* We will not allow the OTP Client to write the         */
               /* OTP_DEFAULT_NEW_FILE_NAME as the Object Name.         */
               if(strncmp((char *)Data->Name.Buffer, OTP_DEFAULT_NEW_FILE_NAME, BTPS_StringLength(OTP_DEFAULT_NEW_FILE_NAME)) != 0)
               {
                  /* We need to make sure that the new name does not    */
                  /* exist on the OTP Server.                           */
                  for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
                  {
                     /* Make sure the OTS Object is valid.              */
                     if(ServerData.Object_List[Index].Valid)
                     {
                        /* Compare the OTS Object Name.                 */
                        if(!strncmp((char *)ServerData.Object_List[Index].Data.Name.Buffer, (char *)Data->Name.Buffer, ServerData.Object_List[Index].Data.Name.Buffer_Length))
                        {
                           /* Reject the write request.                 */
                           ret_val = OTS_ERROR_CODE_OBJECT_NAME_ALREADY_EXISTS;
                        }
                     }
                  }
               }
               else
               {
                  printf("\r\nCannot use the default the new file name: \"%s\".\r\n", OTP_DEFAULT_NEW_FILE_NAME);

                  /* Reject the write request.                          */
                  ret_val = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
               }
            }
            else
            {
               printf("\r\nCannot rename the OTS Directory Listing Object.\r\n");

               /* Reject the write request.                             */
               ret_val = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
            }

            /* If an error did not occur the OTS Object Name may be     */
            /* written.                                                 */
            if(ret_val == OTS_ERROR_CODE_SUCCESS)
            {
               /* If the current size is not zero, then the file exists */
               /* on the file system and we need to rename it.          */
               /* * NOTE * This may be zero if the OTP Client previously*/
               /*          used the OACP Create Procedure and the OTS   */
               /*          Object exists on the OTP Server, but does not*/
               /*          have a file on the file system.  This may    */
               /*          occur if the OTP Client has not used the OACP*/
               /*          Write Procedure following the OACP Create    */
               /*          Procedure.                                   */
               if(CurrentObject->Size.Current_Size != 0)
               {
                  /* Copy the string.                                   */
                  BTPS_MemCopy(NewName, Data->Name.Buffer, Data->Name.Buffer_Length);

                  /* Insert the NULL terminator.                        */
                  NewName[Data->Name.Buffer_Length] = '\0';

                  /* Simply call function to rename the file on the file*/
                  /* system.                                            */
                  if(!RenameFileServer((char *)CurrentObject->Name.Buffer, NewName))
                  {
                     /* Inform the user that the file has been renamed. */
                     printf("\r\nTemporary file has been given a name:\r\n");
                     printf("   Old Name:  \"%s\".\r\n", (char *)CurrentObject->Name.Buffer);
                     printf("   New Name:  \"%s\".\r\n", NewName);

                     /* Update the OTS Object Name.                     */
                     /* * NOTE * The current object's buffer MUST point */
                     /*          to the buffer that is used to hold its */
                     /*          name.                                  */
                     BTPS_MemCopy(CurrentObject->Name.Buffer, Data->Name.Buffer, Data->Name.Buffer_Length);
                     CurrentObject->Name.Buffer_Length = Data->Name.Buffer_Length;

                     /* Insert the null terminator so we can display it */
                     /* later as a c-string.                            */
                     CurrentObject->Name.Buffer[Data->Name.Buffer_Length] = '\0';
                  }
                  else
                  {
                    /* An error will already be printed by              */
                    /* RenameFileServer().                              */

                     /* Reject the write request.                       */
                     ret_val = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
                  }
               }
               else
               {
                  /* Update the OTS Object Name.                        */
                  /* * NOTE * The current object's buffer MUST point to */
                  /*          the buffer that is used to hold its name. */
                  BTPS_MemCopy(CurrentObject->Name.Buffer, Data->Name.Buffer, Data->Name.Buffer_Length);
                  CurrentObject->Name.Buffer_Length = Data->Name.Buffer_Length;

                  /* Insert the null terminator so we can display it    */
                  /* later as a c-string.                               */
                  CurrentObject->Name.Buffer[Data->Name.Buffer_Length] = '\0';
               }
            }
            break;
         case omtObjectType:
            /* We should not have received the event for this request.  */
            printf("\r\nObject Type CANNOT be written.\r\n");
            break;
         case omtObjectSize:
            /* We should not have received the event for this request.  */
            printf("\r\nObject Size CANNOT be written.\r\n");
            break;
         case omtObjectFirstCreated:
            if(OTS_DATE_TIME_VALID(Data->First_Created))
            {
               /* Simply Store the Object First Created Date Time.      */
               CurrentObject->First_Created = Data->First_Created;
            }
            else
            {
               printf("\r\nThe current object's First Created date time is invalid.\r\n");
               ret_val = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
            }
            break;
         case omtObjectLastModified:
            if(OTS_DATE_TIME_VALID(Data->Last_Modified))
            {
               /* Simply Store the Object Last Modified Date Time.      */
               CurrentObject->Last_Modified = Data->Last_Modified;
            }
            else
            {
               printf("\r\nThe current object's Last Modified date time is invalid.\r\n");
               ret_val = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
            }
            break;
         case omtObjectID:
            /* We should not have received the event for this request.  */
            printf("\r\nObject ID CANNOT be written.\r\n");
            break;
         case omtObjectProperties:
            /* Simply Store the Object Properties.                      */
            CurrentObject->Properties = Data->Properties;

            break;
         default:
            /* This SHOULD not occur since we should not have received  */
            /* the event.                                               */
            break;
      }

      /* * NOTE * We could update the current object's Last Modifed date*/
      /*          time here if the current object has been modified.  We*/
      /*          could also send an indication of the Object Changed   */
      /*          Characteristic to other OTP Clients, however that is  */
      /*          beyond the scope for this sample application.         */
   }

   return(ret_val);
}

   /* The following function will read an OTS Characteristic.  This     */
   /* function may be called by the OTP Server or OTP Client.  If called*/
   /* by the OTP Client then a GATT read/read long request will be      */
   /* dispatched.                                                       */
   /* * NOTE * This function requires that the caller correctly set the */
   /*          RequestData.                                             */
static int ReadOTSCharacteristic(void)
{
   int                         ret_val = 0;
   DeviceInfo_t               *DeviceInfo;
   OTS_Object_Data_t          *ObjectDataPtr;
   OTS_Object_Metadata_Data_t  Data;

   /* If we are the OTP Server.                                         */
   if(OTSInstanceID)
   {
      /* Detemermine the OTS request based on the attribute handle type.*/
      switch(RequestData.AttributeHandleType)
      {
         case ahtOTSFeature:
            /* Simply call the internal function to display the OTS     */
            /* Feature.                                                 */
            DisplayOTSFeatureData(&ServerData.OTS_Feature);
            break;
         case ahtObjectMetadata:
            /* Make sure that we are connected and a remote device is   */
            /* selected.                                                */
            if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
            {
               /* Get the device info for the connected device.         */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure the current object is selected.          */
                  if(DeviceInfo->FilteredObjectList.Current_Object)
                  {
                     /* Store a pointer to the current object's data.   */
                     ObjectDataPtr = DeviceInfo->FilteredObjectList.Current_Object->ObjectDataPtr;

                     /* Simply copy the OTS Object data from the current*/
                     /* object to the OTS Object Metadata format.       */
                     /* * NOTE * The Object Metadata type MUST have been*/
                     /*          set in the RequestData.                */
                     ConvertObjectDataToObjectMetadata(RequestData.MetadataType, ObjectDataPtr, &Data);

                     /* Display the OTS Object Metadata.                */
                     printf("\r\nObject Metadata:\r\n");
                     printf("   Request Type:  ");
                     DisplayObjectMetadataType(RequestData.MetadataType);
                     DisplayObjectMetadata(RequestData.MetadataType, &Data);
                  }
                  else
                     printf("\r\nCurrent object is not selected.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nNo connection to an OTP Client.\r\n");
            break;
            break;
         case ahtObjectActionControlPoint:
         case ahtObjectListControlPoint:
         case ahtObjectChanged:
            /* Intentional fall-through since these OTS Characteristics */
            /* CANNOT be read.                                          */
            printf("\r\nOTS Characteristic CANNOT be read.\r\n");
            break;
         case ahtObjectActionControlPoint_CCCD:
         case ahtObjectListControlPoint_CCCD:
         case ahtObjectChanged_CCCD:
            /* Intentional fall-through since we can handle these cases */
            /* similarly.                                               */

            /* Make sure that we are connected and a remote device is   */
            /* selected.                                                */
            if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
            {
               /* Get the device info for the connected device.         */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Display the OTS Characteristic based on the        */
                  /* attribute handle type.                             */
                  switch(RequestData.AttributeHandleType)
                  {
                     case ahtObjectActionControlPoint_CCCD:
                        printf("\r\nObject Action Control Point CCCD:\r\n");
                        printf("   Value: 0x%04X\r\n", DeviceInfo->OTSServerInfo.Object_Action_Control_Point_Configuration);
                        break;
                     case ahtObjectListControlPoint_CCCD:
                        printf("\r\nObject List Control Point CCCD:\r\n");
                        printf("   Value: 0x%04X\r\n", DeviceInfo->OTSServerInfo.Object_List_Control_Point_Configuration);
                        break;
                     case ahtObjectChanged_CCCD:
                        printf("\r\nObject Changed CCCD:\r\n");
                        printf("   Value: 0x%04X\r\n", DeviceInfo->OTSServerInfo.Object_Changed_Configuration);
                        break;
                     default:
                        /* Can't occur.                                 */
                        break;
                  }
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nNo connection to the OTP Client.\r\n");
            break;
         case ahtObjectListFilter_1:
         case ahtObjectListFilter_2:
         case ahtObjectListFilter_3:
            /* Intentional fall-through since we can handle these cases */
            /* similarly.                                               */

            /* Make sure that we are connected and a remote device is   */
            /* selected.                                                */
            if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
            {
               /* Get the device info for the connected device.         */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Simply call the internal function to display the   */
                  /* OTS Object List Filter.                            */
                  /* * NOTE * The first Object List Filter instance is  */
                  /*          zero based so we will simply subtract the */
                  /*          value from the attribute handle type to   */
                  /*          display the correct instance.             */
                  DisplayObjectListFilterData(&(DeviceInfo->FilteredObjectList.List_Filter_Data[(RequestData.AttributeHandleType - ahtObjectListFilter_1)]));
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nNo connection to the OTP Client.\r\n");
            break;
         default:
            printf("\r\nInvalid attribute handle type.\r\n");
            break;
      }
   }
   else
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         /* * NOTE * ConnectionBD_ADDR should be valid if ConnectionID  */
         /*          is.                                                */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Determine the attribute handle based on the attribute */
               /* handle type.                                          */
               switch(RequestData.AttributeHandleType)
               {
                  case ahtOTSFeature:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.OTS_Feature;
                     break;
                  case ahtObjectMetadata:
                     /* We need to set the attribute handle depending on*/
                     /* the OTS Object Metadata type.                   */
                     switch(RequestData.MetadataType)
                     {
                        case omtObjectName:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Name;

                           /* If the user has not specified to issue a  */
                           /* GATT Read Long Value request.             */
                           if(!(RequestData.UseReadLong))
                           {
                              /* Reset the request data buffer to zero  */
                              /* for the first request.                 */
                              /* * NOTE * We will update this value when*/
                              /*          we receive the GATT Read Value*/
                              /*          response and subsequent GATT  */
                              /*          Read Long Value responses that*/
                              /*          may be received.              */
                              RequestData.BufferLength = 0;
                           }
                           break;
                        case omtObjectType:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Type;
                           break;
                        case omtObjectSize:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Size;
                           break;
                        case omtObjectFirstCreated:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_First_Created;
                           break;
                        case omtObjectLastModified:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Last_Modified;
                           break;
                        case omtObjectID:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_ID;
                           break;
                        case omtObjectProperties:
                           RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Properties;
                           break;
                        default:
                           printf("Invalid Object Metadata type.\r\n");
                           break;
                     }
                     break;
                  case ahtObjectActionControlPoint:
                  case ahtObjectListControlPoint:
                  case ahtObjectChanged:
                     /* Intentional fall-through since these OTS        */
                     /* Characteristics CANNOT be read.                 */
                     printf("\r\nOTS Characteristic CANNOT be read.\r\n");
                     ret_val = FUNCTION_ERROR;
                     break;
                  case ahtObjectActionControlPoint_CCCD:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Action_Control_Point_CCCD;
                     break;
                  case ahtObjectListControlPoint_CCCD:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_List_Control_Point_CCCD;
                     break;
                  case ahtObjectChanged_CCCD:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Changed_CCCD;
                     break;
                  case ahtObjectListFilter_1:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_List_Filter[lfiOne];

                     /* If the user has not specified to issue a GATT   */
                     /* Read Long Value request.                        */
                     if(!(RequestData.UseReadLong))
                     {
                        /* Reset the request data buffer to zero for the*/
                        /* first request.                               */
                        /* * NOTE * We will update this value when we   */
                        /*          receive the GATT Read Value response*/
                        /*          and subsequent GATT Read Long Value */
                        /*          responses that may be received.     */
                        RequestData.BufferLength = 0;
                     }
                     break;
                  case ahtObjectListFilter_2:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_List_Filter[lfiTwo];

                     /* If the user has not specified to issue a GATT   */
                     /* Read Long Value request.                        */
                     if(!(RequestData.UseReadLong))
                     {
                        /* Reset the request data buffer to zero for the*/
                        /* first request.                               */
                        /* * NOTE * We will update this value when we   */
                        /*          receive the GATT Read Value response*/
                        /*          and subsequent GATT Read Long Value */
                        /*          responses that may be received.     */
                        RequestData.BufferLength = 0;
                     }
                     break;
                  case ahtObjectListFilter_3:
                     RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_List_Filter[lfiThree];

                     /* If the user has not specified to issue a GATT   */
                     /* Read Long Value request.                        */
                     if(!(RequestData.UseReadLong))
                     {
                        /* Reset the request data buffer to zero for the*/
                        /* first request.                               */
                        /* * NOTE * This will be incremented after we   */
                        /*          receive the first response so we can*/
                        /*          handle additional GATT Read Long    */
                        /*          Value requests that may need to be  */
                        /*          issued if we cannot receive the OTS */
                        /*          Object List Filter in a GATT Read   */
                        /*          response.                           */
                        RequestData.BufferLength = 0;
                     }
                     break;
                  default:
                     printf("\r\nInvalid attribute handle type.\r\n");
                     break;
               }

               /* Make sure an error has not occured.                   */
               if(!ret_val)
               {
                  /* Check to make sure the attribute handle is valid.  */
                  if(RequestData.AttributeHandle)
                  {
                     /* If this a GATT Read Value request.              */
                     if(!(RequestData.UseReadLong))
                     {
                        /* Send the GATT Read Value request.            */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Read_Value_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Read Value Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n",     ret_val);
                           printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                        }
                        else
                           DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                     }
                     else
                     {
                        /* Send the GATT Read Long Value request.       */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Read_Long_Value_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, RequestData.BufferLength, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Read Long Value Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n",     ret_val);
                           printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                           printf("   Offset:            %u\r\n", RequestData.BufferLength);
                        }
                        else
                           DisplayFunctionError("GATT_Read_Long_Value_Request", ret_val);
                     }

                     /* Simply return success to the caller.            */
                     ret_val = 0;
                  }
                  else
                     printf("\r\nInvalid attribute handle.\r\n");
               }
            }
            else
               printf("\r\nOTS Service discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information for the OTP Server.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Server.\r\n");
   }

   return(ret_val);
}

   /* The following function will write an OTS Characteristic.  This    */
   /* function may be called by the OTP Server or OTP Client.  If called*/
   /* by the OTP Client then a GATT write/prepare write request will be */
   /* dispatched.                                                       */
   /* * NOTE * This function requires that the caller correctly set the */
   /*          RequestData.                                             */
static int WriteOTSCharacteristic(void)
{
   int                        ret_val = 0;
   DeviceInfo_t              *DeviceInfo;
   OTS_Object_Data_t         *ObjectDataPtr;
   OTS_Object_Changed_Data_t  ObjectChangedData;

   /* If we are the OTP Server.                                         */
   if(OTSInstanceID)
   {
      /* Detemermine the OTS request based on the attribute handle type.*/
      switch(RequestData.AttributeHandleType)
      {
         case ahtOTSFeature:
            printf("\r\nOTS Characteristic CANNOT be written.\r\n");
            break;
         case ahtObjectMetadata:
            /* Make sure that we are connected and a remote device is   */
            /* selected.                                                */
            if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
            {
               /* Get the device info for the connected device.         */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure the current object is selected.          */
                  if(DeviceInfo->FilteredObjectList.Current_Object)
                  {
                     /* Store a pointer to the current object's         */
                     /* metadata.                                       */
                     ObjectDataPtr = DeviceInfo->FilteredObjectList.Current_Object->ObjectDataPtr;

                     /* Initialize the Object Changed data.             */
                     /* * NOTE * Flags for source of change already set */
                     /*          to OTP Server (0).                     */
                     BTPS_MemInitialize(&ObjectChangedData, 0, OTS_OBJECT_CHANGED_DATA_SIZE);

                     /* We need to determine the OTS Object Metadata    */
                     /* type.                                           */
                     switch(RequestData.MetadataType)
                     {
                        case omtObjectName:
                           /* Make sure the Object Name size is valid.  */
                           if(RequestData.Data.Metadata.Name.Buffer_Length <= OTS_MAXIMUM_OBJECT_NAME_LENGTH)
                           {
                              /* Store the Object Name.                 */
                              BTPS_MemCopy(ObjectDataPtr->Name.Buffer, RequestData.Data.Metadata.Name.Buffer, RequestData.Data.Metadata.Name.Buffer_Length);
                              ObjectDataPtr->Name.Buffer_Length = RequestData.Data.Metadata.Name.Buffer_Length;

                              /* Insert the NULL terminator.            */
                              ObjectDataPtr->Name.Buffer[ObjectDataPtr->Name.Buffer_Length] = '\0';

                              /* Flag that a changed has occured to the */
                              /* Object Metadata.                       */
                              ObjectChangedData.Flags |= OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED;
                           }
                           else
                              printf("\r\nObject Name length is greater than supported.\r\n");
                           break;
                        case omtObjectType:
                           printf("\r\nObject Type CANNOT be written.\r\n");
                           break;
                        case omtObjectSize:
                           printf("\r\nObject Size CANNOT be written.\r\n");
                           break;
                        case omtObjectFirstCreated:
                           /* Make sure the Object First Created date   */
                           /* time is valid.                            */
                           if(OTS_DATE_TIME_VALID(RequestData.Data.Metadata.First_Created))
                           {
                              /* Store the Object First Created date.   */
                              ObjectDataPtr->First_Created = RequestData.Data.Metadata.First_Created;\

                              /* Flag that a changed has occured to the */
                              /* Object Metadata.                       */
                              ObjectChangedData.Flags |= OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED;
                           }
                           else
                              printf("Invalid OTS Date Time.\r\n");
                           break;
                        case omtObjectLastModified:
                           /* Make sure the Object Last Modified date   */
                           /* time is valid.                            */
                           if(OTS_DATE_TIME_VALID(RequestData.Data.Metadata.Last_Modified))
                           {
                              /* Store the Object Last Modified date.   */
                              ObjectDataPtr->Last_Modified = RequestData.Data.Metadata.Last_Modified;

                              /* Flag that a changed has occured to the */
                              /* Object Metadata.                       */
                              ObjectChangedData.Flags |= OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED;
                           }
                           else
                              printf("Invalid OTS Date Time.\r\n");
                           break;
                        case omtObjectID:
                           printf("\r\nObject ID CANNOT be written.\r\n");
                           break;
                        case omtObjectProperties:
                           /* Store the Object Properties.              */
                           ObjectDataPtr->Properties = RequestData.Data.Metadata.Properties;

                           /* Flag that a changed has occured to the    */
                           /* Object Metadata.                          */
                           ObjectChangedData.Flags |= OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED;
                           break;
                        default:
                           printf("Invalid Object Metadata type.\r\n");
                           break;
                     }

                     /* Check if a changed occur to the current object's*/
                     /* metadata.                                       */
                     if(ObjectChangedData.Flags)
                     {
                        /* Format the rest of the Object Changed data.  */
                        ObjectChangedData.Object_ID = ObjectDataPtr->ID;

                        /* Simply call the internal function to send the*/
                        /* indication.                                  */
                        IndicateObjectChanged(&ObjectChangedData);
                     }
                  }
                  else
                     printf("\r\nCurrent object is not selected.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nNo connection to an OTP Client.\r\n");
            break;
            break;
         case ahtObjectActionControlPoint:
         case ahtObjectListControlPoint:
         case ahtObjectChanged:
         case ahtObjectActionControlPoint_CCCD:
         case ahtObjectListControlPoint_CCCD:
         case ahtObjectChanged_CCCD:
         case ahtServiceChanged_CCCD:
         case ahtObjectListFilter_1:
         case ahtObjectListFilter_2:
         case ahtObjectListFilter_3:
            /* Intentional fall-through since these OTS Characteristics */
            /* CANNOT be written by the OTP Server.                     */
            /* * NOTE * These are all used on a Per-Client basis so it  */
            /*          doesn't make sense for the OTP Server to update */
            /*          them.                                           */
            printf("\r\nOTS Characteristic CANNOT be written by the OTP Server.\r\n");
            break;
         default:
            printf("\r\nInvalid attribute handle type.\r\n");
            break;
      }
   }
   else
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery since we   */
            /* need the attribute handle to make the request.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Make sure the attribute handle is set to zero in case */
               /* an error occurs.                                      */
               RequestData.AttributeHandle = 0;

               /* Determine the attribute handle based on the attribute */
               /* handle type.                                          */
               switch(RequestData.AttributeHandleType)
               {
                  case ahtOTSFeature:
                     printf("\r\nOTS Characteristic CANNOT be written.\r\n");
                     break;
                  case ahtObjectMetadata:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendObjectMetadataWriteRequest(DeviceInfo);
                     break;
                  case ahtObjectActionControlPoint:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendOACPWriteRequest(DeviceInfo);
                     break;
                  case ahtObjectListControlPoint:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendOLCPWriteRequest(DeviceInfo);
                     break;
                  case ahtObjectChanged:
                     printf("\r\nOTS Characteristic CANNOT be written.\r\n");
                     break;
                  case ahtObjectActionControlPoint_CCCD:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendCCCDWriteRequest(DeviceInfo, DeviceInfo->OTSClientInfo.Object_Action_Control_Point_CCCD);
                     break;
                  case ahtObjectListControlPoint_CCCD:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendCCCDWriteRequest(DeviceInfo, DeviceInfo->OTSClientInfo.Object_List_Control_Point_CCCD);
                     break;
                  case ahtObjectChanged_CCCD:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendCCCDWriteRequest(DeviceInfo, DeviceInfo->OTSClientInfo.Object_Changed_CCCD);
                     break;
                  case ahtServiceChanged_CCCD:
                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     ret_val = SendCCCDWriteRequest(DeviceInfo, DeviceInfo->GATTClientInfo.ServiceChangedCCCD);
                     break;
                  case ahtObjectListFilter_1:
                  case ahtObjectListFilter_2:
                  case ahtObjectListFilter_3:
                     /* Intentional fall-through since we can handle    */
                     /* these cases similarly.                          */

                     /* Simply call the internal function to send the   */
                     /* write request.                                  */
                     /* * NOTE * We will subtract ahtObjectListFilter_1 */
                     /*          from the attribute handle type to get  */
                     /*          the correct attribute handle for the   */
                     /*          Object List Filter instance.           */
                     ret_val = SendObjectListFilterWriteRequest(DeviceInfo, DeviceInfo->OTSClientInfo.Object_List_Filter[RequestData.AttributeHandleType - ahtObjectListFilter_1]);
                     break;
                  default:
                     printf("\r\nInvalid attribute handle type.\r\n");
                     break;
               }
            }
            else
               printf("\r\nOTS Service discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information for the OTP Server.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Server.\r\n");
   }

   return(ret_val);
}

   /* The following function is a helper function for                   */
   /* WriteOTSCharacteristic() to send a write request for the Object   */
   /* Metadata.                                                         */
static int SendObjectMetadataWriteRequest(DeviceInfo_t *DeviceInfo)
{
   int     ret_val      = 0;
   Word_t  BufferLength = 0;
   Byte_t *Buffer;
   Word_t  MTU;

   /* We need to set the attribute handle depending on the OTS Object   */
   /* Metadata type.                                                    */
   switch(RequestData.MetadataType)
   {
      case omtObjectName:
         if((RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Name) == 0)
            printf("\r\nInvalid attribute handle.\r\n");
         break;
      case omtObjectType:
         printf("\r\nObject Type CANNOT be written.\r\n");
         break;
      case omtObjectSize:
         printf("\r\nObject Size CANNOT be written.\r\n");
         break;
      case omtObjectFirstCreated:
         if((RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_First_Created) == 0)
            printf("\r\nInvalid attribute handle.\r\n");
         break;
      case omtObjectLastModified:
         if((RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Last_Modified) == 0)
            printf("\r\nInvalid attribute handle.\r\n");
         break;
      case omtObjectID:
         printf("\r\nObject ID CANNOT be written.\r\n");
         break;
      case omtObjectProperties:
         if((RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Properties) == 0)
            printf("\r\nInvalid attribute handle.\r\n");
         break;
      default:
         printf("Invalid Object Metadata type.\r\n");
         break;
   }

   /* Check to make sure the attribute handle is valid.                 */
   if(RequestData.AttributeHandle)
   {
      /* We CANNOT use the OTS_Format_Object_Metadata() function for the*/
      /* OTS Object Name since it is already formatted.                 */
      if(RequestData.MetadataType == omtObjectName)
      {
         /* Store the OTS Object Name.                                  */
         Buffer       = RequestData.Data.Metadata.Name.Buffer;
         BufferLength = RequestData.Data.Metadata.Name.Buffer_Length;

         /* Query the GATT MTU to determine whether we can fit the OTS  */
         /* Object Name in GATT Write request or if we need to use one  */
         /* or more GATT Prepare Write requests followed by a GATT      */
         /* Execute Write request.                                      */
         if(!(ret_val = GATT_Query_Connection_MTU(BluetoothStackID, DeviceInfo->ConnectionID, &MTU)))
         {
            /* If the Object Name fits in the (ATT_MTU-3) size then it  */
            /* can fit in a GATT Write request.                         */
            if(BufferLength <= (MTU-3))
            {
               /* Send the GATT Write request.                          */
               /* * NOTE * We will not save the transactionID returned  */
               /*          by this function, which we could use to      */
               /*          cancel the request.                          */
               if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
               {
                  printf("\r\nGATT Write Request sent:\r\n");
                  printf("   TransactionID:     %d\r\n",     ret_val);
                  printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
               }
               else
                  DisplayFunctionError("GATT_Read_Value_Request", ret_val);
            }
            else
            {
               /* We need to set the request data before we issue the   */
               /* first GATT Prepare Write request so we can handle the */
               /* response and send more requests if necessary.         */
               /* * NOTE * We are copying the data since we will lose it*/
               /*          after the request is sent.                   */
               BTPS_MemCopy(RequestData.Buffer, Buffer, BufferLength);
               RequestData.BufferLength = BufferLength;
               RequestData.BufferOffset = 0;

               /* Send the GATT Prepare Write request.                  */
               /* * NOTE * We will not save the transactionID returned  */
               /*          by this function, which we could use to      */
               /*          cancel the request.                          */
               if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, RequestData.BufferOffset, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
               {
                  printf("\r\nGATT Prepare Write Request sent:\r\n");
                  printf("   TransactionID:     %d\r\n",     ret_val);
                  printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                  printf("   Offset:            %u\r\n",     RequestData.BufferOffset);
               }
               else
                  DisplayFunctionError("GATT_Read_Value_Request", ret_val);
            }
         }
         else
            DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
      }
      else
      {
         /* Determine the size of the buffer to send the OACP request.  */
         /* * NOTE * This function will return the size if the          */
         /*          BufferLength is set to zero.                       */
         if((ret_val = OTS_Format_Object_Metadata(RequestData.MetadataType, &(RequestData.Data.Metadata), 0, NULL)) > 0)
         {
            /* Store the buffer length.                                 */
            BufferLength = (Word_t)ret_val;

            /* Store a pointer to the request buffer.                   */
            /* * NOTE * We are guaranteed that this buffer will hold the*/
            /*          data for all requests.                          */
            Buffer = RequestData.Buffer;

            /* Format the buffer.                                       */
            if((ret_val = OTS_Format_Object_Metadata(RequestData.MetadataType, &(RequestData.Data.Metadata), BufferLength, Buffer)) == 0)
            {
               /* Send the GATT Write request.                          */
               /* * NOTE * We will not save the transactionID returned  */
               /*          by this function, which we could use to      */
               /*          cancel the request.                          */
               if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
               {
                  printf("\r\nGATT Write Request sent:\r\n");
                  printf("   TransactionID:     %d\r\n",     ret_val);
                  printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
               }
               else
                  DisplayFunctionError("GATT_Read_Value_Request", ret_val);
            }
            else
               DisplayFunctionError("OTS_Format_Object_Metadata", ret_val);
         }
         else
            DisplayFunctionError("OTS_Format_Object_Metadata", ret_val);
      }

      /* Simply return success to the caller.                           */
      ret_val = 0;
   }

   return(ret_val);
}

   /* The following function is a helper function for                   */
   /* WriteOTSCharacteristic() to send a write request for the Object   */
   /* Action Control Point.                                             */
   /* ** NOTE ** The DecodeDisplayOACPResponseData() function will      */
   /*            peform tasks related to the Object Transfer Channel for*/
   /*            an OACP Read/Write Procedure response.                 */
static int SendOACPWriteRequest(DeviceInfo_t *DeviceInfo)
{
   int                      ret_val      = 0;
   Word_t                   BufferLength = 0;
   Byte_t                  *Buffer;
   OTS_OACP_Request_Data_t *OACP_RequestData;
   Word_t                   MTU;
   char                     FilePath[OTP_MAXIMUM_COMBINED_PATH_LENGTH];
   OTP_Channel_Data_t      *ChannelData;

   /* Store the attribute handle.                                       */
   RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_Action_Control_Point;

   /* Check to make sure the attribute handle is valid.                 */
   if(RequestData.AttributeHandle)
   {
      /* Store a pointer to the request data.                           */
      OACP_RequestData = &(RequestData.Data.OACP_RequestData);

      /* We need to make sure we are ready for the OACP Read Procedure. */
      if(OACP_RequestData->Request_Op_Code == oacpRead)
      {
         /* Make sure the OTP Client has selected the current object.   */
         if(CurrentObject.Valid)
         {
            /* Store a pointer to the channel data.                     */
            ChannelData = &(DeviceInfo->ChannelData);

            /* Make sure a procedure is not already in progress.        */
            if(!(ChannelData->Flags & (OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS | OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS)))
            {
               /* If the OTP Client has currently selected the OTP      */
               /* Directory Listing Object, then we will not be opening */
               /* a file to receive the contents.                       */
               if((CurrentObject.ObjectID.Lower == 0) && (CurrentObject.ObjectID.Upper == 0))
               {
                  /* Make sure the OTP Client has read the current      */
                  /* object's size.                                     */
                  /* * NOTE * This will format the OTS Directory Listing*/
                  /*          Object for this sample application.       */
                  if(CurrentObject.CurrentSize)
                  {
                     /* Make sure the user has requested to read the    */
                     /* entire object's contents.                       */
                     /* * NOTE * This is mandatory for the OTS Directory*/
                     /*          Listing Object.  The offset must be    */
                     /*          zero and the length should match the   */
                     /*          current size.                          */
                     if((OACP_RequestData->Parameter.Read_Data.Offset == 0) && (OACP_RequestData->Parameter.Read_Data.Length == 0))
                     {
                        /* Store the transfer buffer length.            */
                        ChannelData->Buffer_Length = CurrentObject.CurrentSize;

                        /* We will re-use the transfer buffer to hold   */
                        /* the OTS Directory Listing Object's contents  */
                        /* received from the OTP Server.                */
                        if((ChannelData->Buffer = (Byte_t *)BTPS_AllocateMemory(ChannelData->Buffer_Length)) != NULL)
                        {
                           /* Flag that the transfer buffer is ready to */
                           /* receive.                                  */
                           /* * NOTE * This is how we will determine how*/
                           /*          to process the received data     */
                           /*          indication.                      */
                           ChannelData->Flags |= OTP_CHANNEL_FLAGS_BUFFER_READY_READY_TO_RECEIVE;

                           /* Store the starting offset.                */
                           ChannelData->Current_Offset = OACP_RequestData->Parameter.Read_Data.Offset;

                           /* Store the ending offset to the starting   */
                           /* offset plus the specified length to       */
                           /* receive.                                  */
                           ChannelData->Ending_Offset  = CurrentObject.CurrentSize;

                           /* Update the length of the request to the   */
                           /* remaining length from the current offset  */
                           /* to the ending offset.                     */
                           OACP_RequestData->Parameter.Read_Data.Length = (ChannelData->Ending_Offset - ChannelData->Current_Offset);
                        }
                        else
                        {
                           /* Make sure the buffer length is reset.     */
                           ChannelData->Buffer_Length = 0;

                           printf("\r\nCould not allocate memory for the transfer buffer.\r\n");
                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                     {
                        printf("\r\nThe OTP Client MUST have an offset of zero and the length set to the OTS Directory Listing Object's current size.\r\n");
                        ret_val = FUNCTION_ERROR;
                     }
                  }
                  else
                  {
                     printf("\r\nThe OTS Directory Listing Object's current size has not been read or is invalid.\r\n");
                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  /* Prefix the path for the current directory.         */
                  BTPS_StringCopy(FilePath, "./");

                  /* Determine if the currently selected object has a   */
                  /* valid name.                                        */
                  if(BTPS_StringLength((char *)CurrentObject.Name))
                  {
                     /* Use the name.                                   */
                     strcat(FilePath, (char *)CurrentObject.Name);
                  }
                  else
                  {
                     /* We will use the default new file name.          */
                     strcat(FilePath, OTP_DEFAULT_NEW_FILE_NAME);
                  }

                  /* Let's attempt to open the open the file.           */
                  /* * NOTE * The transfer buffer is not used for this  */
                  /*          read procedure.                           */
                  if(!OpenFileContentsClient(&(ChannelData->File_Descriptor), FilePath, omAppend, 0, NULL))
                  {
                     /* We need to make sure the starting offset is     */
                     /* valid.                                          */
                     /* * NOTE * We will simply attempt to seek the     */
                     /*          position.                              */
                     if(BTPS_Seek_File(ChannelData->File_Descriptor, smBeginning, OACP_RequestData->Parameter.Read_Data.Offset))
                     {
                        /* Determine if the user wants to read the      */
                        /* entire file from the OTP Server.             */
                        if((OACP_RequestData->Parameter.Read_Data.Offset == 0) && (OACP_RequestData->Parameter.Read_Data.Length == 0))
                        {
                           /* Make sure the OTP Client has read the     */
                           /* current object's size.                    */
                           if(CurrentObject.CurrentSize)
                           {
                              /* Store the starting offset.             */
                              ChannelData->Current_Offset = OACP_RequestData->Parameter.Read_Data.Offset;

                              /* Store the ending offset to the current */
                              /* object's current size.                 */
                              ChannelData->Ending_Offset  = CurrentObject.CurrentSize;

                              /* Update the length of the request to the*/
                              /* remaining length from the current      */
                              /* offset to the ending offset.           */
                              OACP_RequestData->Parameter.Read_Data.Length = (ChannelData->Ending_Offset - ChannelData->Current_Offset);
                           }
                           else
                           {
                              printf("\r\nThe OTS Object's current size has not been read or is zero.\r\n");
                              ret_val = FUNCTION_ERROR;
                           }
                        }
                        else
                        {
                           /* Store the starting offset.                */
                           ChannelData->Current_Offset = OACP_RequestData->Parameter.Read_Data.Offset;

                           /* Store the ending offset to the starting   */
                           /* offset plus the specified length to       */
                           /* receive.                                  */
                           ChannelData->Ending_Offset  = (ChannelData->Current_Offset + OACP_RequestData->Parameter.Read_Data.Length);
                        }
                     }
                     else
                     {
                        printf("\r\nCould not seek to the position of requested offset.\r\n");
                        ret_val = FUNCTION_ERROR;

                        /* Simply call the internal function to cleanup */
                        /* the channel.                                 */
                        CleanupChannel(&(DeviceInfo->ChannelData));
                     }
                  }
                  else
                  {
                     /* An error has already been printed.              */
                     ret_val = FUNCTION_ERROR;
                  }
               }
            }
            else
            {
               printf("\r\nA procedure is currently in progress.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("\r\nOTP Client has not selected the current object.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }

      /* We need to make sure we are ready for the OACP Write Procedure.*/
      if(OACP_RequestData->Request_Op_Code == oacpWrite)
      {
         /* Make sure the OTP Client has selected the current object.   */
         if(CurrentObject.Valid)
         {
            /* Store a pointer to the channel data.                     */
            ChannelData = &(DeviceInfo->ChannelData);

            /* Make sure a procedure is not already in progress.        */
            if(!(ChannelData->Flags & (OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS | OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS)))
            {
               /* Let's attempt to open the existing file.              */
               /* * NOTE * The transfer buffer length will initially be */
               /*          set to the allocated size of the file.       */
               if(!OpenFileContentsClient(&(ChannelData->File_Descriptor), RequestData.FileName, omReadOnly, RequestData.FileOffset, &(ChannelData->Buffer_Length)))
               {
                  /* If the user has specified a write length then we   */
                  /* need to make sure that it plus the user specified  */
                  /* file offset is less than or equal to the allocated */
                  /* size of the user specified input file.             */
                  /* * NOTE * Otherwise it will be zero to indicate that*/
                  /*          the user wants to use the remaining length*/
                  /*          of the file from the user specified file  */
                  /*          offset, which we will guarantee won't     */
                  /*          exceed the allocate size of the input     */
                  /*          file.                                     */
                  if((OACP_RequestData->Parameter.Write_Data.Length == 0) || ((OACP_RequestData->Parameter.Write_Data.Length + RequestData.FileOffset) <= ChannelData->Buffer_Length))
                  {
                     /* The current offset for the transfer buffer will */
                     /* ALWAYS be zero.                                 */
                     ChannelData->Current_Offset = 0;

                     /* Determine if we are going to use the user       */
                     /* specified write length.                         */
                     /* * NOTE * If the user specified write length is  */
                     /*          zero then the transfer buffer length   */
                     /*          will be the remaining length of the    */
                     /*          user specified file from the user      */
                     /*          specified file offset.                 */
                     if(OACP_RequestData->Parameter.Write_Data.Length)
                     {
                        /* Update the transfer buffer length to the     */
                        /* specified write length.                      */
                        ChannelData->Buffer_Length                    = OACP_RequestData->Parameter.Write_Data.Length;
                     }
                     else
                     {
                        /* Update the transfer buffer length to the     */
                        /* remaining length of the file from the user   */
                        /* specified file offset.                       */
                        ChannelData->Buffer_Length                   -= RequestData.FileOffset;

                        /* Go ahead and set the write length to the     */
                        /* length of the transfer buffer.               */
                        OACP_RequestData->Parameter.Write_Data.Length = ChannelData->Buffer_Length;
                     }

                     /* Allocate memory to hold the transfer buffer.    */
                     /* * NOTE * If an error does not occur this memory */
                     /*          will be freed after a transfer has     */
                     /*          completed or the channel is            */
                     /*          disconnected.                          */
                     if((ChannelData->Buffer = BTPS_AllocateMemory(ChannelData->Buffer_Length)) != NULL)
                     {
                        /* Read the file into the transfer buffer.      */
                        /* * NOTE * If the user specifies the write     */
                        /*          length, then this function will fail*/
                        /*          if the the user specified offset    */
                        /*          plus user specified write length    */
                        /*          exceeds the allocated size of the   */
                        /*          input file.  This means we will stop*/
                        /*          reading once we reach the end of the*/
                        /*          file and we will never read the     */
                        /*          expected number of bytes.           */
                        if(ReadFileContents(ChannelData->File_Descriptor, ChannelData->Buffer_Length, ChannelData->Buffer) == ChannelData->Buffer_Length)
                        {
                           /* Close the file descriptor since we are    */
                           /* finished with it.                         */
                           CloseFileContents(&(ChannelData->File_Descriptor));

                           /* Determine if we are going to use the user */
                           /* specified write length.                   */
                           /* * NOTE * If the user specified write      */
                           /*          length is zero then the transfer */
                           /*          buffer length will be the        */
                           /*          remaining length of the user     */
                           /*          specified file from the user     */
                           /*          specified file offset.           */
                           if(OACP_RequestData->Parameter.Write_Data.Length)
                           {
                              /* Set the ending offset to the user      */
                              /* specified write length.                */
                              ChannelData->Ending_Offset = OACP_RequestData->Parameter.Write_Data.Length;
                           }
                           else
                           {
                              /* The ending offset is ALWAYS the        */
                              /* remaining length of the file from the  */
                              /* user specified file offset.            */
                              /* * NOTE * The transfer buffer length is */
                              /*          currently set to this value.  */
                              ChannelData->Ending_Offset = ChannelData->Buffer_Length;
                           }
                        }
                        else
                        {
                           /* An error has already been printed.        */
                           ret_val = FUNCTION_ERROR;
                        }
                     }
                     else
                     {
                        printf("\r\nCould not allocate memory for the transfer buffer.\r\n");
                        ret_val = FUNCTION_ERROR;
                     }
                  }
                  else
                  {
                     printf("\r\nThe specified write length exceeds the size of the file.\r\n");
                     ret_val = FUNCTION_ERROR;
                  }

                  /* If an error occured we need to cleanup the channel */
                  /* data.                                              */
                  if(ret_val)
                  {
                     /* Simply call the internal function to cleanup the*/
                     /* channel.                                        */
                     CleanupChannel(ChannelData);
                  }
               }
               else
               {
                  /* An error has already been printed.                 */
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               printf("\r\nA procedure is currently in progress.\r\n");
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            printf("\r\nOTP Client has not selected the current object.\r\n");
            ret_val = FUNCTION_ERROR;
         }
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Determine the size of the buffer to send the OACP request.  */
         /* * NOTE * This function will return the size if the          */
         /*          BufferLength is set to zero.                       */
         if((ret_val = OTS_Format_OACP_Request(OACP_RequestData, 0, NULL)) > 0)
         {
            /* Store the buffer length.                                 */
            BufferLength = (Word_t)ret_val;

            /* Store a pointer to the request buffer.                   */
            /* * NOTE * We are guaranteed that this buffer will hold the*/
            /*          data for all requests.                          */
            Buffer = RequestData.Buffer;

            /* Format the buffer.                                       */
            if((ret_val = OTS_Format_OACP_Request(OACP_RequestData, BufferLength, Buffer)) == 0)
            {
               /* Query the GATT MTU to determine whether we can fit the*/
               /* OACP Write reqest in GATT Write request or if we need */
               /* to use one or more GATT Prepare Write requests        */
               /* followed by a GATT Execute Write request.             */
               if(!(ret_val = GATT_Query_Connection_MTU(BluetoothStackID, DeviceInfo->ConnectionID, &MTU)))
               {
                  /* If the Object Name fits in the (ATT_MTU-3) size    */
                  /* then it can fit in a GATT Write request.           */
                  /* * NOTE * This may only exceed the size if this is  */
                  /*          an OACP Create Procedure with a 128-bit   */
                  /*          UUID Object Type.                         */
                  if(BufferLength <= (MTU - 3))
                  {
                     /* Send the GATT Write request.                    */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Write Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
                  else
                  {
                     /* We need to set the request data before we issue */
                     /* the first GATT Prepare Write request so we can  */
                     /* handle the response and send more requests if   */
                     /* necessary.                                      */
                     /* * NOTE * The data is already formatted into the */
                     /*          buffer.                                */
                     RequestData.BufferLength = BufferLength;
                     RequestData.BufferOffset = 0;

                     /* Send the GATT Prepare Write request.            */
                     /* * NOTE * We will not save the transactionID     */
                     /*          returned by this function, which we    */
                     /*          could use to cancel the request.       */
                     if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, RequestData.BufferOffset, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                     {
                        printf("\r\nGATT Prepare Write Request sent:\r\n");
                        printf("   TransactionID:     %d\r\n",     ret_val);
                        printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                        printf("   Offset:            %u\r\n",     RequestData.BufferOffset);
                     }
                     else
                        DisplayFunctionError("GATT_Read_Value_Request", ret_val);
                  }
               }
               else
                  DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
            }
            else
               DisplayFunctionError("OTS_Format_OACP_Request", ret_val);
         }
         else
            DisplayFunctionError("OTS_Format_OACP_Request", ret_val);

         /* Simply return success to the caller.                        */
         ret_val = 0;
      }
   }
   else
      printf("\r\nInvalid attribute handle.\r\n");

   return(ret_val);
}

   /* The following function is a helper function for                   */
   /* WriteOTSCharacteristic() to send a write request for the Object   */
   /* List Control Point.                                               */
static int SendOLCPWriteRequest(DeviceInfo_t *DeviceInfo)
{
   int     ret_val      = 0;
   Word_t  BufferLength = 0;
   Byte_t *Buffer;

   /* Store the attribute handle.                                       */
   RequestData.AttributeHandle = DeviceInfo->OTSClientInfo.Object_List_Control_Point;

   /* Check to make sure the attribute handle is valid.                 */
   if(RequestData.AttributeHandle)
   {
      /* Determine the size of the buffer to send the OLCP request.     */
      /* * NOTE * This function will return the size if the BufferLength*/
      /*          is set to zero.                                       */
      if((ret_val = OTS_Format_OLCP_Request(&(RequestData.Data.OLCP_RequestData), 0, NULL)) > 0)
      {
         /* Store the buffer length.                                    */
         BufferLength = (Word_t)ret_val;

         /* Store a pointer to the request buffer.                      */
         /* * NOTE * We are guaranteed that this buffer will hold the   */
         /*          data for all requests.                             */
         Buffer = RequestData.Buffer;

         /* Format the buffer.                                          */
         if((ret_val = OTS_Format_OLCP_Request(&(RequestData.Data.OLCP_RequestData), BufferLength, Buffer)) == 0)
         {
            /* Send the GATT Write request.                             */
            /* * NOTE * We will not save the transactionID returned by  */
            /*          this function, which we could use to cancel the */
            /*          request.                                        */
            if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
            {
               printf("\r\nGATT Write Request sent:\r\n");
               printf("   TransactionID:     %d\r\n",     ret_val);
               printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
            }
            else
               DisplayFunctionError("GATT_Read_Value_Request", ret_val);
         }
         else
            DisplayFunctionError("OTS_Format_OLCP_Request", ret_val);
      }
      else
         DisplayFunctionError("OTS_Format_OLCP_Request", ret_val);

      /* Simply return success to the caller.                           */
      ret_val = 0;
   }
   else
      printf("\r\nInvalid attribute handle.\r\n");

   return(ret_val);
}

   /* The following function is a helper function for                   */
   /* WriteOTSCharacteristic() to send a write request for an Object    */
   /* List Filter.                                                      */
static int SendObjectListFilterWriteRequest(DeviceInfo_t *DeviceInfo, Word_t AttributeHandle)
{
   int     ret_val      = 0;
   Word_t  BufferLength = 0;
   Byte_t *Buffer;
   Word_t  MTU;

   /* Store the attribute handle.                                       */
   RequestData.AttributeHandle = AttributeHandle;

   /* Check to make sure the attribute handle is valid.                 */
   if(RequestData.AttributeHandle)
   {
      /* Determine the size of the buffer to send the OLCP request.     */
      /* * NOTE * This function will return the size if the BufferLength*/
      /*          is set to zero.                                       */
      if((ret_val = OTS_Format_Object_List_Filter_Data(&(RequestData.Data.ListFilterData), 0, NULL)) > 0)
      {
         /* Store the buffer length.                                    */
         BufferLength = (Word_t)ret_val;

         /* Store a pointer to the request buffer.                      */
         /* * NOTE * We are guaranteed that this buffer will hold the   */
         /*          data for all requests.                             */
         Buffer = RequestData.Buffer;

         /* Format the buffer.                                          */
         if((ret_val = OTS_Format_Object_List_Filter_Data(&(RequestData.Data.ListFilterData), BufferLength, Buffer)) == 0)
         {
            /* Query the GATT MTU to determine whether we can fit the   */
            /* OTS Object List Filter in GATT Write request or if we    */
            /* need to use one or more GATT Prepare Write requests      */
            /* followed by a GATT Execute Write request.                */
            if(!(ret_val = GATT_Query_Connection_MTU(BluetoothStackID, DeviceInfo->ConnectionID, &MTU)))
            {
               /* If the Object Name fits in the (ATT_MTU-3) size then  */
               /* it can fit in a GATT Write request.                   */
               if(BufferLength <= (MTU - 3))
               {
                  /* Send the GATT Write request.                       */
                  /* * NOTE * We will not save the transactionID        */
                  /*          returned by this function, which we could */
                  /*          use to cancel the request.                */
                  if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                  {
                     printf("\r\nGATT Write Request sent:\r\n");
                     printf("   TransactionID:     %d\r\n",     ret_val);
                     printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
               else
               {
                  /* We need to set the request data before we issue the*/
                  /* first GATT Prepare Write request so we can handle  */
                  /* the response and send more requests if necessary.  */
                  /* * NOTE * The data is already formatted into the    */
                  /*          buffer.                                   */
                  RequestData.BufferLength = BufferLength;
                  RequestData.BufferOffset = 0;

                  /* Send the GATT Prepare Write request.               */
                  /* * NOTE * We will not save the transactionID        */
                  /*          returned by this function, which we could */
                  /*          use to cancel the request.                */
                  if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, BufferLength, RequestData.BufferOffset, Buffer, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
                  {
                     printf("\r\nGATT Prepare Write Request sent:\r\n");
                     printf("   TransactionID:     %d\r\n",     ret_val);
                     printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);
                     printf("   Offset:            %u\r\n",     RequestData.BufferOffset);
                  }
                  else
                     DisplayFunctionError("GATT_Read_Value_Request", ret_val);
               }
            }
            else
               DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
         }
         else
            DisplayFunctionError("OTS_Format_Object_List_Filter_Data", ret_val);
      }
      else
         DisplayFunctionError("OTS_Format_Object_List_Filter_Data", ret_val);

      /* Simply return success to the caller.                           */
      ret_val = 0;
   }
   else
      printf("\r\nInvalid attribute handle.\r\n");

   return(ret_val);
}

   /* The following function is a helper function for                   */
   /* WriteOTSCharacteristic() to send a write request for an OTS       */
   /* Characteristic's CCCD.                                            */
static int SendCCCDWriteRequest(DeviceInfo_t *DeviceInfo, Word_t AttributeHandle)
{
   int               ret_val = 0;
   NonAlignedWord_t  Client_Configuration;

   /* Store the attribute handle.                                       */
   RequestData.AttributeHandle = AttributeHandle;

   /* Make sure the attrubute handle is valid.                          */
   if(RequestData.AttributeHandle)
   {
      /* Format the configuration.                                      */
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Client_Configuration, RequestData.Data.Configuration);

      /* Send the write request.                                        */
      /* * NOTE * We will not save the transactionID returned by this   */
      /*          function, which we could use to cancel the request.   */
      if((ret_val = GATT_Write_Request(BluetoothStackID, DeviceInfo->ConnectionID, RequestData.AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Client_Configuration, GATT_ClientEventCallback_OTS, RequestData.AttributeHandle)) > 0)
      {
         printf("\r\nGATT Write Request sent:\r\n");
         printf("   TransactionID:     %d\r\n", ret_val);
         printf("   Attribute Handle:  0x%04X\r\n", RequestData.AttributeHandle);

      }
      else
         DisplayFunctionError("GATT_Write_Request", ret_val);

      /* Simply return success to the caller.                           */
      ret_val = 0;
   }
   else
      printf("\r\nInvalid attribute handle.\r\n");

   return(ret_val);
}

   /* The following function is responsible for reading the Object      */
   /* Metadata for the currently selected object on the OTP Server.     */
   /* ** NOTE ** This function SHOULD ONLY be called by the OTP Client  */
   /*            and if the OTP Client is currently connected to the OTP*/
   /*            Server and the OTP Client has performed service        */
   /*            discovery for OTS.                                     */
   /* * NOTE * We have hardcoded the OTS Object Metadata types based on */
   /*          the OTS_Object_Metadata_Type_t enumerations.             */
static void ReadObjectMetadata(void)
{
   unsigned int Index;

   /* Let the user know we are going to read the OTS Object Metadata.   */
   printf("\r\nReading the current object's OTS Object Metadata.\r\n");

   /* Loop through the Object Metadata types.                           */

   for(Index = 0; Index < 7; Index++)
   {
      /* Format the request data.                                       */
      /* * NOTE * ReadOTSCharacteristic() will format the rest.         */
      RequestData.AttributeHandleType = ahtObjectMetadata;
      RequestData.MetadataType        = (OTS_Object_Metadata_Type_t)Index;

      /* Simply call the interanl function to dispatch the read request.*/
      ReadOTSCharacteristic();
   }
}

   /* The following function is responsible for mapping an attribute    */
   /* handle for a received Object Metadata read response.              */
   /* * NOTE * We need to do this since the stored Object Metadata type */
   /*          in the RequestData may only be used for one request.     */
   /*          However, the ReadObjectMetadata() function will send     */
   /*          multiple requests and if we do not map the correct, then */
   /*          they will all be interpretted as last Object Metadata    */
   /*          Type that was requested.                                 */
static void MapAttributeHandleToObjectMetadataType(OTS_Client_Information_t *ClientInfo, Word_t AttributeHandle, OTS_Object_Metadata_Type_t *Type)
{
   /* We will simply check every attribute handle for the OTS Object    */
   /* Metadata Characteristic.  If we find a match we will set the Type.*/

   /* Object Name.                                                      */
   if(AttributeHandle == ClientInfo->Object_Name)
   {
      /* Assign the Object Name.                                        */
      *Type = omtObjectName;
   }
   else
   {
      /* Object Type.                                                   */
      if(AttributeHandle == ClientInfo->Object_Type)
      {
         /* Assign the Object Type.                                     */
         *Type = omtObjectType;
      }
      else
      {
         /* Object Size.                                                */
         if(AttributeHandle == ClientInfo->Object_Size)
         {
            /* Assign the Object Size.                                  */
            *Type = omtObjectSize;
         }
         else
         {
            /* Object First Created.                                    */
            if(AttributeHandle == ClientInfo->Object_First_Created)
            {
               /* Assign the Object First Created.                      */
               *Type = omtObjectFirstCreated;
            }
            else
            {
               /* Object Last Modified.                                 */
               if(AttributeHandle == ClientInfo->Object_Last_Modified)
               {
                  /* Assign the Object Last Modified.                   */
                  *Type = omtObjectLastModified;
               }
               else
               {
                  /* Object ID.                                         */
                  if(AttributeHandle == ClientInfo->Object_ID)
                  {
                     /* Assign the Object ID.                           */
                     *Type = omtObjectID;
                  }
                  else
                  {
                     /* Object Properties.                              */
                     if(AttributeHandle == ClientInfo->Object_Properties)
                     {
                        /* Assign the Object Properties.                */
                        *Type = omtObjectProperties;
                     }
                  }
               }
            }
         }
      }
   }
}

   /* The following function is responsible for sending an Object       */
   /* Changed Indication.                                               */
static void IndicateObjectChanged(OTS_Object_Changed_Data_t *ObjectChangedData)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the OTP Client.                     */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that the OTP Client has configured the OTS Object */
            /* Changed CCCD for indications.                            */
            if(DeviceInfo->OTSServerInfo.Object_Changed_Configuration & OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
            {
               /* Send the indication.                                  */
               if((ret_val = OTS_Indicate_Object_Changed(BluetoothStackID, OTSInstanceID, DeviceInfo->ConnectionID, ObjectChangedData)) > 0)
               {
                  printf("\r\nOTS Object Changed indication sent:\r\n");
                  printf("   Transaction ID:  %d\r\n", ret_val);
               }
               else
                  DisplayFunctionError("OTS_Indicate_Object_Changed", ret_val);
            }
            else
               printf("\r\nOTP Client has not configured the OTS Object Changed CCCD for indications.\r\n");
         }
         else
            printf("\r\nNo device information for the OTP Client.\r\n");
      }
      else
         printf("\r\nNot connected to the OTP Client.\r\n");
   }
   else
      printf("\r\nOnly the OTP Server may send the Object Changed indication.\r\n");
}

   /* The following function is responsible for getting the current     */
   /* connection mode for the Object Transfer Channel.                  */
static void GetConnectionMode(void)
{
   int                            Result;
   OTS_Channel_Connection_Mode_t  Mode;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Simply query the connection mode.                              */
      if((Result = OTS_Channel_Get_Connection_Mode(BluetoothStackID, OTSInstanceID, &Mode))== 0)
      {
         /* Display the connection mode.                                */
         printf("\r\nConnection Mode:  ");
         switch(Mode)
         {
            case ocmAutomaticAccept:
               printf("ocmAutomaticAccept.\r\n");
               break;
            case ocmAutomaticReject:
               printf("ocmAutomaticReject.\r\n");
               break;
            case ocmManualAccept:
               printf("ocmManualAccept.\r\n");
               break;
            default:
               printf("Invalid.\r\n");
               break;
         }
      }
      else
         DisplayFunctionError("OTS_Channel_Get_Connection_Mode", Result);
   }
   else
      printf("\r\nOnly the OTP Server may get the connection mode.\r\n");
}


   /* The following function is responsible for setting the connection  */
   /* mode for the Object Transfer Channel.                             */
static void SetConnectionMode(OTS_Channel_Connection_Mode_t Mode)
{
   int Result;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Make sure the connection mode is valid.                        */
      if((Mode >= ocmAutomaticAccept) && (Mode <= ocmManualAccept))
      {
         /* Simply set the connection mode.                             */
         if((Result = OTS_Channel_Set_Connection_Mode(BluetoothStackID, OTSInstanceID, Mode)) == 0)
         {
            /* Display the connection mode.                             */
            printf("\r\nConnection Mode:  ");
            switch(Mode)
            {
               case ocmAutomaticAccept:
                  printf("ocmAutomaticAccept.\r\n");
                  break;
               case ocmAutomaticReject:
                  printf("ocmAutomaticReject.\r\n");
                  break;
               case ocmManualAccept:
                  printf("ocmManualAccept.\r\n");
                  break;
               default:
                  printf("Invalid.\r\n");
                  break;
            }
         }
         else
            DisplayFunctionError("OTS_Channel_Set_Connection_Mode", Result);
      }
      else
         printf("\r\nInvalid connection mode.\r\n");
   }
   else
      printf("\r\nOnly the OTP Server may set the connection mode.\r\n");
}

   /* The following function is responsible for setting the Object      */
   /* Transfer Channel configuration options.                           */
   /* * NOTE * The LE_Channel_Parameters may only be set for the OTP    */
   /*          Client since they are required for the OTP Server when   */
   /*          the service is registered.                               */
static void SetParameters(L2CA_LE_Channel_Parameters_t *LE_Channel_Parameters, L2CA_Queueing_Parameters_t *Queueing_Parametes)
{
   DeviceInfo_t *DeviceInfo;

   /* Make sure that we are connected and a remote device is selected.  */
   if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Get the device info.                                           */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Make sure we are not connected over the Object Transfer     */
         /* Channel.                                                    */
         if(!(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_CONNECTED))
         {
            /* Check if we need to store the LE Channel Parameters.     */
            if(LE_Channel_Parameters)
            {
               /* Simply store the parameters.                          */
               DeviceInfo->ChannelData.LE_Channel_Parameters = *LE_Channel_Parameters;
            }

            /* Check if we need to store the LE Channel Parameters.     */
            if(Queueing_Parametes)
            {
               /* Simply store the parameters.                          */
               DeviceInfo->ChannelData.Queueing_Parameters = *Queueing_Parametes;
            }
         }
         else
            printf("\r\nThe Object Transfer Channel is currently connected.\r\n");
      }
      else
         printf("\r\nNo device information.\r\n");
   }
   else
      printf("\r\nNo connection.\r\n");
}

   /* The following function is responsible for opening the Object      */
   /* Transfer Channel (OTC).                                           */
   /* * NOTE * This function should only be called by the OTP Client.   */
static void OpenChannel(void)
{
   int                     Result;
   DeviceInfo_t           *DeviceInfo;
   GATT_Connection_Type_t  Type;

   /* Make sure we are the OTP Client.                                  */
   if(!OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the OTP Server.                     */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we are not already connected over the Object   */
            /* Transfer Channel.                                        */
            if(!(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_CONNECTED))
            {
               /* If we are connected over the BR/EDR transport for     */
               /* GATT.                                                 */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED)
               {
                  /* Store the connection type.                         */
                  Type = gctBR_EDR;
               }
               else
               {
                  /* Store the connection type.                         */
                  Type = gctLE;
               }

               /* Open the Object Transfer Channel.                     */
               /* * NOTE * The Channel Parameters are only required if  */
               /*          we are currently connected to the OTP Server */
               /*          over the GATT LE transport.                  */
               if((Result = OTS_Channel_Connect_Request(BluetoothStackID, Type, ConnectionBD_ADDR, OTS_Channel_EventCallback, BluetoothStackID, ((Type == gctLE) ? &(DeviceInfo->ChannelData.LE_Channel_Parameters) : NULL))) > 0)
               {
                  /* Store the Channel ID.                              */
                  DeviceInfo->ChannelData.Channel_ID = (Word_t)Result;

                  /* Let the user know that the connection request has  */
                  /* been sent.                                         */
                  printf("\r\nObject Transfer Channel connection request sent. (CID = 0x%04X).\r\n", DeviceInfo->ChannelData.Channel_ID);
               }
               else
                  DisplayFunctionError("OTS_Channel_Connect_Request", Result);
            }
            else
               printf("\r\nThe Object Transfer Channel is already open.\r\n");
         }
         else
            printf("\r\nNo device information for the OTP Server.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Server.\r\n");
   }
   else
      printf("\r\nOnly the OTP Client may open the Object Transfer Channel (OTC).\r\n");
}

   /* The following function is responsible for closing the Object      */
   /* Transfer Channel (OTC).                                           */
   /* * NOTE * This function should only be called by the OTP Client.   */
static void CloseChannel(void)
{
   DeviceInfo_t       *DeviceInfo;
   OTP_Channel_Data_t *ChannelData;

   /* Make sure we are the OTP Client.                                  */
   if(!OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the OTP Server.                     */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Store a pointer to the Channel Data.                     */
            ChannelData = &(DeviceInfo->ChannelData);

            /* Make sure the Object Transfer Channel is currently       */
            /* connected.                                               */
            if(ChannelData->Flags & OTP_CHANNEL_FLAGS_CONNECTED)
            {
               /* Simply flag that we want to cleanup/disconnect the    */
               /* Object Transfer Channel.                              */
               ChannelData->Flags |= (OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL | OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL);

               /* Simply call the internal function to handle the       */
               /* request.                                              */
               CleanupChannel(ChannelData);
            }
            else
               printf("\r\nObject Transfer Channel not connected.\r\n");
         }
         else
            printf("\r\nNo device information for the OTP Server.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Server.\r\n");
   }
   else
      printf("\r\nOnly the OTP Client may open the Object Transfer Channel (OTC).\r\n");
}

   /* The following function is helper function responsible for writing */
   /* file data over the Object Transfer Channel (OTC).                 */
   /* ** NOTE ** No checks will be performed on the parameters.  This   */
   /*            function SHOULD NOT BE CALLED unless the Object        */
   /*            Transfer Channel is open and the data has been prepared*/
   /*            to send.                                               */
   /* * NOTE * This function will disconnect the channel if we fail to  */
   /*          send the data.                                           */
   /* * NOTE * This function may be called again when the               */
   /*          etOTS_Channel_Buffer_Empty_Indication event is received. */
static void SendObjectContents(OTP_Channel_Data_t *ChannelData)
{
   DWord_t *CurrentOffset = &(ChannelData->Current_Offset);
   DWord_t  EndingOffset  = ChannelData->Ending_Offset;
   int      Result = 0;
   DWord_t  RemainingLength;
   Word_t   DataLength;
   Byte_t  *Buffer;

   /* Try to send data until there is insufficient buffer space to send */
   /* any more data.                                                    */
   /* * NOTE * We can send more data when the                           */
   /*          etOTS_Channel_Buffer_Empty_Indication event is received. */
   while(Result != BTPS_ERROR_INSUFFICIENT_BUFFER_SPACE)
   {
      /* Determine how much remaining data we need to send.             */
      /* * NOTE * We CANNOT send more data than the negotiated SDU size */
      /*          for the channel.  This should have been stored when   */
      /*          the Object Transfer Channel first connected.          */
      RemainingLength = (EndingOffset - *CurrentOffset);
      DataLength      = (Word_t)((RemainingLength >= ChannelData->Max_Data_Size) ? (ChannelData->Max_Data_Size) : (RemainingLength));
      Buffer          = (ChannelData->Buffer + *CurrentOffset);

      /* Attempt to send the packet.                                    */
      /* * NOTE * This function does not allow sending a zero length    */
      /*          value.                                                */
      if((Result = OTS_Channel_Send_Data(BluetoothStackID, ChannelData->Channel_ID, &(ChannelData->Queueing_Parameters), DataLength, Buffer)) != BTPS_ERROR_INSUFFICIENT_BUFFER_SPACE)
      {
         /* If the packets were successfully queued.                    */
         if(Result >= 0)
         {
            /* Update the current offset.                               */
            *CurrentOffset += DataLength;

            /* Simply call the internal function to display our         */
            /* progress.                                                */
            DisplayTransferProgress(ChannelData);

            /* Check if we have sent all the data.                      */
            if(*CurrentOffset >= EndingOffset)
            {
               /* Let the user know that the object transfer has        */
               /* completed.                                            */
               printf("\r\n\r\nObject Transfer Complete.\r\n");
               printf("   Current Offset:    %u\r\n", *CurrentOffset);
               printf("   Ending Offset:     %u\r\n", EndingOffset);
               printf("   Remaining Length:  %u\r\n", (EndingOffset - *CurrentOffset));

               /* Check to make sure we didn't send too much data.      */
               if(*CurrentOffset > EndingOffset)
                  printf("\r\nToo much data sent. %u.\r\n",(*CurrentOffset - EndingOffset));

               /* If we are the OTP Client.                             */
               if(!OTSInstanceID)
               {
                  /* Let the user know to disconect the Object Transfer */
                  /* Channel.                                           */
                  printf("\r\nUse command 'Close', to disconnect the Object Transfer Channel.\r\n");

                  /* If this is an OACP Write Procedure.                */
                  if(ChannelData->Flags & OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS)
                  {
                     /* Simply call the internal function to read it's  */
                     /* Object Metadata since the size has changed.     */
                     ReadObjectMetadata();
                  }
               }

               /* Flag that we need to cleanup the Object Transfer      */
               /* Channel since we completed the transfer.              */
               ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;

               /* ** NOTE ** The OTP Client will disconnect the channel.*/
               break;
            }
         }
         else
         {
            /* Display the error information.                           */
            DisplayFunctionError("\r\n\r\nOTS_Channel_Send_Data", Result);
            printf("   Current Offset:    %u\r\n", *CurrentOffset);
            printf("   Ending Offset:     %u\r\n", EndingOffset);
            printf("   Remaining Length:  %u\r\n", (EndingOffset - *CurrentOffset));
            printf("   Data Length:       %u\r\n", DataLength);

            /* Flag that we need to cleanup/disconnect the Object       */
            /* Transfer Channel since an error occured.                 */
            ChannelData->Flags |= (OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL | OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL);
            break;
         }
      }
   }
}

   /* The following function is a helper function to process the        */
   /* received OTS Object contents from the a remote device via the     */
   /* etOTS_Channel_Data_Indication event.                              */
   /* ** NOTE ** This function SHOULD NOT BE CALLED for the OTS         */
   /*            Directory Listing Object.  It is handled differently   */
   /*            and only the OTP Client can process it.                */
   /* * NOTE * The file descriptor will already be open and the offset  */
   /*          position will remain in the same place from the previous */
   /*          data indication.                                         */
   /* * NOTE * The file descriptor will be opened by the OTP Server if  */
   /*          the OTP Client sends a request to start an OACP Write    */
   /*          Procedure.  The OTP Server will process the data received*/
   /*          from the OTP Client and write it to the file.            */
   /* * NOTE * The file descriptor will be opened by the OTP Client     */
   /*          before the OACP Read Procedure is started.  The OTP      */
   /*          Client will process the data received from the OTP Server*/
   /*          and write it to the file.                                */
static void ProcessObjectContents(OTP_Filtered_Object_Node_t *CurrentObject, OTP_Channel_Data_t *ChannelData, Word_t DataLength, Byte_t *Data)
{
   DWord_t            *CurrentOffset  = &(ChannelData->Current_Offset);
   DWord_t             EndingOffset   = ChannelData->Ending_Offset;
   DWord_t             BytesWritten;
   OTS_Object_Data_t  *ObjectDataPtr;
   unsigned int        Index;
   OTP_Object_Entry_t *ObjectEntryPtr = NULL;

   /* Write the data into the file and update the current offset        */
   /* position.                                                         */
   if((BytesWritten = WriteFileContents(ChannelData->File_Descriptor, DataLength, Data)) == (DWord_t)DataLength)
   {
      /* Update the current offset by the number of bytes successfully  */
      /* written.                                                       */
      *CurrentOffset += BytesWritten;

      /* If we are the OTP Server we may need to update the             */
      /* current/allocated size for the currently selected OTS Object.  */
      if(OTSInstanceID)
      {
         /* Store a pointer to the current object's OTS Object data.    */
         /* * NOTE * This MUST be valid or we should have not started   */
         /*          the procedure.                                     */
         ObjectDataPtr = CurrentObject->ObjectDataPtr;

         /* Check if we need to update the current size of the OTS      */
         /* Object.                                                     */
         /* * NOTE * We can only update the allocated size of the OTS   */
         /*          Object is the current size of the OTS Object       */
         /*          changes since (current size <= allocated size)     */
         if(*CurrentOffset > ObjectDataPtr->Size.Current_Size)
         {
            /* Update the current size of the OTS Object.               */
            ObjectDataPtr->Size.Current_Size = *CurrentOffset;

            /* Determine if we need to increase the allocated size of   */
            /* the OTS Object.                                          */
            /* * NOTE * We already checked in the OACP Write Procedure  */
            /*          if we were allowed to increase the allocated    */
            /*          size, so we will assume we can.                 */
            if(*CurrentOffset >= ObjectDataPtr->Size.Allocated_Size)
            {
               /* Update the allocated size of the OTS Object.          */
               ObjectDataPtr->Size.Allocated_Size = *CurrentOffset;
            }
         }
      }

      /* Simply call the internal function to display our progress.     */
      DisplayTransferProgress(ChannelData);

      /* Determine if we have received all the data.                    */
      /* * NOTE * We will make sure we stop if we go over the expected  */
      /*          length.                                               */
      if(*CurrentOffset >= EndingOffset)
      {
         /* Let the user know that the object transfer has completed.   */
         printf("\r\n\r\nObject Transfer Complete.\r\n");
         printf("   Current Offset:    %u\r\n", *CurrentOffset);
         printf("   Ending Offset:     %u\r\n", EndingOffset);
         printf("   Remaining Length:  %u\r\n", (EndingOffset - *CurrentOffset));

         /* Check to make sure we didn't go over the expected length.   */
         if(*CurrentOffset > EndingOffset)
         {
            printf("\r\nToo much data written: %u\r\n", (*CurrentOffset - EndingOffset));

            /* Flag that we need to disconnect the Object Transfer      */
            /* Channel since an error occured.                          */
            /* * NOTE * If this is not done, we may continue to receive */
            /*          data indications.                               */
            ChannelData->Flags |= OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL;
         }

         /* If we are the OTP Server.                                   */
         /* * NOTE * If we are the OTP Server then this MUST be the     */
         /*          write procedure and we need to unlock the current  */
         /*          object since the transfer is done.                 */
         if(OTSInstanceID)
         {
            /* Loop through the objects list and find the current       */
            /* object.                                                  */
            for(Index = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
            {
               /* Make sure the OTS Object is valid.                    */
               if(ServerData.Object_List[Index].Valid)
               {
                  /* If we find the current object.                     */
                  /* * NOTE * We are guaranteed to find it based on     */
                  /*          previous checks.                          */
                  if(ServerData.Object_List[Index].Data.ID.Lower == CurrentObject->ObjectDataPtr->ID.Lower)
                  {
                     ObjectEntryPtr = &(ServerData.Object_List[Index]);

                     /* We are done.                                    */
                     break;
                  }
               }
            }

            /* Make sure that the currently selected object is unlocked.*/
            if((ObjectEntryPtr) && (ObjectEntryPtr->Locked == TRUE))
            {
               ObjectEntryPtr->Locked = FALSE;
            }
         }
         else
         {
            /* Let the user know to disconect the Object Transfer       */
            /* Channel since we are the OTP Client.                     */
            printf("\r\nUse command 'Close', to disconnect the Object Transfer Channel.\r\n");
         }

         /* Flag that we need to cleanup the Object Transfer Channel    */
         /* since we completed the transfer.                            */
         ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;
      }
   }
   else
   {
      /* Flag that we need to cleanup/disconnect the Object Transfer    */
      /* Channel since an error occured.                                */
      ChannelData->Flags |= (OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL | OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL);
   }
}

   /* The following function is a helper function to process the        */
   /* received OTS Directory Listing Object's contents from the a remote*/
   /* device via the etOTS_Channel_Data_Indication event.               */
   /* ** NOTE ** This function SHOULD ONLY BE CALLED for the OTS        */
   /*            Directory Listing Object.  It is handled differently   */
   /*            and only the OTP Client can process it.                */
   /* ** NOTE ** The OTP Client should have alredy allocated the        */
   /*            transfer buffer to hold the contents.                  */
static void ProcessDirectoryListingObjectContents(OTP_Channel_Data_t *ChannelData, Word_t DataLength, Byte_t *Data)
{
   int                Result;
   DWord_t           *CurrentOffset = &(ChannelData->Current_Offset);
   DWord_t            EndingOffset  = ChannelData->Ending_Offset;
   DWord_t            NumberOfObjects;
   OTS_Object_Data_t *ObjectDataPtr;
   DWord_t            Index;

   /* Make sure the transfer buffer has been allocated to hold the      */
   /* received contents.                                                */
   if((ChannelData->Buffer_Length) && (ChannelData->Buffer))
   {
      /* Store the received data in the transfer buffer.                */
      BTPS_MemCopy((ChannelData->Buffer + *CurrentOffset), Data, DataLength);

      /* Update the current offset by the number of bytes received.     */
      *CurrentOffset += DataLength;

      /* Simply call the internal function to display our progress.     */
      DisplayTransferProgress(ChannelData);

      /* Determine if we have received all the data.                    */
      /* * NOTE * We will make sure we stop if we go over the expected  */
      /*          length.                                               */
      if(*CurrentOffset >= EndingOffset)
      {
         /* Let the user know that the object transfer has completed.   */
         printf("\r\n\r\nObject Transfer Complete.\r\n");
         printf("   Current Offset:    %u\r\n", *CurrentOffset);
         printf("   Ending Offset:     %u\r\n", EndingOffset);
         printf("   Remaining Length:  %u\r\n", (EndingOffset - *CurrentOffset));

         /* Check to make sure we didn't go over the expected length.   */
         if(*CurrentOffset > EndingOffset)
         {
            printf("\r\nToo much data written: %u\r\n", (*CurrentOffset - EndingOffset));

            /* Flag that we need to disconnect the Object Transfer      */
            /* Channel since an error occured.                          */
            /* * NOTE * If this is not done, we may continue to receive */
            /*          data indications.                               */
            ChannelData->Flags |= OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL;
         }
         else
         {
            /* Let the user know we are ready to display the OTS        */
            /* Directory Listing Object.                                */
            printf("\r\nOTS Directory Listing Object:\r\n");

            /* Let's decode the OTS Directory List Object's contents    */
            /* from the transfer buffer.                                */
            if((Result = OTS_Decode_Directory_Listing_Object_Contents(ChannelData->Buffer_Length, ChannelData->Buffer, 0, NULL)) > 0)
            {
               /* Store the number of objects contained in the transfer */
               /* buffer.                                               */
               NumberOfObjects = (DWord_t)Result;

               /* Let the user know how many OTS Objects have been      */
               /* decoded.                                              */
               printf("   OTS Objects: %u.\r\n", NumberOfObjects);

               /* Allocate memory to hold the decode OTS Objects.       */
               if((ObjectDataPtr = (OTS_Object_Data_t *)BTPS_AllocateMemory((OTS_OBJECT_DATA_SIZE * NumberOfObjects))) != NULL)
               {
                  /* Initialize the memory.                             */
                  BTPS_MemInitialize(ObjectDataPtr, 0, (OTS_OBJECT_DATA_SIZE * NumberOfObjects));

                  /* We need to allocate memory for each OTS Object     */
                  /* Name.                                              */
                  for(Index = 0; Index < NumberOfObjects; Index++)
                  {
                     /* Allocate memory for each OTS Object Name.       */
                     if((ObjectDataPtr[Index].Name.Buffer = (Byte_t *)BTPS_AllocateMemory(OTS_MAXIMUM_OBJECT_NAME_LENGTH + 1)) != NULL)
                     {
                        /* Initialize the buffer length to zero.        */
                        ObjectDataPtr[Index].Name.Buffer_Length = 0;
                     }
                     else
                     {
                        printf("\r\nCould not allocate memory for the OTS Object Name.\r\n");
                        break;
                     }
                  }

                  /* Decode the transfer buffer.                        */
                  if((Result = OTS_Decode_Directory_Listing_Object_Contents(ChannelData->Buffer_Length, ChannelData->Buffer, NumberOfObjects, ObjectDataPtr)) == 0)
                  {
                     /* Print the decoded OTS Objects.                  */
                     for(Index = 0; Index < NumberOfObjects; Index++)
                     {
                        /* Insert the NULL terminator so we can display */
                        /* the OTS Object Name as a c-string.           */
                        ObjectDataPtr[Index].Name.Buffer[ObjectDataPtr[Index].Name.Buffer_Length] = '\0';

                        /* Simply call the internal function to display */
                        /* the data.                                    */
                        printf("\r\nOTS Object (%u):\r\n", Index);
                        DisplayObjectData(&(ObjectDataPtr[Index]));
                     }
                  }
                  else
                     DisplayFunctionError("OTS_Decode_Directory_Listing_Object_Contents", Result);

                  /* We need to free the memory for each OTS Object     */
                  /* Name.                                              */
                  for(Index = 0; Index < NumberOfObjects; Index++)
                  {
                     /* If memory is allocated for the OTS Object Name. */
                     if(ObjectDataPtr[Index].Name.Buffer)
                     {
                        BTPS_FreeMemory(ObjectDataPtr[Index].Name.Buffer);
                        ObjectDataPtr[Index].Name.Buffer = NULL;
                     }
                  }

                  /* Free the memory allocated for the OTS Objects.     */
                  BTPS_FreeMemory(ObjectDataPtr);
                  ObjectDataPtr = NULL;
               }
               else
                  printf("\r\nCould not allocate memory to hold the decoded OTS Objects.\r\n");
            }
            else
               DisplayFunctionError("OTS_Decode_Directory_Listing_Object_Contents", Result);

            /* Let the user know to disconect the Object Transfer       */
            /* Channel.                                                 */
            printf("\r\nUse command 'Close', to disconnect the Object Transfer Channel.\r\n");
         }

         /* Flag that we need to cleanup the Object Transfer Channel    */
         /* since we completed the transfer.                            */
         ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;
      }
   }
   else
   {
      /* Flag that we need to cleanup/disconnect the Object Transfer    */
      /* Channel since an error occured.                                */
      ChannelData->Flags |= (OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL | OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL);
   }
}

   /* The following function is a helper function to inform the user of */
   /* the transfer progress.                                            */
static inline void DisplayTransferProgress(OTP_Channel_Data_t *ChannelData)
{
   DWord_t       CurrentOffset = ChannelData->Current_Offset;
   DWord_t       EndingOffset  = ChannelData->Ending_Offset;
   unsigned int  Index;
   int           Percentage;

   /* Determine the percentage complete.                                */
   Percentage = (int)(((double)CurrentOffset / (double)EndingOffset) * 100);

   /* Clear the current line and reset to beginning.                    */
   /* * NOTE * This will erase the display prompt that appears on every */
   /*          line.                                                    */
   printf("\33[2K\r");
   fflush(stdout);

   /* Inform the user on our progress.                                  */
   printf("  Transfer: [");

   /* Print the progress equal to the percentage.                       */
   for(Index = 0; Index < Percentage; Index++)
   {
      if(!(Index % 2))
         printf("=");
   }

   /* Print spaces for the remaining progress.                          */
   for(Index = Percentage; Index < 100; Index++)
   {
      if(!(Index % 2))
         printf(" ");
   }

   /* Display the percentage.                                           */
   printf("] %3d%%", Percentage);
}

   /* The following function is a helper function to cleanup the Object */
   /* Transfer Channel after a transfer has completed or an error       */
   /* occurs.  All Channel Data resources will be freed (File stream and*/
   /* buffer).  Other fields will be set to zero.                       */
   /* ** NOTE ** This function should be called if the Flags field of   */
   /*            the OTP_Channel_Data_t structure specifies the         */
   /*            OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL bit.                 */
   /* * NOTE * This function will optionally disconnect the channel if  */
   /*          the Flags field of the OTP_Channel_Data_t structure      */
   /*          specifies the OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL bit.  */
   /*          This bit SHOULD ONLY be used by the OTP Server if an     */
   /*          error has occured.  Otherwise, the OTP Client SHOULD     */
   /*          DISCONNECT the channel and the                           */
   /*          OTP_CHANNEL_FLAGS_CONNECTED bit will be cleared when the */
   /*          disconnect indication has been received from the OTP     */
   /*          Client.                                                  */
static void CleanupChannel(OTP_Channel_Data_t *ChannelData)
{
   /* Make sure the parameters are semi-valid.                          */
   if(ChannelData)
   {
      /* Check if we need to disconnect the channel prior to cleanup.   */
      /* * NOTE * Some data and flags will remain set if the disconnect */
      /*          flag is not set.  This way another transfer can be    */
      /*          started without closing the Object Transfer Channel.  */
      if(ChannelData->Flags & OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL)
      {
         /* Clear the disconnect channel flag.                          */
         ChannelData->Flags &= ~OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL;

         /* Disconnect the Object Transfer Channel.                     */
         /* * NOTE * If we received a etOTS_Channel_Close_Indication    */
         /*          disconnect event, then this function will fail     */
         /*          since we are already disconnected.  This way we    */
         /*          will still clear the necessary flags and fields    */
         /*          that have not been cleaned up even if the Object   */
         /*          Transfer Channel has already been disconnected.    */
         if(!OTS_Channel_Close_Connection(BluetoothStackID, ChannelData->Channel_ID))
         {
            /* Let the user know that a disconnection request has been  */
            /* sent.                                                    */
            printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", ChannelData->Channel_ID);
         }

         /* Flag that we are no longer connected over the Object        */
         /* Transfer Channel.                                           */
         ChannelData->Flags &= ~OTP_CHANNEL_FLAGS_CONNECTED;

         /* Clear the response requested flag.                          */
         ChannelData->Flags &= ~OTP_CHANNEL_FLAGS_RESPONSE_REQUESTED;

         /* Reset the Channel ID.                                       */
         ChannelData->Channel_ID = 0;

         /* Reset the Max SDU Size.                                     */
         ChannelData->Max_Data_Size = 0;
      }

      /* Clean up fields and flags used for the transfer.               */
      /* * NOTE * We will not clear the Channel ID in case the OTP      */
      /*          Client closes the Object Transfer Channel.  We will   */
      /*          also not clear the Max SDU Size.  This way we can     */
      /*          start another transfer without the OTP Client closing */
      /*          the channel.                                          */
      ChannelData->Current_Offset  = 0;
      ChannelData->Ending_Offset   = 0;
      ChannelData->Flags          &= ~(OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS | OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS);

      /* Free the transfer buffer if it is currently holding data to    */
      /* send over the Object Transfer Channel.                         */
      if((ChannelData->Buffer) && (ChannelData->Buffer_Length))
      {
         BTPS_FreeMemory(ChannelData->Buffer);
      }

      /* Make sure the transfer buffer is reset.                        */
      ChannelData->Buffer        = NULL;
      ChannelData->Buffer_Length = 0;

      /* Flag that we cleaned up the transfer buffer.                   */
      ChannelData->Flags &= ~(OTP_CHANNEL_FLAGS_BUFFER_READY_READY_TO_RECEIVE);

      /* If the file stream is open we need to close it.                */
      if(ChannelData->File_Descriptor)
      {
         /* Simply call the internal function to close the file stream. */
         CloseFileContents(&ChannelData->File_Descriptor);
      }

      /* Flag that we cleaned up the channel.                           */
      ChannelData->Flags &= ~OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;
   }
}

   /* The following function is a utility function to decode and display*/
   /* the OTS Feature data received in a read response from the OTP     */
   /* Server.                                                           */
static void DecodeDisplayOTSFeature(unsigned int ValueLength, Byte_t *Value)
{
   int                 Result;
   OTS_Feature_Data_t  OTSFeature;

   /* Make sure the parameters are semi-valid.                          */
   if((ValueLength) && (Value))
   {
      /* Decode the OTS Feature data.                                   */
      if((Result = OTS_Decode_OTS_Feature(ValueLength, Value, &OTSFeature)) == 0)
      {
         /* Simply call the internal function to display the data.      */
         DisplayOTSFeatureData(&OTSFeature);
      }
      else
         DisplayFunctionError("OTS_Decode_OACP_Response", Result);
   }
   else
      printf("\r\nInvalid parameter.\r\n");
}

   /* The following function is a utility function to decode and display*/
   /* the OTS Object Metadata received in a read response from the OTP  */
   /* Server.                                                           */
   /* ** NOTE ** We will NOT check the parameters since the OTS Object  */
   /*            Name may be an empty string (zero length).             */
   /* * NOTE * This function will store the OTS Object Metadata for the */
   /*          currently selected OTS Object on the OTP Server.         */
static void DecodeDisplayObjectMetadata(OTS_Client_Information_t *ClientInfo, Word_t AttributeHandle, unsigned int ValueLength, Byte_t *Value)
{
   int                         Result;
   OTS_Object_Metadata_Type_t  Type = RequestData.MetadataType;
   OTS_Object_Metadata_Data_t  Data;

   /* Set the OTS Object Metadata type based on the attribute handle    */
   /* sent in the request.                                              */
   /* * NOTE * This MUST be done in case multiple read requests are     */
   /*          dispatched.                                              */
   MapAttributeHandleToObjectMetadataType(ClientInfo, AttributeHandle, &Type);

   /* If this is a GATT Read Long response the internal buffer will be  */
   /* set from the first response.                                      */
   /* * NOTE * This MUST be set to zero when the first request has been */
   /*          sent for the OTS Object Name.  It should not be changed  */
   /*          unless we issue another request besides a GATT Read Long */
   /*          request.                                                 */
   if((Type == omtObjectName) && (RequestData.BufferLength))
   {
      /* Append the data received for the GATT Read Long response to the*/
      /* data we previously stored.                                     */
      BTPS_MemCopy((RequestData.Buffer + RequestData.BufferLength), Value, ValueLength);
      RequestData.BufferLength += ValueLength;

      /* Decode the OTS Object Metadata.                                */
      /* * NOTE * We will make sure the total OTS Object Name length is */
      /*          semi-valid.                                           */
      if((Result = OTS_Decode_Object_Metadata(RequestData.BufferLength, RequestData.Buffer, Type, &Data)) == 0)
      {
         /* Display the OTS Object Metadata.                            */
         printf("\r\nObject Metadata:\r\n");
         printf("   Request Type:  ");
         DisplayObjectMetadataType(Type);
         DisplayObjectMetadata(Type, &Data);

         /* Store the data for the current object.                      */
         BTPS_MemCopy((CurrentObject.Name + RequestData.BufferLength), Value, ValueLength);

         /* Insert the NULL terminator so we can display it as a        */
         /* c-string.                                                   */
         CurrentObject.Name[RequestData.BufferLength] = '\0';
      }
      else
         DisplayFunctionError("OTS_Decode_Object_Metadata", Result);
   }
   else
   {
      /* Decode the OTS Object Metadata.                                */
      if((Result = OTS_Decode_Object_Metadata(ValueLength, Value, Type, &Data)) == 0)
      {
         /* Display the OTS Object Metadata.                            */
         printf("\r\nObject Metadata:\r\n");
         printf("   Request Type:  ");
         DisplayObjectMetadataType(Type);
         DisplayObjectMetadata(Type, &Data);

         /* Determine if we need to store the requested data for the OTP*/
         /* Client.                                                     */
         switch(Type)
         {
            case omtObjectName:
               /* Store the OTS Object Name in the request buffer in    */
               /* case we need to issue a GATT Read Long request for the*/
               /* rest of the OTS Object Name.                          */
               BTPS_MemCopy(RequestData.Buffer, Data.Name.Buffer, Data.Name.Buffer_Length);
               RequestData.BufferLength = Data.Name.Buffer_Length;

               /* Store the data for the current object.                */
               BTPS_MemCopy(CurrentObject.Name, Data.Name.Buffer, Data.Name.Buffer_Length);

               /* Insert the NULL terminator so we can display it as a  */
               /* c-string.                                             */
               CurrentObject.Name[Data.Name.Buffer_Length] = '\0';
               break;
            case omtObjectSize:
               /* Store the data for the current object.                */
               CurrentObject.CurrentSize = Data.Size.Current_Size;
               break;
            case omtObjectID:
               /* Store the data for the current object.                */
               CurrentObject.ObjectID    = Data.ID;
               break;
            default:
               /* Prevent compiler warnings.                            */
               break;
         }
      }
      else
         DisplayFunctionError("OTS_Decode_Object_Metadata", Result);
   }
}

   /* The following function is a utility function to decode and display*/
   /* the Object Action Control Point response data received via an     */
   /* indication from the OTP Server.                                   */
   /* ** NOTE ** This function will set the flags for the Object        */
   /*            Transfer Channel based on the received response.       */
static void DecodeDisplayOACPResponseData(unsigned int ValueLength, Byte_t *Value, OTP_Channel_Data_t *ChannelData)
{
   int                       Result;
   OTS_OACP_Response_Data_t  ResponseData;

   /* Make sure the parameters are semi-valid.                          */
   if((ValueLength) && (Value))
   {
      /* Decode the OACP Response data.                                 */
      if((Result = OTS_Decode_OACP_Response(ValueLength, Value, &ResponseData)) == 0)
      {
         /* Simply call the internal function to display the data.      */
         DisplayOACPResponseData(&ResponseData);

         /* Determine if we need to start the transfer for the OACP     */
         /* Write Procedure.                                            */
         if(ResponseData.Request_Op_Code == oacpWrite)
         {
            /* If the write procedure is successful.                    */
            if(ResponseData.Result_Code == oarSuccess)
            {
               /* Flag that the OACP Write Procedure is in progress.    */
               ChannelData->Flags |= OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS;
            }
            else
            {
               /* Flag that we need to perform cleanup for the Channel. */
               ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;
            }
         }

         /* Determine if a transfer is in progress for the OACP Read    */
         /* Procedure..                                                 */
         if((ResponseData.Request_Op_Code == oacpRead) && (ResponseData.Result_Code == oarSuccess))
         {
            /* Flag that the OACP Read Procedure is in progress.        */
            ChannelData->Flags |= OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS;
         }

         /* Determine if a new OTS Object has been selected as the      */
         /* current object.                                             */
         if((ResponseData.Request_Op_Code == oacpCreate) && (ResponseData.Result_Code == oarSuccess))
         {
            /* We will flag that the current object is selected.        */
            CurrentObject.Valid = TRUE;

            /* Simply call the internal function to read it's Object    */
            /* Metadata.                                                */
            ReadObjectMetadata();
         }

         /* Determine if the delete procedure was successful.           */
         if((ResponseData.Request_Op_Code == oacpDelete) && (ResponseData.Result_Code == oarSuccess))
         {
            /* We will flag that the current object is no longer valid. */
            CurrentObject.Valid = FALSE;
         }

         /* Determine if the abort procedure was successful.            */
         if((ResponseData.Request_Op_Code == oacpAbort) && (ResponseData.Result_Code == oarSuccess))
         {
            /* Make sure a Read Procedure is in progress.               */
            if(ChannelData->Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)
            {
               /* Flag that the OACP Read Procedure is no longer in     */
               /* progress.                                             */
               ChannelData->Flags &= ~(OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS);

               /* Flag that we need to perform cleanup for the Channel. */
               ChannelData->Flags |= OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL;
            }
            else
               printf("\r\nWarning - Read Procedure is not in progress.\r\n");
         }
      }
      else
         DisplayFunctionError("OTS_Decode_OACP_Response", Result);
   }
   else
      printf("\r\nInvalid parameter.\r\n");
}

   /* The following function is a utility function to decode and display*/
   /* the Object List Control Point response data received via an       */
   /* indication from the OTP Server.                                   */
   /* * NOTE * This function will dispatch GATT Read requests for the   */
   /*          current object's Object Metadata if a new current Object */
   /*          has been selected via an OLCP Procedure..                */
static void DecodeDisplayOLCPResponseData(unsigned int ValueLength, Byte_t *Value)
{
   int                       Result;
   OTS_OLCP_Response_Data_t  ResponseData;

   /* Make sure the parameters are semi-valid.                          */
   if((ValueLength) && (Value))
   {
      /* Decode the OLCP Response data.                                 */
      if((Result = OTS_Decode_OLCP_Response(ValueLength, Value, &ResponseData)) == 0)
      {
         /* Simply call the internal function to display the data.      */
         DisplayOLCPResponseData(&ResponseData);

         /* Determine if a new OTS Object has been selected as the      */
         /* current object.                                             */
         if((ResponseData.Request_Op_Code >= olcpFirst) && (ResponseData.Request_Op_Code <= olcpGoTo) && (ResponseData.Result_Code == olrSuccess))
         {
            /* We will flag that the current object is selected.        */
            CurrentObject.Valid = TRUE;

            /* Simply call the internal function to read it's Object    */
            /* Metadata.                                                */
            ReadObjectMetadata();
         }
      }
      else
         DisplayFunctionError("OTS_Decode_OLCP_Response", Result);
   }
   else
      printf("\r\nInvalid parameter.\r\n");
}

   /* The following function is a utility function to decode and display*/
   /* the OTS Object List Filter data received in a read response from  */
   /* the OTP Server.                                                   */
static void DecodeDisplayObjectListFilterData(unsigned int ValueLength, Byte_t *Value)
{
   int                            Result;
   OTS_Object_List_Filter_Data_t  ListFilterData;

   /* Make sure the parameters are semi-valid.                          */
   if((ValueLength) && (Value))
   {
      /* If this is a GATT Read Long response the internal buffer we    */
      /* stored the first response will be set.                         */
      /* * NOTE * This MUST be set to zero when the first request has   */
      /*          been sent for the OTS Object Name.  It should not be  */
      /*          changed unless we issue another request besides a GATT*/
      /*          Read Long request.                                    */
      if(RequestData.BufferLength)
      {
         /* Append the data received for the GATT Read Long response to */
         /* the data we previously stored.                              */
         BTPS_MemCopy((RequestData.Buffer + RequestData.BufferLength), Value, ValueLength);
         RequestData.BufferLength += ValueLength;

         /* Decode the OLCP Response data.                              */
         if((Result = OTS_Decode_Object_List_Filter_Data(RequestData.BufferLength, RequestData.Buffer, &ListFilterData)) == 0)
         {
            /* Simply call the internal function to display the data.   */
            DisplayObjectListFilterData(&ListFilterData);
         }
         else
            DisplayFunctionError("OTS_Decode_Object_List_Filter_Data", Result);
      }
      else
      {
         /* Decode the OLCP Response data.                              */
         if((Result = OTS_Decode_Object_List_Filter_Data(ValueLength, Value, &ListFilterData)) == 0)
         {
            /* Simply call the internal function to display the data.   */
            DisplayObjectListFilterData(&ListFilterData);

            /* If the type is for an OTS Object Name Filter type.       */
            if((ListFilterData.Type >= lftNameStartsWith) && (ListFilterData.Type <= lftNameIsExactly))
            {
               /* Store the OTS Object List Filter data in the request  */
               /* buffer in case we need to issue a GATT Read Long      */
               /* request for the rest of the OTS Object List Filter.   */
               BTPS_MemCopy(RequestData.Buffer, Value, ValueLength);
               RequestData.BufferLength = ValueLength;
            }
         }
         else
            DisplayFunctionError("OTS_Decode_Object_List_Filter_Data", Result);
      }
   }
   else
      printf("\r\nInvalid parameter.\r\n");
}

   /* The following function is a utility function to decode and display*/
   /* the Object Changed data received via an indication from the OTP   */
   /* Server.                                                           */
static void DecodeDisplayObjectChangedData(unsigned int ValueLength, Byte_t *Value)
{
   int                        Result;
   OTS_Object_Changed_Data_t  ObjectChangedData;

   /* Make sure the parameters are semi-valid.                          */
   if((ValueLength) && (Value))
   {
      /* Decode the Object Changed data.                                */
      if((Result = OTS_Decode_Object_Changed_Data(ValueLength, Value, &ObjectChangedData)) == 0)
      {
         /* Simply call the internal function to display the data.      */
         DisplayObjectChangedData(&ObjectChangedData);

         /* If the currently selected OTS Object's Metadata has changed.*/
         if((ObjectChangedData.Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED) && (CurrentObject.ObjectID.Lower == ObjectChangedData.Object_ID.Lower) && (CurrentObject.ObjectID.Upper == ObjectChangedData.Object_ID.Upper))
         {
            /* Simply call the internal function to read it's Object    */
            /* Metadata.                                                */
            ReadObjectMetadata();
         }

         /* If the currently selected OTS Object has changed.           */
         /* * NOTE * This sample application OTP Server has the ability */
         /*          to create file objects with CreateFileObject().    */
         /*          This function will automatically set the new OTS   */
         /*          Object as the current object so we need to handle  */
         /*          the change.                                        */
         if((ObjectChangedData.Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_CREATION) &&
            (ObjectChangedData.Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED) &&
            (ObjectChangedData.Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_CONTENTS_CHANGED))
         {
            /* We will flag that the current object is valid.           */
            /* * NOTE * It may not be valid if the OTP Client has not   */
            /*          selected a current object.                      */
            CurrentObject.Valid = TRUE;

            /* Simply call the internal function to read it's Object    */
            /* Metadata.                                                */
            ReadObjectMetadata();
         }

         /* If the currently selected OTS Object has been deleted.      */
         if((ObjectChangedData.Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_DELETION) && (CurrentObject.ObjectID.Lower == ObjectChangedData.Object_ID.Lower) && (CurrentObject.ObjectID.Upper == ObjectChangedData.Object_ID.Upper))
         {
            /* We will flag that the current object is no longer valid. */
            CurrentObject.Valid = FALSE;
         }
      }
      else
         DisplayFunctionError("OTS_Decode_Object_Changed_Data", Result);
   }
   else
      printf("\r\nInvalid parameter.\r\n");
}

   /* Display the OTS Feature data.                                     */
static void DisplayOTSFeatureData(OTS_Feature_Data_t *OTSFeature)
{
   printf("\r\nOTS Feature:\r\n");
   printf("   \r\nOACP Features:  0x%08X\r\n", OTSFeature->OACP_Features);
   printf("      Create Op Code:              %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_CREATE_OP_CODE_SUPPORTED)             ? "Supported" : "Not supported");
   printf("      Delete Op Code:              %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_DELETE_OP_CODE_SUPPORTED)             ? "Supported" : "Not supported");
   printf("      Calculate Checksum Op Code:  %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_CALCULATE_CHECKSUM_OP_CODE_SUPPORTED) ? "Supported" : "Not supported");
   printf("      Execute Op Code:             %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_EXECUTE_OP_CODE_SUPPORTED)            ? "Supported" : "Not supported");
   printf("      Read Op Code:                %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_READ_OP_CODE_SUPPORTED)               ? "Supported" : "Not supported");
   printf("      Write Op Code:               %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_WRITE_OP_CODE_SUPPORTED)              ? "Supported" : "Not supported");
   printf("      Appending Op Code:           %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_APPENDING_SUPPORTED)                  ? "Supported" : "Not supported");
   printf("      Truncation Op Code:          %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_TRUNCATION_SUPPORTED)                 ? "Supported" : "Not supported");
   printf("      Patching Op Code:            %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_PATCHING_SUPPORTED)                   ? "Supported" : "Not supported");
   printf("      Abort Op Code:               %s\r\n", (OTSFeature->OACP_Features & OTS_FEATURE_OACP_ABORT_OP_CODE_SUPPORTED)              ? "Supported" : "Not supported");
   printf("   \r\nOLCP Features:  0x%08X.\r\n", OTSFeature->OLCP_Features);
   printf("      Go To Op Code:                      %s\r\n", (OTSFeature->OLCP_Features & OTS_FEATURE_OLCP_GO_TO_OP_CODE_SUPPORTED)                     ? "Supported" : "Not supported");
   printf("      Order Op Code:                      %s\r\n", (OTSFeature->OLCP_Features & OTS_FEATURE_OLCP_ORDER_OP_CODE_SUPPORTED)                     ? "Supported" : "Not supported");
   printf("      Request Number Of Objects Op Code:  %s\r\n", (OTSFeature->OLCP_Features & OTS_FEATURE_OLCP_REQUEST_NUMBER_OF_OBJECTS_OP_CODE_SUPPORTED) ? "Supported" : "Not supported");
   printf("      Clear Marking Op Code:              %s\r\n", (OTSFeature->OLCP_Features & OTS_FEATURE_OLCP_CLEAR_MARKING_OP_CODE_SUPPORTED)             ? "Supported" : "Not supported");
}

   /* The following function displays the OTS Object Metadata Type.     */
static void DisplayObjectMetadataType(OTS_Object_Metadata_Type_t Type)
{
   /* Determine the type.                                               */
   switch(Type)
   {
      case omtObjectName:
         printf("omtObjectName.\r\n");
         break;
      case omtObjectType:
         printf("omtObjectType.\r\n");
         break;
      case omtObjectSize:
         printf("omtObjectSize.\r\n");
         break;
      case omtObjectFirstCreated:
         printf("omtObjectFirstCreated.\r\n");
         break;
      case omtObjectLastModified:
         printf("omtObjectLastModified.\r\n");
         break;
      case omtObjectID:
         printf("omtObjectID.\r\n");
         break;
      case omtObjectProperties:
         printf("omtObjectProperties.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }
}

   /* The following function is responsible for displaying the OTS      */
   /* Object Metadata.                                                  */
static void DisplayObjectMetadata(OTS_Object_Metadata_Type_t Type, OTS_Object_Metadata_Data_t *Metadata)
{
   char temp[OTS_MAXIMUM_OBJECT_NAME_LENGTH+1];

   if(Metadata)
   {
      /* Determine how to display the data based on the type.           */
      switch(Type)
      {
         case omtObjectName:
            /* Copy the string.                                         */
            BTPS_MemCopy(temp, Metadata->Name.Buffer, Metadata->Name.Buffer_Length);

            /* Insert the NULL terminator so we can display it as a     */
            /* c-string.                                                */
            temp[Metadata->Name.Buffer_Length] = '\0';

            /* Display the name.                                        */
            printf("   Name:          \"%s\".\r\n", temp);
            break;
         case omtObjectType:
            /* Simply display the UUID.                                 */
            printf("   Type:          0x");
            DisplayUUID(&(Metadata->Type));
            break;
         case omtObjectSize:
            printf("   Size:\r\n");
            printf("      Current:    %u.\r\n", Metadata->Size.Current_Size);
            printf("      Allocated:  %u.\r\n", Metadata->Size.Allocated_Size);
            break;
         case omtObjectFirstCreated:
            printf("   First Created:\r\n");
            printf("      Year:       %u.\r\n", Metadata->First_Created.Year);
            printf("      Month:      %u.\r\n", Metadata->First_Created.Month);
            printf("      Day:        %u.\r\n", Metadata->First_Created.Day);
            printf("      Hour:       %u.\r\n", Metadata->First_Created.Hours);
            printf("      Minute:     %u.\r\n", Metadata->First_Created.Minutes);
            printf("      Seconds:    %u.\r\n", Metadata->First_Created.Seconds);
            break;
         case omtObjectLastModified:
            printf("   Last Modified:\r\n");
            printf("      Year:       %u.\r\n", Metadata->Last_Modified.Year);
            printf("      Month:      %u.\r\n", Metadata->Last_Modified.Month);
            printf("      Day:        %u.\r\n", Metadata->Last_Modified.Day);
            printf("      Hour:       %u.\r\n", Metadata->Last_Modified.Hours);
            printf("      Minute:     %u.\r\n", Metadata->Last_Modified.Minutes);
            printf("      Seconds:    %u.\r\n", Metadata->Last_Modified.Seconds);
            break;
         case omtObjectID:
            /* * NOTE * This sample applilcation only uses the lower    */
            /*          portion of the OTS Object ID.                   */
            printf("   ID:            0x%08X\r\n", Metadata->ID.Lower);
            break;
         case omtObjectProperties:
            printf("   Properties:    0x%08X\r\n", Metadata->Properties);
            printf("      Delete:     %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_DELETE)   ? "Supported" : "Not supported");
            printf("      Execute:    %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_EXECUTE)  ? "Supported" : "Not supported");
            printf("      Read:       %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_READ)     ? "Supported" : "Not supported");
            printf("      Write:      %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_WRITE)    ? "Supported" : "Not supported");
            printf("      Append:     %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_APPEND)   ? "Supported" : "Not supported");
            printf("      Truncate:   %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_TRUNCATE) ? "Supported" : "Not supported");
            printf("      Patch:      %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_PATCH)    ? "Supported" : "Not supported");
            printf("      Mark:       %s.\r\n", (Metadata->Properties & OTS_OBJECT_PROPERTIES_MARK)     ? "Supported" : "Not supported");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* Display the OACP request data.                                    */
static void DisplayOACPRequestData(OTS_OACP_Request_Data_t *RequestData)
{
   if(RequestData)
   {
      printf("\r\nObject Action Control Point (OACP) Request:\r\n");

      /* Display the request op code and any parameters.                */
      printf("   Request type:  ");
      switch(RequestData->Request_Op_Code)
      {
         case oacpCreate:
            printf("oacpCreate.\r\n");
            printf("   Size:          %u.\r\n", RequestData->Parameter.Create_Data.Size);
            printf("   Type:          0x");
            DisplayUUID(&(RequestData->Parameter.Create_Data.UUID));
            break;
         case oacpDelete:
            printf("oacpDelete.\r\n");
            break;
         case oacpCalculateChecksum:
            printf("oacpCalculateChecksum.\r\n");
            printf("   Offset:        %u.\r\n", RequestData->Parameter.Calculate_Checksum_Data.Offset);
            printf("   Length:        %u.\r\n", RequestData->Parameter.Calculate_Checksum_Data.Length);
            break;
         case oacpExecute:
            /* * NOTE * This sample application does nothing for the    */
            /*          execute procedure.                              */
            printf("oacpExecute.\r\n");
            break;
         case oacpRead:
            printf("oacpRead.\r\n");
            printf("   Offset:        %u.\r\n", RequestData->Parameter.Read_Data.Offset);
            printf("   Length:        %u.\r\n", RequestData->Parameter.Read_Data.Length);
            break;
         case oacpWrite:
            printf("oacpWrite.\r\n");
            printf("   Offset:        %u.\r\n", RequestData->Parameter.Write_Data.Offset);
            printf("   Length:        %u.\r\n", RequestData->Parameter.Write_Data.Length);
            printf("   Mode:          0x%02X.\r\n", RequestData->Parameter.Write_Data.Mode);
            printf("      Truncate:   %s.\r\n", (RequestData->Parameter.Write_Data.Mode & OTS_OACP_WRITE_MODE_TRUNCATE) ? "Supported" : "Not supported");
            break;
         case oacpAbort:
            printf("oacpAbort.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* Display the OACP response data.                                   */
static void DisplayOACPResponseData(OTS_OACP_Response_Data_t *ResponseData)
{
   printf("\r\nObject Action Control Point (OACP) Response:\r\n");

   /* Display the request op code and any parameters.                   */
   printf("   Request type:  ");
   switch(ResponseData->Request_Op_Code)
   {
      case oacpCreate:
         printf("oacpCreate.\r\n");
         break;
      case oacpDelete:
         printf("oacpDelete.\r\n");
         break;
      case oacpCalculateChecksum:
         printf("oacpCalculateChecksum.\r\n");
         break;
      case oacpExecute:
         /* * NOTE * This sample application does nothing for the       */
         /*          execute procedure.                                 */
         printf("oacpExecute.\r\n");
         break;
      case oacpRead:
         printf("oacpRead.\r\n");
         break;
      case oacpWrite:
         printf("oacpWrite.\r\n");
         break;
      case oacpAbort:
         printf("oacpAbort.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }

   /* Display the result code.                                          */
   printf("   Result code:   ");
   switch(ResponseData->Result_Code)
   {
      case oarSuccess:
         printf("Success.\r\n");
         break;
      case oarOpcodeNotSupported:
         printf("Op Code Not Supported.\r\n");
         break;
      case oarInvalidParameter:
         printf("Invalid Parameter.\r\n");
         break;
      case oarInsufficientResources:
         printf("Insufficient Resources.\r\n");
         break;
      case oarInvalidObject:
         printf("Invalid Object.\r\n");
         break;
      case oarChannelUnavailable:
         printf("Channel Unavailable.\r\n");
         break;
      case oarUnsupportedType:
         printf("Unsupported Type.\r\n");
         break;
      case oarProcedureNotPermitted:
         printf("Procedure Not Permitted.\r\n");
         break;
      case oarObjectLocked:
         printf("Object Locked.\r\n");
         break;
      case oarOperationFailed:
         printf("Operation Failed.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }

   /* Display any attached parameters.                                  */
   if((ResponseData->Result_Code == oarSuccess) && (ResponseData->Request_Op_Code == oacpCalculateChecksum))
   {
      printf("   Parameters:\r\n");
      printf("      Checksum:   0x%08X.\r\n", ResponseData->Parameter.Checksum);
   }
}

   /* Display the OLCP request data.                                    */
static void DisplayOLCPRequestData(OTS_OLCP_Request_Data_t *RequestData)
{
   if(RequestData)
   {
      printf("\r\nObject List Control Point (OLCP) Request:\r\n");

      /* Display the request op code and any parameters.                */
      printf("   Request type:  ");
      switch(RequestData->Request_Op_Code)
      {
         case olcpFirst:
            printf("olcpFirst.\r\n");
            break;
         case olcpLast:
            printf("olcpLast.\r\n");
            break;
         case olcpPrevious:
            printf("olcpPrevious.\r\n");
            break;
         case olcpNext:
            printf("olcpNext.\r\n");
            break;
         case olcpGoTo:
            printf("olcpGoTo.\r\n");
            /* * NOTE * This sample applilcation only uses the lower    */
            /*          portion of the OTS Object ID.                   */
            printf("   ID:            0x%08X\r\n", RequestData->Parameter.Object_ID.Lower);
            break;
         case olcpOrder:
            printf("olcpOrder.\r\n");
            printf("   Type:          ");
            switch(RequestData->Parameter.List_Sort_Order)
            {
               case lstOrderAscendingByName:
                  printf("lstOrderAscendingByName.\r\n");
                  break;
               case lstOrderAscendingByObjectType:
                  printf("lstOrderAscendingByObjectType.\r\n");
                  break;
               case lstOrderAscendingByObjectCurrentSize:
                  printf("lstOrderAscendingByObjectCurrentSize.\r\n");
                  break;
               case lstOrderAscendingByObjectFirstCreated:
                  printf("lstOrderAscendingByObjectFirstCreated.\r\n");
                  break;
               case lstOrderAscendingByObjectLastModified:
                  printf("lstOrderAscendingByObjectLastModified.\r\n");
                  break;
               case lstOrderDescendingByName:
                  printf("lstOrderDescendingByName.\r\n");
                  break;
               case lstOrderDescendingByObjectType:
                  printf("lstOrderDescendingByObjectType.\r\n");
                  break;
               case lstOrderDescendingByObjectCurrentSize:
                  printf("lstOrderDescendingByObjectCurrentSize.\r\n");
                  break;
               case lstOrderDescendingByObjectFirstCreated:
                  printf("lstOrderDescendingByObjectFirstCreated.\r\n");
                  break;
               case lstOrderDescendingByObjectLastModified:
                  printf("lstOrderDescendingByObjectLastModified.\r\n");
                  break;
               default:
                  printf("Invalid.\r\n");
                  break;
            }
            break;
         case olcpRequestNumberOfObjects:
            printf("olcpRequestNumberOfObjects.\r\n");
            break;
         case olcpClearMarking:
            printf("olcpClearMarking.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* Display the OLCP Response data.                                   */
static void DisplayOLCPResponseData(OTS_OLCP_Response_Data_t *ResponseData)
{
   printf("\r\nObject List Control Point (OLCP) Response:\r\n");

   /* Display the request op code and any parameters.                   */
   printf("   Request type:  ");
   switch(ResponseData->Request_Op_Code)
   {
      case olcpFirst:
         printf("olcpFirst.\r\n");
         break;
      case olcpLast:
         printf("olcpLast.\r\n");
         break;
      case olcpPrevious:
         printf("olcpPrevious.\r\n");
         break;
      case olcpNext:
         printf("olcpNext.\r\n");
         break;
      case olcpGoTo:
         printf("olcpGoTo.\r\n");
         break;
      case olcpOrder:
         printf("olcpOrder.\r\n");
         break;
      case olcpRequestNumberOfObjects:
         printf("olcpRequestNumberOfObjects.\r\n");
         break;
      case olcpClearMarking:
         printf("olcpClearMarking.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }

   /* Display the result code.                                          */
   printf("   Result code:   ");
   switch(ResponseData->Result_Code)
   {
      case olrSuccess:
         printf("Success.\r\n");
         break;
      case olrOpcodeNotSupported:
         printf("Op Code Not Supported.\r\n");
         break;
      case olrInvalidParameter:
         printf("Invalid Parameter.\r\n");
         break;
      case olrOperationFailed:
         printf("Operation Failed.\r\n");
         break;
      case olrOutOfBounds:
         printf("Out of Bounds.\r\n");
         break;
      case olrTooManyObjects:
         printf("Too Many Objects.\r\n");
         break;
      case olrNoObject:
         printf("No Object.\r\n");
         break;
      case olrObjectIDNotFound:
         printf("Object ID Not Found.\r\n");
         break;
      default:
         printf("Invalid.\r\n");
         break;
   }

   /* Display any attached parameters.                                  */
   if(ResponseData->Request_Op_Code == olcpRequestNumberOfObjects)
   {
      printf("   Parameters:\r\n");
      printf("      Objects:    %u.\r\n", ResponseData->Parameter.Total_Number_Of_Objects);
   }
}

   /* The following function is responsible for displaying the OTS      */
   /* Object List Filter data.                                          */
static void DisplayObjectListFilterData(OTS_Object_List_Filter_Data_t *ListFilterData)
{
   char temp[OTS_MAXIMUM_OBJECT_NAME_LENGTH+1];

   if(ListFilterData)
   {
      printf("\r\nObject List Filter:\r\n");
      printf("   Type:         ");

      /* Determine the OTS Object List Filter type and any parameters.  */
      switch(ListFilterData->Type)
      {
         case lftNoFilter:
            printf("lftNoFilter.\r\n");
            break;
         case lftNameStartsWith:
            printf("lftNameStartsWith.\r\n");

            /* Copy the string.                                         */
            BTPS_MemCopy(temp, ListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer_Length);

            /* Insert the NULL terminator so we can display it as a     */
            /* c-string.                                                */
            temp[ListFilterData->Data.Name.Buffer_Length] = '\0';

            /* Display the name.                                        */
            printf("   Name:         \"%s\".\r\n", temp);
            printf("   Length:       %u.\r\n", ListFilterData->Data.Name.Buffer_Length);
            break;
         case lftNameEndsWith:
            printf("lftNameEndsWith.\r\n");

            /* Copy the string.                                         */
            BTPS_MemCopy(temp, ListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer_Length);

            /* Insert the NULL terminator so we can display it as a     */
            /* c-string.                                                */
            temp[ListFilterData->Data.Name.Buffer_Length] = '\0';

            /* Display the name.                                        */
            printf("   Name:         \"%s\".\r\n", temp);
            printf("   Length:       %u.\r\n", ListFilterData->Data.Name.Buffer_Length);
            break;
         case lftNameContains:
            printf("lftNameContains.\r\n");

            /* Copy the string.                                         */
            BTPS_MemCopy(temp, ListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer_Length);

            /* Insert the NULL terminator so we can display it as a     */
            /* c-string.                                                */
            temp[ListFilterData->Data.Name.Buffer_Length] = '\0';

            /* Display the name.                                        */
            printf("   Name:         \"%s\".\r\n", temp);
            printf("   Length:       %u.\r\n", ListFilterData->Data.Name.Buffer_Length);
            break;
         case lftNameIsExactly:
            printf("lftNameIsExactly.\r\n");

            /* Copy the string.                                         */
            BTPS_MemCopy(temp, ListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer_Length);

            /* Insert the NULL terminator so we can display it as a     */
            /* c-string.                                                */
            temp[ListFilterData->Data.Name.Buffer_Length] = '\0';

            /* Display the name.                                        */
            printf("   Name:         \"%s\".\r\n", temp);
            printf("   Length:       %u.\r\n", ListFilterData->Data.Name.Buffer_Length);
            break;
         case lftObjectType:
            printf("lftObjectType.\r\n");

            /* Simply display the UUID.                                 */
            printf("   Type:  0x");
            DisplayUUID(&(ListFilterData->Data.Type));
            break;
         case lftCreatedBetween:
            printf("lftCreatedBetween.\r\n");
            printf("   Minimum:\r\n");
            printf("      Year:      %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Year);
            printf("      Month:     %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Month);
            printf("      Day:       %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Day);
            printf("      Hour:      %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Hours);
            printf("      Minute:    %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Minutes);
            printf("      Seconds:   %u.\r\n\r\n", ListFilterData->Data.Time_Range.Minimum.Seconds);

            printf("   Maximum:\r\n");
            printf("      Year:      %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Year);
            printf("      Month:     %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Month);
            printf("      Day:       %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Day);
            printf("      Hour:      %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Hours);
            printf("      Minute:    %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Minutes);
            printf("      Seconds:   %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Seconds);
            break;
         case lftModifiedBetween:
            printf("lftModifiedBetween.\r\n");
            printf("   Minimum:\r\n");
            printf("      Year:      %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Year);
            printf("      Month:     %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Month);
            printf("      Day:       %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Day);
            printf("      Hour:      %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Hours);
            printf("      Minute:    %u.\r\n", ListFilterData->Data.Time_Range.Minimum.Minutes);
            printf("      Seconds:   %u.\r\n\r\n", ListFilterData->Data.Time_Range.Minimum.Seconds);

            printf("   Maximum:\r\n");
            printf("      Year:      %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Year);
            printf("      Month:     %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Month);
            printf("      Day:       %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Day);
            printf("      Hour:      %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Hours);
            printf("      Minute:    %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Minutes);
            printf("      Seconds:   %u.\r\n", ListFilterData->Data.Time_Range.Maximum.Seconds);
            break;
         case lftCurrentSizeBetween:
            printf("lftCurrentSizeBetween.\r\n");
            printf("   Minimum:      %u.\r\n", ListFilterData->Data.Size_Range.Minimum);
            printf("   Maximum:      %u.\r\n", ListFilterData->Data.Size_Range.Maximum);
            break;
         case lftAllocatedSizeBetween:
            printf("lftAllocatedSizeBetween.\r\n");
            printf("   Minimum:      %u.\r\n", ListFilterData->Data.Size_Range.Minimum);
            printf("   Maximum:      %u.\r\n", ListFilterData->Data.Size_Range.Maximum);
            break;
         case lftMarkedObjects:
            printf("lftMarkedObjects.\r\n");
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* The following function displays the Object Changed data.          */
static void DisplayObjectChangedData(OTS_Object_Changed_Data_t *ObjectChangedData)
{
   Byte_t Flags = ObjectChangedData->Flags;

   /* Display the Changed Data.                                         */
   /* * NOTE * We will only display the lower portion of the Object ID  */
   /*          since this sample only uses the lower portion.           */
   printf("\r\nObject Changed data.\r\n");
   printf("   Object ID:         0x%08X\r\n", ObjectChangedData->Object_ID.Lower);
   printf("   Source of Change:  %s.\r\n",  (Flags & OTS_OBJECT_CHANGED_FLAGS_SOURCE_OF_CHANGE_CLIENT) ? "Client" : "Server");
   printf("   Contents Changed:  %s.\r\n",  (Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_CONTENTS_CHANGED) ? "TRUE" : "FALSE");
   printf("   Metadata Changed:  %s.\r\n",  (Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED) ?  "TRUE" : "FALSE");
   printf("   Creation:          %s.\r\n",  (Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_CREATION) ?  "TRUE" : "FALSE");
   printf("   Deletion:          %s.\r\n",  (Flags & OTS_OBJECT_CHANGED_FLAGS_OBJECT_DELETION) ?  "TRUE" : "FALSE");
}

   /* The following function displays the OTS Object data.              */
static void DisplayObjectData(OTS_Object_Data_t *ObjectDataPtr)
{
   if(ObjectDataPtr)
   {
      printf("   Flags:          0x%02X\r\n", ObjectDataPtr->Flags);
      printf("   Name:           \"%s\"\r\n", ObjectDataPtr->Name.Buffer);
      printf("   Type:           0x");
      DisplayUUID(&(ObjectDataPtr->Type));
      printf("   Size:\r\n");
      printf("      Current:     %u\r\n", ObjectDataPtr->Size.Current_Size);
      printf("      Allocated:   %u\r\n", ObjectDataPtr->Size.Allocated_Size);
      printf("   First Created:  %04u/%02u/%02u %02u:%02u:%02u\r\n", ObjectDataPtr->First_Created.Year, ObjectDataPtr->First_Created.Month, ObjectDataPtr->First_Created.Day, ObjectDataPtr->First_Created.Hours, ObjectDataPtr->First_Created.Minutes, ObjectDataPtr->First_Created.Seconds);
      printf("   Last Modified:  %04u/%02u/%02u %02u:%02u:%02u\r\n", ObjectDataPtr->Last_Modified.Year, ObjectDataPtr->Last_Modified.Month, ObjectDataPtr->Last_Modified.Day, ObjectDataPtr->Last_Modified.Hours, ObjectDataPtr->Last_Modified.Minutes, ObjectDataPtr->Last_Modified.Seconds);
      printf("   ID:             0x%08X\r\n", ObjectDataPtr->ID.Lower);
      printf("   Properties:     0x%08X\r\n", ObjectDataPtr->Properties);
      printf("      Delete:      %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_DELETE) ? "Supported" : "Not supported");
      printf("      Execute:     %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_EXECUTE) ? "Supported" : "Not supported");
      printf("      Read:        %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_READ) ? "Supported" : "Not supported");
      printf("      Write:       %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_WRITE) ? "Supported" : "Not supported");
      printf("      Append:      %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_APPEND) ? "Supported" : "Not supported");
      printf("      Truncate:    %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_TRUNCATE) ? "Supported" : "Not supported");
      printf("      Patch:       %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_PATCH) ? "Supported" : "Not supported");
      printf("      Mark:        %s\r\n", (ObjectDataPtr->Properties & OTS_OBJECT_PROPERTIES_MARK) ? "Supported" : "Not supported");
      printf("   Marked:         %s\r\n", (ObjectDataPtr->Marked) ? "Yes" : "No");
   }
}

   /* The following command is responsible for registering OTS.         */
static int RegisterOTSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(RegisterOTS());
}

   /* The following command is responsible for un-registering OTS.      */
static int UnregisterOTSCommand(ParameterList_t *TempParam)
{
   /* Simply call and return the internal function.                     */
   return(UnregisterOTS());
}

   /* The following command is responsible for discovering attribute    */
   /* handles for OTS on a remote OTP Server.                           */
static int DiscoverOTSCommand(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;

   /* Verify that we are not configured as the OTP Server               */
   if(!OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that no service discovery is outstanding for this */
            /* device.                                                  */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
            {
               /* Configure the filter so that only the OTS Service is  */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               OTS_ASSIGN_OTS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, DeviceInfo->ConnectionID, (sizeof(UUID) / sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdOTS);
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
            printf("\r\nUnknown OTP Server.\r\n");
      }
      else
         printf("\r\nMust be connected to an OTP Server.\r\n");
   }
   else
      printf("\r\nOnly an OTP Client can discover OTS.\r\n");

   return(ret_val);
}

   /* The following command is responsible for discovering all services.*/
static int DiscoverAllServicesCommand(ParameterList_t *TempParam)
{
   int           ret_val = 0;
   DeviceInfo_t *DeviceInfo;

   /* Verify that we are not configured as the OTP Server               */
   if(!OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that no service discovery is outstanding for this */
            /* device.                                                  */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
            {
               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, DeviceInfo->ConnectionID, 0, NULL, GATT_Service_Discovery_Event_Callback, sdAll);
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
            printf("\r\nUnknown OTP Server.\r\n");
      }
      else
         printf("\r\nMust be connected to an OTP Server.\r\n");
   }
   else
      printf("\r\nOnly an OTP Client can discover OTS.\r\n");

   return(ret_val);
}

   /* The following command is responsible for getting the connection   */
   /* mode for the Object Transfer Channel.                             */
static int GetConnectionModeCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   GetConnectionMode();

   return(0);
}

   /* The following command is responsible for setting the connection   */
   /* mode for the Object Transfer Channel.                             */
static int SetConnectionModeCommand(ParameterList_t *TempParam)
{
   int                            ret_val = 0;
   OTS_Channel_Connection_Mode_t  Mode;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the parameter.                                           */
      Mode = (OTS_Channel_Connection_Mode_t)TempParam->Params[0].intParam;

      /* Simply call the internal function.                             */
      SetConnectionMode(Mode);
   }
   else
      DisplaySetConnectionModeUsage();

   return(ret_val);
}

   /* The following command is responsible for setting the connection   */
   /* parameters for the Object Transfer Channel.                       */
static int SetParametersCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;
   L2CA_LE_Channel_Parameters_t ChannelParameters;
   L2CA_Queueing_Parameters_t   QueueingParameters;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* If LE Parameters.                                              */
      if(TempParam->Params[0].intParam == 0)
      {
         /* Make sure we have all the LE Parameters.                    */
         if(TempParam->NumberofParameters >= 6)
         {
            /* Initialize the LE Channel Parameters.                    */
            BTPS_MemInitialize(&(ChannelParameters), 0, L2CA_LE_CHANNEL_PARAMETERS_SIZE);

            /* Determine the flags.                                     */
            if(TempParam->Params[1].intParam == 1)
            {
               ChannelParameters.ChannelFlags |= L2CA_LE_CHANNEL_PARAMETER_FLAGS_MANUAL_CREDIT_MODE;
            }

            /* Set the remaining fields.                                */
            ChannelParameters.MaxSDUSize     = (Word_t)TempParam->Params[2].intParam;
            ChannelParameters.MaxPDUSize     = (Word_t)TempParam->Params[3].intParam;
            ChannelParameters.PDUQueueDepth  = (Word_t)TempParam->Params[4].intParam;
            ChannelParameters.MaximumCredits = (Word_t)TempParam->Params[5].intParam;

            /* Simply call the internal function to set the parameters. */
            SetParameters(&ChannelParameters, NULL);
         }
         else
            DisplaySetParametersUsage();
      }
      else
      {
         /* If Queueing Parameters.                                     */
         if(TempParam->Params[0].intParam == 1)
         {
            /* Make sure parameters are smei-valid.                     */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Initialize the Queueing Parameters.                   */
               BTPS_MemInitialize(&QueueingParameters, 0, L2CA_QUEUEING_PARAMETERS_SIZE);

               /* Set the remaining fields.                             */
               QueueingParameters.Flags        = L2CA_QUEUEING_FLAG_LIMIT_BY_PACKETS;
               QueueingParameters.QueueLimit   = (DWord_t)TempParam->Params[1].intParam;
               QueueingParameters.LowThreshold = (DWord_t)TempParam->Params[2].intParam;

               /* Simply call the internal function to set the          */
               /* parameters.                                           */
               SetParameters(NULL, &QueueingParameters);
            }
            else
               DisplaySetParametersUsage();
         }
         else
            DisplaySetParametersUsage();
      }
   }
   else
      DisplaySetParametersUsage();

   return(ret_val);
}

   /* The following command is responsible for opening the Object       */
   /* Transfer Channel.                                                 */
static int OpenChannelCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   OpenChannel();

   return(0);
}

   /* The following command is responsible for closing the Object       */
   /* Transfer Channel.                                                 */
static int CloseChannelCommand(ParameterList_t *TempParam)
{
   /* Simply call the internal function.                                */
   CloseChannel();

   return(0);
}

   /* The following command is responsible for reading an OTS           */
   /* Characteristic.                                                   */
static int ReadOTSCharacteristicCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = (OTP_Attribute_Handle_Type_t)TempParam->Params[0].intParam;

      /* Store additional parameters based on the attribute handle type.*/
      switch(RequestData.AttributeHandleType)
      {
         case ahtOTSFeature:
            /* No parameters.                                           */
            break;
         case ahtObjectMetadata:
            /* Make sure the Object Metadata type is specified.         */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the OTS Object Metadata type.                   */
               RequestData.MetadataType = (OTS_Object_Metadata_Type_t)TempParam->Params[1].intParam;

               /* If this the OTS Object Name we need to determine if   */
               /* the user wants to issue a GATT Read Long Value        */
               /* request.                                              */
               if(RequestData.MetadataType == omtObjectName)
               {
                  /* Make sure the Read Long boolean is specified.      */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the boolean.                              */
                     RequestData.UseReadLong = (TempParam->Params[2].intParam == 0) ? FALSE : TRUE;
                  }
                  else
                  {
                     printf("\r\nInvalid parameter.\r\n");
                     DisplayReadCharacteristicCommandUsage();

                     /* Flag that an error occured.                     */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
               }
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayReadCharacteristicCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case ahtObjectActionControlPoint:
         case ahtObjectActionControlPoint_CCCD:
         case ahtObjectListControlPoint:
         case ahtObjectListControlPoint_CCCD:
            /* No parameters.                                           */
            break;
         case ahtObjectListFilter_1:
         case ahtObjectListFilter_2:
         case ahtObjectListFilter_3:
            /* Make sure the Read Long boolean is specified.            */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the boolean.                                    */
               RequestData.UseReadLong = (TempParam->Params[1].intParam == 0) ? FALSE : TRUE;
            }
            else
            {
               printf("\r\nInvalid parameter.\r\n");
               DisplayReadCharacteristicCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case ahtObjectChanged:
         case ahtObjectChanged_CCCD:
            /* No parameters.                                           */
            break;
         default:
            break;
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Simply call the internal function to handle the read        */
         /* request.                                                    */
         ret_val = ReadOTSCharacteristic();
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayReadCharacteristicCommandUsage();
   }

   return(ret_val);
}

   /* The following command is responsible for writing an Object Metata */
   /* Characteristic.                                                   */
static int WriteObjectMetadataCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = ahtObjectMetadata;

      /* Store the Object Metadata type.                                */
      RequestData.MetadataType = (OTS_Object_Metadata_Type_t)TempParam->Params[0].intParam;

      /* Store any optional parameters.                                 */
      switch(RequestData.MetadataType)
      {
         case omtObjectName:
            /* Make sure the Name is specified..                        */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Name parameter.                             */
               /* * NOTE * We will need to copy the string if we need   */
               /*          use the GATT Write Long procedure for a      */
               /*          string that cannot fit in the MTU for a GATT */
               /*          Write request.                               */
               /* * NOTE * We will not store the NULL terminator.       */
               RequestData.Data.Metadata.Name.Buffer        = (Byte_t *)TempParam->Params[1].strParam;
               RequestData.Data.Metadata.Name.Buffer_Length = BTPS_StringLength(TempParam->Params[1].strParam);
            }
            else
            {
               DisplayWriteObjectMetadataCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case omtObjectType:
            /* Cannot be written.                                       */
            break;
         case omtObjectSize:
            /* Cannot be written.                                       */
            break;
         case omtObjectFirstCreated:
         case omtObjectLastModified:
            /* Intentional fall-through since we can handle both these  */
            /* cases similarly.                                         */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Date Time.                                  */
               /* * NOTE * We will use the First Created Date Time since*/
               /*          these structures align.                      */
               if(StrToOTSDateTime(TempParam->Params[1].strParam, &(RequestData.Data.Metadata.First_Created)) != 0)
               {
                  DisplayWriteObjectMetadataCommandUsage();

                  /* Flag that an error occured.                        */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               DisplayWriteObjectMetadataCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case omtObjectID:
            /* Cannot be written.                                       */
            break;
         case omtObjectProperties:
            /* Make sure the Properties is specified..                  */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Properties parameter.                       */
               RequestData.Data.Metadata.Properties = (DWord_t)TempParam->Params[1].intParam;
            }
            else
            {
               DisplayWriteObjectMetadataCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         default:
            /* Prevent compiler warnings.                               */
            break;
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Simply call the internal function to handle the write       */
         /* request.                                                    */
         ret_val = WriteOTSCharacteristic();
      }
   }
   else
      DisplayWriteObjectMetadataCommandUsage();

   return(ret_val);
}

   /* The following command is responsible for writing the Object Action*/
   /* Control Point Characteristic.                                     */
static int WriteOACPCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = ahtObjectActionControlPoint;

      /* Store the OACP Request type.                                   */
      RequestData.Data.OACP_RequestData.Request_Op_Code = (OTS_OACP_Request_Type_t)TempParam->Params[0].intParam;

      /* Store any optional parameters.                                 */
      switch(RequestData.Data.OACP_RequestData.Request_Op_Code)
      {
         case oacpCreate:
            /* Make sure the Size and UUID parameters are attached.     */
            if(TempParam->NumberofParameters >= 3)
            {
               /* Store the Size parameter.                             */
               RequestData.Data.OACP_RequestData.Parameter.Create_Data.Size = (DWord_t)TempParam->Params[1].intParam;

               /* Convert the string to UUID.                           */
               if(StrToOTSObjectType(TempParam->Params[2].strParam, &(RequestData.Data.OACP_RequestData.Parameter.Create_Data.UUID)) != 0)
               {
                  DisplayWriteOACPCommandUsage();

                  /* Flag that an error occured.                        */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               DisplayWriteOACPCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case oacpDelete:
            break;
         case oacpRead:
         case oacpCalculateChecksum:
            /* Intentional fall-through.                                */

            /* Make sure the Offset and Length parameters are attached. */
            if(TempParam->NumberofParameters >= 3)
            {
               /* Store the parameters.                                 */
               /* * NOTE * We will simply use the Read_Data since the   */
               /*          data for both procedures align.              */
               RequestData.Data.OACP_RequestData.Parameter.Read_Data.Offset = (DWord_t)TempParam->Params[1].intParam;
               RequestData.Data.OACP_RequestData.Parameter.Read_Data.Length = (DWord_t)TempParam->Params[2].intParam;
            }
            else
            {
               DisplayWriteOACPCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case oacpExecute:
            break;
         case oacpWrite:
            /* Make sure the Offset and Length parameters are attached. */
            if(TempParam->NumberofParameters >= 6)
            {
               /* Store the parameters.                                 */
               RequestData.FileName                                          = TempParam->Params[1].strParam;
               RequestData.FileOffset                                        = (DWord_t)TempParam->Params[2].intParam;
               RequestData.Data.OACP_RequestData.Parameter.Write_Data.Offset = (DWord_t)TempParam->Params[3].intParam;
               RequestData.Data.OACP_RequestData.Parameter.Write_Data.Length = (DWord_t)TempParam->Params[4].intParam;
               RequestData.Data.OACP_RequestData.Parameter.Write_Data.Mode   = (OTS_Write_Mode_Type_t)TempParam->Params[5].intParam;
            }
            else
            {
               DisplayWriteOACPCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case oacpAbort:
            break;
         default:
            /* Prevent compiler warnings.                               */
            break;
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Simply call the internal function to handle the write       */
         /* request.                                                    */
         ret_val = WriteOTSCharacteristic();
      }
   }
   else
      DisplayWriteOACPCommandUsage();

   return(ret_val);
}

   /* The following command is responsible for writing the Object List  */
   /* Control Point Characteristic.                                     */
static int WriteOLCPCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the attribute handle type.                               */
      RequestData.AttributeHandleType = ahtObjectListControlPoint;

      /* Store the OLCP Request type.                                   */
      RequestData.Data.OLCP_RequestData.Request_Op_Code = (OTS_OLCP_Request_Type_t)TempParam->Params[0].intParam;

      /* Store any optional parameters.                                 */
      switch(RequestData.Data.OLCP_RequestData.Request_Op_Code)
      {
         case olcpFirst:
         case olcpLast:
         case olcpPrevious:
         case olcpNext:
            /* Intentional fall-through since there are no parameters.  */
            break;
         case olcpGoTo:
            /* Make sure the Object ID parameter is specified.          */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the Object ID.                                  */
               /* * NOTE * This sample application only uses the Lower  */
               /*          portion of the Object ID.                    */
               RequestData.Data.OLCP_RequestData.Parameter.Object_ID.Lower = (DWord_t)TempParam->Params[1].intParam;
               RequestData.Data.OLCP_RequestData.Parameter.Object_ID.Upper = 0;
            }
            else
            {
               DisplayWriteOLCPCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case olcpOrder:
            /* Make sure the sort order parameter is specified.         */
            if(TempParam->NumberofParameters >= 2)
            {
               /* Store the parameter.                                  */
               RequestData.Data.OLCP_RequestData.Parameter.List_Sort_Order = (OTS_List_Sort_Order_Type_t)TempParam->Params[1].intParam;
            }
            else
            {
               DisplayWriteOLCPCommandUsage();

               /* Flag that an error occured.                           */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case olcpRequestNumberOfObjects:
         case olcpClearMarking:
            /* Intentional fall-through since there are no parameters.  */
            break;
         default:
            /* Prevent compiler warnings.                               */
            break;
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Simply call the internal function to handle the write       */
         /* request.                                                    */
         ret_val = WriteOTSCharacteristic();
      }
   }
   else
      DisplayWriteOLCPCommandUsage();

   return(ret_val);
}

   /* The following command is responsible for writing an Object List   */
   /* Filter Characteristic.                                            */
static int WriteObjectListFilterCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Make sure the instance is valid.                               */
      if(((OTS_Object_List_Filter_Instance_t)TempParam->Params[0].intParam >= lfiOne) && ((OTS_Object_List_Filter_Instance_t)TempParam->Params[0].intParam <= lfiThree))
      {
         /* Store the attribute handle type.                            */
         /* * NOTE * We will add the instance to select the correct     */
         /*          attribute handle type.                             */
         RequestData.AttributeHandleType = ahtObjectListFilter_1 + TempParam->Params[0].intParam;

         /* Make sure the Object List Filter type is specified.         */
         if(TempParam->NumberofParameters >= 2)
         {
            /* Store the Object List Filter instance.                   */
            RequestData.Data.ListFilterData.Type = (OTS_Object_List_Filter_Type_t)TempParam->Params[1].intParam;

            /* Store any optional parameters.                           */
            switch(RequestData.Data.ListFilterData.Type)
            {
               case lftNoFilter:
                  /* No parameters.                                     */
                  break;
               case lftNameStartsWith:
               case lftNameEndsWith:
               case lftNameContains:
               case lftNameIsExactly:
                  /* Intentional fall-through since we can handle both  */
                  /* these cases similarly.                             */

                  /* Make sure the Name parameter is specified.         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Store the Name parameter.                       */
                     /* * NOTE * We will need to copy the string if we  */
                     /*          need use the GATT Write Long procedure */
                     /*          for a string that cannot fit in the MTU*/
                     /*          for a GATT Write request.              */
                     /* * NOTE * We will not store the NULL terminator. */
                     RequestData.Data.ListFilterData.Data.Name.Buffer        = (Byte_t *)TempParam->Params[2].strParam;
                     RequestData.Data.ListFilterData.Data.Name.Buffer_Length = BTPS_StringLength(TempParam->Params[2].strParam);
                  }
                  else
                  {
                     DisplayWriteObjectListFilterCommandUsage();

                     /* Flag that an error occured.                     */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case lftObjectType:
                  /* Make sure the Type parameter is specified.         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Convert the string to UUID.                     */
                     if(StrToOTSObjectType(TempParam->Params[2].strParam, &(RequestData.Data.ListFilterData.Data.Type)) != 0)
                     {
                        DisplayWriteObjectListFilterCommandUsage();

                        /* Flag that an error occured.                  */
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     DisplayWriteObjectListFilterCommandUsage();

                     /* Flag that an error occured.                     */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case lftCreatedBetween:
               case lftModifiedBetween:
                  /* Intentional fall-through since we can handle both  */
                  /* these cases similarly.                             */

                  /* Make sure the Min and Max parameters are specified.*/
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Convert the string to the Date Time format.     */
                     if(StrToOTSDateTime(TempParam->Params[2].strParam, &(RequestData.Data.ListFilterData.Data.Time_Range.Minimum)) == 0)
                     {
                        /* Convert the string to the Date Time format.  */
                        if(StrToOTSDateTime(TempParam->Params[3].strParam, &(RequestData.Data.ListFilterData.Data.Time_Range.Maximum)) != 0)
                        {
                           DisplayWriteObjectListFilterCommandUsage();

                           /* Flag that an error occured.               */
                           ret_val = INVALID_PARAMETERS_ERROR;
                        }
                     }
                     else
                     {
                        DisplayWriteObjectListFilterCommandUsage();

                        /* Flag that an error occured.                  */
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     DisplayWriteObjectListFilterCommandUsage();

                     /* Flag that an error occured.                     */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case lftCurrentSizeBetween:
               case lftAllocatedSizeBetween:
                  /* Intentional fall-through since we can handle both  */
                  /* these cases similarly.                             */

                  /* Make sure the Min and Max parameters are specified.*/
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Store the size parameters.                      */
                     RequestData.Data.ListFilterData.Data.Size_Range.Minimum = TempParam->Params[2].intParam;
                     RequestData.Data.ListFilterData.Data.Size_Range.Maximum = TempParam->Params[3].intParam;
                  }
                  else
                  {
                     DisplayWriteObjectListFilterCommandUsage();

                     /* Flag that an error occured.                     */
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case lftMarkedObjects:
                  /* No parameters.                                     */
                  break;
               default:
                  /* Prevent compiler warnings.                         */
                  break;
            }

            /* If an error did not occur.                               */
            if(!ret_val)
            {
               /* Simply call the internal function to handle the write */
               /* request.                                              */
               ret_val = WriteOTSCharacteristic();
            }
         }
      }
      else
         DisplayWriteObjectListFilterCommandUsage();
   }
   else
      DisplayWriteObjectListFilterCommandUsage();

   return(ret_val);
}

   /* The following command is responsible for writing an OTS           */
   /* Characteristic CCCD.                                              */
static int WriteCCCDCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 2))
   {
      /* Store the attribute handle type based on the OTS CCCD          */
      /* Characteristic type.                                           */
      switch(TempParam->Params[0].intParam)
      {
         case cctObjectActionControlPoint:
            /* Store the attribute handle type.                         */
            RequestData.AttributeHandleType = ahtObjectActionControlPoint_CCCD;
            break;
         case cctObjectListControlPoint:
            /* Store the attribute handle type.                         */
            RequestData.AttributeHandleType = ahtObjectListControlPoint_CCCD;
            break;
         case cctObjectChanged:
            /* Store the attribute handle type.                         */
            RequestData.AttributeHandleType = ahtObjectChanged_CCCD;
            break;
         case cctServiceChanged:
            /* Store the attribute handle type.                         */
            RequestData.AttributeHandleType = ahtServiceChanged_CCCD;
            break;
         default:
            printf("\r\nInvalid OTS CCCD Characteristic type.\r\n");
            DisplayWriteCCCDCommandUsage();

            ret_val = INVALID_PARAMETERS_ERROR;
            break;
      }

      /* Store the CCCD.                                                */
      if(TempParam->Params[1].intParam == 0)
      {
         RequestData.Data.Configuration = 0;
      }
      else
      {
         if(TempParam->Params[1].intParam == 1)
         {
            RequestData.Data.Configuration = (Word_t)OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE;
         }
         else
         {
            RequestData.Data.Configuration = (Word_t)OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE;
         }
      }

      /* If an error did not occur.                                     */
      if(!ret_val)
      {
         /* Simply call the internal function to handle the write       */
         /* request.                                                    */
         ret_val = WriteOTSCharacteristic();
      }
   }
   else
   {
      printf("\r\nInvalid parameter.\r\n");
      DisplayWriteCCCDCommandUsage();
   }

   return(ret_val);
}

   /* The following command is responsible for creating a new OTS Object*/
   /* and file associated with it on the file system.                   */
static int CreateFileObjectCommand(ParameterList_t *TempParam)
{
   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 2))
   {
      /* Simply call the internal function to perform the work.         */
      CreateFileObject(TempParam->Params[0].strParam, TempParam->Params[1].intParam);
   }
   else
      DisplayCreateFileObjectCommandUsage();


   return(0);
}

   /* The following command is responsible for printing information     */
   /* about the currently selected object.                              */
static int PrintCurrentObjectCommand(ParameterList_t *TempParam)
{
   int                ret_val = 0;
   DeviceInfo_t      *DeviceInfo;
   OTS_Object_Data_t *ObjectDataPtr;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure the current object is selected.                */
            if(DeviceInfoList->FilteredObjectList.Current_Object)
            {
               /* Store a pointer to the OTS Object data.               */
               ObjectDataPtr = DeviceInfoList->FilteredObjectList.Current_Object->ObjectDataPtr;

               /* Print the information.                                */
               printf("\r\nCurrent Object:\r\n");
               DisplayObjectData(ObjectDataPtr);
            }
            else
               printf("\r\nCurrent object not selected.\r\n");
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Client.\r\n");
   }
   else
      printf("\r\nOnly the OTP Server may print the current object's data.\r\n");

   return(ret_val);
}

   /* The following command is responsible for printing information for */
   /* each Node in the Filtered Object List.                            */
static int PrintFilteredObjectListCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   DeviceInfo_t               *DeviceInfo;
   unsigned int                Index;
   OTP_Filtered_Object_List_t *List;
   OTP_Filtered_Object_Node_t *Node;
   OTS_Object_Data_t          *ObjectDataPtr;

   /* Make sure we are the OTP Server.                                  */
   if(OTSInstanceID)
   {
      /* Make sure that we are connected and a remote device is         */
      /* selected.                                                      */
      if((Connections) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
      {
         /* Get the device info for the connected device.               */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Store a pointer to the Filtered Objects List.            */
            List = &(DeviceInfo->FilteredObjectList);

            /* Make sure the Filtered Object List is not empty.         */
            if(List->Number_Of_Nodes)
            {
               /* Loop through the list.                                */
               for(Index = 0, Node = List->HeadPtr; (Node) && (Index < (unsigned int)List->Number_Of_Nodes); Index++, Node = Node->NextPtr)
               {
                  /* Store a pointer to the OTS Object data.            */
                  ObjectDataPtr = Node->ObjectDataPtr;

                  /* Print the information.                             */
                  printf("\r\nOTS Object (%u):\r\n", Index);
                  DisplayObjectData(ObjectDataPtr);
               }
            }
            else
               printf("\r\nFiltered objects list is empty.\r\n");
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nNo connection to the OTP Client.\r\n");
   }
   else
      printf("\r\nOnly the OTP Server may print the current object's data.\r\n");

   return(ret_val);
}

   /* The following command is responsible for marking an OTS Object.   */
static int MarkObjectCommand(ParameterList_t *TempParam)
{
   unsigned int Index;
   /* Make sure the parameters are semi-valid.                          */
   if((TempParam) && (TempParam->NumberofParameters >= 1))
   {
      /* Store the parameter.                                           */
      Index = TempParam->Params[0].intParam;

      /* Make sure the index is valid.                                  */
      if(Index < OTP_MAXIMUM_SUPPORTED_OBJECTS)
      {
         /* Make sure the OTS Object is valid.                          */
         if(ServerData.Object_List[Index].Valid)
         {
            /* Make sure we are allowed to mark the OTS Object.         */
            if(ServerData.Object_List[Index].Data.Properties & OTS_OBJECT_PROPERTIES_MARK)
            {
               /* Mark the OTS Object.                                  */
               ServerData.Object_List[Index].Data.Marked = TRUE;

               /* Let the user know what has been marked.               */
               printf("\r\nSuccessfully marked OTS Object:\r\n");
               printf("   Name:  \"%s\".", (char *)ServerData.Object_List[Index].Data.Name.Buffer);
            }
            else
               printf("\r\nMarking the OTS Object is not allowed.\r\n");
         }
         else
            printf("\r\nOTS Object is not valid.\r\n");
      }
      else
         printf("\r\nInvalid Object Index.\r\n");
   }
   else
      printf("\r\nUsage: Mark [Object Index (0-%u)].\r\n", OTP_MAXIMUM_SUPPORTED_OBJECTS);

   return(0);
}

   /* The following function displays the usage for the                 */
   /* SetConnectionModeCommand() function.                              */
static void DisplaySetConnectionModeUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: SetConnectionMode [Mode (UINT8)]\r\n");
   printf("\r\nWhere Mode is:\r\n");
   printf("   0 = Auto Accept.\r\n");
   printf("   1 = Auto Reject.\r\n");
   printf("   2 = Manual Accept.\r\n");
}

   /* The following function displays the usage for the                 */
   /* SetParametersCommand() function.                                  */
static void DisplaySetParametersUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: SetParameters [Type (0=LE, 1=Queueing)] [Opt. params (below)]\r\n");
   printf("\r\nWhere (LE) Opt. Params are:\r\n");
   printf("   [Credit Based Mode (0 = Automatic, 1 = Manual)].\r\n");
   printf("   [Max SDU Size (UINT16)]\r\n");
   printf("   [Max PDU Size (UINT16)]\r\n");
   printf("   [Max PDU Queue Depth (UINT16)]\r\n");
   printf("   [Max Credits (UINT16)].\r\n");
   printf("\r\nWhere (Queueing) Opt. Params are:\r\n");
   printf("   [Queue Limit (UINT32)]\r\n");
   printf("   [Queue Threshold (UINT32)]\r\n");
}

   /* The following function displays the usage for the                 */
   /* ReadOTSCharacteristicCommand() function.                          */
static void DisplayReadCharacteristicCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: Read [Attribute Handle Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Attribute Handle Type is:\r\n");
   printf("   0  = OTS Feature.\r\n");
   printf("   1  = Object Metadata [Type].\r\n");
   printf("   2  = Object Action Control Point (CANNOT BE READ)\r\n");
   printf("   3  = Object Action Control Point CCCD\r\n");
   printf("   4  = Object List Control Point (CANNOT BE READ)\r\n");
   printf("   5  = Object List Control Point CCCD\r\n");
   printf("   6  = Object List Filter (1) [Read Long(Boolean)]\r\n");
   printf("   7  = Object List Filter (2) [Read Long(Boolean)]\r\n");
   printf("   8  = Object List Filter (3) [Read Long(Boolean)]\r\n");
   printf("   9  = Object Changed (CANNOT BE READ)\r\n");
   printf("   10 = Object Changed CCCD\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("       0 = Name [Read Long(Boolean)].\r\n");
   printf("       1 = Type.\r\n");
   printf("       2 = Size.\r\n");
   printf("       3 = First Created.\r\n");
   printf("       4 = Last Modified.\r\n");
   printf("       5 = ID.\r\n");
   printf("       6 = Properties.\r\n");
   printf("\r\nNOTE: To use Read Long, the GATT Read request MUST be used first, followed by GATT Read Long for subsequent requests.\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteObjectMetadataCommand() function.                            */
static void DisplayWriteObjectMetadataCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteMetadata [ Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("   0 = Name [String (UTF8)].\r\n");
   printf("   1 = Type (CANNOT BE WRITTEN).\r\n");
   printf("   2 = Size (CANNOT BE WRITTEN).\r\n");
   printf("   3 = First Created [Date Time (YYYYMMDDHHMMSS)].\r\n");
   printf("   4 = Last Modified [Date Time (YYYYMMDDHHMMSS)].\r\n");
   printf("   5 = ID (CANNOT BE WRITTEN).\r\n");
   printf("   6 = Properties [Bitmask (UINT32)].\r\n");
   printf("\r\nWhere Bitmask values are:\r\n");
   printf("       0x01 = Delete.\r\n");
   printf("       0x02 = Execute.\r\n");
   printf("       0x04 = Read.\r\n");
   printf("       0x08 = Write.\r\n");
   printf("       0x10 = Append.\r\n");
   printf("       0x20 = Truncate.\r\n");
   printf("       0x40 = Patch.\r\n");
   printf("       0x80 = Mark.\r\n");
   printf("\r\nNOTE: OTS Date Time MUST be valid for the First Created and Last Modified options.\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteOACPCommand() function.                                      */
static void DisplayWriteOACPCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteOACP [Request Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Request Type is:\r\n");
   printf("   1  = Create Procedure [Size (UINT32)] [Type (16/128 bit UUID ie. 0x00)].\r\n");
   printf("   2  = Delete Procedure.\r\n");
   printf("   3  = Calculate Checksum Procedure [Offset (UINT32)] [Length (UINT32)].\r\n");
   printf("   4  = Execute Procedure.\r\n");
   printf("   5  = Read Procedure (Read Length from specified Offset):\r\n");
   printf("       (General)                      [Offset (UINT32)] [Length (UINT32)].\r\n");
   printf("       (Entire File)                  [Offset = 0]      [Length = 0].\r\n");
   printf("       (OTS Directory Listing Object) [Offset = 0]      [Length = 0].\r\n");
   printf("   6  = Write Procedure (Write Length from specified Offset):\r\n");
   printf("        (General)     [File Path (UTF8)] [File Offset(UINT32)] [Offset (UINT32)] [Length (UINT32)] [Mode (0 = None, 1 = Truncate)].\r\n");
   printf("        (Entire File) [File Path (UTF8)] [File Offset = 0]     [Offset = 0]      [Length = 0]      [Mode (0 = None, 1 = Truncate)].\r\n");
   printf("   7  = Abort Procedure (For a Read Procedure ONLY)\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteOLCPCommand() function.                                      */
static void DisplayWriteOLCPCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteOLCP [Request Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Request Type is:\r\n");
   printf("   1  = First Procedure.\r\n");
   printf("   2  = Last Procedure.\r\n");
   printf("   3  = Previous Procedure.\r\n");
   printf("   4  = Next Procedure.\r\n");
   printf("   5  = Go To Procedure [Object ID (UINT32)].\r\n");
   printf("   6  = Order Procedure [Order Type].\r\n");
   printf("   7  = Request Number of Objects Procedure\r\n");
   printf("   8  = Clear Marking Procedure\r\n");
   printf("\r\nWhere Order Type is:\r\n");
   printf("       1  = Name Ascending\r\n");
   printf("       2  = Type Ascending\r\n");
   printf("       3  = Current Size Ascending\r\n");
   printf("       4  = First Created Ascending\r\n");
   printf("       5  = Last Modified Ascending\r\n");
   printf("       17 = Name Descending\r\n");
   printf("       18 = Type Descending\r\n");
   printf("       19 = Current Size Descending\r\n");
   printf("       20 = First Created Descending\r\n");
   printf("       21 = Last Modified Descending\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteObjectListFilterCommand() function.                          */
static void DisplayWriteObjectListFilterCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteListFilter [ Instance (0-2)] [Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("   0  = No Filter.\r\n");
   printf("   1  = Name Starts With [String (UTF8)].\r\n");
   printf("   2  = Name Ends With [String (UTF8)].\r\n");
   printf("   3  = Name Contains [String (UTF8)].\r\n");
   printf("   4  = Name is Exactly [String (UTF8)].\r\n");
   printf("   5  = Object Type [Type (16/128 bit UUID ie. 0x00)].\r\n");
   printf("   6  = Created Between [Min Date Time (YYYYMMDDHHMMSS)] [Max Date Time (YYYYMMDDHHMMSS)].\r\n");
   printf("   7  = Modified Between [Min Date Time (YYYYMMDDHHMMSS)] [Max Date Time (YYYYMMDDHHMMSS)].\r\n");
   printf("   8  = Current Size Between [Min Size (UINT32)] [Max Size (UINT32)].\r\n");
   printf("   9  = Allocated Size Between [Min Size (UINT32)] [Max Size (UINT32)].\r\n");
   printf("   10 = Marked Objects.\r\n");
   printf("\r\nNOTE: OTS Date Time MUST be valid for the Created Between and Modified Between options.\r\n");
}

   /* The following function displays the usage for the                 */
   /* WriteCCCDCommand() function.                                      */
static void DisplayWriteCCCDCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: WriteCCCD [Type (UINT8)] [Opt. params (below)]\r\n");
   printf("\r\nWhere Type is:\r\n");
   printf("   0 = Object Action Control Point CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
   printf("   1 = Object List Control Point CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
   printf("   2 = Object Changed CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
   printf("   3 = Service Changed CCCD [Configuration (0 = disable, 1 = indicate, 2 = notify)].\r\n");
}

   /* The following function displays the usage for the                 */
   /* CreateFileObjectCommand() function.                               */
static void DisplayCreateFileObjectCommandUsage(void)
{
   /* Display the usage.                                                */
   printf("\r\nUsage: Create [Name (UTF8)] [Properties (Bitmask)]\r\n");
   printf("\r\nWhere Properies values are:\r\n");
   printf("   0x01 = Delete.\r\n");
   printf("   0x02 = Execute.\r\n");
   printf("   0x04 = Read.\r\n");
   printf("   0x08 = Write.\r\n");
   printf("   0x10 = Append.\r\n");
   printf("   0x20 = Truncate.\r\n");
   printf("   0x40 = Patch.\r\n");
   printf("   0x80 = Mark.\r\n");
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
                  ConnectionData.LinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                  /* Display the link key that has being created.       */
                  printf("Link Key: 0x");

                  for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     printf("%02X", ((Byte_t *)(&(ConnectionData.LinkKey)))[Index]);
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
   BD_ADDR_t                                     RemoteDevice;
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
                  RemoteDevice        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;

                  /* Store the LE Address type so we can store it later */
                  /* in the device information when the                 */
                  /* etGATT_Connection_Device_Connection event is       */
                  /* received.                                          */
                  ConnectionData.AddressType = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;

                  /* Check if the device information exists.            */
                  /* * NOTE * This will be the case if we have          */
                  /*          previously connected and paired with the  */
                  /*          remote device.                            */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
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

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, RemoteDevice, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
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
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               printf("   BD_ADDR: %s.\r\n", BoardStr);
               printf("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               printf("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);
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

   /* The following is a OTP Server Event Callback.  This function will */
   /* be called whenever an OTP Server event occurs that is associated  */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the OTS Event Data that        */
   /* occurred and the OTS Event Callback Parameter that was specified  */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the OTS Event Data ONLY in the context If the caller  */
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
   /* (this argument holds anyway because another OTS Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving OTS Event Packets.  */
   /*            A Deadlock WILL occur because NO OTS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI OTS_EventCallback(unsigned int BluetoothStackID, OTS_Event_Data_t *OTS_Event_Data, unsigned long CallbackParameter)
{
   int                                Result = 0;
   BoardStr_t                         BoardStr;
   DeviceInfo_t                      *DeviceInfo;
   unsigned int                       Index;

   /* OTS Common Event fields.                                          */
   unsigned int                       InstanceID;
   unsigned int                       ConnectionID;
   unsigned int                       TransactionID;
   BD_ADDR_t                          RemoteDevice;
   GATT_Connection_Type_t             ConnectionType;

   OTS_Object_Data_t                 *ObjectDataPtr;
   OTS_Object_Metadata_Type_t         Type;
   OTS_Object_Metadata_Data_t         Data;
   Byte_t                             Offset;
   OTS_OACP_Request_Data_t           *OACP_RequestData;
   OTS_OACP_Response_Data_t           OACP_ResponseData;
   OTS_OLCP_Request_Data_t           *OLCP_RequestData;
   OTS_OLCP_Response_Data_t           OLCP_ResponseData;
   OTS_Object_List_Filter_Instance_t  Instance;
   OTS_Object_List_Filter_Data_t     *ListFilterData;
   OTS_CCCD_Characteristic_Type_t     CCCD_Type;
   Word_t                             Configuration;
   Byte_t                             Status;
   Word_t                             BytesWritten;
   OTS_Object_List_Filter_Data_t     *CurrentListFilterData;
   Byte_t                             ErrorCode;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (OTS_Event_Data))
   {
      /* Switch through the event type.                                 */
      switch(OTS_Event_Data->Event_Data_Type)
      {
         case etOTS_Server_Read_OTS_Feature_Request:
            printf("\r\netOTS_Server_Read_OTS_Feature_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Read_OTS_Feature_Request_Data->RemoteDevice;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Simply respond with the OTS Feature.         */
                        if((Result = OTS_Read_OTS_Feature_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, &(ServerData.OTS_Feature))) != 0)
                           DisplayFunctionError("OTS_Read_OTS_Feature_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Read_OTS_Feature_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, NULL)) != 0)
                           DisplayFunctionError("OTS_Read_OTS_Feature_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Read_OTS_Feature_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, NULL)) != 0)
                        DisplayFunctionError("OTS_Read_OTS_Feature_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Read_OTS_Feature_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, NULL)) != 0)
                     DisplayFunctionError("OTS_Read_OTS_Feature_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Read_Object_Metadata_Request:
            printf("\r\netOTS_Server_Read_Object_Metadata_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->RemoteDevice;
               Type           = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->Type;
               Offset         = OTS_Event_Data->Event_Data.OTS_Read_Object_Metadata_Request_Data->Offset;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Request Type:     ");
               DisplayObjectMetadataType(Type);
               printf("   Offset:           %u.\r\n", Offset);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Make sure the current object is selected.    */
                        if(DeviceInfo->FilteredObjectList.Current_Object)
                        {
                           /* Store a pointer to the current object's   */
                           /* data.                                     */
                           ObjectDataPtr = DeviceInfo->FilteredObjectList.Current_Object->ObjectDataPtr;

                           /* Determine if the Directory Listing Object */
                           /* is selected.                              */
                           /* * NOTE * We will simply go ahead and      */
                           /*          update the Directory Listing     */
                           /*          Object size since it may have    */
                           /*          changed since it was last read   */
                           /*          (OTS Objects added/removed from  */
                           /*          the OTP Server).                 */
                           if((ObjectDataPtr->ID.Lower == 0) && (ObjectDataPtr->ID.Upper == 0))
                           {
                              /* Determine the number of objects on the */
                              /* OTS Server and Prepare to format them  */
                              /* as the OTS Directory Listing Object.   */
                              for(Index = 0, ServerData.Number_Of_Objects = 0; Index < (unsigned int)OTP_MAXIMUM_SUPPORTED_OBJECTS; Index++)
                              {
                                 /* If the OTS Object is valid.         */
                                 if(ServerData.Object_List[Index].Valid == TRUE)
                                 {
                                    /* Store a copy of the OTS Object.  */
                                    ServerData.Directory_List[ServerData.Number_Of_Objects] = ServerData.Object_List[Index].Data;

                                    /* Increment the number of OTS      */
                                    /* Objects included in the Directory*/
                                    /* Listing Object.                  */
                                    ServerData.Number_Of_Objects++;
                                 }
                              }
                              /* Determine the size of the Directory    */
                              /* Listing Object.                        */
                              /* * NOTE * We will not format the OTS    */
                              /*          Directory Listing Object into */
                              /*          the transfer buffer until an  */
                              /*          OACP Read Procedure has been  */
                              /*          requested for the OTS         */
                              /*          Directory Listing Object.     */
                              if((Result = OTS_Format_Directory_Listing_Object_Contents(ServerData.Number_Of_Objects, ServerData.Directory_List, 0, NULL)) > 0)
                              {
                                 /* Simply Update the Directory Listing */
                                 /* Objects size.                       */
                                 ObjectDataPtr->Size.Current_Size   = (DWord_t)Result;
                                 ObjectDataPtr->Size.Allocated_Size = (DWord_t)Result;
                              }
                              else
                                 DisplayFunctionError("OTS_Format_Directory_Listing_Object_Contents", Result);
                           }

                           /* Simply convert the OTS Object data from   */
                           /* the current object to the OTS Object      */
                           /* Metadata format.                          */
                           ConvertObjectDataToObjectMetadata(Type, ObjectDataPtr, &Data);

                           /* Send the read response.                   */
                           if((Result = OTS_Read_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, Type, &Data, Offset)) != 0)
                              DisplayFunctionError("OTS_Read_Object_Metadata_Request_Response", Result);
                        }
                        else
                        {
                           /* Display the error.                        */
                           printf("\r\nObject not selected.\r\n");

                           /* Send the error response.                  */
                           if((Result = OTS_Read_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_OBJECT_NOT_SELECTED, Type, NULL, Offset)) != 0)
                              DisplayFunctionError("OTS_Read_Object_Metadata_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Read_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, Type, NULL, Offset)) != 0)
                           DisplayFunctionError("OTS_Read_Object_Metadata_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Read_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Type, NULL, Offset)) != 0)
                        DisplayFunctionError("OTS_Read_Object_Metadata_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Read_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, Type, NULL, Offset)) != 0)
                     DisplayFunctionError("OTS_Read_Object_Metadata_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Write_Object_Metadata_Request:
            printf("\r\netOTS_Server_Write_Object_Metadata_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->RemoteDevice;
               Type           = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->Type;
               Data           = OTS_Event_Data->Event_Data.OTS_Write_Object_Metadata_Request_Data->Metadata;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Request Type:     ");
               DisplayObjectMetadataType(Type);

               /* Display the data we received.                         */
               printf("\r\nObject Metadata:\r\n");
               DisplayObjectMetadata(Type, &Data);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Make sure the current object is selected.    */
                        if(DeviceInfo->FilteredObjectList.Current_Object)
                        {
                           /* Store a pointer to the current object's   */
                           /* data.                                     */
                           ObjectDataPtr = DeviceInfo->FilteredObjectList.Current_Object->ObjectDataPtr;

                           /* Simply copy the OTS Object Metadata to the*/
                           /* current object OTS Object Data format.    */
                           /* * NOTE * This function may return an      */
                           /*          ErrorCode other than             */
                           /*          OTS_ERROR_CODE_SUCCESS if write  */
                           /*          request cannot be completed.     */
                           ErrorCode = StoreObjectMetadataForCurrentObject(Type, &Data, ObjectDataPtr);

                           /* Send the write response.                  */
                           if((Result = OTS_Write_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, Type)) != 0)
                              DisplayFunctionError("OTS_Write_Object_Metadata_Request_Response", Result);
                        }
                        else
                        {
                           /* Display the error.                        */
                           printf("\r\nObject not selected.\r\n");

                           /* Send the error response.                  */
                           if((Result = OTS_Write_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_OBJECT_NOT_SELECTED, Type)) != 0)
                              DisplayFunctionError("OTS_Write_Object_Metadata_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Write_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, Type)) != 0)
                           DisplayFunctionError("OTS_Write_Object_Metadata_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Write_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Type)) != 0)
                        DisplayFunctionError("OTS_Write_Object_Metadata_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Write_Object_Metadata_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, Type)) != 0)
                     DisplayFunctionError("OTS_Write_Object_Metadata_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Write_OACP_Request:
            printf("\r\netOTS_Server_Write_OACP_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data)
            {
               /* Store event information.                              */
               InstanceID       = OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->InstanceID;
               ConnectionID     = OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->ConnectionID;
               ConnectionType   = OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->ConnectionType;
               TransactionID    = OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->TransactionID;
               RemoteDevice     = OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->RemoteDevice;
               OACP_RequestData = &(OTS_Event_Data->Event_Data.OTS_Write_OACP_Request_Data->RequestData);

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Display the OACP request data.                        */
               DisplayOACPRequestData(OACP_RequestData);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Make sure the CCCD is configured for         */
                        /* indications.                                 */
                        if(DeviceInfo->OTSServerInfo.Object_Action_Control_Point_Configuration & OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                        {
                           /* Make sure that an OACP Procedure is not in*/
                           /* progress or that this is a OACP request   */
                           /* for the Abort Procedure.                  */
                           if((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS)) || (OACP_RequestData->Request_Op_Code == oacpAbort))
                           {
                              /* Flag that a procedure is in progress.  */
                              DeviceInfo->Flags |= DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS;

                              /* Send the Write OACP Request Response.  */
                              if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS)) != 0)
                                 DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);

                              /* Initiailze the respose data so we do   */
                              /* not have any unexpected behaviour.     */
                              BTPS_MemInitialize(&OACP_ResponseData, 0, OTS_OACP_RESPONSE_DATA_SIZE);

                              /* Go ahead and set the OACP Response     */
                              /* default information.                   */
                              /* * NOTE * We will set the Result Code to*/
                              /*          'operation failed' in case    */
                              /*          something unexpected happens. */
                              OACP_ResponseData.Request_Op_Code = OACP_RequestData->Request_Op_Code;
                              OACP_ResponseData.Result_Code     = oarOperationFailed;

                              /* Determine the OACP Procedure to        */
                              /* execute.                               */
                              switch(OACP_RequestData->Request_Op_Code)
                              {
                                 case oacpCreate:
                                    CreateProcedure(&(DeviceInfo->FilteredObjectList), OACP_RequestData, &OACP_ResponseData);
                                    break;
                                 case oacpDelete:
                                    DeleteProcedure(&(DeviceInfo->FilteredObjectList), &OACP_ResponseData);
                                    break;
                                 case oacpExecute:
                                    ExecuteProcedure(DeviceInfoList->FilteredObjectList.Current_Object, &OACP_ResponseData);
                                    break;
                                 case oacpCalculateChecksum:
                                    CalculateChecksumProcedure(DeviceInfoList->FilteredObjectList.Current_Object, OACP_RequestData, &OACP_ResponseData);
                                    break;
                                 case oacpRead:
                                    ReadProcedure(DeviceInfoList->FilteredObjectList.Current_Object, &(DeviceInfo->ChannelData), OACP_RequestData, &OACP_ResponseData);
                                    break;
                                 case oacpWrite:
                                    WriteProcedure(DeviceInfoList->FilteredObjectList.Current_Object, &(DeviceInfo->ChannelData), OACP_RequestData, &OACP_ResponseData);
                                    break;
                                 case oacpAbort:
                                    AbortProcedure(DeviceInfoList->FilteredObjectList.Current_Object, &(DeviceInfo->ChannelData), &OACP_ResponseData);
                                    break;
                                 default:
                                    OACP_ResponseData.Result_Code = oarOpcodeNotSupported;
                                    break;
                              }

                              /* Send the response indication.          */
                              if((Result = OTS_Indicate_OACP_Response(BluetoothStackID, InstanceID, ConnectionID, &OACP_ResponseData)) > 0)
                              {
                                 printf("\r\nOACP response indication sent:\r\n");
                                 printf("   TransactionID: %d\r\n", Result);

                                 /* If we send the response indication  */
                                 /* for an OACP Read Procedure request  */
                                 /* that was successful, then the Object*/
                                 /* Transfer Channel MUST be open and we*/
                                 /* are ready to start the transfer of  */
                                 /* data from the OTP Server to the OTP */
                                 /* Client.                             */
                                 if((OACP_ResponseData.Request_Op_Code == oacpRead) && (OACP_ResponseData.Result_Code == oarSuccess))
                                 {
                                    /* Flag that the Object Transfer has*/
                                    /* started for the read procedure.  */
                                    DeviceInfo->ChannelData.Flags |= OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS;
                                 }
                              }
                              else
                                 DisplayFunctionError("OTS_Indicate_OACP_Response", Result);
                           }
                           else
                           {
                              /* Display the error.                     */
                              printf("\r\nProcedure already in progress.\r\n");

                              /* Send the error response.               */
                              if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)) != 0)
                                 DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);
                           }
                        }
                        else
                        {
                           /* Display the error.                        */
                           printf("\r\nClient has not configured CCCD for indications.\r\n");

                           /* Send the error response.                  */
                           if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)) != 0)
                              DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED)) != 0)
                           DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION)) != 0)
                        DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Write_OACP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                     DisplayFunctionError("OTS_Write_OACP_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Write_OLCP_Request:
            printf("\r\netOTS_Server_Write_OLCP_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data)
            {
               /* Store event information.                              */
               InstanceID       = OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->InstanceID;
               ConnectionID     = OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->ConnectionID;
               ConnectionType   = OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->ConnectionType;
               TransactionID    = OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->TransactionID;
               RemoteDevice     = OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->RemoteDevice;
               OLCP_RequestData = &(OTS_Event_Data->Event_Data.OTS_Write_OLCP_Request_Data->RequestData);

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Display the OLCP request data.                        */
               DisplayOLCPRequestData(OLCP_RequestData);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Make sure the CCCD is configured for         */
                        /* indications.                                 */
                        if(DeviceInfo->OTSServerInfo.Object_List_Control_Point_Configuration & OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                        {
                           /* Make sure that an OLCP Procedure is not in*/
                           /* progress.                                 */
                           if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_OLCP_PROCEDURE_IN_PROGRESS))
                           {
                              /* Flag that a procedure is in progress.  */
                              DeviceInfo->Flags |= DEVICE_INFO_FLAGS_OTS_OLCP_PROCEDURE_IN_PROGRESS;

                              /* Send the Write OLCP Request Response.  */
                              if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS)) != 0)
                                 DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);

                              /* Initiailze the respose data so we do   */
                              /* not have any unexpected behaviour.     */
                              BTPS_MemInitialize(&OLCP_ResponseData, 0, OTS_OLCP_RESPONSE_DATA_SIZE);

                              /* Go ahead and set the OLCP Response     */
                              /* default information.                   */
                              /* * NOTE * We will set the Result Code to*/
                              /*          'operation failed' in case    */
                              /*          something unexpected happens. */
                              OLCP_ResponseData.Request_Op_Code = OLCP_RequestData->Request_Op_Code;
                              OLCP_ResponseData.Result_Code     = olrOperationFailed;

                              /* Determine the OLCP Procedure to        */
                              /* execute.                               */
                              switch(OLCP_RequestData->Request_Op_Code)
                              {
                                 case olcpFirst:
                                    FirstProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 case olcpLast:
                                    LastProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 case olcpPrevious:
                                    PreviousProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 case olcpNext:
                                    NextProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 case olcpGoTo:
                                    GoToProcedure(&(DeviceInfo->FilteredObjectList), OLCP_RequestData, &OLCP_ResponseData);
                                    break;
                                 case olcpOrder:
                                    OrderProcedure(&(DeviceInfo->FilteredObjectList), OLCP_RequestData, &OLCP_ResponseData);
                                    break;
                                 case olcpRequestNumberOfObjects:
                                    RequestNumberOfObjectsProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 case olcpClearMarking:
                                    ClearMarkingProcedure(&(DeviceInfo->FilteredObjectList), &OLCP_ResponseData);
                                    break;
                                 default:
                                    OLCP_ResponseData.Result_Code = olrOpcodeNotSupported;
                                    break;
                              }

                              /* Send the response indication.          */
                              if((Result = OTS_Indicate_OLCP_Response(BluetoothStackID, InstanceID, ConnectionID, &OLCP_ResponseData)) > 0)
                              {
                                 printf("\r\nOLCP response indication sent:\r\n");
                                 printf("   TransactionID: %d\r\n", Result);
                              }
                              else
                                 DisplayFunctionError("OTS_Indicate_OLCP_Response", Result);
                           }
                           else
                           {
                              /* Display the error.                     */
                              printf("\r\nProcedure already in progress.\r\n");

                              /* Send the error response.               */
                              if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)) != 0)
                                 DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);
                           }
                        }
                        else
                        {
                           /* Display the error.                        */
                           printf("\r\nClient has not configured CCCD for indications.\r\n");

                           /* Send the error response.                  */
                           if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)) != 0)
                              DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED)) != 0)
                           DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION)) != 0)
                        DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Write_OLCP_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                     DisplayFunctionError("OTS_Write_OLCP_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Read_Object_List_Filter_Request:
            printf("\r\netOTS_Server_Read_Object_List_Filter_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->RemoteDevice;
               Instance       = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->Instance;
               Offset         = OTS_Event_Data->Event_Data.OTS_Read_Object_List_Filter_Request_Data->Offset;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Instance:         ");
               switch(Instance)
               {
                  case lfiOne:
                     printf("lfiOne.\r\n");
                     break;
                  case lfiTwo:
                     printf("lfiTwo.\r\n");
                     break;
                  case lfiThree:
                     printf("lfiThree.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }
               printf("   Offset:           %u.\r\n", Offset);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Store a pointer to the OTS Object List       */
                        /* Filter.                                      */
                        ListFilterData = &(DeviceInfo->FilteredObjectList.List_Filter_Data[Instance]);

                        /* Send the response.                           */
                        if((Result = OTS_Read_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, Instance, ListFilterData, Offset)) != 0)
                           DisplayFunctionError("OTS_Read_Object_List_Filter_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Read_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, Instance, NULL, Offset)) != 0)
                           DisplayFunctionError("OTS_Read_Object_List_Filter_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Read_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Instance, NULL, Offset)) != 0)
                        DisplayFunctionError("OTS_Read_Object_List_Filter_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Read_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, Instance, NULL, Offset)) != 0)
                     DisplayFunctionError("OTS_Read_Object_List_Filter_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Write_Object_List_Filter_Request:
            printf("\r\netOTS_Server_Write_Object_List_Filter_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->RemoteDevice;
               Instance       = OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->Instance;
               ListFilterData = &(OTS_Event_Data->Event_Data.OTS_Write_Object_List_Filter_Request_Data->ListFilterData);

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Instance:         ");
               switch(Instance)
               {
                  case lfiOne:
                     printf("lfiOne.\r\n");
                     break;
                  case lfiTwo:
                     printf("lfiTwo.\r\n");
                     break;
                  case lfiThree:
                     printf("lfiThree.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }

               /* Display the OTS Object List filter data.              */
               DisplayObjectListFilterData(ListFilterData);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* ** NOTE ** We need to handle an OTS Object   */
                        /*            Name filter specially since the   */
                        /*            name is allocated and we need to  */
                        /*            copy the name from the event data.*/

                        /* Store a pointer to the OTS Object List       */
                        /* Filter.                                      */
                        CurrentListFilterData = &(DeviceInfo->FilteredObjectList.List_Filter_Data[Instance]);

                        /* If the Object Filter Type is a OTS Object    */
                        /* Name filter we may need to allocate memory to*/
                        /* hold the value if it does not exist.         */
                        switch(ListFilterData->Type)
                        {
                           case lftNameStartsWith:
                           case lftNameEndsWith:
                           case lftNameContains:
                           case lftNameIsExactly:
                              /* We need to the allocate memory if the  */
                              /* previous Object Filter Type was not an */
                              /* OTS Object Name filter type.  Otherwise*/
                              /* the memory is already allocated and we */
                              /* can re-use it.                         */
                              if((CurrentListFilterData->Type <= lftNameStartsWith) || (CurrentListFilterData->Type >= lftNameIsExactly))
                              {
                                 CurrentListFilterData->Data.Name.Buffer = (Byte_t *)BTPS_AllocateMemory(OTS_MAXIMUM_OBJECT_NAME_LENGTH + 1);
                              }

                              /* Make sure memory has been allocated.   */
                              if(CurrentListFilterData->Data.Name.Buffer)
                              {
                                 /* Copy the OTS Object Name filter.    */
                                 BTPS_MemCopy(CurrentListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer, ListFilterData->Data.Name.Buffer_Length);
                                 CurrentListFilterData->Data.Name.Buffer_Length = ListFilterData->Data.Name.Buffer_Length;

                                 /* Insert the null terminator so we can*/
                                 /* display it as a c-string.           */
                                 CurrentListFilterData->Data.Name.Buffer[CurrentListFilterData->Data.Name.Buffer_Length] = '\0';
                              }
                              else
                              {
                                 printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
                                 ErrorCode = OTS_ERROR_CODE_UNLIKELY_ERROR;
                                 Result    = FUNCTION_ERROR;
                              }
                              break;
                           default:
                              /* We need to the free memory if the      */
                              /* previous Object Filter Type was an OTS */
                              /* Object Name filter type and the new    */
                              /* filter type is not.                    */
                              if((ListFilterData->Type >= lftNameStartsWith) && (ListFilterData->Type <= lftNameIsExactly))
                              {
                                 /* Make sure memory has been allocated.*/
                                 if((CurrentListFilterData->Data.Name.Buffer_Length) && (CurrentListFilterData->Data.Name.Buffer))
                                 {
                                    /* Free the memory.                 */
                                    BTPS_FreeMemory(CurrentListFilterData->Data.Name.Buffer);
                                    CurrentListFilterData->Data.Name.Buffer        = NULL;
                                    CurrentListFilterData->Data.Name.Buffer_Length = 0;
                                 }
                                 else
                                 {
                                    printf("\r\nWarning - Memory is not allocated for the Object Name Filter type.\r\n");
                                    ErrorCode = OTS_ERROR_CODE_UNLIKELY_ERROR;
                                    Result    = FUNCTION_ERROR;
                                 }
                              }

                              /* Determine if this is a range filter    */
                              /* type.                                  */
                              /* * NOTE * We need to verify the minimum */
                              /*          and maximum values.           */
                              if((ListFilterData->Type >= lftCreatedBetween) && (ListFilterData->Type <= lftAllocatedSizeBetween))
                              {
                                 /* If this is an OTS Date Time range   */
                                 /* filter type.                        */
                                 if((ListFilterData->Type == lftCreatedBetween) || (ListFilterData->Type == lftModifiedBetween))
                                 {
                                    /* Make sure the minimum is less    */
                                    /* than the maximum.                */
                                    /* * NOTE * The function will return*/
                                    /*          -1 if the minimum is    */
                                    /*          greater than the        */
                                    /*          maximum.                */
                                    if(DateTimeCompare(&(ListFilterData->Data.Time_Range.Minimum), &(ListFilterData->Data.Time_Range.Maximum)) <= 0)
                                    {
                                       /* Store the OTS Object List     */
                                       /* Filter for the specified      */
                                       /* instance.                     */
                                       *CurrentListFilterData = *ListFilterData;
                                    }
                                    else
                                    {
                                       printf("\r\nMinimum Date Time exceeds Maximum Date Time.\r\n");
                                       ErrorCode = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
                                       Result    = FUNCTION_ERROR;
                                    }
                                 }
                                 else
                                 {
                                    /* Make sure the minimum is less    */
                                    /* than the maximum.                */
                                    if(ListFilterData->Data.Size_Range.Minimum <= ListFilterData->Data.Size_Range.Maximum)
                                    {
                                       /* Store the OTS Object List     */
                                       /* Filter for the specified      */
                                       /* instance.                     */
                                       *CurrentListFilterData = *ListFilterData;
                                    }
                                    else
                                    {
                                       printf("\r\nMinimum size exceeds Maximum size.\r\n");
                                       ErrorCode = OTS_ERROR_CODE_WRITE_REQUEST_REJECTED;
                                       Result    = FUNCTION_ERROR;
                                    }
                                 }
                              }
                              else
                              {
                                 /* Store the OTS Object List Filter for*/
                                 /* the specified instance.             */
                                 *CurrentListFilterData = *ListFilterData;
                              }
                              break;
                        }

                        /* If an error did not occur.                   */
                        if(!Result)
                        {
                           /* Update the current list filter type since */
                           /* we were successful.                       */
                           /* * NOTE * We do this here so that if we    */
                           /*          fail it remains set to its       */
                           /*          previous value.                  */
                           CurrentListFilterData->Type = ListFilterData->Type;

                           /* A new Object List Filter has been written */
                           /* so we need to update the Filtered Objects */
                           /* List to reflect the change.               */
                           UpdateFilteredObjectsList(&(DeviceInfo->FilteredObjectList));

                           /* Send the response.                        */
                           if((Result = OTS_Write_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, Instance)) != 0)
                              DisplayFunctionError("OTS_Write_Object_List_Filter_Request_Response", Result);
                        }
                        else
                        {
                           /* Send the error response.                  */
                           if((Result = OTS_Write_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, ErrorCode, Instance)) != 0)
                              DisplayFunctionError("OTS_Write_Object_List_Filter_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Write_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, Instance)) != 0)
                           DisplayFunctionError("OTS_Write_Object_List_Filter_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encryped.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Write_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, Instance)) != 0)
                        DisplayFunctionError("OTS_Write_Object_List_Filter_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Write_Object_List_Filter_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, Instance)) != 0)
                     DisplayFunctionError("OTS_Write_Object_List_Filter_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Read_CCCD_Request:
            printf("\r\netOTS_Server_Read_CCCD_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->RemoteDevice;
               CCCD_Type      = OTS_Event_Data->Event_Data.OTS_Read_CCCD_Request_Data->Type;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Type:             ");
               switch(CCCD_Type)
               {
                  case cctObjectActionControlPoint:
                     printf("cctObjectActionControlPoint.\r\n");
                     break;
                  case cctObjectListControlPoint:
                     printf("cctObjectListControlPoint.\r\n");
                     break;
                  case cctObjectChanged:
                     printf("cctObjectChanged.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Determine which OTS Characteristic CCCD has  */
                        /* been requested.                              */
                        switch(CCCD_Type)
                        {
                           case cctObjectActionControlPoint:
                              Configuration = DeviceInfo->OTSServerInfo.Object_Action_Control_Point_Configuration;
                              break;
                           case cctObjectListControlPoint:
                              Configuration = DeviceInfo->OTSServerInfo.Object_List_Control_Point_Configuration;
                              break;
                           case cctObjectChanged:
                              Configuration = DeviceInfo->OTSServerInfo.Object_Changed_Configuration;
                              break;
                           default:
                              /* Shouldn't occur.  The event would not  */
                              /* be received.                           */
                              Configuration = 0;
                              break;
                        }

                        /* Send the response.                           */
                        if((Result = OTS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, CCCD_Type, Configuration)) != 0)
                           DisplayFunctionError("OTS_Read_CCCD_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, CCCD_Type, 0)) != 0)
                           DisplayFunctionError("OTS_Read_CCCD_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, CCCD_Type, 0)) != 0)
                        DisplayFunctionError("OTS_Read_CCCD_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, CCCD_Type, 0)) != 0)
                     DisplayFunctionError("OTS_Read_CCCD_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Write_CCCD_Request:
            printf("\r\netOTS_Server_Write_CCCD_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->RemoteDevice;
               CCCD_Type      = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->Type;
               Configuration  = OTS_Event_Data->Event_Data.OTS_Write_CCCD_Request_Data->Configuration;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Type:             ");
               switch(CCCD_Type)
               {
                  case cctObjectActionControlPoint:
                     printf("cctObjectActionControlPoint.\r\n");
                     break;
                  case cctObjectListControlPoint:
                     printf("cctObjectListControlPoint.\r\n");
                     break;
                  case cctObjectChanged:
                     printf("cctObjectChanged.\r\n");
                     break;
                  default:
                     printf("Invalid.\r\n");
                     break;
               }
               printf("   Configuration:    0x%04X\r\n", Configuration);

               /* Get the device information for the OTP Client.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  /* * NOTE * This is a requirement.                    */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Make sure this client has not exceeded the      */
                     /* concurrency limit.                              */
                     if(!(DeviceInfo->ConcurrencyLimitExceeded))
                     {
                        /* Determine which OTS Characteristic CCCD has  */
                        /* been requested.                              */
                        switch(CCCD_Type)
                        {
                           case cctObjectActionControlPoint:
                              DeviceInfo->OTSServerInfo.Object_Action_Control_Point_Configuration = Configuration;
                              break;
                           case cctObjectListControlPoint:
                              DeviceInfo->OTSServerInfo.Object_List_Control_Point_Configuration = Configuration;
                              break;
                           case cctObjectChanged:
                              DeviceInfo->OTSServerInfo.Object_Changed_Configuration = Configuration;
                              break;
                           default:
                              /* Shouldn't occur.  The event would not  */
                              /* be received.                           */
                              break;
                        }

                        /* Send the response.                           */
                        if((Result = OTS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS, CCCD_Type)) != 0)
                           DisplayFunctionError("OTS_Write_CCCD_Request_Response", Result);
                     }
                     else
                     {
                        /* Display the error.                           */
                        printf("\r\nConcurrency Limit Exceeded.\r\n");

                        /* Send the error response.                     */
                        if((Result = OTS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED, CCCD_Type)) != 0)
                           DisplayFunctionError("OTS_Write_CCCD_Request_Response", Result);
                     }
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION, CCCD_Type)) != 0)
                        DisplayFunctionError("OTS_Write_CCCD_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR, CCCD_Type)) != 0)
                     DisplayFunctionError("OTS_Write_CCCD_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Prepare_Write_Request:
            printf("\r\netOTS_Server_Prepare_Write_Request with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data->ConnectionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data->ConnectionType;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data->TransactionID;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Prepare_Write_Request_Data->RemoteDevice;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we are encrypted.                        */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)
                  {
                     /* Send the response.                              */
                     if((Result = OTS_Prepare_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_SUCCESS)) != 0)
                        DisplayFunctionError("OTS_Prepare_Write_Request_Response", Result);
                  }
                  else
                  {
                     /* Display the error.                              */
                     printf("\r\nNot encrypted.\r\n");

                     /* Send the error response.                        */
                     if((Result = OTS_Prepare_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION)) != 0)
                        DisplayFunctionError("OTS_Prepare_Write_Request_Response", Result);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Send the error response.                           */
                  if((Result = OTS_Prepare_Write_Request_Response(BluetoothStackID, InstanceID, TransactionID, OTS_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                     DisplayFunctionError("OTS_Prepare_Write_Request_Response", Result);
               }
            }
            break;
         case etOTS_Server_Confirmation_Data:
            printf("\r\netOTS_Server_Confirmation_Data with size %u.\r\n", OTS_Event_Data->Event_Data_Size);
            if(OTS_Event_Data->Event_Data.OTS_Confirmation_Data)
            {
               /* Store event information.                              */
               InstanceID     = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->InstanceID;
               ConnectionID   = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->ConnectionID;
               TransactionID  = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->TransactionID;
               ConnectionType = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->ConnectionType;
               RemoteDevice   = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->RemoteDevice;
               Status         = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->Status;
               BytesWritten   = OTS_Event_Data->Event_Data.OTS_Confirmation_Data->BytesWritten;

               /* Print event information.                              */
               printf("   Instance ID:      %u.\r\n", InstanceID);
               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Status:           0x%02X.\r\n", Status);
               printf("   Bytes Written:    %u.\r\n", BytesWritten);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Flag that an OACP procedure is no longer in        */
                  /* progres.                                           */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS)
                  {
                     /* Flag that the procedure is no longer in progress*/
                     /* since we received the confirmation.             */
                     /* * NOTE * This does not mean that a transfer is  */
                     /*          not in progress.                       */
                     DeviceInfo->Flags &= ~(DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS);

                     /* Check if we need to start the transfer for the  */
                     /* OACP Read Procedure.                            */
                     if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)
                     {
                        /* Display information about the transfer.      */
                        printf("\r\nObject Transfer started.\r\n");
                        printf("  Procedure:       Read\r\n");
                        printf("  Object Name:     \"%s\"\r\n", DeviceInfoList->FilteredObjectList.Current_Object->ObjectDataPtr->Name.Buffer);
                        printf("  Current Offset:  %u\r\n",     DeviceInfo->ChannelData.Current_Offset);
                        printf("  Ending Offset:   %u\r\n",     DeviceInfo->ChannelData.Ending_Offset);
                        printf("  Buffer Length:   %u\r\n\r\n", DeviceInfo->ChannelData.Buffer_Length);

                        /* Start sending the OTS Object's contents.     */
                        SendObjectContents(&(DeviceInfo->ChannelData));
                     }

                     /* Check if we should display information for the  */
                     /* start of the OACP Write Procedure.              */
                     if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS)
                     {
                        printf("\r\nObject Transfer started.\r\n");
                        printf("  Procedure:       Write\r\n");
                        printf("  Object Name:     \"%s\"\r\n", DeviceInfoList->FilteredObjectList.Current_Object->ObjectDataPtr->Name.Buffer);
                        printf("  Current Offset:  %u\r\n",     DeviceInfo->ChannelData.Current_Offset);
                        printf("  Ending Offset:   %u\r\n",     DeviceInfo->ChannelData.Ending_Offset);
                        printf("  Buffer Length:   %u\r\n",     DeviceInfo->ChannelData.Buffer_Length);
                     }
                  }

                  /* Flag that an OLCP procedure is no longer in        */
                  /* progres.                                           */
                  if(DEVICE_INFO_FLAGS_OTS_OLCP_PROCEDURE_IN_PROGRESS)
                  {
                     /* Flag that the procedure is no longer in progress*/
                     /* since we received the confirmation.             */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_OTS_OLCP_PROCEDURE_IN_PROGRESS;
                  }

                  /* If we flagged for cleanup.                         */
                  if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL)
                  {
                     /* Simply call the internal function to cleanup the*/
                     /* channel.                                        */
                     /* * NOTE * This function will also optionally     */
                     /*          disconnect the channel if requested.   */
                     CleanupChannel(&(DeviceInfo->ChannelData));
                  }
               }
               else
                  printf("\r\nUnknown OTP Client.\r\n");
            }
            break;
         default:
            printf("\r\nUnknown OTS Event.\r\n");
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nOTS Callback Data: Event_Data = NULL.\r\n");
   }

   DisplayPrompt();
}

   /* The following is the OTS Channel Event Callback (Object Transfer  */
   /* Channel (OTC) event callback).  This function will be called      */
   /* whenever an OTS Channel event occurs that is associated with the  */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the OTS Event Data that occurred and the  */
   /* OTS Channel Event Callback Parameter that was specified when this */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the OTS Channel Event Data ONLY in the context If the caller      */
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
   /* (this argument holds anyway because another OTS Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving OTS Channel Event   */
   /*            Packets.  A Deadlock WILL occur because NO OTS Channel */
   /*            Event Callbacks will be issued while this function is  */
   /*            currently outstanding.                                 */
static void BTPSAPI OTS_Channel_EventCallback(unsigned int BluetoothStackID, OTS_Channel_Event_Data_t *OTS_Channel_Event_Data, unsigned long CallbackParameter)
{
   int                            Result;
   BoardStr_t                     BoardStr;
   DeviceInfo_t                  *DeviceInfo;
   OTP_Channel_Data_t            *ChannelData = NULL;

   /* OTS Channel Common Event fields.                                  */
   BD_ADDR_t                      RemoteDevice;
   OTS_Channel_Connection_Role_t  Role;
   OTS_Channel_Connection_Type_t  Type;
   Word_t                         CID;
   Word_t                         MaxSDUSize;
   Word_t                         Credits;
   Word_t                         Status;
   Word_t                         Reason;
   Word_t                         DataLength;
   Byte_t                        *Data;
   Word_t                         Error;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (OTS_Channel_Event_Data))
   {
      /* Switch through the event type.                                 */
      switch(OTS_Channel_Event_Data->Event_Data_Type)
      {
         case etOTS_Channel_Open_Indication:
            printf("\r\netOTS_Channel_Open_Indication with size %u.\r\n", OTS_Channel_Event_Data->Event_Data_Size);
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->Role;
               Type         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->Type;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->CID;
               MaxSDUSize   = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->MaxSDUSize;
               Credits      = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Indication_Data->InitialCredits;

               /* Print event information.                              */
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Role:             %s.\r\n", (Role) ? "crServer" : "crClient");
               printf("   Type:             %s.\r\n", (Type) ? "octBR_EDR" : "octLE");
               printf("   CID:              0x%04X.\r\n", CID);
               printf("   Max SDU Size:     %u.\r\n", MaxSDUSize);

               /* Display event information only if this is an LE       */
               /* Connection.                                           */
               if(Type == octLE)
               {
                  printf("   Initial Credits:  %u.\r\n", Credits);
               }

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Store a pointer to the channel data.               */
                  ChannelData = &(DeviceInfo->ChannelData);

                  /* Store the SDU.                                     */
                  ChannelData->Max_Data_Size = MaxSDUSize;

                  /* Store the Channel ID.                              */
                  ChannelData->Channel_ID = CID;

                  /* Flag that the Object Transfer Channel is open since*/
                  /* we do not need to respond to this indication.      */
                  ChannelData->Flags |= OTP_CHANNEL_FLAGS_CONNECTED;
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Let the user know that a disconnection request is  */
                  /* going to be sent.                                  */
                  printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", CID);

                  /* Disconnect the channel since an error occured.     */
                  /* * NOTE * We CANNOT cleanup the channel information,*/
                  /*          so we will simply disconnect the channel. */
                  /*          This shouldn't occur.                     */
                  OTS_Channel_Close_Connection(BluetoothStackID, CID);
               }
            }
            break;
         case etOTS_Channel_Open_Request_Indication:
            printf("\r\netOTS_Channel_Open_Request_Indication with size %u.\r\n", OTS_Channel_Event_Data->Event_Data_Size);
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->Role;
               Type         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->Type;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->CID;
               MaxSDUSize   = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->MaxSDUSize;
               Credits      = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Request_Indication_Data->InitialCredits;

               /* Print event information.                              */
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Role:             %s.\r\n", (Role) ? "crServer" : "crClient");
               printf("   Type:             %s.\r\n", (Type) ? "octBR_EDR" : "octLE");
               printf("   CID:              0x%04X.\r\n", CID);

               /* Display event information only if this is an LE       */
               /* Connection.                                           */
               if(Type == octLE)
               {
                  printf("   Max SDU Size:     %u.\r\n", MaxSDUSize);
                  printf("   Initial Credits:  %u.\r\n", Credits);
               }

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* If this is an LE Connection, we need to store the  */
                  /* Channel Information since we are going to accept   */
                  /* the request.                                       */
                  /* * NOTE * LE will NOT receive the                   */
                  /*          etOTS_Channel_Open_Indication after this  */
                  /*          event.  However, BR/EDR will and the      */
                  /*          channel information will be set there.    */
                  if(Type == octLE)
                  {
                     /* Store a pointer to the channel data.            */
                     ChannelData = &(DeviceInfo->ChannelData);

                     /* Store the SDU.                                  */
                     ChannelData->Max_Data_Size = MaxSDUSize;

                     /* Store the Channel ID.                           */
                     ChannelData->Channel_ID = CID;

                     /* Flag that the Object Transfer Channel is open   */
                     /* since we do not need to respond to this         */
                     /* indication.                                     */
                     ChannelData->Flags |= OTP_CHANNEL_FLAGS_CONNECTED;
                  }

                  /* Send the response.                                 */
                  if((Result = OTS_Channel_Open_Connection_Request_Response(BluetoothStackID, OTSInstanceID, CID, TRUE)) != 0)
                     DisplayFunctionError("OTS_Channel_Open_Connection_Request_Response", Result);
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Client.\r\n");

                  /* Let the user know that a disconnection request is  */
                  /* going to be sent.                                  */
                  printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", CID);

                  /* Disconnect the channel since an error occured.     */
                  /* * NOTE * We CANNOT cleanup the channel information,*/
                  /*          so we will simply disconnect the channel. */
                  /*          This shouldn't occur.                     */
                  OTS_Channel_Close_Connection(BluetoothStackID, CID);
               }
            }
            break;
         case etOTS_Channel_Open_Confirmation:
            printf("\r\netOTS_Channel_Open_Confirmation with size %u.\r\n", OTS_Channel_Event_Data->Event_Data_Size);
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->Role;
               Type         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->Type;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->CID;
               Status       = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->Status;
               MaxSDUSize   = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->MaxSDUSize;
               Credits      = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Open_Confirmation_Data->InitialCredits;

               /* Print event information.                              */
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Role:             %s.\r\n", (Role) ? "crServer" : "crClient");
               printf("   Type:             %s.\r\n", (Type) ? "octBR_EDR" : "octLE");
               printf("   CID:              0x%04X.\r\n", CID);
               printf("   Status:           ");
               switch(Status)
               {
                  case OTS_CHANNEL_OPEN_STATUS_SUCCESS:
                     printf("Success.\r\n");
                     break;
                  case OTS_CHANNEL_OPEN_STATUS_CONNECTION_TIMEOUT:
                     printf("Timeout.\r\n");
                     break;
                  case OTS_CHANNEL_OPEN_STATUS_CONNECTION_REFUSED:
                     printf("Refused.\r\n");
                     break;
                  default:
                     printf("Unknown error.\r\n");
                     break;
               }
               printf("   Max SDU Size:     %u.\r\n", MaxSDUSize);

               /* Display event information only if this is an LE       */
               /* Connection.                                           */
               if(Type == octLE)
               {
                  printf("   Initial Credits:  %u.\r\n", Credits);
               }

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure we successfully opened the Object        */
                  /* Transfer Channel before storing the channel        */
                  /* information.                                       */
                  if(Status == OTS_CHANNEL_OPEN_STATUS_SUCCESS)
                  {
                     /* Store a pointer to the channel data.            */
                     ChannelData = &(DeviceInfo->ChannelData);

                     /* Store the SDU.                                  */
                     ChannelData->Max_Data_Size = MaxSDUSize;

                     /* Flag that the Object Transfer Channel is open   */
                     /* since we do not need to respond to this         */
                     /* indication.                                     */
                     ChannelData->Flags |= OTP_CHANNEL_FLAGS_CONNECTED;
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Server.\r\n");

                  /* Let the user know that a disconnection request is  */
                  /* going to be sent.                                  */
                  printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", CID);

                  /* Disconnect the channel since an error occured.     */
                  /* * NOTE * We CANNOT cleanup the channel information,*/
                  /*          so we will simply disconnect the channel. */
                  /*          This shouldn't occur.                     */
                  OTS_Channel_Close_Connection(BluetoothStackID, CID);
               }
            }
            break;
         case etOTS_Channel_Close_Indication:
            printf("\r\netOTS_Channel_Close_Indication with size %u.\r\n", OTS_Channel_Event_Data->Event_Data_Size);
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data->Role;
               Type         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data->Type;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data->CID;
               Reason       = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Close_Indication_Data->Reason;

               /* Print event information.                              */
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:  %s.\r\n", BoardStr);
               printf("   Role:           %s.\r\n", (Role) ? "crServer" : "crClient");
               printf("   Type:           %s.\r\n", (Type) ? "octBR_EDR" : "octLE");
               printf("   CID:            0x%04X.\r\n", CID);
               printf("   Reason:         0x%04X.\r\n", Reason);

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Store a pointer to the channel data.               */
                  ChannelData = &(DeviceInfo->ChannelData);

                  /* Determine if a transfer was in progress when the   */
                  /* connection was terminated.                         */
                  if((ChannelData->Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS) |
                     (ChannelData->Flags & OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS))
                  {
                     /* Let the user know the current progress of the   */
                     /* object transfer.                                */
                     /* * NOTE * This information may be needed for     */
                     /*          recovery.                              */
                     printf("\r\n\r\nObject Transfer Failed.\r\n");
                     printf("   Current Offset:    %u\r\n", (unsigned int)(ChannelData->Current_Offset));
                     printf("   Ending Offset:     %u\r\n", (unsigned int)(ChannelData->Ending_Offset));
                     printf("   Remaining Length:  %u\r\n", (unsigned int)((ChannelData->Ending_Offset - ChannelData->Current_Offset)));
                  }

                  /* Flag that we need to cleanup the channel.          */
                  /* * NOTE * The OTS Channel is already disconnected,  */
                  /*          however there are Channel data and flags  */
                  /*          that MUST be cleared if we have           */
                  /*          disconnected so we will simply set the    */
                  /*          flag.                                     */
                  ChannelData->Flags |= (OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL | OTP_CHANNEL_FLAGS_DISCONNECT_CHANNEL);
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown OTP Server.\r\n");
               }
            }
            break;
         case etOTS_Channel_Data_Indication:
            /* * NOTE * We will not display any information about this  */
            /*          event.                                          */
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->Role;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->CID;
               DataLength   = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->DataLength;
               Data         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->Data;
               Credits      = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Indication_Data->CreditsConsumed;

               /* Get the device information.                           */
               /* * NOTE * We can receive this event for both sides of  */
               /*          the connection.                              */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Store a pointer to the channel data.               */
                  ChannelData = &(DeviceInfo->ChannelData);

                  /* We need to make sure if we are the OTP Client and  */
                  /* the Read Procedure has been aborted we do not      */
                  /* process and received data.                         */
                  /* * NOTE * The channel has already been cleaned up at*/
                  /*          this point.                               */
                  if((Role == crServer) || ((Role == crClient) && (ChannelData->Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)))
                  {
                     /* Determine if this is the OTS Directory Listing  */
                     /* Object.                                         */
                     if(ChannelData->Flags & OTP_CHANNEL_FLAGS_BUFFER_READY_READY_TO_RECEIVE)
                     {
                        /* Process the data that has been received.     */
                        ProcessDirectoryListingObjectContents(ChannelData, DataLength, Data);
                     }
                     else
                     {
                        /* Process the data that has been received.     */
                        ProcessObjectContents(DeviceInfo->FilteredObjectList.Current_Object, ChannelData, DataLength, Data);
                     }
                  }

                  /* If this is an LE based connection.                 */
                  if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_BR_EDR_CONNECTED))
                  {
                     /* If we are in manual credit mode we need to grant*/
                     /* credits back to the device.                     */
                     if(ChannelData->LE_Channel_Parameters.ChannelFlags & L2CA_LE_CHANNEL_PARAMETER_FLAGS_MANUAL_CREDIT_MODE)
                     {
                        if((Result = OTS_Channel_Grant_Credits(BluetoothStackID, CID, Credits)) != 0)
                           DisplayFunctionError("OTS_Channel_Grant_Credits", Result);
                     }
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown remote device.\r\n");

                  /* Let the user know that a disconnection request is  */
                  /* going to be sent.                                  */
                  printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", CID);

                  /* Disconnect the channel since an error occured.     */
                  /* * NOTE * We CANNOT cleanup the channel information,*/
                  /*          so we will simply disconnect the channel. */
                  /*          This shouldn't occur.                     */
                  OTS_Channel_Close_Connection(BluetoothStackID, CID);
               }
            }
            break;
         case etOTS_Channel_Data_Error_Indication:
            printf("\r\netetOTS_Channel_Data_Error_Indication with size %u.\r\n", OTS_Channel_Event_Data->Event_Data_Size);
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data->Role;
               Type         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data->Type;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data->CID;
               Error        = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Data_Error_Indication_Data->Error;

               /* Print event information.                              */
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:  %s.\r\n", BoardStr);
               printf("   Role:           %s.\r\n", (Role) ? "crServer" : "crClient");
               printf("   Type:           %s.\r\n", (Type) ? "octBR_EDR" : "octLE");
               printf("   CID:            0x%04X.\r\n", CID);
               printf("   Error:          0x%04X.\r\n", Error);

               /* ** NOTE ** If this event is received then an          */
               /*            unrecoverable error occured while receiving*/
               /*            an SDU.  Any previous data received will   */
               /*            still be valid.  It is the OTP Client's    */
               /*            responsibility to perform recovery.  For   */
               /*            example, the OTP Client may re-open the    */
               /*            channel and restart the transfer with the  */
               /*            starting offset set to the offset of the   */
               /*            object that was not transfered and the     */
               /*            length set to the remaining length of the  */
               /*            object that was not received.              */
            }
            break;
         case etOTS_Channel_Buffer_Empty_Indication:
            /* * NOTE * We will not display any information about this  */
            /*          event.                                          */
            if(OTS_Channel_Event_Data->Event_Data.OTS_Channel_Buffer_Empty_Indication_Data)
            {
               /* Store event information.                              */
               RemoteDevice = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Buffer_Empty_Indication_Data->RemoteDevice;
               Role         = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Buffer_Empty_Indication_Data->Role;
               CID          = OTS_Channel_Event_Data->Event_Data.OTS_Channel_Buffer_Empty_Indication_Data->CID;

               /* Get the device information.                           */
               /* * NOTE * We can receive this event for both sides of  */
               /*          the connection.                              */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Store a pointer to the channel data.               */
                  ChannelData = &(DeviceInfo->ChannelData);

                  /* We need to make sure if we are the OTP Server and  */
                  /* the Read Procedure has been aborted we do not send */
                  /* anymore data.                                      */
                  /* * NOTE * The channel has already been cleaned up at*/
                  /*          this point.                               */
                  if((Role == crClient) || ((Role == crServer) && (ChannelData->Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)))
                  {
                     /* Send more data.                                 */
                     SendObjectContents(ChannelData);
                  }
               }
               else
               {
                  /* Display the error.                                 */
                  printf("\r\nUnknown remote device.\r\n");

                  /* Let the user know that a disconnection request is  */
                  /* going to be sent.                                  */
                  printf("\r\nObject Transfer Channel disconnect request sent. (CID = 0x%04X).\r\n", CID);

                  /* Disconnect the channel since an error occured.     */
                  /* * NOTE * We CANNOT cleanup the channel information,*/
                  /*          so we will simply disconnect the channel. */
                  /*          This shouldn't occur.                     */
                  OTS_Channel_Close_Connection(BluetoothStackID, CID);
               }
            }
            break;
         default:
            printf("\r\nUnknown OTS Channel Event.\r\n");
            break;
      }

      /* If the channel data is valid.                                  */
      if(ChannelData)
      {
         /* If we flagged for cleanup.                                  */
         if(ChannelData->Flags & OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL)
         {
            /* Simply call the internal function to cleanup the channel.*/
            /* * NOTE * This function will also optionally disconnect   */
            /*          the channel if requested.                       */
            CleanupChannel(ChannelData);
         }
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\nOTS Channel Callback Data: Event_Data = NULL.\r\n");
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
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
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
static void BTPSAPI GATT_ClientEventCallback_OTS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   int                        Result;
   DeviceInfo_t              *DeviceInfo;
   BoardStr_t                 BoardStr;

   /* GATT OTP Client Event Callback data types.                        */
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   GATT_Request_Error_Type_t  ErrorType;
   Byte_t                    *Value;
   Word_t                     ValueOffset;
   Word_t                     ValueLength;
   Word_t                     AttributeHandle;
   Word_t                     BytesWritten;
   Word_t                     MTU;
   Word_t                     Configuration;
   Byte_t                    *Buffer;
   Word_t                     BufferLength;

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
                     case OTS_ERROR_CODE_WRITE_REQUEST_REJECTED:
                        printf("   Error Mesg:       OTS_ERROR_CODE_WRITE_REQUEST_REJECTED\r\n");
                        break;
                     case OTS_ERROR_CODE_OBJECT_NOT_SELECTED:
                        printf("   Error Mesg:       OTS_ERROR_CODE_OBJECT_NOT_SELECTED\r\n");
                        break;
                     case OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED:
                        printf("   Error Mesg:       OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED\r\n");
                        break;
                     case OTS_ERROR_CODE_OBJECT_NAME_ALREADY_EXISTS:
                        printf("   Error Mesg:       OTS_ERROR_CODE_OBJECT_NAME_ALREADY_EXISTS\r\n");
                        break;
                     case OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION:
                        printf("   Error Mesg:       OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION\r\n");
                        break;
                     case OTS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED:
                        printf("   Error Mesg:       OTS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED\r\n");
                        break;
                     case OTS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS:
                        printf("   Error Mesg:       OTS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS\r\n");
                        break;
                     case OTS_ERROR_CODE_UNLIKELY_ERROR:
                        printf("   Error Mesg:       OTS_ERROR_CODE_UNLIKELY_ERROR\r\n");
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
                  switch(RequestData.AttributeHandleType)
                  {
                     case ahtOTSFeature:
                        /* Simply call the internal function to decode  */
                        /* and display the OTS Feature.                 */
                        DecodeDisplayOTSFeature(ValueLength, Value);
                        break;
                     case ahtObjectMetadata:
                        /* Simply call the internal function to decode  */
                        /* and display the OTS Object Metadata.         */
                        DecodeDisplayObjectMetadata(&(DeviceInfo->OTSClientInfo), AttributeHandle, ValueLength, Value);
                        break;
                     case ahtObjectActionControlPoint:
                     case ahtObjectListControlPoint:
                     case ahtObjectChanged:
                        /* Intentional fall-through since these OTS     */
                        /* Characteristics CANNOT be read.              */
                        break;
                     case ahtObjectActionControlPoint_CCCD:
                     case ahtObjectListControlPoint_CCCD:
                     case ahtObjectChanged_CCCD:
                        /* Intentional fall-through since we can handle */
                        /* these cases similarly.                       */

                        /* Verify the ValueLength.                      */
                        if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                        {
                           /* Decode the Value.                         */
                           Configuration = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                           /* Display the OTS Characteristic based on   */
                           /* the attribute handle type.                */
                           switch(RequestData.AttributeHandleType)
                           {
                              case ahtObjectActionControlPoint_CCCD:
                                 printf("\r\nObject Action Control Point CCCD:\r\n");
                                 break;
                              case ahtObjectListControlPoint_CCCD:
                                 printf("\r\nObject List Control Point CCCD:\r\n");
                                 break;
                              case ahtObjectChanged_CCCD:
                                 printf("\r\nObject Changed CCCD:\r\n");
                                 break;
                              default:
                                 /* Can't occur.                        */
                                 break;
                           }

                           /* Display the value.                        */
                           printf("   Value: 0x%04X\r\n", Configuration);
                        }
                        else
                           printf("\r\nError - Invalid length.\r\n");
                        break;
                     case ahtObjectListFilter_1:
                     case ahtObjectListFilter_2:
                     case ahtObjectListFilter_3:
                        /* Intentional fall-through since we can handle */
                        /* these cases similarly.                       */

                        /* Simply call the internal function to decode  */
                        /* and display the OTS Object List Filter.      */
                        DecodeDisplayObjectListFilterData(ValueLength, Value);
                        break;
                     default:
                        printf("\r\nInvalid attribute handle type.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nUnknown OTP Server.\r\n");
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
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
               printf("   Transaction ID:   %u.\r\n", TransactionID);
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   BD_ADDR:          %s.\r\n", BoardStr);
               printf("   Handle:           0x%04X.\r\n", AttributeHandle);
               printf("   Value Length:     %u.\r\n", ValueLength);

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if((Value) && (ValueLength != 0))
               {
                  /* Make sure we can get the device information.       */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* Use the attribute handle type set when the      */
                     /* request was sent to handle the response.        */
                     switch(RequestData.AttributeHandleType)
                     {
                        case ahtObjectMetadata:
                           /* Simply call the internal function to      */
                           /* decode and display the OTS Object         */
                           /* Metadata.                                 */
                           DecodeDisplayObjectMetadata(&(DeviceInfo->OTSClientInfo), AttributeHandle, ValueLength, Value);
                           break;
                        case ahtObjectListFilter_1:
                        case ahtObjectListFilter_2:
                        case ahtObjectListFilter_3:
                           /* Intentional fall-through since we can     */
                           /* handle these cases similarly.             */

                           /* Simply call the internal function to      */
                           /* decode and display the OTS Object List    */
                           /* Filter.                                   */
                           DecodeDisplayObjectListFilterData(ValueLength, Value);
                           break;
                        default:
                           printf("\r\nInvalid attribute handle type.\r\n");
                           break;
                     }
                  }
                  else
                     printf("\r\nUnknown OTP Server.\r\n");
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
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
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
               if((Value) && (ValueLength != 0))
               {
                  /* Get the device information.                        */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
                  {
                     /* ** NOTE ** We will handle all requests similarly*/
                     /*            and the request data MUST have been  */
                     /*            set previously before issuing the    */
                     /*            first GATT Prepare Write request.    */

                     /* Increment the offset that we have prepared thus */
                     /* far.                                            */
                     RequestData.BufferOffset += ValueLength;

                     /* Determine if we have prepared all the data.     */
                     /* * NOTE * We will stop if we go over the expected*/
                     /*          length.                                */
                     if(RequestData.BufferOffset >= RequestData.BufferLength)
                     {
                        /* If we have prepared all the data.            */
                        if(RequestData.BufferOffset == RequestData.BufferLength)
                        {
                           /* Send the GATT Execute Write request to    */
                           /* write the prepared data.                  */
                           /* * NOTE * We will not save the             */
                           /*          transactionID returned by this   */
                           /*          function, which we could use to  */
                           /*          cancel the request.              */
                           if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, FALSE, GATT_ClientEventCallback_OTS, AttributeHandle)) > 0)
                           {
                              printf("\r\nGATT Execute Write request sent:\r\n");
                              printf("   TransactionID:  %d\r\n", Result);
                              printf("   Cancel Write:   No\r\n");
                           }
                        }
                        else
                        {
                           printf("\r\nBuffer offset is greater than the buffer length.\r\n");

                           /* Send the GATT Execute Write request to    */
                           /* cancel the request since an error was     */
                           /* detected.                                 */
                           /* * NOTE * We will not save the             */
                           /*          transactionID returned by this   */
                           /*          function, which we could use to  */
                           /*          cancel the request.              */
                           if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, TRUE, GATT_ClientEventCallback_OTS, AttributeHandle)) > 0)
                           {
                              printf("\r\nGATT Execute Write request sent:\r\n");
                              printf("   TransactionID:  %d\r\n", Result);
                              printf("   Cancel Write:   Yes\r\n");
                           }
                        }
                     }
                     else
                     {
                        /* Update the Buffer and BufferLength that we   */
                        /* are going to send since we have more data to */
                        /* prepare.                                     */
                        Buffer       = (RequestData.Buffer + RequestData.BufferOffset);
                        BufferLength = (RequestData.BufferLength - RequestData.BufferOffset);

                        /* Send the next GATT Prepare Write Request.    */
                        if((Result = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, BufferLength, RequestData.BufferOffset, Buffer, GATT_ClientEventCallback_OTS, AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Prepare Write Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n", Result);
                           printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                           printf("   Offset:            %u\r\n", RequestData.BufferOffset);
                        }
                        else
                           DisplayFunctionError("GATT_Prepare_Write_Request", Result);
                     }
                  }
                  else
                     printf("\r\nUnknown OTP Server.\r\n");
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
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
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
               printf("   Connection Type:   %s.\r\n", (ConnectionType == gctLE) ? "LE" : "BR/EDR");
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
   int                    Result;
   DeviceInfo_t          *DeviceInfo;
   GAP_Encryption_Mode_t  EncryptionMode;
   BoardStr_t             BoardStr;
   BD_ADDR_t              RemoteDevice;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   Word_t                 MTU;
   Word_t                 AttributeHandle;
   Word_t                 ValueLength;
   Byte_t                *Value;
   GAP_LE_Address_Type_t  AddressType;

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
               ConnectionID   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
               ConnectionType = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType;
               RemoteDevice   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;
               MTU            = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);
               printf("   Connection MTU:   %u.\r\n", MTU);

               /* We will always set the currently selected remote      */
               /* device to the remote device that just connected.      */
               ConnectionBD_ADDR = RemoteDevice;

               /* Let the user know the new selected remote device.     */
               printf("\r\nSelected remote device: %s\r\n", BoardStr);

               /* Check if the device information exists.               */
               /* * NOTE * This will be the case if we have previously  */
               /*          connected and bonded with the remote device. */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) == NULL)
               {
                  if(ConnectionType == gctLE)
                  {
                     /* Store the address type saved when the GAP LE    */
                     /* Connection Complete event was received.         */
                     AddressType = ConnectionData.AddressType;
                  }
                  else
                  {
                     /* We will always set the address type for BR/EDR  */
                     /* to public even though it will not be used.      */
                     AddressType = latPublic;
                  }

                  /* No entry exists so create one.                     */
                  /* * NOTE * We will set the LE address type to public */
                  /*          for the br/edr connection (required for   */
                  /*          this function).                           */
                  if(CreateNewDeviceInfoEntry(&DeviceInfoList, AddressType, RemoteDevice))
                  {
                     /* Find the device info entry that was just        */
                     /* created.                                        */
                     DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice);
                  }
                  else
                     printf("\r\nFailed to find/add device information.\r\n");
               }

               /* Make sure the device information is valid.            */
               if(DeviceInfo)
               {
                  /* Increment the number of connections.               */
                  Connections++;

                  /* Store the GATT Connection ID for the remote device.*/
                  DeviceInfo->ConnectionID = ConnectionID;

                  /* Determine the transport for the connection to      */
                  /* finish configuring the device information.         */
                  if(ConnectionType == gctLE)
                  {
                     /* Store whether the remote device is the master of*/
                     /* the connection.                                 */
                     /* * NOTE * This is not used for BR/EDR.           */
                     DeviceInfo->RemoteDeviceIsMaster = ((LocalDeviceIsMaster) ? FALSE : TRUE);

                     /* Let's query the encryption mode to determine if */
                     /* we need to flag that encryption has been        */
                     /* enabled.                                        */
                     if((Result = GAP_LE_Query_Encryption_Mode(BluetoothStackID, RemoteDevice, &EncryptionMode)) == 0)
                     {
                        if(EncryptionMode != emDisabled)
                        {
                           /* * NOTE * LE will not have a LTK created   */
                           /*          during the connection unlike     */
                           /*          BR/EDR, however it will already  */
                           /*          be stored if the remote device   */
                           /*          was previously bonded.           */

                           /* Flag that we are encrypted.               */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                        }
                     }
                     else
                        DisplayFunctionError("GAP_Query_Encryption_Mode", Result);

                     /* If we are the master in the connection.         */
                     if(LocalDeviceIsMaster)
                     {
                        /* Attempt to update the MTU to the maximum     */
                        /* supported.                                   */
                        GATT_Exchange_MTU_Request(BluetoothStackID, DeviceInfo->ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_OTS, 0);
                     }
                  }
                  else
                  {
                     /* Flag that we are now connected over the BR/EDR  */
                     /* transport.                                      */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_BR_EDR_CONNECTED;

                     /* Let's query the encryption mode to determine if */
                     /* we need to flag that encryption has been        */
                     /* enabled.                                        */
                     if((Result = GAP_Query_Encryption_Mode(BluetoothStackID, RemoteDevice, &EncryptionMode)) == 0)
                     {
                        if(EncryptionMode != emDisabled)
                        {
                           /* If we do not have a stored link key then  */
                           /* we need to save the link key that was     */
                           /* created during pairing.                   */
                           if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID))
                           {
                              /* Store the link key.                    */
                              DeviceInfo->LinkKey = ConnectionData.LinkKey;
                              DeviceInfo->Flags  |= DEVICE_INFO_FLAGS_LINK_KEY_VALID;
                           }

                           /* Flag that we are encrypted.               */
                           DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                        }
                     }
                     else
                        DisplayFunctionError("GAP_Query_Encryption_Mode", Result);
                  }

                  /* If this is the OTP Server                          */
                  if(OTSInstanceID)
                  {
                     /* Make sure we have not exceeded the maximum      */
                     /* number of supported clients.                    */
                     if(Connections <= OTP_MAXIMUM_SUPPORTED_CLIENTS)
                     {
                        /* Flag that this device may read/write OTS     */
                        /* Characteristics and descriptors.             */
                        DeviceInfo->ConcurrencyLimitExceeded = FALSE;
                     }
                     else
                     {
                        /* Flag that this device may read/write OTS     */
                        /* Characteristics and descriptors.             */
                        DeviceInfo->ConcurrencyLimitExceeded = TRUE;
                     }
                  }
               }
               else
               {
                  /* Disconnect the link depending on the connection    */
                  /* type.                                              */
                  if(ConnectionType == gctLE)
                  {
                     if((Result = GAP_LE_Disconnect(BluetoothStackID, RemoteDevice)) != 0)
                        DisplayFunctionError("GAP_LE_Disconnect", Result);
                  }
                  else
                  {
                     if((Result = GATT_Disconnect(BluetoothStackID, ConnectionID)) != 0)
                        DisplayFunctionError("GATT_Disconnect", Result);
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
               ConnectionID   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID;
               ConnectionType = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType;
               RemoteDevice   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:    %s.\r\n", BoardStr);

               /* Get the device information for the remote device.     */
               /* * NOTE * We will NOT be connected if the stack is     */
               /*          currently closing (All device information    */
               /*          will be freed).                              */
               if((Connections) && (DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* If we are the OTP Server.                          */
                  if(OTSInstanceID)
                  {
                     /* Simply call the internal function to            */
                     /* cleanup/free all resources for the OTP Server   */
                     /* following a disconnection.                      */
                     DisconnectionCleanup(DeviceInfo);
                  }

                  /* Decrement the number of connections.               */
                  Connections--;

                  /* Clear the Connection ID.                           */
                  DeviceInfo->ConnectionID = 0;

                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* Flag that there is no BR_EDR connection for the    */
                  /* device.                                            */
                  /* * NOTE * This will only be set for a BR/EDR        */
                  /*          connection.                               */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_BR_EDR_CONNECTED;

                  /* If this device is not bonded, then delete it.  The */
                  /* link will be encrypted if the device is bonded.    */
                  if((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED)) || (!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_KEY_VALID)))
                  {
                     if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, RemoteDevice)) != NULL)
                        FreeDeviceInfoEntryMemory(DeviceInfo);
                  }
                  else
                  {
                     /* Flag that the Link is no longer encrypted since */
                     /* we have disconnected.                           */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                  }
               }

               /* Check if we are still connected to a remote device.   */
               if(Connections)
               {
                  /* Loop through the device information list.          */
                  DeviceInfo = DeviceInfoList;
                  while(DeviceInfo)
                  {
                     /* If we find a remote device that is connected.   */
                     if(DeviceInfo->ConnectionID)
                     {
                        /* Make the first remote device found the       */
                        /* currently selected remote device.            */
                        ConnectionBD_ADDR = DeviceInfo->ConnectionBD_ADDR;
                        break;
                     }

                     /* Get the next devices information.               */
                     DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
                  }
               }
               else
               {
                  /* Make sure the selected device is invalid.          */
                  ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               }

               /* Let the user know if there is a new selected remote   */
               /* device.                                               */
               BD_ADDRToStr(ConnectionBD_ADDR, BoardStr);
               printf("\r\nSelected remote device: %s\r\n", BoardStr);
            }
            else
               printf("\r\nError - Null Disconnection Data.\r\n");
            break;
         case etGATT_Connection_Device_Connection_MTU_Update:
            printf("\r\netGATT_Connection_Device_Connection_MTU_Update with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size);
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data)
            {
               /* Store the event data.                                 */
               ConnectionID   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionID;
               ConnectionType = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionType;
               RemoteDevice   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->RemoteDevice;
               MTU            = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->MTU;

               printf("   Connection ID:    %u.\r\n", ConnectionID);
               printf("   Connection Type:  %s.\r\n", (ConnectionType == gctLE)?"LE":"BR/EDR");
               BD_ADDRToStr(RemoteDevice, BoardStr);
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
               RemoteDevice    = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->RemoteDevice;
               ConnectionID    = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionID;
               TransactionID   = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->TransactionID;
               ConnectionType  = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->ConnectionType;
               AttributeHandle = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeHandle;
               ValueLength     = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValueLength;
               Value           = GATT_Connection_Event_Data->Event_Data.GATT_Server_Indication_Data->AttributeValue;

               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Connection ID:     %u.\r\n", ConnectionID);
               printf("   Transaction ID:    %u.\r\n", TransactionID);
               printf("   Connection Type:   %s.\r\n", ((ConnectionType == gctLE) ? "LE" : "BR/EDR"));
               printf("   Remote Device:     %s.\r\n", BoardStr);
               printf("   Attribute Handle:  0x%04X.\r\n", AttributeHandle);
               printf("   Attribute Length:  %d.\r\n", ValueLength);

               /* Send the GATT confirmation.                           */
               if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, ConnectionID, TransactionID)) != 0)
                  DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);

               /* Get the remote device information.                    */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  /* * NOTE * The OTP Client should have written the    */
                  /*          corresponding CCCD before an indication is*/
                  /*          received.  This means service discovery   */
                  /*          MUST have been performed.  Otherwise the  */
                  /*          OTP Server sent an indication for an      */
                  /*          unconfigured CCCD.  This is an error.     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Check if the indication is for the Object Action*/
                     /* Control Point attribute handle.                 */
                     if(AttributeHandle == DeviceInfo->OTSClientInfo.Object_Action_Control_Point)
                     {
                        /* Simply call the internal function to decode  */
                        /* and display the OACP response data.          */
                        DecodeDisplayOACPResponseData(ValueLength, Value, &(DeviceInfo->ChannelData));

                        /* Determine if the OACP Read Procedure is in   */
                        /* progress.                                    */
                        if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_READ_PROCEDURE_IN_PROGRESS)
                        {
                           /* Display information about the transfer.   */
                           printf("\r\nObject Transfer started.\r\n");
                           printf("  Procedure:       Read\r\n");
                           printf("  Object Name:     \"%s\"\r\n", CurrentObject.Name);
                           printf("  Current Offset:  %u\r\n",     DeviceInfo->ChannelData.Current_Offset);
                           printf("  Ending Offset:   %u\r\n",     DeviceInfo->ChannelData.Ending_Offset);
                           printf("  Buffer Length:   %u\r\n",     DeviceInfo->ChannelData.Buffer_Length);
                        }

                        /* Determine if we need to start the transfer   */
                        /* for the OACP Write Procedure.                */
                        if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_WRITE_PROCEDURE_IN_PROGRESS)
                        {
                           /* Display information about the transfer.   */
                           printf("\r\nObject Transfer started.\r\n");
                           printf("  Procedure:       Write\r\n");
                           printf("  Object Name:     \"%s\"\r\n", CurrentObject.Name);
                           printf("  Current Offset:  %u\r\n",     DeviceInfo->ChannelData.Current_Offset);
                           printf("  Ending Offset:   %u\r\n",     DeviceInfo->ChannelData.Ending_Offset);
                           printf("  Buffer Length:   %u\r\n\r\n", DeviceInfo->ChannelData.Buffer_Length);

                           /* Simply call the internal function to start*/
                           /* the transfer.                             */
                           /* * NOTE * The OTP Client could not have    */
                           /*          sent the OACP request if the     */
                           /*          buffer was not ready.            */
                           SendObjectContents(&(DeviceInfo->ChannelData));
                        }

                        /* If we flagged for cleanup.                   */
                        if(DeviceInfo->ChannelData.Flags & OTP_CHANNEL_FLAGS_CLEANUP_CHANNEL)
                        {
                           /* If we are performing cleanup here, then we*/
                           /* need to make sure that an OACP Procedure  */
                           /* is no longer in progress.                 */
                           DeviceInfo->Flags &= ~(DEVICE_INFO_FLAGS_OTS_OACP_PROCEDURE_IN_PROGRESS);

                           /* Simply call the internal function to      */
                           /* cleanup the channel.                      */
                           CleanupChannel(&(DeviceInfo->ChannelData));
                        }
                     }
                     else
                     {
                        /* Check if the indication is for the Object    */
                        /* List Control Point attribute handle.         */
                        if(AttributeHandle == DeviceInfo->OTSClientInfo.Object_List_Control_Point)
                        {
                           /* Simply call the internal function to      */
                           /* decode and display the OLCP response data.*/
                           DecodeDisplayOLCPResponseData(ValueLength, Value);
                        }
                        else
                        {
                           /* Check if the indication is for the Object */
                           /* Changed attribute handle.                 */
                           if(AttributeHandle == DeviceInfo->OTSClientInfo.Object_Changed)
                           {
                              /* Simply call the internal function to   */
                              /* decode and display the Descriptor Value*/
                              /* Changed Characteristic.                */
                              DecodeDisplayObjectChangedData(ValueLength, Value);
                           }
                           else
                              printf("\r\nError - Attribute handle is unknown for indication.\r\n");
                        }
                     }

                  }
                  else
                     printf("\r\nError - Service discovery has not been performed.\r\n");


               }
               else
                  printf("\r\nError - Unknown OTP Server.\r\n");
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
   unsigned int                Index;
   DeviceInfo_t               *DeviceInfo;
   GATT_Service_Information_t *ServiceInfo;
   GATT_Service_Information_t *IncludedServiceInfo;
   unsigned int                NumberOfCharacteristics;
   unsigned int                NumberOfIncludedServices;

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
                        OTSPopulateHandles(&(DeviceInfo->OTSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                     case sdOTS:
                        /* Attempt to populate the handles for the OTS  */
                        /* Service.                                     */
                        OTSPopulateHandles(&(DeviceInfo->OTSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summaries.     */
                        DisplayGATTDiscoverySummary(&(DeviceInfo->GATTClientInfo));
                        DisplayDISDiscoverySummary(&(DeviceInfo->DISClientInfo));
                        DisplayGAPSDiscoverySummary(&(DeviceInfo->GAPSClientInfo));
                        DisplayOTSDiscoverySummary(&(DeviceInfo->OTSClientInfo));
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
                     case sdOTS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_OTS_SERVICE_DISCOVERY_COMPLETE;

                        /* Display the service discovery summary.       */
                        DisplayOTSDiscoverySummary(&(DeviceInfo->OTSClientInfo));
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
         case OTSP_PARAMETER_VALUE:
            /* The Transport selected was OTSP, check to see if the     */
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
         printf("Parameter Error (Expecting = [USB = 0, UART = 1, OTSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
      }
   }
   else
   {
      /* An invalid number of parameters was entered on the Command     */
      /* Line.                                                          */
      printf("Parameter Error (Expecting = [USB = 0, UART = 1, OTSP = 2 Flag] [IF !USB [Comm Port or Device File] [Baud Rate]])\r\n");
   }

   return 0;
}
