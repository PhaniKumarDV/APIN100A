/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< linuxaiop.c >**********************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXAIOP - Linux Bluetooth Automation IO Service using GATT (LE/BREDR)   */
/*             sample application.                                            */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/02/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxAIOP.h"           /* Application Header.                       */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */

#include "SS1BTAIOS.h"           /* Main SS1 AIOS Service Header.             */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */

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
                                                         /* create a AIOS     */
                                                         /* Server.           */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

#define UNABLE_TO_ALLOCATE_MEMORY                  (-11) /* Denotes that we   */
                                                         /* failed because we */
                                                         /* couldn't allocate */
                                                         /* memory.           */

#define AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS   (2)   /* Denotes the number*/
                                                         /* of                */
                                                         /* characteristics   */
                                                         /* supported by the  */
                                                         /* AIOS Server.      */

#define AIOP_NUMBER_OF_SUPPORTED_INSTANCES         (2)   /* Denotes the number*/
                                                         /* of                */
                                                         /* instances for each*/
                                                         /* Characteristic    */
                                                         /* supported by the  */
                                                         /* AIOS Server.      */

#define AIOP_DEFAULT_NUMBER_OF_DIGITALS            (8)   /* Denotes the number*/
                                                         /* of default digital*/
                                                         /* signals           */
                                                         /* supported by each.*/
                                                         /* Digital           */
                                                         /* Characteristic.   */
                                                         /* MUST be a multiple*/
                                                         /* of 4 for this     */
                                                         /* application.      */

#define AIOP_DIGITAL_ARRAY_SIZE                    (2)   /* Denotes the number*/
                                                         /* of bytes needed   */
                                                         /* to hold all       */
                                                         /* digital signals.  */
                                                         /* MUST be (number of*/
                                                         /* digitals /        */
                                                         /* 4). This is       */
                                                         /* included for      */
                                                         /* simplicity.       */

   /* Do not change the Indicate properties below.  It will break the   */
   /* application.                                                      */

#define AIOP_DEFAULT_INPUT_CHARACTERISTIC_PROPERTY_FLAGS  (AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE)
                                                         /* Denotes the default*/
                                                         /* input              */
                                                         /* Characteristic     */
                                                         /* Property Flags.    */

#define AIOP_DEFAULT_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS (AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE                  | \
                                                           AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE | \
                                                           AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_READ                   | \
                                                           AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE)
                                                         /* Denotes the default*/
                                                         /* output             */
                                                         /* Characteristic     */
                                                         /* Property Flags.    */

#define AIOP_DEFAULT_AGGREGATE_PROPERTY_FLAGS             (AIOS_AGGREGATE_PROPERTY_FLAGS_INDICATE)
                                                         /* Denotes the default*/
                                                         /* Aggregate          */
                                                         /* Characteristic     */
                                                         /* Property Flags.    */

   /* Do not change the Indicate properties above.  It will break the   */
   /* application.                                                      */

#define AIOP_DEFAULT_DIGITAL_DESCRIPTOR_FLAGS      (AIOS_DESCRIPTOR_FLAGS_USER_DESCRIPTION      | \
                                                    AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | \
                                                    AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING)
                                                         /* Denotes the       */
                                                         /* default descriptor*/
                                                         /* flags for digital */
                                                         /* characteristics.  */
                                                         /* Number of digitals*/
                                                         /* is mandatory.     */

#define AIOP_DEFAULT_ANALOG_DESCRIPTOR_FLAGS       (AIOS_DESCRIPTOR_FLAGS_USER_DESCRIPTION      | \
                                                    AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | \
                                                    AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING  | \
                                                    AIOS_DESCRIPTOR_FLAGS_VALID_RANGE)
                                                         /* Denotes the       */
                                                         /* default descriptor*/
                                                         /* flags for analog  */
                                                         /* characteristics.  */

#define AIOP_USER_DESCRIPTION_LENGTH               (255)

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxAIOP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxAIOP_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxAIOP"

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
   sdAIOS
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

   /* The following structure contains the information for an AIOS      */
   /* Characteristic instance that the AIOS Server will need to store.  */
   /* * NOTE * The Instance_Entry below will need to be copied to the   */
   /*          AIOS_Characteristic_Entry_t structure, a sub-structure of*/
   /*          AIOS_Initialize_Data_t structure, that is expected as a  */
   /*          parameter to AIOS_Initialize_Service().  This is REQUIRED*/
   /*          to intialize the service and allows us to retain the     */
   /*          information that we used to initialize the service.      */
   /* * NOTE * Some fields of this structure will not be used.  The     */
   /*          fields depend on the optional AIOS Characteristic        */
   /*          descriptors included for this Characteristic instance    */
   /*          specified by the Instance_Entry field and whether this   */
   /*          instance is a Digital or Analog Characteristic.          */
typedef struct _tagAIOP_Server_Instance_Data_t
{
   AIOS_Characteristic_Instance_Entry_t  Instance_Entry;

   union
   {
      Byte_t                             Digital[AIOP_DIGITAL_ARRAY_SIZE];
      Word_t                             Analog;
   } Data;

   Word_t                                Client_Configuration;
   AIOS_Presentation_Format_Data_t       Presentation_Format;
   char                                  User_Description[AIOP_USER_DESCRIPTION_LENGTH];
   AIOS_Value_Trigger_Data_t             Value_Trigger_Setting;
   AIOS_Time_Trigger_Data_t              Time_Trigger_Setting;
   Byte_t                                Number_Of_Digitals;
   AIOS_Valid_Range_Data_t               Valid_Range;
} AIOP_Server_Instance_Data_t;

#define AIOP_SERVER_INSTANCE_DATA_SIZE                   (sizeof(AIOP_Server_Instance_Data_t))

   /* The following structure contains the information for each AIOS    */
   /* Digital/Analog Characteristc that the AIOS Server will need to    */
   /* store.  Information for each AIOS Characteristic instance will be */
   /* stored by the Instances field.                                    */
   /* * NOTE * The Characteristic_Entry field below will need to be     */
   /*          copied to the AIOS_Initialize_Data_t structure that is   */
   /*          expected as a parameter to AIOS_Initialize_Service().    */
   /*          This is REQUIRED to initialize the service and allows us */
   /*          to retain the information that we used to intialize the  */
   /*          service.                                                 */
typedef struct _tagAIOP_Server_Characteristic_Data_t
{
   AIOS_Characteristic_Entry_t  Characteristic_Entry;
   AIOP_Server_Instance_Data_t  Instances[AIOP_NUMBER_OF_SUPPORTED_INSTANCES];
} AIOP_Server_Characteristic_Data_t;

#define AIOP_SERVER_CHARACTERISTIC_DATA_SIZE             (sizeof(AIOP_Server_Characteristic_Data_t))

   /* The following structure contains the AIOS Server information.     */
   /* This information (and sub structures) are needed to initialize the*/
   /* AIOS Server with a call to AIOS_Initialize_Service().  This       */
   /* structure will also hold the information needed to process AIOS   */
   /* Server events and will retain the values for AIOS Characteristics */
   /* and descriptors.                                                  */
   /* * NOTE * Some fields below will need to be copied to the          */
   /*          AIOS_Initialize_Data_t structure that is expected as a   */
   /*          parameter to AIOS_Initialize_Service().  This is REQUIRED*/
   /*          to initialize the service and allows us to retain the    */
   /*          information that we used to intialize the service.       */
   /* * NOTE * The Aggregate Characteristic is stored in this structure */
   /*          since there is only one.  Digital/Analog Characteristics */
   /*          and all their instances will be stored in the            */
   /*          Characteristic field.                                    */
   /* * NOTE * The Aggregate_CC or Aggregate Client Configuration will  */
   /*          not be valid unless Notify or Indicate is supported by   */
   /*          the Aggregate_Property_Flags field.  Include_Aggregate   */
   /*          MUST be TRUE for the Aggregate Characteristic to be used.*/
typedef struct _tagAIOP_Server_Information_t
{
   AIOP_Server_Characteristic_Data_t  Characteristic[AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS];
   Boolean_t                          Aggregate_Supported;
   Byte_t                             Aggregate_Property_Flags;
   Word_t                             Aggregate_CC;
} AIOP_Server_Information_t;

#define AIOP_SERVER_INFORMATION_DATA_SIZE                (sizeof(AIOP_Server_Information_t))

   /* The following enumeration will be used to determine the correct   */
   /* Attribute Handle to select for an AIOS Characteristic or          */
   /* Descriptor.                                                       */
typedef enum
{
   ahtCharacteristic,
   ahtExtendedProperties,
   ahtClientCharacteristicConfig,
   ahtPresentationFormat,
   ahtValueTriggerSetting,
   ahtTimeTriggerSetting,
   ahtUserDescription,
   ahtNumberOfDigitals,
   ahtValidRange
} AIOP_Attribute_Handle_Type_t;

   /* The following structure is used to store information that needs to*/
   /* be stored for an AIOP Client write request.                       */
typedef struct _tagAIOP_Write_Request_Data_t
{
   Boolean_t                      UseWriteWithoutResponse;
   Word_t                         UserDescriptionLength;
   union
   {
      AIOS_Characteristic_Data_t  Characteristic;
      Word_t                      CCCD;
      AIOS_Value_Trigger_Data_t   ValueTriggerSetting;
      AIOS_Time_Trigger_Data_t    TimeTriggerSetting;
      Byte_t                      Configuration;
      char                        UserDescription[AIOP_USER_DESCRIPTION_LENGTH];
      AIOS_Valid_Range_Data_t     ValidRange;
   } Data;
} AIOP_Write_Request_Data_t;

#define AIOP_WRITE_REQUEST_DATA_SIZE                     (sizeof(AIOP_Write_Request_Data_t))

   /* The following structure holds the request information that the    */
   /* AIOP Client may store before issuing a GATT request to the AIOS   */
   /* Server.                                                           */
   /* ** NOTE ** The Type and ID fields MUST be valid for all requests  */
   /*            since this information is required to quickly look up  */
   /*            the AIOS Characteristic Instances information          */
   /*            associated with the request in the                     */
   /*            GATT_ClientEventCallback_AIOS() when the response is   */
   /*            received.                                              */
   /* ** NOTE ** The AttributeHandleType field (Mandatory) allows us to */
   /*            specify the type of the attribute handle we are        */
   /*            expecting in the response.  This way with the Type and */
   /*            ID fields, we can quickly locate the correct attribute */
   /*            handle to verify.  Otherwise we would need to check    */
   /*            every attribute handle for a match to know how to      */
   /*            process the response.                                  */
typedef struct _tagAIOP_Client_Request_Info_t
{
   AIOS_Characteristic_Type_t    Type;
   unsigned int                  ID;
   AIOP_Attribute_Handle_Type_t  AttributeHandleType;
   char                          UserDescription[AIOP_USER_DESCRIPTION_LENGTH];
   Word_t                        UserDescriptionOffset;
   AIOP_Write_Request_Data_t     Write_Data;
} AIOP_Client_Request_Info_t;

   /* The following structure contains the information that needs to be */
   /* stored by an AIOS Client for each AIOS Characteristic instance    */
   /* discovered during service discovery.  This struture also stores   */
   /* the information that the AIOP Client needs to store when          */
   /* read/writing AIOS Characteristic instances.                       */
   /* * NOTE * The Valid field indicates that the Attribute handle      */
   /*          information for this AIOS Characteristic instance has    */
   /*          previously been stored during service discovery.         */
   /* * NOTE * The Properties field will simply be used to store the    */
   /*          Characteristic instance properties found during service  */
   /*          discovery.  If we determined that the Aggregate          */
   /*          Characteristic is supported by the AIOS Server we can    */
   /*          simply check if the Digital/Analog Characteristic        */
   /*          instance has the read property.  If it does, it is       */
   /*          included in the Aggregate Characteristic.                */
   /* * NOTE * The Number_Of_Digitals will hold the number of digitals  */
   /*          that has been automatically read by the AIOS Client if   */
   /*          the Aggregate Characteristic is discoverd and after      */
   /*          service discovery has been peformed.  This is REQUIRED   */
   /*          since in order to decode the Aggregate Characteristic we */
   /*          MUST know how many digitals are included for each Digital*/
   /*          Characteristic that is part of the Aggregate             */
   /*          Characteristic.                                          */
   /* * NOTE * Some handles may not be cached or used if the AIOS Server*/
   /*          does not support some descriptors.                       */
   /* * NOTE * Either the Digital_Characteristic_Handle or              */
   /*          Analog_Charactersitic_Handle will be cached.  Only one   */
   /*          will be cached for this instance and can be determined by*/
   /*          the Characteristic type (Type field) of the parent       */
   /*          structure below.                                         */
   /* * NOTE * The AIOS_Number_Of_Digitals_Handle and                   */
   /*          AIOS_Valid_Range_Handle will only be cached depending on */
   /*          the Characteristic type (Type field) of the parent       */
   /*          structure.  Must be an Analog Characteristic instance.   */
typedef struct _tagAIOP_Client_Instance_Info_t
{
   Boolean_t       Valid;
   Byte_t          Properties;
   AIOS_IO_Type_t  IOType;
   Byte_t          Number_Of_Digitals;

   Word_t          Analog_Charactersitic_Handle;
   Word_t          Digital_Characteristic_Handle;
   Word_t          Extended_Properties_Handle;
   Word_t          CCCD_Handle;
   Word_t          Presentation_Format_Handle;
   Word_t          User_Description_Handle;
   Word_t          Value_Trigger_Setting_Handle;
   Word_t          Time_Trigger_Setting_Handle;
   Word_t          Number_Of_Digitals_Handle;
   Word_t          Valid_Range_Handle;
} AIOP_Client_Instance_Info_t;

#define AIOP_CLIENT_INSTANCE_INFO_SIZE                   (sizeof(AIOP_Client_Instance_Info_t))

   /* The following structure contains the information that needs to be */
   /* stored by an AIOS Client for a specified AIOS Characteristic type */
   /* and all of its instances that may be cached by an AIOP Client     */
   /* during service discovery.                                         */
   /* * NOTE * The Number_Of_Instances will be determined during service*/
   /*          discovery depending on how many instances of a Digital or*/
   /*          Analog Characteristic are discovered.                    */
   /* * NOTE * The AIOS Client will use a unique InstanceID to access   */
   /*          the AIOS Characteristic instance's information (the      */
   /*          Instances field).  InstanceID's start at zero and will be*/
   /*          assigned implicitly (we won't store these values, however*/
   /*          they will be capped by the Number_Of_Instances field).   */
   /*          For simplicitly the InstanceID's will directly correspond*/
   /*          to ID field of the AIOS_Characteristic_Info_t structure  */
   /*          used to identify AIOS Characteristic instances on the    */
   /*          this sample application running as the AIOS Server.  It  */
   /*          is worth noting that if this sample application running  */
   /*          as the AIOS Client is used with another AIOS Server this */
   /*          MAY NOT hold, however we will still access AIOS          */
   /*          Characteristic instances by using the InstanceID.        */
typedef struct _tagAIOP_Client_Characteristic_Info_t
{
   AIOS_Characteristic_Type_t   Type;
   Byte_t                       Number_Of_Instances;
   AIOP_Client_Instance_Info_t *Instances;
} AIOP_Client_Characteristic_Info_t;

#define AIOP_CLIENT_CHARACTERISTIC_INFO_SIZE             (sizeof(AIOP_Client_Characteristic_Info_t))

   /* The following structure contains the information that will need to*/
   /* be cached by a AIOS Client in order to only do service discovery  */
   /* once.  This structure also contains the information that needs to */
   /* be stored by an AIOP Client when read/writing AIOS Characteristic */
   /* instances.                                                        */
   /* * NOTE * The Characteristics field may only be valid for a Digital*/
   /*          or Analog Characteristic.  The Aggregate Characteristic  */
   /*          information will be stored in this structure separately  */
   /*          if it is found during service discovery.                 */
   /* * NOTE * The Number_Digital_Characteristics_In_Aggregate field    */
   /*          will be used to quickly determine how many Digital       */
   /*          Characteristics are included in the Aggregate            */
   /*          Characteristic during service discovery.  We can use this*/
   /*          information to automatically issue GATT read requests    */
   /*          (after service discovery has been peformed) for the      */
   /*          Number Of Digitals descriptor for each Digital           */
   /*          Characteristic included in the Aggregate that needs to be*/
   /*          cached in order to decode the Aggregate Characteristic.  */
typedef struct _tagAIOP_Client_Information_t
{
   AIOP_Client_Characteristic_Info_t   Characteristics[AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS];
   Byte_t                              Num_Digital_Characteristics_In_Aggregate;
   Word_t                              Aggregate_Handle;
   Word_t                              Aggregate_CCCD_Handle;
   AIOP_Client_Request_Info_t          Client_Request_Info;
} AIOP_Client_Information_t;

#define AIOP_CLIENT_INFORMATION_DATA_SIZE                (sizeof(AIOP_Client_Information_t))

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
   AIOP_Client_Information_t  AIOPClientInfo;
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
#define DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE   0x0080
#define DEVICE_INFO_FLAGS_READING_NUMBER_OF_DIGITALS        0x0100

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
static unsigned int        AIOSInstanceID;
                                                    /* The following holds the AIOS    */
                                                    /* Instance IDs that are returned  */
                                                    /* from AIOS_Initialize_XXX().     */

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

static AIOP_Server_Information_t  AIOSServerInfo;   /* Variable which holds the        */
                                                    /* information needed by the AIOS  */
                                                    /* Server for characteristics.     */

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

   /* Auotmation IO Profile functions.                                  */
static void InitializeAIOSServer(void);

static void AIOSPopulateHandles(AIOP_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreAttributeHandles(AIOP_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Boolean_t AggregateFound);
static void StoreDescriptorHandles(AIOP_Client_Instance_Info_t *InstanceInfoPtr, GATT_Characteristic_Information_t *CharacteristicInfoPtr);

static AIOP_Client_Instance_Info_t *GetClientInstanceInfoPtr(AIOP_Client_Information_t *ClientInfo);
static AIOP_Server_Instance_Data_t *GetServerInstanceInfoPtr(AIOS_Characteristic_Info_t *CharacteristicInfo);

static int ReadAIOSCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, AIOP_Attribute_Handle_Type_t AttributeHandleType);
static int ReadLongUserDescriptionRequest(AIOS_Characteristic_Info_t *CharacteristicInfo);
static int WriteAIOSCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, AIOP_Attribute_Handle_Type_t AttributeHandleType, AIOP_Write_Request_Data_t *WriteRequestData);

static int IndicateNotifyCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, Boolean_t Notify);

static int FormatAggregateCharacteristic(AIOS_Aggregate_Characteristic_Data_t *AggregateData);

static void DisplayCharacteristicInfo(AIOS_Characteristic_Info_t *CharacteristicInfo);
static void DisplayPresentationFormatData(AIOS_Presentation_Format_Data_t *PresentationFormatData);
static void DisplayValueTriggerSetting(AIOS_Value_Trigger_Data_t *ValueTriggerData);
static void DisplayTimeTriggerSetting(AIOS_Time_Trigger_Data_t *TimeTriggerData);
static void DisplayDigitalCharacteristic(AIOS_Digital_Characteristic_Data_t *DigitalData, unsigned int ID);
static void DisplayDigitalByte(Byte_t DigitalByte);
static void DisplayAnalogCharacteristic(Word_t AnalogData, unsigned int ID);

static void DecodeDisplayDigitalCharacteristic(Word_t ValueLength, Byte_t *Value, unsigned int ID);
static void DecodeDisplayAnalogCharacteristic(Word_t ValueLength, Byte_t *Value, unsigned int ID);
static void DecodeDisplayAggregateCharacteristic(AIOP_Client_Information_t *ClientInfo, Word_t ValueLength, Byte_t *Value);

   /* Automation IO Profile commands.                                   */
static int RegisterAIOSCommand(ParameterList_t *TempParam);
static int UnRegisterAIOSCommand(ParameterList_t *TempParam);
static int DiscoverAIOSCommand(ParameterList_t *TempParam);

static int ReadAIOSCharacteristicCommand(ParameterList_t *TempParam);

static int WriteCCCDCommand(ParameterList_t *TempParam);
static int WriteUserDescriptionCommand(ParameterList_t *TempParam);
static int WriteValueTriggerSettingCommand(ParameterList_t *TempParam);
static int WriteTimeTriggerSettingCommand(ParameterList_t *TempParam);
static int WriteDigitalCommand(ParameterList_t *TempParam);
static int WriteAnalogCommand(ParameterList_t *TempParam);

static int IndicateNotifyCommand(ParameterList_t *TempParam);

static void DisplayReadAIOSCharacteristicUsage(void);
static void DisplayWriteCCCDUsage(void);
static void DisplayWriteValueTriggerUsage(void);
static void DisplayWriteTimeTriggerUsage(void);
static void DisplayIndicateNotifyUsage(void);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI AIOS_EventCallback(unsigned int BluetoothStackID, AIOS_Event_Data_t *AIOS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_DIS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_AIOS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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
   /*          applications for use with AIOS/AIOP.                     */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   /* If there were Digital Characteristic instances discovered we need */
   /* to free their information that was allocated during service       */
   /* discovery.                                                        */
   if((EntryToFree->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances) && (EntryToFree->AIOPClientInfo.Characteristics[actDigital].Instances))
   {
      BTPS_FreeMemory(EntryToFree->AIOPClientInfo.Characteristics[actDigital].Instances);
      EntryToFree->AIOPClientInfo.Characteristics[actDigital].Instances           = NULL;
      EntryToFree->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances = 0;
   }

   /* If there were Analog Characteristic instances discovered we need  */
   /* to free their information that was allocated during service       */
   /* discovery.                                                        */
   if((EntryToFree->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances) && (EntryToFree->AIOPClientInfo.Characteristics[actAnalog].Instances))
   {
      BTPS_FreeMemory(EntryToFree->AIOPClientInfo.Characteristics[actAnalog].Instances);
      EntryToFree->AIOPClientInfo.Characteristics[actAnalog].Instances           = NULL;
      EntryToFree->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances = 0;
   }

   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
   /* * NOTE * This function has been modified from other sample        */
   /*          applications for use with AIOS/AIOP.                     */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   DeviceInfo_t *Index;

   /* Loop through the device information list.                         */
   Index = *ListHead;
   while(Index)
   {
      /* If there were Digital Characteristic instances discovered we   */
      /* need to free their information that was allocated during       */
      /* service discovery.                                             */
      if((Index->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances) && (Index->AIOPClientInfo.Characteristics[actDigital].Instances))
      {
         BTPS_FreeMemory(Index->AIOPClientInfo.Characteristics[actDigital].Instances);
         Index->AIOPClientInfo.Characteristics[actDigital].Instances           = NULL;
         Index->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances = 0;
      }

      /* If there were Analog Characteristic instances discovered we    */
      /* need to free their information that was allocated during       */
      /* service discovery.                                             */
      if((Index->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances) && (Index->AIOPClientInfo.Characteristics[actAnalog].Instances))
      {
         BTPS_FreeMemory(Index->AIOPClientInfo.Characteristics[actAnalog].Instances);
         Index->AIOPClientInfo.Characteristics[actAnalog].Instances           = NULL;
         Index->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances = 0;
      }

      /* Get the next devices information.                              */
      Index = Index->NextDeviceInfoPtr;
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
   AddCommand("REGISTERAIOS", RegisterAIOSCommand);
   AddCommand("UNREGISTERAIOS", UnRegisterAIOSCommand);
   AddCommand("DISCOVERAIOS", DiscoverAIOSCommand);
   AddCommand("READ", ReadAIOSCharacteristicCommand);
   AddCommand("WRITECCCD", WriteCCCDCommand);
   AddCommand("WRITEUSERDESCRIPTION", WriteUserDescriptionCommand);
   AddCommand("WRITEVALUETRIGGER", WriteValueTriggerSettingCommand);
   AddCommand("WRITETIMETRIGGER", WriteTimeTriggerSettingCommand);
   AddCommand("WRITEDIGITAL", WriteDigitalCommand);
   AddCommand("WRITEANALOG", WriteAnalogCommand);
   AddCommand("NOTIFY", IndicateNotifyCommand);
   AddCommand("INDICATE", IndicateNotifyCommand);
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

               /* We will check for the AIOS Service data.  Make sure   */
               /* the mandatory UUID is present.                        */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the AIOS Service data.            */
                  if(AIOS_COMPARE_AIOS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
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
   printf("\r\nLinuxAIOP>");

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

      /* Cleanup AIOS Service.                                          */
      if(AIOSInstanceID)
      {
         AIOS_Cleanup_Service(BluetoothStackID, AIOSInstanceID);
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
   printf("* AIOP Sample Application                                        *\r\n");
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
   printf("* Command Options AIOS:                                          *\r\n");
   printf("*                Server:   RegisterAIOS                          *\r\n");
   printf("*                          UnRegisterAIOS,                       *\r\n");
   printf("*                          Notify,                               *\r\n");
   printf("*                          Indicate,                             *\r\n");
   printf("*                Client:   DiscoverAIOS,                         *\r\n");
   printf("*                          WriteCCCD,                            *\r\n");
   printf("*                Both:     Read,                                 *\r\n");
   printf("*                          WriteUserDescription,                 *\r\n");
   printf("*                          WriteValueTrigger,                    *\r\n");
   printf("*                          WriteTimeTrigger,                     *\r\n");
   printf("*                          WriteDigital,                         *\r\n");
   printf("*                          WriteAnalog                           *\r\n");
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
               printf("Passkey Response success.\r\n");

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

            /* Configure the flags field aiosed on the Discoverability  */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            if(AIOSInstanceID)
            {
               /* Advertise the Battery Server(1 byte type and 2 bytes  */
               /* UUID)                                                 */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               AIOS_ASSIGN_AIOS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));
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

   /* The following function will initialize the AIOS Server.           */
static void InitializeAIOSServer(void)
{
   unsigned int                          Index;
   unsigned int                          Index2;
   AIOS_Characteristic_Type_t            Type;
   AIOP_Server_Characteristic_Data_t    *CharacteristicDataPtr;
   AIOP_Server_Instance_Data_t          *InstanceDataPtr;

   /* This SHOULD already be zero but we will just initialize it again. */
   BTPS_MemInitialize(&AIOSServerInfo, 0, AIOP_SERVER_INFORMATION_DATA_SIZE);

   /* Let's go ahead and initialize the root AIOS Server information.   */
   /* We will enable all supported features be default.                 */
   /* * NOTE * If we do not want some features they will be cleared     */
   /*          before we make the call the AIOS_Initialize_Service()    */
   /*          function.                                                */
   /* * NOTE * Each AIOS Characteristic's information is present in the */
   /*          root information, however we will not initialize it till */
   /*          we process the Characteristics below.                    */
   AIOSServerInfo.Aggregate_Supported      = TRUE;
   AIOSServerInfo.Aggregate_Property_Flags = (Byte_t)AIOP_DEFAULT_AGGREGATE_PROPERTY_FLAGS;

   /* Aggregate CC will alreay be set to zero.                          */

   /* Loop to initialize the AIOS Characteristic's information that will*/
   /* be used for all AIOS Characteristic instances.                    */
   for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS; Index++)
   {
      /* The Index will be used to indicate the AIOS Characteristic     */
      /* type.                                                          */
      Type = (AIOS_Characteristic_Type_t)Index;

      /* Store a pointer to the Characteristic information.             */
      CharacteristicDataPtr = &(AIOSServerInfo.Characteristic[Type]);

      /* Let's go ahead and initialize the Characteristic's information.*/
      /* * NOTE * We will not set the Instances field since it will not */
      /*          be used for the AIOS Server.  It will be set for the  */
      /*          AIOS_Initialize_Data_t structure we when copy this    */
      /*          information to it.                                    */
      CharacteristicDataPtr->Characteristic_Entry.Type                = (AIOS_Characteristic_Type_t)Index;
      CharacteristicDataPtr->Characteristic_Entry.Number_Of_Instances = (Word_t)AIOP_NUMBER_OF_SUPPORTED_INSTANCES;
      CharacteristicDataPtr->Characteristic_Entry.Instances           = NULL;

      /* Loop to initialize all AIOS Characteristic instance's          */
      /* information for this Characteristic type.                      */
      for(Index2 = 0; Index2 < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index2++)
      {
         /* Store a pointer to the Characteristic instance information. */
         /* * NOTE * The Index2 will be used to identify the AIOS       */
         /*          Characteristic instance.  This also directly       */
         /*          corresponds to the ID field that the service will  */
         /*          use for the AIOS_Characteristic_Info_t structure.  */
         InstanceDataPtr = &(CharacteristicDataPtr->Instances[Index2]);

         /* Let's go ahead and initialize the instances IO Type.        */
         /* * NOTE * We will make even indexes inputs and odd indexes   */
         /*          outputs.                                           */
         InstanceDataPtr->Instance_Entry.IO_Type = (Index2 % 2 == 0) ? ioInput : ioOutput;

         /* We will go ahead and initiailze the fields that depend on   */
         /* the IOType.                                                 */
         switch(InstanceDataPtr->Instance_Entry.IO_Type)
         {
            case ioInput:
               InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags = (Byte_t)AIOP_DEFAULT_INPUT_CHARACTERISTIC_PROPERTY_FLAGS;

               /* ** NOTE ** Inputs will automatically be given the read*/
               /*            property since it is mandatory.  However,  */
               /*            since we are going to retain the           */
               /*            Instance_Entry on AIOS Server we will go   */
               /*            ahead and flag this property.              */
               /*            AIOS_Initialize_Service() will NOT FAIL if */
               /*            this flag is set (even though it is not    */
               /*            required for inputs), since it is a valid  */
               /*            property for inputs.  This will also allow */
               /*            the AIOS Server to not have to check if the*/
               /*            AIOS Characteristic is an input or it has  */
               /*            the read property.  We can simply check if */
               /*            it has the read property.                  */
               InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags |= GATT_CHARACTERISTIC_PROPERTIES_READ;

               /* We will go ahead and set the User Description server  */
               /* value to the IO Type.                                 */
               BTPS_StringCopy(InstanceDataPtr->User_Description, "Input Signal");
               break;
            case ioOutput:
               InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags = (Byte_t)AIOP_DEFAULT_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS;

               /* We will go ahead and set the User Description server  */
               /* value to the IO Type.                                 */
               BTPS_StringCopy(InstanceDataPtr->User_Description, "Output Signal");
               break;
            default:
               /* Prevent compiler warnings.                            */
               break;
         }

         /* Let's initialize the Descriptor flags.  This depends on the */
         /* Characteristic Type.                                        */
         switch(Type)
         {
            case actDigital:
               InstanceDataPtr->Instance_Entry.Descriptor_Flags = (Byte_t)AIOP_DEFAULT_DIGITAL_DESCRIPTOR_FLAGS;
               break;
            case actAnalog:
               InstanceDataPtr->Instance_Entry.Descriptor_Flags = (Byte_t)AIOP_DEFAULT_ANALOG_DESCRIPTOR_FLAGS;
               break;
            default:
               /* Prevent compiler warnings.                            */
               break;
         }

         /* Let's initialzize the Descriptor Property Flags.            */
         /* * NOTE * All instances will make the User Description       */
         /*          Writeable.  This will also force the service to    */
         /*          include an Extended Properties descriptor with the */
         /*          'Write Auxilaries' bit enabled.  Otherwise the     */
         /*          Extended Properties descriptor will be excluded.   */
         InstanceDataPtr->Instance_Entry.Descriptor_Property_Flags = (Byte_t)AIOS_DESCRIPTOR_PROPERTY_FLAGS_USER_DESCRIPTION_WRITABLE;

         /* We will not use a custom trigger by default.  If this is    */
         /* enabled then the Value Trigger Setting and Time Trigger     */
         /* Setting descriptors will be excluded.  All conditions for   */
         /* the custom trigger MUST still be met (Notify or Indicate    */
         /* properties).  See AIOSAPI.h for more information.           */
         InstanceDataPtr->Instance_Entry.Use_Custom_Trigger = FALSE;

         /* Now they we have set up the instance entry that will be     */
         /* needed to initialize the Characteristic instance with a call*/
         /* to AIOS_Initialize_Service(), we will setup initial values  */
         /* for the instance's data.                                    */

         /* The Characteristic value, Client Configuration, and Valid   */
         /* Range will already be initialized to zero or disabled for   */
         /* the Client Configuration.                                   */

         /* Initialize the Presentation Format data.                    */
         /* * NOTE * The Description field use to identify the AIOS     */
         /*          Characteristic instance is always 1 greater than   */
         /*          the ID (Index2) that we use to identify it.        */
         if(Type == actDigital)
            InstanceDataPtr->Presentation_Format.Format   = (Byte_t)AIOS_DIGITAL_PRESENTATION_FORMAT_STRUCT;
         else
            InstanceDataPtr->Presentation_Format.Format   = (Byte_t)AIOS_ANALOG_PRESENTATION_FORMAT_UINT16;
         InstanceDataPtr->Presentation_Format.Exponent    = 0;
         InstanceDataPtr->Presentation_Format.Unit        = 0;
         InstanceDataPtr->Presentation_Format.NameSpace   = (Byte_t)AIOS_PRESENTATION_FORMAT_NAMESPACE_BT_SIG;
         InstanceDataPtr->Presentation_Format.Description = (Word_t)(Index2+1);

         /* We will initialize the Value Trigger Setting and Time       */
         /* Trigger Setting to their disabled states.                   */
         InstanceDataPtr->Value_Trigger_Setting.Condition = vttNoValueTrigger;
         InstanceDataPtr->Time_Trigger_Setting.Condition  = tttNoTimeBasedTrigger;

         /* Initialize the Number Of Digitals descriptor.               */
         InstanceDataPtr->Number_Of_Digitals = (Byte_t)AIOP_DEFAULT_NUMBER_OF_DIGITALS;
      }
   }
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a AIOS Client Information structure with  */
   /* the information discovered from a GDIS Discovery operation.       */
static void AIOSPopulateHandles(AIOP_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;
   Boolean_t                          AggregateFound = FALSE;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (AIOS_COMPARE_AIOS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
     /* Loop through all characteristics discovered in the service and  */
     /* populate the correct entry.                                     */
     CharacteristicInfoPtr = ServiceDiscoveryData->CharacteristicInformationList;
     if(CharacteristicInfoPtr)
     {
        /* First we need to go through the AIOS Characteristics and     */
        /* count the number of Digital and Analog Characteristic        */
        /* instances.  This way we can determine how much memory to     */
        /* allocate to hold the instances.                              */
        for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
        {
           /* All AIOS UUIDs are defined to be 16 bit UUIDs.            */
           if(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID_Type == guUUID_16)
           {
              if(AIOS_COMPARE_DIGITAL_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
              {
                 /* Increment the number of Digital Characteristic      */
                 /* instances.  Keep in mind there may be none.         */
                 /* * NOTE * We will hardcode the Characteristic with   */
                 /*          the Digital type to access its number of   */
                 /*          instances.                                 */
                 ClientInfo->Characteristics[actDigital].Number_Of_Instances++;
              }
              else
              {
                 if(AIOS_COMPARE_ANALOG_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
                 {
                    /* Increment the number of Analog Characteristic    */
                    /* instances.  Keep in mind there may be none.      */
                    /* * NOTE * We will hardcode the Characteristic with*/
                    /*          the Analolg type to access its number of*/
                    /*          instances.                              */
                    ClientInfo->Characteristics[actAnalog].Number_Of_Instances++;
                 }
                 else
                 {
                    if(AIOS_COMPARE_AGGREGATE_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
                    {
                       /* Make sure we have not found the Aggregate     */
                       /* Characteristic.                               */
                       if(!AggregateFound)
                       {
                          /* Flag that we discovered the Aggregate      */
                          /* Characteristic.                            */
                          AggregateFound = TRUE;
                       }
                       else
                       {
                          printf("\r\nWarning - More than one Aggregate Characteristic discovered.\r\n");
                       }
                    }
                 }
              }
           }
           else
              printf("\r\nWarning - UUID not 16-bit.\r\n");
        }

        /* Check that we discovered Digital Characteristic instances.   */
        if(ClientInfo->Characteristics[actDigital].Number_Of_Instances)
        {
           /* Allocate memory for the instance's attribute handle       */
           /* information.                                              */
           /* * NOTE * We will hardcode the Characteristic with the     */
           /*          Digital type to access it instances.             */
           if((ClientInfo->Characteristics[actDigital].Instances = (AIOP_Client_Instance_Info_t *)BTPS_AllocateMemory(AIOP_CLIENT_INSTANCE_INFO_SIZE * ClientInfo->Characteristics[actDigital].Number_Of_Instances)) != NULL)
           {
              /* Initialize the memory so we do not have unexpected     */
              /* behaviour.                                             */
              /* * NOTE * We will hardcode the Characteristic with the  */
              /*          Digital type to access it instances           */
              /*          information.                                  */
              BTPS_MemInitialize(ClientInfo->Characteristics[actDigital].Instances, 0, (AIOP_CLIENT_INSTANCE_INFO_SIZE * ClientInfo->Characteristics[actDigital].Number_Of_Instances));
           }
           else
           {
              /* Print an error and return early since we failed.       */
              printf("\r\nFailure - Could not allocate memory for the Digital Characteristic instances.\r\n");
              return;
           }
        }

        /* Check that we discovered Analog Characteristic instances.    */
        if(ClientInfo->Characteristics[actAnalog].Number_Of_Instances)
        {
           /* Allocate memory for the instance's attribute handle       */
           /* information.                                              */
           /* * NOTE * We will hardcode the Characteristic with the     */
           /*          Analog type to access it instances.              */
           if((ClientInfo->Characteristics[actAnalog].Instances = (AIOP_Client_Instance_Info_t *)BTPS_AllocateMemory(AIOP_CLIENT_INSTANCE_INFO_SIZE * ClientInfo->Characteristics[actAnalog].Number_Of_Instances)) != NULL)
           {
              /* Initialize the memory so we do not have unexpected     */
              /* behaviour.                                             */
              /* * NOTE * We will hardcode the Characteristic with the  */
              /*          Analog type to access it instances            */
              /*          information.                                  */
              BTPS_MemInitialize(ClientInfo->Characteristics[actAnalog].Instances, 0, (AIOP_CLIENT_INSTANCE_INFO_SIZE * ClientInfo->Characteristics[actAnalog].Number_Of_Instances));
           }
           else
           {
              /* Print an error and return early since we failed.       */
              printf("\r\nFailure - Could not allocate memory for the Analog Characteristic instances.\r\n");
              return;
           }
        }

        /* Now that we have allocated memory to hold the AIOS           */
        /* Characteristic instance's attribute handle information, we   */
        /* need to populate it..                                        */
        for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
        {
           /* All AIOS UUIDs are defined to be 16 bit UUIDs.            */
           if(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID_Type == guUUID_16)
           {
              /* Print the characteristic information discovered if     */
              /* needed.                                                */
              if(PRINT_SERVICE_DISCOVERY_INFORMATION)
              {
                 printf("\r\nCharacteristic:\r\n");
                 printf("   Handle:        0x%04X\r\n", CharacteristicInfoPtr[Index].Characteristic_Handle);
                 printf("   Properties:    0x%02X\r\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
                 printf("   UUID:          0x");
                 DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));

                 /* * NOTE * More information will be printed by the    */
                 /*          StoreAttributeHandles helper function.     */
              }

              /* Simply call the helper function to determine the       */
              /* Characteristic Type and store the attribute handles.   */
              StoreAttributeHandles(ClientInfo, &(CharacteristicInfoPtr[Index]), AggregateFound);
           }
           else
              printf("\r\nWarning - UUID not 16-bit.\r\n");
        }
     }
   }
}

   /* The following function is a helper function for                   */
   /* AIOSPopulateHandles() and is responsible for storing the attribute*/
   /* handles for all AIOS Characteristics.                             */
static void StoreAttributeHandles(AIOP_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfoPtr, Boolean_t AggregateFound)
{
   unsigned int                                  Index;
   AIOP_Client_Instance_Info_t                  *InstanceInfoPtr;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfoPtr;

   /* If this is a Digital Characteristic.                              */
   if(AIOS_COMPARE_DIGITAL_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
   {
      /* Find an empty AIOP Client Instance Information structure to    */
      /* store this attribute handle information.                       */
      for(Index = 0; Index < ClientInfo->Characteristics[actDigital].Number_Of_Instances; Index++)
      {
         /* Store a pointer to the Attribute Handle instance to aid in  */
         /* readability.                                                */
         InstanceInfoPtr = &(ClientInfo->Characteristics[actDigital].Instances[Index]);

         /* If this instance information has not been used we will store*/
         /* the attribute handle information.                           */
         /* * NOTE * We determined the number of instances earlier for  */
         /*          all the AIOS Characteristics so there MUST be an   */
         /*          empty entry.                                       */
         if(InstanceInfoPtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            InstanceInfoPtr->Valid = TRUE;

            /* Store the properties.                                    */
            InstanceInfoPtr->Properties = CharacteristicInfoPtr->Characteristic_Properties;

            /* Check the Digital Characteristic properties to determine */
            /* information that needs to be stored.                     */
            if((InstanceInfoPtr->Properties & (AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE | AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE)))
            {
               /* Store the IO Type.                                    */
               InstanceInfoPtr->IOType = ioOutput;
            }
            else
            {
               /* * NOTE * It is impossible to determine if this is an  */
               /*          error, but we will assume that since Write or*/
               /*          Write without response is not supported this */
               /*          MUST be an input Characteristic.             */

               /* This MUST be a Digital Characteristic input so lets   */
               /* check for Mandatory read property.                    */
               if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
               {
                  //* Store the IO Type.                                */
                  InstanceInfoPtr->IOType = ioInput;
               }
               else
               {
                  printf("Warning - Invalid properties for the Digital Characteristic instance!\r\n");
               }
            }

            /* If the Aggregate Characteristic is supported we will     */
            /* check to see if the Characteristic has the read property.*/
            if(AggregateFound)
            {
               if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
               {
                  /* Increment the number of Digital Characteristics in */
                  /* the Aggregate Characteristic.                      */
                  ClientInfo->Num_Digital_Characteristics_In_Aggregate++;

                  /* Print that this Characteristic instance is included*/
                  /* in the Aggregate.                                  */
                  if(PRINT_SERVICE_DISCOVERY_INFORMATION)
                  {
                     printf("   In Aggregate:  Yes\r\n");
                  }
               }
            }

            /* Print the Characteristic instance information discovered */
            /* if needed.                                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {

               printf("   Type:          Digital\r\n");
               if(InstanceInfoPtr->IOType == ioInput)
                  printf("   IO Type:       Input\r\n");
               else
                  printf("   IO Type:       Output.\r\n");
               printf("   Instance:      %u\r\n", Index);
            }

            /* Store the attribute handle information for this Digital  */
            /* Characteristic.                                          */
            InstanceInfoPtr->Digital_Characteristic_Handle = CharacteristicInfoPtr->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(InstanceInfoPtr, CharacteristicInfoPtr);

            /* Return since we found the AIOS Characteristic type.      */
            return;
         }
      }
   }

   /* If this is a Analog Characteristic.                               */
   if(AIOS_COMPARE_ANALOG_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
   {
      /* Find an empty AIOP Client Instance Information structure to    */
      /* store this attribute handle information.                       */
      for(Index = 0; Index < ClientInfo->Characteristics[actAnalog].Number_Of_Instances; Index++)
      {
         /* Store a pointer to the Attribute Handle instance to aid in  */
         /* readability.                                                */
         InstanceInfoPtr = &(ClientInfo->Characteristics[actAnalog].Instances[Index]);

         /* If this instance information has not been used we will store*/
         /* the attribute handle information.                           */
         /* * NOTE * We determined the number of instances earlier for  */
         /*          all the AIOS Characteristics so there MUST be an   */
         /*          empty entry.                                       */
         if(InstanceInfoPtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            InstanceInfoPtr->Valid = TRUE;

            /* Store the properties.                                    */
            InstanceInfoPtr->Properties = CharacteristicInfoPtr->Characteristic_Properties;

            /* Check the Analog Characteristic properties to determine  */
            /* information that needs to be stored.                     */
            if((InstanceInfoPtr->Properties & (AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE | AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE_WITHOUT_RESPONSE)))
            {
               /* Store the IO Type.                                    */
               InstanceInfoPtr->IOType = ioOutput;
            }
            else
            {
               /* * NOTE * It is impossible to determine if this is an  */
               /*          error, but we will assume that since Write or*/
               /*          Write without response is not supported this */
               /*          MUST be an input Characteristic.             */

               /* This MUST be a Analog Characteristic input so lets    */
               /* check for Mandatory read property.                    */
               if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
               {
                  //* Store the IO Type.                                */
                  InstanceInfoPtr->IOType = ioInput;
               }
               else
               {
                  printf("Warning - Invalid properties for the Analog Characteristic instance!\r\n");
               }
            }

            /* If the Aggregate Characteristic is supported we will     */
            /* check to see if the Characteristic has the read property.*/
            if(AggregateFound)
            {
               if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
               {
                  /* Print that this Characteristic instance is included*/
                  /* in the Aggregate.                                  */
                  if(PRINT_SERVICE_DISCOVERY_INFORMATION)
                  {
                     printf("   In Aggregate:  Yes\r\n");
                  }
               }
            }

            /* Print the Characteristic instance information discovered */
            /* if needed.                                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("   Type:          Analog\r\n");
               if(InstanceInfoPtr->IOType == ioInput)
                  printf("   IO Type:       Input\r\n");
               else
                  printf("   IO Type:       Output.\r\n");
               printf("   Instance:      %u\r\n", Index);
            }

            /* Store the attribute handle information for this Analog   */
            /* Characteristic.                                          */
            InstanceInfoPtr->Analog_Charactersitic_Handle = CharacteristicInfoPtr->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(InstanceInfoPtr, CharacteristicInfoPtr);

            /* Return since we found the AIOS Characteristic type.      */
            return;
         }
      }
   }

   /* If this is the Aggregate Characteristic.                          */
  if(AIOS_COMPARE_AGGREGATE_CHARACTERISTIC_UUID_TO_UUID_16(CharacteristicInfoPtr->Characteristic_UUID.UUID.UUID_16))
  {
     /* Verify that the read property is supported.                     */
     if(!(CharacteristicInfoPtr->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
        printf("\r\nWarning - Mandatory read property of the Aggregate characteristic not supported!\r\n");

     /* Store the Aggregate Characteristic handle.                      */
     ClientInfo->Aggregate_Handle = CharacteristicInfoPtr->Characteristic_Handle;

     /* Store a pointer to the descriptor list.                         */
     DescriptorInfoPtr = CharacteristicInfoPtr->DescriptorList;

     /* Populate the Agggregate Characteristic descriptors.             */
     for(Index = 0; Index < CharacteristicInfoPtr->NumberOfDescriptors; Index++)
     {
        /* Make sure the descriptor is a 16 bit UUID.                   */
        if(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
        {
           /* Check for the Aggregate CCCD.                             */
           if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
           {
              if(PRINT_SERVICE_DISCOVERY_INFORMATION)
              {
                printf("\r\nDescriptor:\r\n");
                 printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
                 printf("   UUID:    0x");
                 DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
                 printf("   Type:    CCCD\r\n");
              }

              /* Store the handle.                                      */
              ClientInfo->Aggregate_CCCD_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
              continue;
           }

           /* Always print information about unknown Characteristic     */
           /* descriptors.                                              */
           if(PRINT_SERVICE_DISCOVERY_INFORMATION)
           {
              printf("\r\nWarning - Unknown characteristic descriptor.\r\n");
           }
           else
           {
              printf("\r\nDescriptor:\r\n");
              printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
              printf("   UUID:    0x");
              DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
              printf("\r\nWarning - Unknown characteristic descriptor.\r\n");
           }
        }
        else
           printf("\r\nUUID Type not 16.\r\n");
     }

     /* Return since we found the AIOS Characteristic type.             */
     return;
  }

   /* Always print warnings about unknown AIOS Characteristics.         */
   if(PRINT_SERVICE_DISCOVERY_INFORMATION)
   {
      /* We already printed the hanlde information so just print the    */
      /* warning.                                                       */
      printf("\r\nWarning - Unknown AIOS Characteristic.\r\n");
   }
   else
   {
      printf("\r\nCharacteristic:\r\n");
      printf("   Handle:        0x%04X\r\n", CharacteristicInfoPtr[Index].Characteristic_Handle);
      printf("   Properties:    0x%02X\r\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
      printf("   UUID:          0x");
      DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));
   }
}

   /* The following function is a helper function for                   */
   /* StoreAttributeHandles() to populate the descriptor handles for an */
   /* AIOS Characteristic instance.                                     */
static void StoreDescriptorHandles(AIOP_Client_Instance_Info_t *InstanceInfoPtr, GATT_Characteristic_Information_t *CharacteristicInfoPtr)
{
   unsigned int                                  Index;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfoPtr;

   /* Store a pointer to the Descriptor information.                    */
   DescriptorInfoPtr = CharacteristicInfoPtr->DescriptorList;

   /* Loop through the descriptor information and store the descriptor  */
   /* handles.                                                          */
   for(Index = 0; Index < CharacteristicInfoPtr->NumberOfDescriptors; Index++)
   {
      /* Print the characteristic descriptor information if needed.     */
      if(PRINT_SERVICE_DISCOVERY_INFORMATION)
      {
         printf("\r\nDescriptor:\r\n");
         printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
         printf("   UUID:    0x");
         DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
         printf("   Type:    ");
      }

      /* Make sure the Descriptor UUID is 16-bit.                       */
      if(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
      {
         /* Check for the extended properties.                          */
         if(GATT_COMPARE_CHARACTERISTIC_EXTENDED_PROPERTIES_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Extended Properties\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Extended_Properties_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the CCCD.                                         */
         if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("CCCD\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->CCCD_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Presentation Format descriptor.               */
         if(GATT_COMPARE_CHARACTERISTIC_PRESENTATION_FORMAT_ATTRIBUTE_TYPE_TO_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Presentation Format\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Presentation_Format_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the User Description descriptor.                  */
         if(GATT_COMPARE_CHARACTERISTIC_USER_DESCRIPTION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("User Description\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->User_Description_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Value Trigger Setting descriptor.             */
         if(AIOS_COMPARE_VALUE_TRIGGER_SETTING_CD_UUID_TO_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Value Trigger Setting\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Value_Trigger_Setting_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Time Trigger Setting descriptor.              */
         if(AIOS_COMPARE_TIME_TRIGGER_SETTING_CD_UUID_TO_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Time Trigger Setting\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Time_Trigger_Setting_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Number Of Digitals descriptor.                */
         /* * NOTE * This should ONLY be received for Digital           */
         /*          Characteristics, however we will not check this    */
         /*          here.                                              */
         if(AIOS_COMPARE_NUMBER_OF_DIGITALS_CD_UUID_TO_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Number Of Digitals\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Number_Of_Digitals_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Valid Range descriptor.                       */
         /* * NOTE * This should ONLY be received for Analog            */
         /*          Characteristics, however we will not check this    */
         /*          here.                                              */
         if(AIOS_COMPARE_VALID_RANGE_CD_UUID_TO_UUID_16(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            /* Print the Descriptor type.                               */
            if(PRINT_SERVICE_DISCOVERY_INFORMATION)
            {
               printf("Valid Range\r\n");
            }

            /* Store the handle.                                        */
            InstanceInfoPtr->Valid_Range_Handle = DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Print the Descriptor type.                                  */
         if(PRINT_SERVICE_DISCOVERY_INFORMATION)
         {
            /* Only print a warning for the unknown Characteristic      */
            /* descriptor since we already printed handle information.  */
            printf("\r\nWarning - Unknown Characteristic descriptor.\r\n");
         }
         else
         {
            /* Always print warnings for unknown characteristic         */
            /* descriptors.                                             */
            printf("\r\nDescriptor:\r\n");
            printf("   Handle:  0x%04X\r\n", DescriptorInfoPtr[Index].Characteristic_Descriptor_Handle);
            printf("   UUID:    0x");
            DisplayUUID(&(DescriptorInfoPtr[Index].Characteristic_Descriptor_UUID));
            printf("\r\nWarning - Unknown Characteristic descriptor.\r\n");
         }
      }
   }
}

   /* The following function is responsible for getting a pointer to the*/
   /* AIOSS Client Characteristic instance's information based on the   */
   /* DeviceInfo parameter.  If this function is successful a pointer to*/
   /* the AIOS Client's instance information will be returned.          */
   /* Otherwise NULL will be returned.                                  */
   /* * NOTE * This function should NOT be used for the Aggregate       */
   /*          Characteristic.                                          */
static AIOP_Client_Instance_Info_t *GetClientInstanceInfoPtr(AIOP_Client_Information_t *ClientInfo)
{
   AIOP_Client_Instance_Info_t *ret_val = NULL;
   AIOS_Characteristic_Type_t   Type;
   unsigned int                 ID;

   /* Make sure the parameters are semi-valid.                          */
   if(ClientInfo)
   {
      /* Store the Type and ID to make the code more readable.          */
      Type = ClientInfo->Client_Request_Info.Type;
      ID   = ClientInfo->Client_Request_Info.ID;

      /* Make sure the type is valid.                                   */
      if((Type >= actDigital) && (Type <= actAnalog))
      {
         /* Make sure the ID is less than or equal to the number of     */
         /* attribute handle information entries.                       */
         /* * NOTE * This should directly correspond to the number of   */
         /*          instances of an AIOS Characteristic Type we        */
         /*          discovered.                                        */
         if(ID < ClientInfo->Characteristics[Type].Number_Of_Instances)
         {
            /* Since we have the type and instance identifier we can use*/
            /* these values to index the ESS Client Attribute Handle    */
            /* information and get the correct instance.                */
            ret_val = &(ClientInfo->Characteristics[Type].Instances[ID]);
         }
         else
            printf("\r\nWarning - Invalid Characteristic instance ID.\r\n");
      }
      else
         printf("\r\nWarning - Invalid Characteristic type.\r\n");
   }
   else
      printf("\r\nWarning - Device information invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for getting a pointer to the*/
   /* AIOS Server Characteristic instance information based on the      */
   /* CharacteristicInfo parameter.  If this function is successful a   */
   /* pointer to the instance will be returned.  Otherwise NULL will be */
   /* returned.                                                         */
   /* * NOTE * This function should NOT be used for the Aggregate       */
   /*          Characteristic.                                          */
static AIOP_Server_Instance_Data_t *GetServerInstanceInfoPtr(AIOS_Characteristic_Info_t *CharacteristicInfo)
{
   AIOP_Server_Instance_Data_t *ret_val = NULL;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Make sure the type is valid.                                   */
      if((CharacteristicInfo->Type >= actDigital) && (CharacteristicInfo->Type <= actAnalog))
      {
         /* Make sure the ID is less than the number of supported       */
         /* instances.                                                  */
         /* * NOTE * This should directly correspond to the number of   */
         /*          instances of an AIOS Characteristic Type we        */
         /*          discovered.                                        */
         if(CharacteristicInfo->ID < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES)
         {
            /* Since we have the type and instance identifier we can use*/
            /* these values to index the AIOS Server Characteristic     */
            /* instance information.                                    */
            ret_val = &(AIOSServerInfo.Characteristic[CharacteristicInfo->Type].Instances[CharacteristicInfo->ID]);
         }
         else
            printf("\r\nWarning - Invalid Characteristic instance ID.\r\n");
      }
      else
         printf("\r\nWarning - Invalid Characteristic type.\r\n");
   }
   else
      printf("\r\nWarning - Characteristic information invalid.\r\n");

   return (ret_val);
}

   /* The following function is an internal function to read an AIOS    */
   /* Characteristic or its descriptors.  This function may be called by*/
   /* the AIOS Server or AIOS Client.                                   */
static int ReadAIOSCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, AIOP_Attribute_Handle_Type_t AttributeHandleType)
{
   int                                 ret_val = 0;
   unsigned int                        Index;
   DeviceInfo_t                       *DeviceInfo;
   AIOS_Digital_Characteristic_Data_t  DigitalData;
   AIOS_Characteristic_Type_t          Type;
   unsigned int                        ID;
   Word_t                              AttributeHandle = 0;
   AIOP_Server_Instance_Data_t        *ServerInstancePtr;
   AIOP_Client_Characteristic_Info_t  *ClientCharacteristicPtr;
   AIOP_Client_Instance_Info_t        *ClientInstancePtr;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the AIOS Characteristic Type is valid.               */
      if((Type >= actDigital) && (Type <= actAggregate))
      {
         /* If we are the AIOS Server.                                  */
         if(AIOSInstanceID)
         {
            /* We will handle the Aggregate Characteristic type         */
            /* separately.                                              */
            if(Type == actAggregate)
            {
               /* Make sure the Aggregate Characteristic is supported.  */
               if(AIOSServerInfo.Aggregate_Supported)
               {
                  /* Determine the attribute handle type to display the */
                  /* correct Characteristic or descriptor.              */
                  switch(AttributeHandleType)
                  {
                     case ahtCharacteristic:
                        /* Display the Digital Characteristic's that are*/
                        /* included in the Aggregate Characteristic     */
                        /* first.                                       */
                        for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index++)
                        {
                           /* Store a pointer to the instance data to   */
                           /* aid in readability.                       */
                           ServerInstancePtr = &(AIOSServerInfo.Characteristic[actDigital].Instances[Index]);

                           /* Check that this Characteristic is included*/
                           /* in the Aggregate Characteristic.          */
                           if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
                           {
                              /* Set the Digital Characteristic to      */
                              /* display since the internal function    */
                              /* expects this type.                     */
                              DigitalData.Buffer = ServerInstancePtr->Data.Digital;
                              DigitalData.Length = AIOP_DIGITAL_ARRAY_SIZE;

                              /* Display the Digital Characteristic.    */
                              DisplayDigitalCharacteristic(&DigitalData, Index);
                           }
                        }

                        /* Display the Analog Characteristic's that are */
                        /* included in the Aggregate Characteristic     */
                        /* first.                                       */
                        for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index++)
                        {
                           /* Store a pointer to the instance data to   */
                           /* aid in readability.                       */
                           ServerInstancePtr = &(AIOSServerInfo.Characteristic[actAnalog].Instances[Index]);

                           /* Check that this Characteristic is included*/
                           /* in the Aggregate Characteristic.          */
                           if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
                           {
                              /* Display the Analog Characteristic.     */
                              DisplayAnalogCharacteristic(ServerInstancePtr->Data.Analog, Index);
                           }
                        }
                        break;
                     case ahtClientCharacteristicConfig:
                        printf("\r\nClient Configuration:\r\n");
                        printf("   Value:  0x%04X\r\n", AIOSServerInfo.Aggregate_CC);
                        break;
                     default:
                        printf("\r\nInvalid attribute handle type.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nAggregate Characteristic not supported.\r\n");
            }
            else
            {
               /* This MUST be a Digital/Analog Characteristic instance.*/

               /* Make sure the instance specified exists for the AIOS  */
               /* Characteristic Type.                                  */
               if(ID < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES)
               {
                  /* Store a pointer to the instance data to aid in     */
                  /* readability.                                       */
                  /* * NOTE * We already checked the Type and ID earlier*/
                  /*          so it MUST be valid.                      */
                  ServerInstancePtr = &(AIOSServerInfo.Characteristic[Type].Instances[ID]);

                  /* Determine the attribute handle type to display the */
                  /* correct Characteristic or descriptor.              */
                  switch(AttributeHandleType)
                  {
                     case ahtCharacteristic:
                        /* Determine the how to display the             */
                        /* Characteristic depending on the              */
                        /* Characteristic type.                         */
                        switch(Type)
                        {
                           case actDigital:
                              /* Set the Digital Characteristic to      */
                              /* display since the internal function    */
                              /* expects this type.                     */
                              DigitalData.Buffer = ServerInstancePtr->Data.Digital;
                              DigitalData.Length = AIOP_DIGITAL_ARRAY_SIZE;

                              /* Display the Digital Characteristic.    */
                              DisplayDigitalCharacteristic(&DigitalData, ID);
                              break;
                           case actAnalog:
                              /* Display the Analog Characteristic.     */
                              DisplayAnalogCharacteristic(ServerInstancePtr->Data.Analog, ID);
                              break;
                           default:
                              printf("\r\nInvalid Characteristic type.\r\n");
                              break;
                        }
                        break;
                     case ahtExtendedProperties:
                        printf("\r\nError - AIOS Server cannot read the Extended Properties descriptor.\r\n");
                        break;
                     case ahtClientCharacteristicConfig:
                        printf("\r\nClient Configuration:\r\n");
                        printf("   Value:  0x%04X\r\n", ServerInstancePtr->Client_Configuration);
                        break;
                     case ahtPresentationFormat:
                        DisplayPresentationFormatData(&ServerInstancePtr->Presentation_Format);
                        break;
                     case ahtValueTriggerSetting:
                        DisplayValueTriggerSetting(&ServerInstancePtr->Value_Trigger_Setting);
                        break;
                     case ahtTimeTriggerSetting:
                        DisplayTimeTriggerSetting(&ServerInstancePtr->Time_Trigger_Setting);
                        break;
                     case ahtUserDescription:
                        printf("\r\nUser Description:\r\n");
                        printf("   Value:   %s\r\n", ServerInstancePtr->User_Description);
                        break;
                     case ahtNumberOfDigitals:
                        /* Determine the how to display the             */
                        /* Characteristic depending on the              */
                        /* Characteristic type.                         */
                        switch(Type)
                        {
                           case actDigital:
                              printf("\r\nNumber Of Digitals:\r\n");
                              printf("   Value:  %u\r\n", ServerInstancePtr->Number_Of_Digitals);
                              break;
                           default:
                              printf("\r\nInvalid Characteristic type.\r\n");
                              break;
                        }
                        break;
                     case ahtValidRange:
                        /* Determine the how to display the             */
                        /* Characteristic depending on the              */
                        /* Characteristic type.                         */
                        switch(Type)
                        {
                           case actAnalog:
                              printf("\r\nValid Range:\r\n");
                              printf("   Lower:  0x%04X\r\n", ServerInstancePtr->Valid_Range.LowerBound);
                              printf("   Upper:  0x%04X\r\n", ServerInstancePtr->Valid_Range.UpperBound);
                              break;
                           default:
                              printf("\r\nInvalid Characteristic type.\r\n");
                              break;
                        }
                        break;
                     default:
                        printf("\r\nInvalid attribute handle type.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nInvalid instance ID: (%u).\r\n", ID);
            }
         }
         else
         {
            /* We are the AIOS Client so we need to make sure that a    */
            /* connection exists to the AIOS Server.                    */
            if(ConnectionID)
            {
               /* Get the device info for the connection device.        */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery since*/
                  /* we need the attribute handle to make the request.  */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* We will handle the Aggregate Characteristic     */
                     /* separately.                                     */
                     if(Type == actAggregate)
                     {
                        /* Determine the attribute handle to send in the*/
                        /* request.                                     */
                        switch(AttributeHandleType)
                        {
                           case ahtCharacteristic:
                              AttributeHandle = DeviceInfo->AIOPClientInfo.Aggregate_Handle;
                              break;
                           case ahtClientCharacteristicConfig:
                              AttributeHandle = DeviceInfo->AIOPClientInfo.Aggregate_CCCD_Handle;
                              break;
                           default:
                              printf("\r\nInvalid attribute handle type.\r\n");
                              break;
                        }
                     }
                     else
                     {
                        /* This MUST be a Digital or Analog             */
                        /* Characteristic instance.                     */

                        /* Store a pointer to the specified AIOS Client */
                        /* Characteristic's information.                */
                        ClientCharacteristicPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[Type]);

                        /* Make sure we have discovered an instance of  */
                        /* this AIOS Characteristic.                    */
                        if((ClientCharacteristicPtr->Number_Of_Instances) && (ClientCharacteristicPtr->Instances))
                        {
                           /* Make sure the instance ID is valid.       */
                           if(ID < ClientCharacteristicPtr->Number_Of_Instances)
                           {
                              /* Store a pointer to the instance data to*/
                              /* aid in readability.                    */
                              ClientInstancePtr = &(ClientCharacteristicPtr->Instances[ID]);

                              /* Determine the attribute handle to send */
                              /* in the request.                        */
                              switch(AttributeHandleType)
                              {
                                 case ahtCharacteristic:
                                    /* Determine the correct attribute  */
                                    /* handle based on the AIOS         */
                                    /* Characteristic type.             */
                                    switch(Type)
                                    {
                                       case actDigital:
                                          AttributeHandle = ClientInstancePtr->Digital_Characteristic_Handle;
                                          break;
                                       case actAnalog:
                                          AttributeHandle = ClientInstancePtr->Analog_Charactersitic_Handle;
                                          break;
                                       default:
                                          printf("\r\nInvalid Characteristic type.\r\n");
                                          break;
                                    }
                                    break;
                                 case ahtExtendedProperties:
                                    AttributeHandle = ClientInstancePtr->Extended_Properties_Handle;
                                    break;
                                 case ahtClientCharacteristicConfig:
                                    AttributeHandle = ClientInstancePtr->CCCD_Handle;
                                    break;
                                 case ahtPresentationFormat:
                                    AttributeHandle = ClientInstancePtr->Presentation_Format_Handle;
                                    break;
                                 case ahtValueTriggerSetting:
                                    AttributeHandle = ClientInstancePtr->Value_Trigger_Setting_Handle;
                                    break;
                                 case ahtTimeTriggerSetting:
                                    AttributeHandle = ClientInstancePtr->Time_Trigger_Setting_Handle;
                                    break;
                                 case ahtUserDescription:
                                    AttributeHandle = ClientInstancePtr->User_Description_Handle;
                                    break;
                                 case ahtNumberOfDigitals:
                                    /* Determine the correct attribute  */
                                    /* handle based on the AIOS         */
                                    /* Characteristic type.             */
                                    switch(Type)
                                    {
                                       case actDigital:
                                          AttributeHandle = ClientInstancePtr->Number_Of_Digitals_Handle;
                                          break;
                                       default:
                                          printf("\r\nInvalid Characteristic type.\r\n");
                                          break;
                                    }
                                    break;
                                 case ahtValidRange:
                                    /* Determine the correct attribute  */
                                    /* handle based on the AIOS         */
                                    /* Characteristic type.             */
                                    switch(Type)
                                    {
                                       case actAnalog:
                                          AttributeHandle = ClientInstancePtr->Valid_Range_Handle;
                                          break;
                                       default:
                                          printf("\r\nInvalid Characteristic type.\r\n");
                                          break;
                                    }
                                    break;
                                 default:
                                    printf("\r\nInvalid attribute handle type.\r\n");
                                    break;
                              }
                           }
                           else
                              printf("\r\nInvalid instance ID: (%u).\r\n", ID);
                        }
                        else
                           printf("\r\nNo instances were discovered for the AIOS Characteristic type.\r\n");
                     }

                     /* Make sure the attribute handle is valid.        */
                     if(AttributeHandle)
                     {
                        /* Set the AIOP Client request information so we*/
                        /* can handle simplify handling the response.   */
                        DeviceInfo->AIOPClientInfo.Client_Request_Info.Type                = Type;
                        DeviceInfo->AIOPClientInfo.Client_Request_Info.ID                  = ID;
                        DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType = AttributeHandleType;

                        /* * NOTE * We will always reset the User       */
                        /*          Description offset for a standard   */
                        /*          read request.  If we receive an     */
                        /*          incomplete User Description then we */
                        /*          need to use a read long request to  */
                        /*          get the rest of the User            */
                        /*          Description.                        */
                        DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset = 0;

                        /* Send the GATT read request.                  */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
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
                        printf("\r\nAttribute handle has not been discovered.\r\n");
                  }
                  else
                     printf("\r\nService discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nDevice information does not exist.\r\n");
            }
            else
               printf("\r\nNo connection to remote AIOS Server.\r\n");
         }
      }
      else
         printf("\r\nInvalid Characteristic type.\r\n");
   }

   return (ret_val);
}

   /* The following function is responsible for formatting an AIOS      */
   /* Characteristic and notifying or indicating that Characteristic.   */
static int IndicateNotifyCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, Boolean_t Notify)
{
   int                          ret_val = 0;
   AIOS_Characteristic_Data_t   Data;
   AIOP_Server_Instance_Data_t *InstanceDataPtr = NULL;

   /* Determine how to format the AIOS Characteristic based on the type.*/
   switch(CharacteristicInfo->Type)
   {
      case actDigital:
         if((InstanceDataPtr = GetServerInstanceInfoPtr(CharacteristicInfo)) != NULL)
         {
            /* Determine if this is a request to notify or indication.  */
            if(Notify)
            {
               /* Make sure the instance supports notification.         */
               if(InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)
               {
                  /* Make sure we are configured for notifications.     */
                  if(InstanceDataPtr->Client_Configuration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                  {
                     /* Format the Digital Characteristic.              */
                     Data.Digital.Length  = (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
                     Data.Digital.Buffer = InstanceDataPtr->Data.Digital;

                     /* Send the notification.                          */
                     if((ret_val = AIOS_Notify_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nNotification sent:\r\n");
                        printf("   Bytes Written:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Notify_Characteristic", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic instance is not configured for notifications.\r\n");
               }
               else
                  printf("\r\nCharacteristic instance does not support support notifications.\r\n");
            }
            else
            {
               /* Make sure the instance supports indication.           */
               if(InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
               {
                  /* Make sure we are configured for indications.       */
                  if(InstanceDataPtr->Client_Configuration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                  {
                     /* Format the Digital Characteristic.              */
                     Data.Digital.Length  = (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
                     Data.Digital.Buffer = InstanceDataPtr->Data.Digital;

                     /* Send the indication.                                  */
                     if((ret_val = AIOS_Indicate_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nIndication sent:\r\n");
                        printf("   TransactionID:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Indicate_Characteristic", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic instance is not configured for indications.\r\n");

               }
               else
                  printf("\r\nCharacteristic instance does not support support indications.\r\n");
            }
         }
         else
            printf("\r\nCould not get AIOS Server instance information.\r\n");
         break;
      case actAnalog:
         if((InstanceDataPtr = GetServerInstanceInfoPtr(CharacteristicInfo)) != NULL)
         {
            /* Determine if this is a request to notify or indication.  */
            if(Notify)
            {
               /* Make sure the instance supports notification.         */
               if(InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)
               {
                  /* Make sure we are configured for notifications.     */
                  if(InstanceDataPtr->Client_Configuration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                  {
                     /* Format the Analog Characteristic.               */
                     Data.Analog = InstanceDataPtr->Data.Analog;

                     /* Send the notification.                          */
                     if((ret_val = AIOS_Notify_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nNotification sent:\r\n");
                        printf("   Bytes Written:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Notify_Characteristic", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic instance is not configured for notifications.\r\n");
               }
               else
                  printf("\r\nCharacteristic instance does not support support notifications.\r\n");
            }
            else
            {
               /* Make sure the instance supports indication.           */
               if(InstanceDataPtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
               {
                  /* Make sure we are configured for indications.       */
                  if(InstanceDataPtr->Client_Configuration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                  {
                     /* Format the Analog Characteristic.                        */
                     Data.Analog = InstanceDataPtr->Data.Analog;

                     /* Send the indication.                                  */
                     if((ret_val = AIOS_Indicate_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nIndication sent:\r\n");
                        printf("   TransactionID:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Indicate_Characteristic", ret_val);
                  }
                  else
                     printf("\r\nCharacteristic instance is not configured for indications.\r\n");

               }
               else
                  printf("\r\nCharacteristic instance does not support support indications.\r\n");
            }
         }
         else
            printf("\r\nCould not get AIOS Server instance information.\r\n");
         break;
      case actAggregate:
         /* Make sure the Aggregate is supported.                       */
         if(AIOSServerInfo.Aggregate_Supported)
         {
            /* Determine if this is a request to notify or indication.  */
            if(Notify)
            {
               /* Make sure the instance supports notification.         */
               if(AIOSServerInfo.Aggregate_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)
               {
                  /* Make sure we are configured for notifications.     */
                  if(AIOSServerInfo.Aggregate_CC & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                  {
                     /* Call the internal function to format the        */
                     /* Aggregate data.                                 */
                     FormatAggregateCharacteristic(&(Data.Aggregate));

                     /* Send the notification.                          */
                     if((ret_val = AIOS_Notify_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nNotification sent:\r\n");
                        printf("   Bytes Written:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Notify_Characteristic", ret_val);

                     /* If we sent the Aggregate Characteristic we need to free  */
                     /* the memory for the buffer.                               */
                     if((Data.Aggregate.Length) && (Data.Aggregate.Buffer))
                     {
                        /* Free the AggregateBuffer field of the Aggregate Data  */
                        /* just sent.                                            */
                        BTPS_FreeMemory(Data.Aggregate.Buffer);
                        Data.Aggregate.Buffer = NULL;
                        Data.Aggregate.Length = 0;
                     }
                  }
                  else
                     printf("\r\nAggregate Characteristic is not configured for notifications.\r\n");
               }
               else
                  printf("\r\nAggregate Characteristic does not support support notifications.\r\n");
            }
            else
            {
               /* Make sure the instance supports indication.           */
               if(AIOSServerInfo.Aggregate_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_INDICATE)
               {
                  /* Make sure we are configured for indications.             */
                  if(AIOSServerInfo.Aggregate_CC & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                  {
                     /* Call the internal function to format the Aggregate data. */
                     FormatAggregateCharacteristic(&(Data.Aggregate));

                     /* Send the indication.                                  */
                     if((ret_val = AIOS_Indicate_Characteristic(BluetoothStackID, AIOSInstanceID, ConnectionID, CharacteristicInfo, &Data)) > 0)
                     {
                        printf("\r\nIndication sent:\r\n");
                        printf("   TransactionID:  %d\r\n", ret_val);

                        /* Simply return success to the caller.         */
                        ret_val = 0;
                     }
                     else
                        DisplayFunctionError("AIOS_Indicate_Characteristic", ret_val);

                     /* If we sent the Aggregate Characteristic we need to free  */
                     /* the memory for the buffer.                               */
                     if((Data.Aggregate.Length) && (Data.Aggregate.Buffer))
                     {
                        /* Free the AggregateBuffer field of the Aggregate Data  */
                        /* just sent.                                            */
                        BTPS_FreeMemory(Data.Aggregate.Buffer);
                        Data.Aggregate.Buffer = NULL;
                        Data.Aggregate.Length = 0;
                     }
                  }
                  else
                     printf("\r\nAggregate Characteristic is not configured for indications.\r\n");
               }
               else
                  printf("\r\nAggregate Characteristic does not support support indications.\r\n");
            }
         }
         else
            printf("\r\nAggregate Characteristic not supported.\r\n");
         break;
      default:
         printf("\r\nInvalid Characteristic type.\r\n");
         break;
   }

   return(ret_val);
}

   /* The following function is an internal function to perform a Long  */
   /* Read for the User Description.  This function may be called by the*/
   /* AIOS Client only.                                                 */
static int ReadLongUserDescriptionRequest(AIOS_Characteristic_Info_t *CharacteristicInfo)
{
   int                                 ret_val = 0;
   DeviceInfo_t                       *DeviceInfo;
   AIOS_Characteristic_Type_t          Type;
   unsigned int                        ID;
   Word_t                              AttributeHandle;
   AIOP_Client_Characteristic_Info_t  *ClientCharacteristicPtr;
   AIOP_Client_Instance_Info_t        *ClientInstancePtr;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the AIOS Characteristic Type is valid.               */
      if((Type >= actDigital) && (Type <= actAnalog))
      {
         /* We are the AIOS Client so we need to make sure that a       */
         /* connection exists to the AIOS Server.                       */
         if(ConnectionID)
         {
            /* Get the device info for the connection device.           */
            /* * NOTE * ConnectionBD_ADDR should be valid if            */
            /*          ConnectionID is.                                */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery since we*/
               /* need the attribute handle to make the request.        */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Store a pointer to the specified AIOS Client       */
                  /* Characteristic's information.                      */
                  ClientCharacteristicPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[Type]);

                  /* Make sure we have discovered an instance of this   */
                  /* AIOS Characteristic.                               */
                  if((ClientCharacteristicPtr->Number_Of_Instances) && (ClientCharacteristicPtr->Instances))
                  {
                     /* Make sure the instance ID is valid.             */
                     if(ID < ClientCharacteristicPtr->Number_Of_Instances)
                     {
                        /* Store a pointer to the instance data to aid  */
                        /* in readability.                              */
                        ClientInstancePtr = &(ClientCharacteristicPtr->Instances[ID]);

                        /* Store the attribute handle.                  */
                        AttributeHandle = ClientInstancePtr->User_Description_Handle;

                        /* Make sure the attribute handle is valid.     */
                        if(AttributeHandle)
                        {
                           /* Make sure a GATT Read Value request has   */
                           /* been previously issued.                   */
                           /* * NOTE * This value is set when the first */
                           /*          response is received.  This value*/
                           /*          is next offset to read the       */
                           /*          remaining User Description if it */
                           /*          could not fit in the GATT Read   */
                           /*          response.                        */
                           if(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset)
                           {
                              /* Set the AIOS Client request information*/
                              /* so we can handle simplify handling the */
                              /* response.                              */
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.Type                = Type;
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.ID                  = ID;
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType = ahtUserDescription;

                              /* Send the GATT read request.            */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Read_Long_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Read Long Value Request sent:\r\n");
                                 printf("   TransactionID:     %d\r\n",     ret_val);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Read_Value_Request", ret_val);

                              /* Simply return success to the caller.   */
                              ret_val = 0;
                           }
                           else
                              printf("\r\nThe GATT Read Value request for the User Description has not been sent.\r\n");
                        }
                        else
                           printf("\r\nUser description does not exist on the AIOS Server.\r\n");
                     }
                     else
                        printf("\r\nInvalid instance ID: (%u).\r\n", ID);
                  }
                  else
                     printf("\r\nNo instances were discovered for the AIOS Characteristic type.\r\n");
               }
               else
                  printf("\r\nService discovery has not been performed.\r\n");
            }
            else
               printf("\r\nDevice information does not exist.\r\n");
         }
         else
            printf("\r\nNo connection to remote AIOS Server.\r\n");
      }
      else
         printf("\r\nInvalid Characteristic type.\r\n");
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}


   /* The following function is an internal function to write an AIOS   */
   /* Characteristic or its descriptors.  This function may be called by*/
   /* the AIOS Server or AIOS Client.                                   */
static int WriteAIOSCharacteristic(AIOS_Characteristic_Info_t *CharacteristicInfo, AIOP_Attribute_Handle_Type_t AttributeHandleType, AIOP_Write_Request_Data_t *WriteRequestData)
{
   int                                 ret_val = 0;
   DeviceInfo_t                       *DeviceInfo;
   AIOS_Characteristic_Type_t          Type;
   unsigned int                        ID;
   Word_t                              AttributeHandle;
   AIOP_Server_Instance_Data_t        *ServerInstancePtr;
   AIOP_Client_Characteristic_Info_t  *ClientCharacteristicPtr;
   AIOP_Client_Instance_Info_t        *ClientInstancePtr;
   Word_t                              MTU;

   Byte_t                             *Buffer;
   Word_t                              BufferLength;

   NonAlignedWord_t                    Analog;
   NonAlignedWord_t                    CCCD;

   /* Make sure the parameters are semi-valid.                          */
   if((CharacteristicInfo) && (WriteRequestData))
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the AIOS Characteristic Type is valid.               */
      if((Type >= actDigital) && (Type <= actAggregate))
      {
         /* Check the InstanceID to see if we are the AIOS Server.      */
         if(AIOSInstanceID)
         {
            /* Make sure the instance specified exists for the AIOS     */
            /* Characteristic Type.                                     */
            if(ID < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES)
            {
               /* If this is a Digital/Analog Characteristic instance.  */
               /* * NOTE * We CANNOT write anything for the Aggregate   */
               /*          Characteristic on the AIOS Server.           */
               if((Type == actDigital) || (Type == actAnalog))
               {
                  /* Store a pointer to the instance data to aid in     */
                  /* readability.                                       */
                  /* * NOTE * We already checked the Type and ID earlier*/
                  /*          so it MUST be valid.                      */
                  ServerInstancePtr = &(AIOSServerInfo.Characteristic[Type].Instances[ID]);

                  /* Determine the how to handle the write request.     */
                  switch(AttributeHandleType)
                  {
                     case ahtCharacteristic:
                        /* Determine how to write the AIOS              */
                        /* Characteristic instance based on the AIOS    */
                        /* Characteristic type.                         */
                        switch(Type)
                        {
                           case actDigital:
                              /* Verify the length.                     */
                              if(WriteRequestData->Data.Characteristic.Digital.Length == (Word_t)AIOP_DIGITAL_ARRAY_SIZE)
                              {
                                 /* Simply copy the new Digital         */
                                 /* Characteristic.                     */
                                 BTPS_MemCopy(ServerInstancePtr->Data.Digital, WriteRequestData->Data.Characteristic.Digital.Buffer, AIOP_DIGITAL_ARRAY_SIZE);
                              }
                              break;
                           case actAnalog:
                              ServerInstancePtr->Data.Analog = WriteRequestData->Data.Characteristic.Analog;
                              break;
                           default:
                              printf("\r\nInvalid Characteristic type.\r\n");
                              break;
                        }
                        break;
                     case ahtExtendedProperties:
                        printf("\r\nAIOS Server cannot write the Extended Properties.\r\n");
                        break;
                     case ahtClientCharacteristicConfig:
                        printf("\r\nAIOS Server cannot write the CCCD.\r\n");
                        break;
                     case ahtPresentationFormat:
                        printf("\r\nAIOS Server cannot write the Presentation Format.\r\n");
                        break;
                     case ahtValueTriggerSetting:
                        ServerInstancePtr->Value_Trigger_Setting = WriteRequestData->Data.ValueTriggerSetting;
                        break;
                     case ahtTimeTriggerSetting:
                        ServerInstancePtr->Time_Trigger_Setting = WriteRequestData->Data.TimeTriggerSetting;
                        break;
                     case ahtUserDescription:
                        /* Store the User Description.                  */
                        if(WriteRequestData->UserDescriptionLength < (Word_t)AIOP_USER_DESCRIPTION_LENGTH)
                        {
                           /* Simply copy the User Description.         */
                           BTPS_StringCopy(ServerInstancePtr->User_Description, WriteRequestData->Data.UserDescription);
                        }
                        else
                           printf("\r\nUser Description length greater than the AIOS Server supports.\r\n");
                        break;
                     case ahtNumberOfDigitals:
                        printf("\r\nAIOS Server cannot write the Number Of Digitals.\r\n");
                        break;
                     case ahtValidRange:
                        /* We will allow the AIOS Server to update the  */
                        /* Valid Range for testing purposes, however    */
                        /* this descriptor should never be written by   */
                        /* the AIOS Server.                             */
                        ServerInstancePtr->Valid_Range = WriteRequestData->Data.ValidRange;
                        break;
                     default:
                        printf("\r\nInvalid attribute handle type.\r\n");
                        break;
                  }
               }
               else
                  printf("\r\nAIOS Server cannot write the Characteristic type.\r\n");
            }
            else
               printf("\r\nInvalid instance ID: (%u).\r\n", ID);
         }
         else
         {
            /* We are the AIOS Client so we need to make sure that a    */
            /* connection exists to the AIOS Server.                    */
            if(ConnectionID)
            {
               /* Get the device info for the connection device.        */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery since*/
                  /* we need the attribute handle to make the request.  */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* We will handle the Aggregate Characteristic     */
                     /* separately.                                     */
                     if(Type == actAggregate)
                     {
                        /* Determine the attribute handle to send in the*/
                        /* request.                                     */
                        /* * NOTE * The Aggregate Characteristic CANNOT */
                        /*          be written.                         */
                        switch(AttributeHandleType)
                        {
                           case ahtClientCharacteristicConfig:
                              /* Store the attribute handle.            */
                              AttributeHandle = DeviceInfo->AIOPClientInfo.Aggregate_CCCD_Handle;

                              /* Send the write request.                */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&(WriteRequestData->Data.CCCD), GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Write Request sent:\r\n");
                                 printf("   TransactionID:     %d\r\n", ret_val);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Write_Request", ret_val);

                              /* Simply return success to the caller.   */
                              ret_val = 0;
                              break;
                           default:
                              printf("\r\nInvalid attribute handle type.\r\n");
                              break;
                        }
                     }
                     else
                     {
                        /* This MUST be a Digital or Analog             */
                        /* Characteristic instance.                     */

                        /* Store a pointer to the specified AIOS Client */
                        /* Characteristic's information.                */
                        ClientCharacteristicPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[Type]);

                        /* Make sure we have discovered an instance of  */
                        /* this AIOS Characteristic.                    */
                        if((ClientCharacteristicPtr->Number_Of_Instances) && (ClientCharacteristicPtr->Instances))
                        {
                           /* Make sure the instance ID is valid.       */
                           if(ID < ClientCharacteristicPtr->Number_Of_Instances)
                           {
                              /* Store a pointer to the instance data to*/
                              /* aid in readability.                    */
                              ClientInstancePtr = &(ClientCharacteristicPtr->Instances[ID]);

                              /* Determine the attribute handle based on*/
                              /* the Attribute handle type.             */
                              switch(AttributeHandleType)
                              {
                                 case ahtCharacteristic:
                                    /* Determine how to write the AIOS  */
                                    /* Characteristic instance based on */
                                    /* the AIOS Characteristic type.    */
                                    switch(Type)
                                    {
                                       case actDigital:
                                          /* Store the attribute handle.*/
                                          AttributeHandle = ClientInstancePtr->Digital_Characteristic_Handle;

                                          if(AttributeHandle)
                                          {
                                             /* Determine if we should  */
                                             /* use write without       */
                                             /* response.               */
                                             if(WriteRequestData->UseWriteWithoutResponse)
                                             {
                                                /* Send the write       */
                                                /* without response     */
                                                /* request.             */
                                                if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->Data.Characteristic.Digital.Length, WriteRequestData->Data.Characteristic.Digital.Buffer)) > 0)
                                                {
                                                   printf("\r\nGATT Write Without Response Request sent:\r\n");
                                                   printf("   Bytes Written:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                             }
                                             else
                                             {
                                                /* Send the write       */
                                                /* request.             */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->Data.Characteristic.Digital.Length, WriteRequestData->Data.Characteristic.Digital.Buffer, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Request", ret_val);
                                             }

                                             /* Simply return success to*/
                                             /* the caller.             */
                                             ret_val = 0;
                                          }
                                          else
                                             printf("\r\nDigital Characteristic attribute handle is invalid.\r\n");
                                          break;
                                       case actAnalog:
                                          /* Store the attribute handle.*/
                                          AttributeHandle = ClientInstancePtr->Analog_Charactersitic_Handle;

                                          if(AttributeHandle)
                                          {
                                             /* Format the Analog       */
                                             /* Characteristic.         */
                                             ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Analog, WriteRequestData->Data.Characteristic.Analog);

                                             /* Determine if we should  */
                                             /* use write without       */
                                             /* response.               */
                                             if(WriteRequestData->UseWriteWithoutResponse)
                                             {
                                                /* Send the write       */
                                                /* without response     */
                                                /* request.             */
                                                if((ret_val = GATT_Write_Without_Response_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Analog)) > 0)
                                                {
                                                   printf("\r\nGATT Write Without Response Request sent:\r\n");
                                                   printf("   Bytes Written:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Without_Response_Request", ret_val);
                                             }
                                             else
                                             {
                                                /* Send the write       */
                                                /* request.             */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Analog, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Request", ret_val);
                                             }

                                             /* Simply return success to*/
                                             /* the caller.             */
                                             ret_val = 0;
                                          }
                                          else
                                             printf("\r\nDigital Characteristic attribute handle is invalid.\r\n");
                                          break;
                                       default:
                                          printf("\r\nInvalid Characteristic type.\r\n");
                                          break;
                                    }
                                    break;
                                 case ahtExtendedProperties:
                                    printf("\r\nAIOS Client cannot write the Extended Properties descriptor.\r\n");
                                    break;
                                 case ahtClientCharacteristicConfig:
                                    /* Store the attribute handle.      */
                                    AttributeHandle = ClientInstancePtr->CCCD_Handle;

                                    if(AttributeHandle)
                                    {
                                       /* Format the CCCD.              */
                                       ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&CCCD, WriteRequestData->Data.CCCD);

                                       /* Send the write request.       */
                                       /* * NOTE * We will not save the */
                                       /*          transactionID        */
                                       /*          returned by this     */
                                       /*          function, which we   */
                                       /*          could use to cancel  */
                                       /*          the request.         */
                                       if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&CCCD, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                       {
                                          printf("\r\nGATT Write Request sent:\r\n");
                                          printf("   TransactionID:     %d\r\n", ret_val);
                                          printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                       }
                                       else
                                          DisplayFunctionError("GATT_Write_Request", ret_val);
                                    }
                                    else
                                       printf("\r\nCCCD does not exist on the AIOS Server.\r\n");

                                    /* Simply return success to the     */
                                    /* caller.                          */
                                    ret_val = 0;
                                    break;
                                 case ahtPresentationFormat:
                                    printf("\r\nAIOS Client cannot write the Presentation Format.\r\n");
                                    break;
                                 case ahtValueTriggerSetting:
                                    /* Store the attribute handle.      */
                                    AttributeHandle = ClientInstancePtr->Value_Trigger_Setting_Handle;

                                    if(AttributeHandle)
                                    {
                                       /* Determine the size to allocate*/
                                       /* for the request buffer.       */
                                       if((ret_val = AIOS_Format_Value_Trigger_Setting(&(WriteRequestData->Data.ValueTriggerSetting), 0, NULL)) > 0)
                                       {
                                          BufferLength = (unsigned int)ret_val;

                                          /* Allocate the request       */
                                          /* buffer.                    */
                                          if((Buffer = (Byte_t *)BTPS_AllocateMemory(NON_ALIGNED_BYTE_SIZE * BufferLength)) != NULL)
                                          {
                                             /* Format the request      */
                                             /* buffer.                 */
                                             if((ret_val = AIOS_Format_Value_Trigger_Setting(&(WriteRequestData->Data.ValueTriggerSetting), BufferLength, Buffer)) == 0)
                                             {
                                                /* Send the GATT Write  */
                                                /* request.             */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, BufferLength, (Byte_t *)Buffer, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Request", ret_val);
                                             }
                                             else
                                                DisplayFunctionError("AIOS_Format_Value_Trigger_Setting", ret_val);
                                          }
                                          else
                                             printf("\r\nCould not allocate memory for the request buffer.\r\n");
                                       }
                                       else
                                          DisplayFunctionError("AIOS_Format_Value_Trigger_Setting", ret_val);
                                    }
                                    else
                                       printf("\r\nValue Trigger Setting does not exist on the AIOS Server.\r\n");

                                    /* Simply return success to the     */
                                    /* caller.                          */
                                    ret_val = 0;
                                    break;
                                 case ahtTimeTriggerSetting:
                                    /* Store the attribute handle.      */
                                    AttributeHandle = ClientInstancePtr->Time_Trigger_Setting_Handle;

                                    if(AttributeHandle)
                                    {
                                       /* Determine the size to allocate*/
                                       /* for the request buffer.       */
                                       if((ret_val = AIOS_Format_Time_Trigger_Setting(&(WriteRequestData->Data.TimeTriggerSetting), 0, NULL)) > 0)
                                       {
                                          BufferLength = (unsigned int)ret_val;

                                          /* Allocate the request       */
                                          /* buffer.                    */
                                          if((Buffer = (Byte_t *)BTPS_AllocateMemory(NON_ALIGNED_BYTE_SIZE * BufferLength)) != NULL)
                                          {
                                             /* Format the request      */
                                             /* buffer.                 */
                                             if((ret_val = AIOS_Format_Time_Trigger_Setting(&(WriteRequestData->Data.TimeTriggerSetting), BufferLength, Buffer)) == 0)
                                             {
                                                /* Send the GATT Write  */
                                                /* request.             */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, BufferLength, (Byte_t *)Buffer, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Request", ret_val);
                                             }
                                             else
                                                DisplayFunctionError("AIOS_Format_Time_Trigger_Setting", ret_val);
                                          }
                                          else
                                             printf("\r\nCould not allocate memory for the request buffer.\r\n");
                                       }
                                       else
                                          DisplayFunctionError("AIOS_Format_Time_Trigger_Setting", ret_val);
                                    }
                                    else
                                       printf("\r\nTime Trigger Setting does not exist on the AIOS Server.\r\n");

                                    /* Simply return success to the     */
                                    /* caller.                          */
                                    ret_val = 0;
                                    break;
                                 case ahtUserDescription:
                                    /* Make sure the User Description   */
                                    /* length is valid.                 */
                                    if(WriteRequestData->UserDescriptionLength < (Word_t)AIOP_USER_DESCRIPTION_LENGTH)
                                    {
                                       /* Store the attribute handle.   */
                                       AttributeHandle = ClientInstancePtr->User_Description_Handle;

                                       if(AttributeHandle)
                                       {
                                          /* Query the GATT Connection  */
                                          /* MTU to determine how we    */
                                          /* should handle the write    */
                                          /* request.                   */
                                          if((ret_val = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                                          {
                                             /* Check whether we need to*/
                                             /* send a write request or */
                                             /* a prepare write request.*/
                                             if(WriteRequestData->UserDescriptionLength <= (MTU-3))
                                             {
                                                /* Send the GATT Write  */
                                                /* request.             */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->UserDescriptionLength, (Byte_t *)(WriteRequestData->Data.UserDescription), GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                                }
                                                else
                                                   DisplayFunctionError("GATT_Write_Request", ret_val);
                                             }
                                             else
                                             {
                                                /* Store the request    */
                                                /* information so we can*/
                                                /* handle the Prepare   */
                                                /* Write request        */
                                                /* response.            */
                                                /* * NOTE * We will     */
                                                /*          increment   */
                                                /*          the offset  */
                                                /*          when we     */
                                                /*          receive the */
                                                /*          response.   */
                                                DeviceInfo->AIOPClientInfo.Client_Request_Info.Type                  = Type;
                                                DeviceInfo->AIOPClientInfo.Client_Request_Info.ID                    = ID;
                                                DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset = 0;

                                                /* Copy the User        */
                                                /* Description into the */
                                                /* Buffer.              */
                                                BTPS_StringCopy(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription, WriteRequestData->Data.UserDescription);

                                                /* Send the GATT Prepare*/
                                                /* Write request.       */
                                                /* * NOTE * We will not */
                                                /*          save the    */
                                                /*          transactionI*/
                                                /*          returned by */
                                                /*          this        */
                                                /*          function,   */
                                                /*          which we    */
                                                /*          could use to*/
                                                /*          cancel the  */
                                                /*          request.    */
                                                if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->UserDescriptionLength, 0, (Byte_t *)(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription), GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                                {
                                                   printf("\r\nGATT Prepare Write Request sent:\r\n");
                                                   printf("   TransactionID:     %d\r\n", ret_val);
                                                   printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                                }
                                                else
                                                   DisplayFunctionError("GATT_Prepare_Write_Request", ret_val);
                                             }
                                          }
                                          else
                                             DisplayFunctionError("GATT_Query_Connection_MTU", ret_val);
                                       }
                                       else
                                          printf("\r\nUser description does not exist on the AIOS Server.\r\n");

                                       /* Simply return success to the  */
                                       /* caller.                       */
                                       ret_val = 0;
                                    }
                                    else
                                       printf("\r\nUser Description length greater than the buffer can store.\r\n");
                                    break;
                                 case ahtNumberOfDigitals:
                                    printf("\r\nAIOS Client cannot write the Number Of Digitals descriptor.\r\n");
                                    break;
                                 case ahtValidRange:
                                    printf("\r\nAIOS Client cannot write the Valid Range descriptor.\r\n");
                                    break;
                                 default:
                                    printf("\r\nInvalid attribute handle type.\r\n");
                                    break;
                              }
                           }
                           else
                              printf("\r\nInvalid instance ID: (%u).\r\n", ID);
                        }
                        else
                           printf("\r\nNo instances were discovered for the AIOS Characteristic type.\r\n");
                     }
                  }
                  else
                     printf("\r\nService discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nDevice information does not exist.\r\n");
            }
            else
               printf("\r\nNo connection to remote AIOS Server.\r\n");
         }
      }
      else
         printf("\r\nInvalid Characteristic type.\r\n");
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is a helper function to build the Aggregate*/
   /* characteristic data from the server information.  This includes   */
   /* calculating the array sizes and allocating the memory.  This      */
   /* function is REQUIRED to build the Aggregate characteristic for a  */
   /* read request, notification, and indication.                       */
   /* ** NOTE ** This function will allocate the memory for the         */
   /*            Aggregate data fields.  Care MUST be taken to free the */
   /*            memory after the data is sent or an error occurs in the*/
   /*            caller.                                                */
   /* * NOTE * This function EXPECTS that the AggregateData has been    */
   /*          intialized.                                              */
   /* * NOTE * This function will not check if the aggregate is enabled */
   /*          and should only be called if it was registered in a call */
   /*          to AIOS_Initialize_XXX().                                */
static int FormatAggregateCharacteristic(AIOS_Aggregate_Characteristic_Data_t *AggregateData)
{
   int                          ret_val = 0;
   unsigned int                 Index;
   Byte_t                      *AggregateBufferIndex;
   AIOP_Server_Instance_Data_t *ServerInstancePtr;

   /* Make sure the parameter is semi-valid.                            */
   if(AggregateData)
   {
      /* Initialize the fields.                                         */
      AggregateData->Length = 0;
      AggregateData->Buffer = NULL;

      /* Loop through the Digital characteristics to determine the total*/
      /* size we need to allocate for the Digital Characteristic portion*/
      /* of the Aggregate Characteristc.                                */
      /* * NOTE * Only Digital characteristics with read permission may */
      /*          be included in the aggregate.                         */
      for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index++)
      {
         /* Store a pointer to the Characteristic instance data for     */
         /* readability.                                                */
         /* * NOTE * we will hard code the actDigital as the            */
         /*          Characteristic type to access Digital              */
         /*          Characteristic instances.                          */
         ServerInstancePtr = &(AIOSServerInfo.Characteristic[actDigital].Instances[Index]);

         /* Check if the Digital Characteristic is included in the      */
         /* Aggregate Characteristic.  It will have the read property.  */
         /* * NOTE * We will simply use the Input Characteristic        */
         /*          property since, however we could have used output  */
         /*          since they align.                                  */
         if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
         {
            /* For this sample application we know that the array size  */
            /* is fixed for Digital Characteristics, but we could check */
            /* here if it was dynamic.                                  */
            AggregateData->Length += (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
         }
      }

      /* Loop through the Analog characteristics to determine the total */
      /* size we need to allocate for the Analog Characteristic portion */
      /* of the Aggregate Characteristc.                                */
      /* * NOTE * Only Analog characteristics with read permission may  */
      /*          be included in the aggregate.                         */
      for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index++)
      {
         /* Store a pointer to the Characteristic instance data for     */
         /* readability.                                                */
         /* * NOTE * we will hard code the actAnalog as the             */
         /*          Characteristic type to access Analog Characteristic*/
         /*          instances.                                         */
         ServerInstancePtr = &(AIOSServerInfo.Characteristic[actAnalog].Instances[Index]);

         /* Check if the Analog Characteristic is included in the       */
         /* Aggregate Characteristic.  It will have the read property.  */
         /* * NOTE * We will simply use the Input Characteristic        */
         /*          property since, however we could have used output  */
         /*          since they align.                                  */
         if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
         {
            /* Add WORD_SIZE for two octets since each Analog           */
            /* Characteristic is a WORD.                                */
            AggregateData->Length += NON_ALIGNED_WORD_SIZE;
         }
      }

      /* Allocate the memory for the Aggregate Characteristic buffer.   */
      /* * NOTE * We will NOT free the memory until the request is sent.*/
      if((AggregateData->Buffer = (Byte_t *)BTPS_AllocateMemory(sizeof(Byte_t) * AggregateData->Length)) != NULL)
      {
         /* Set the buffer index to the start of the buffer.            */
         AggregateBufferIndex = AggregateData->Buffer;

         /* Format the Digital Characteristic instances into the        */
         /* Aggregate Characteristic buffer.                            */
         for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS; Index++)
         {
            /* Store a pointer to the Characteristic instance data for  */
            /* readability.                                             */
            /* * NOTE * we will hard code the actDigital as the         */
            /*          Characteristic type to access Digital           */
            /*          Characteristic instances.                       */
            ServerInstancePtr = &(AIOSServerInfo.Characteristic[actDigital].Instances[Index]);

            /* Check if the Digital Characteristic is included in the   */
            /* Aggregate Characteristic.  It will have the read         */
            /* property.                                                */
            /* * NOTE * We will simply use the Input Characteristic     */
            /*          property since, however we could have used      */
            /*          output since they align.                        */
            if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
            {
               /* Copy the Digital Characteristic.                      */
               BTPS_MemCopy(AggregateBufferIndex, ServerInstancePtr->Data.Digital, (unsigned long)AIOP_DIGITAL_ARRAY_SIZE);

               /* Increment the index for the next digital              */
               /* characteristic.                                       */
               AggregateBufferIndex += AIOP_DIGITAL_ARRAY_SIZE;
            }
         }

         /* Format the Analog Characteristic instances into the         */
         /* Aggregate Characteristic buffer.                            */
         for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS; Index++)
         {
            /* Store a pointer to the Characteristic instance data for  */
            /* readability.                                             */
            /* * NOTE * we will hard code the actAnalog as the          */
            /*          Characteristic type to access Analog            */
            /*          Characteristic instances.                       */
            ServerInstancePtr = &(AIOSServerInfo.Characteristic[actAnalog].Instances[Index]);

            /* Check if the Analog Characteristic is included in the    */
            /* Aggregate Characteristic.  It will have the read         */
            /* property.                                                */
            /* * NOTE * We will simply use the Input Characteristic     */
            /*          property since, however we could have used      */
            /*          output since they align.                        */
            if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
            {
               /* Assign the Analog Characteristic.                     */
               /* * NOTE * This MUST be assigned in Little-endian       */
               /*          format.                                      */
              ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(AggregateBufferIndex, ServerInstancePtr->Data.Analog);

               /* Increment the index for the next analog               */
               /* characteristic.                                       */
               AggregateBufferIndex += NON_ALIGNED_WORD_SIZE;
            }
         }
      }
      else
         ret_val = UNABLE_TO_ALLOCATE_MEMORY;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is responsible for display the             */
   /* characteristic info in AIOS Events.                               */
static void DisplayCharacteristicInfo(AIOS_Characteristic_Info_t *CharacteristicInfo)
{
   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      printf("   Characteristic Info:\r\n");
      printf("      Type:              ");
      switch(CharacteristicInfo->Type)
      {
         case actDigital:
            printf("Digital.\r\n");
            break;
         case actAnalog:
            printf("Analogt.\r\n");
            break;
         case actAggregate:
            printf("Aggregate.\r\n");
            break;
         default:
            printf("Invalid(%u).\r\n", CharacteristicInfo->Type);
      }
      printf("      IOType:            ");
      switch(CharacteristicInfo->IOType)
      {
         case ioInput:
            printf("Input.\r\n");
            break;
         case ioOutput:
            printf("Output.\r\n");
            break;
         case ioInputOutput:
             printf("Input and Output.\r\n");
            break;
         default:
            printf("Invalid(%u).\r\n", CharacteristicInfo->Type);
      }
      printf("      ID:                %u\r\n", CharacteristicInfo->ID);
   }
}

   /* The following function is responsible for displaying the          */
   /* presentation format data received in a read presentation format   */
   /* data request.  event.                                             */
static void DisplayPresentationFormatData(AIOS_Presentation_Format_Data_t *PresentationFormatData)
{
   /* Make sure the data is semi-valid.                                 */
   if(PresentationFormatData)
   {
      printf("\r\nPresentation Format Data:\r\n");
      printf("      Format:       0x%02X\r\n", PresentationFormatData->Format);
      printf("      Exponent:     0x%02X\r\n", PresentationFormatData->Exponent);
      printf("      Unit:         0x%04X\r\n", PresentationFormatData->Unit);
      printf("      NameSpace:    0x%02X\r\n", PresentationFormatData->NameSpace);
      printf("      Description:  0x%04X\r\n", PresentationFormatData->Description);
   }
}

      /* The following function is responsible for displaying the time  */
      /* trigger setting received in an update time trigger request     */
      /* event.                                                         */
static void DisplayValueTriggerSetting(AIOS_Value_Trigger_Data_t *ValueTriggerData)
{
   /* Make sure the parameters are semi-valid.                          */
   if(ValueTriggerData)
   {
      printf("\r\nValue Trigger Data:\r\n");
      printf("      Condition:         ");
      /* Switch on the condition.                                       */
      switch(ValueTriggerData->Condition)
      {
         case vttStateChanged:
            printf("State changed (digital/analog).\r\n");
            break;
         case vttNoValueTrigger:
            printf("No value tigger.\r\n");
            break;
         case vttCrossedBoundaryAnalogValue:
            printf("Crossed boundary.\r\n");
            printf("      Analog:\r\n");
            printf("         Value:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogValue);
            break;
         case vttOnBoundaryAnalogValue:
            printf("On boundary.\r\n");
            printf("      Analog:\r\n");
            printf("         Value:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogValue);
            break;
         case vttStateChangedAnalogValue:
            printf("State changed (analog value).\r\n");
            printf("      Analog:\r\n");
            printf("         Value:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogValue);
            break;
         case vttDigitalStateChangedBitMask:
            printf("Digital state changed bit mask.\r\n");
            printf("      Bit Mask:\r\n");
            /* * NOTE * We will pass in zero for the ID even though it  */
            /*          is not valid.  We simply want to print the      */
            /*          digital arrray.                                 */
            DisplayDigitalCharacteristic(&ValueTriggerData->ComparisonValue.BitMask, 0);
            break;
         case vttCrossedBoundaryAnalogInterval:
            printf("Crossed boundary (analog interval).\r\n");
            printf("      Analog Interval:\r\n");
            printf("         Lower:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogInterval.LowerBound);
            printf("         Upper:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogInterval.UpperBound);
            break;
         case vttOnBoundaryAnalogInterval:
            printf("On boundary (analog interval).\r\n");
            printf("      Analog Interval:\r\n");
            printf("         Lower:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogInterval.LowerBound);
            printf("         Upper:       0x%04X\r\n", ValueTriggerData->ComparisonValue.AnalogInterval.UpperBound);
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

      /* The following function is responsible for displaying the time  */
      /* trigger setting received in an update time trigger request     */
      /* event.                                                         */
static void DisplayTimeTriggerSetting(AIOS_Time_Trigger_Data_t *TimeTriggerData)
{
   /* Make sure the parameters are semi-valid.                          */
   if(TimeTriggerData)
   {
      printf("\r\nTime Trigger Data:\r\n");
      printf("      Condition:         ");
      /* Switch on the condition.                                       */
      switch(TimeTriggerData->Condition)
      {
         case tttNoTimeBasedTrigger:
            printf("No time trigger.\r\n");
            break;
         case tttTimeIntervalIgnoreValueTrigger:
            printf("Time interval (Ignore Value Trigger Setting).\r\n");
            printf("      Time Interval (sec):\r\n");
            printf("         Lower:       %u\r\n", TimeTriggerData->ComparisonValue.TimeInterval.Lower);
            printf("         Upper:       %u\r\n", TimeTriggerData->ComparisonValue.TimeInterval.Upper);
            break;
         case tttTimeIntervalCheckValueTrigger:
            printf("Time interval (Check Value Trigger Setting).\r\n");
            printf("      Time Interval (sec):\r\n");
            printf("         Lower:       %u\r\n", TimeTriggerData->ComparisonValue.TimeInterval.Lower);
            printf("         Upper:       %u\r\n", TimeTriggerData->ComparisonValue.TimeInterval.Upper);
            break;
         case tttCountChangedMoreOftenThan:
            printf("Count changed more often than.\r\n");
            printf("      Count:          %u\r\n", TimeTriggerData->ComparisonValue.Count);
            break;
         default:
            printf("Invalid.\r\n");
            break;
      }
   }
}

   /* The following function is responsible for displaying a Digital    */
   /* Characteristic.                                                   */
static void DisplayDigitalCharacteristic(AIOS_Digital_Characteristic_Data_t *DigitalData, unsigned int ID)
{
   unsigned int Index;
   unsigned int Index2;
   Byte_t       DigitalPosition;
   Byte_t       DigitalByte;
   Byte_t       DigitalCtr = 0;

   /* Make sure the parameter is semi-valid.                            */
   if(DigitalData)
   {
      /* Display the Characteristic type.                               */
      printf("\r\nDigital Characteristic (ID = %u):\r\n", ID);

      for(Index = 0; Index < (unsigned int)(DigitalData->Length); Index++)
      {
         /* Only 4 digital signals in a byte.  We will update the       */
         /* digital position after each iteration.                      */
         /* * NOTE * We start the digital at 0x03 or the lowest two bits*/
         /*          positions.  Shift left by two each iteration for   */
         /*          the next two bits.                                 */
         for(Index2 = 0, DigitalPosition = 0x03; Index2 < 4; Index2++, DigitalPosition <<= 2)
         {
            /* Print the digital signal number.                         */
            printf("   (%u). Digital: ", DigitalCtr++);

            /* Store a copy of the byte so we don't modify it.          */
            DigitalByte       = DigitalData->Buffer[Index];

            /* Get the state for the digital by using the position.     */
            DigitalByte      &= DigitalPosition;

            /* Shift the digital signal value to the two lowest bits.   */
            /* The number of right shifts will be (Index2*2).  This may */
            /* be zero for the first digital signal, which should       */
            /* already be at the lowest two bits.  This must be done so */
            /* we can print the value correctly.                        */
            DigitalByte     >>= (Index2*2);

            /* Print the digital byte.                                  */
            DisplayDigitalByte(DigitalByte);
         }
      }
   }
}

   /* Display the digital signal.  This is included so we don't have to */
   /* print this statement multiple times in the above function.        */
static void DisplayDigitalByte(Byte_t DigitalByte)
{
   /* Simply print the state in string format.                          */
   switch(DigitalByte)
   {
      case AIOS_DIGITAL_CHARACTERISTIC_STATE_INACTIVE:
         printf("Inactive State.\r\n");
         break;
      case AIOS_DIGITAL_CHARACTERISTIC_STATE_ACTIVE:
         printf("Active State.\r\n");
         break;
      case AIOS_DIGITAL_CHARACTERISTIC_STATE_TRI_STATE:
         printf("Tri State.\r\n");
         break;
      case AIOS_DIGITAL_CHARACTERISTIC_STATE_UNKNOWN:
         printf("Unknown State.\r\n");
         break;
      default:
         /* Can't occur, but here for compiler warnings.                */
         break;
   }
}

   /* The following function is responsible for displaying an Analog    */
   /* Characteristic.                                                   */
static void DisplayAnalogCharacteristic(Word_t AnalogData, unsigned int ID)
{
   /* Display the Analog Characteristic.                                */
   printf("\r\nAnalog Characteristic (ID = %u):\r\n", ID);
   printf("   Value:   0x%04X\r\n", AnalogData);
}

   /* The following function displays received digtal characteristic    */
   /* data.                                                             */
static void DecodeDisplayDigitalCharacteristic(Word_t ValueLength, Byte_t *Value, unsigned int ID)
{
   AIOS_Digital_Characteristic_Data_t  DigitalData;

   DigitalData.Length = (Byte_t)ValueLength;
   DigitalData.Buffer = Value;

   /* Display the Digital Characteristic.                               */
   DisplayDigitalCharacteristic(&DigitalData, ID);
}

   /* The following function displays the received analog characteristic*/
   /* data.                                                             */
static void DecodeDisplayAnalogCharacteristic(Word_t ValueLength, Byte_t *Value, unsigned int ID)
{
   Word_t  AnalogData;

   /* Verify the length.                                                */
   if(ValueLength >= NON_ALIGNED_WORD_SIZE)
   {
      AnalogData = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

      /* Display the Analog Characteristic.                             */
      DisplayAnalogCharacteristic(AnalogData, ID);
   }
   else
   {
      printf("\r\nAnalog Characteristic has an invalid length.\r\n");
   }
}

/* The following function is responsible for decoding and displaying the*/
/* aggregate characteristic.                                            */
/* ** NOTE ** It is important that the AIOS Client has read the Number  */
/*            Of Digitals descriptor for each Digital Characteristic    */
/*            included in the Aggregate Characteristic.  If this is not */
/*            the case then there is no way to separate the Digital     */
/*            Characteristics from the Analog Characteristics to display*/
/*            or store the value.  This is because Digital              */
/*            Characteristics may have a variable length depending on   */
/*            the value of the Number Of Digitals descriptor.           */
static void DecodeDisplayAggregateCharacteristic(AIOP_Client_Information_t *ClientInfo, Word_t ValueLength, Byte_t *Value)
{
   unsigned int                        Index;
   Boolean_t                           AllNumberOfDigitalsValid = TRUE;
   AIOS_Digital_Characteristic_Data_t  DigitalData;
   Word_t                              AnalogValue;
   AIOP_Client_Instance_Info_t        *InstanceInfoPtr;

   /* Make sure the parameters are semi-valid.                          */
   if((ClientInfo) && (Value) && (ValueLength))
   {
      /* First let's make sure that the AIOS client has stored the      */
      /* Number Of Digitals for each Digital Characteristc that is      */
      /* included in the Aggregate.  We will simply loop over all       */
      /* Digital Characteristics.                                       */
      /* * NOTE * We have hardcoded the actDigital type to access the   */
      /*          instance information for all Digital Characteristic   */
      /*          instances.                                            */
      for(Index = 0; Index < ClientInfo->Characteristics[actDigital].Number_Of_Instances; Index++)
      {
         /* Store a pointer to the AIOS Client's Characteristic instance*/
         /* information.                                                */
         InstanceInfoPtr = &(ClientInfo->Characteristics[actDigital].Instances[Index]);

         /* Make sure the AIOS Digital Characteristic is included in the*/
         /* Aggregate.                                                  */
         if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
         {
            /* Make sure the Number Of Digitals has been read and is    */
            /* valid.                                                   */
            if(InstanceInfoPtr->Number_Of_Digitals == 0)
            {
               printf("\r\nNumber of Digitals descriptor for Instance: %u, has not been read.\r\n", Index);
               AllNumberOfDigitalsValid = FALSE;
               break;
            }
         }
      }

      /* If we have verified that all the required Digital              */
      /* Characteristics that are included in the Aggregate have had    */
      /* their Number Of Digitals descriptor read then we can continue..*/
      if(AllNumberOfDigitalsValid)
      {
         /* Decode the Digital Characteristics since they should have   */
         /* been formatted first according to the spec.                 */
         /* * NOTE * We have hardcoded the actDigital type to access the*/
         /*          instance information for all Digital Characteristic*/
         /*          instances.                                         */
         for(Index = 0; Index < ClientInfo->Characteristics[actDigital].Number_Of_Instances; Index++)
         {
            /* Store a pointer to the AIOS Client's Characteristic      */
            /* instance information.                                    */
            InstanceInfoPtr = &(ClientInfo->Characteristics[actDigital].Instances[Index]);

            /* Make sure the AIOS Digital Characteristic is included in */
            /* the Aggregate.                                           */
            if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
            {
               /* If the length is divisible by 4 then we will have the */
               /* exact length, otherwise if there is a remainder we    */
               /* need to add one more for the extra byte.              */
               DigitalData.Length = InstanceInfoPtr->Number_Of_Digitals / 4;

               if(InstanceInfoPtr->Number_Of_Digitals % 4 != 0)
               {
                  DigitalData.Length += 1;
               }

               DigitalData.Buffer = Value;

               /* Make sure we still have enough data left to decode and*/
               /* display the Digital Characteristic.                   */
               if(ValueLength >= DigitalData.Length)
               {
                  /* Display the Digital Characteristic.                */
                  DisplayDigitalCharacteristic(&DigitalData, Index);

                  /* Decrement the remaining length of the Aggregate    */
                  /* Characteristic data and Increment the value to the */
                  /* next Characteristic that is included in the        */
                  /* Aggregate Characteristic.                          */
                  Value       += DigitalData.Length;
                  ValueLength -= DigitalData.Length;
               }
               else
               {
                  printf("\r\nInsufficient space to decode the Digital Characteristic.\r\n");
                  break;
               }
            }
         }

         /* Next we will decode the Analog Characteristics if they are  */
         /* present in the Aggregate Characteristic.                    */
         /* * NOTE * We have hardcoded the actAnalog type to access the */
         /*          instance information for all Analog Characteristic */
         /*          instances.                                         */
         for(Index = 0; Index < ClientInfo->Characteristics[actAnalog].Number_Of_Instances; Index++)
         {
            /* Store a pointer to the AIOS Client's Characteristic      */
            /* instance information.                                    */
            InstanceInfoPtr = &(ClientInfo->Characteristics[actAnalog].Instances[Index]);

            /* Make sure the AIOS Characteristic is included in the     */
            /* Aggregate Characteristic.                                */
            if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
            {
               /* Make sure we still have enough data left to decode and*/
               /* display.                                              */
               if(ValueLength >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Decode the Analog Value.                           */
                  AnalogValue  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                  /* Display the Analog Characteristic.                 */
                  DisplayAnalogCharacteristic(AnalogValue, Index);

                  /* Decrement the remaining length of the Aggregate    */
                  /* Characteristic data and Increment the value to the */
                  /* next Analog Characteristic.                        */
                  Value       += NON_ALIGNED_WORD_SIZE;
                  ValueLength -= NON_ALIGNED_WORD_SIZE;
               }
               else
               {
                  printf("\r\nInsufficient space to decode the Analog Characteristic.\r\n");
                  break;
               }
            }
         }
      }
   }
}

   /* The following function is responsible for registering the AIOS    */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
   /* ** NOTE ** If Use_Custom_Trigger is TRUE and the                  */
   /*            Server_Configuration is 0, then the Custom Trigger will*/
   /*            be disabled even if specified.                         */
static int RegisterAIOSCommand(ParameterList_t *TempParam)
{
   int                                   ret_val = 0;
   unsigned int                          Index;
   unsigned int                          Index2;
   AIOS_Initialize_Data_t                InitializeData;
   AIOS_Characteristic_Type_t            Type;
   AIOP_Server_Characteristic_Data_t    *CharacteristicDataPtr;
   AIOP_Server_Instance_Data_t          *InstanceDataPtr;
   AIOS_Characteristic_Entry_t          *CharacteristicEntryPtr;
   AIOS_Characteristic_Instance_Entry_t *InstanceEntryPtr;
   Boolean_t                             Aggregate_Supported;
   Byte_t                                Server_Configuration;
   Boolean_t                             Use_Custom_Trigger;


   /* Verify that the parameters are semi valid.                        */
   if((TempParam) && (TempParam->NumberofParameters == 3) && (TempParam->Params[1].intParam >= 0) && (TempParam->Params[1].intParam <= 2))
   {
      /* Verify that there is no active connection.                     */
      if(!ConnectionID)
      {
         /* Verify that the Service is not already registered.          */
         if(!AIOSInstanceID)
         {
            /* Initialize the AIOS Server information.                  */
            /* * NOTE * This function will initialize all default       */
            /*          Characteristic properties, descriptors, and     */
            /*          descriptor properties.  This function will set  */
            /*          default data values for Characteristics and     */
            /*          descriptors.  We will simply disable the        */
            /*          features we do not want depending on what the   */
            /*          application has specified.                      */
            InitializeAIOSServer();

            /* Initialize the AIOS InitializeData structure so we do not*/
            /* have any unexpected behaviour.                           */
            BTPS_MemInitialize(&InitializeData, 0, AIOS_INITIALIZE_DATA_SIZE);

            /* Store the parameters.                                    */
            Aggregate_Supported  = (Boolean_t)TempParam->Params[0].intParam;
            Server_Configuration = (Byte_t)TempParam->Params[1].intParam;
            Use_Custom_Trigger   = (Boolean_t)TempParam->Params[2].intParam;

            /* Go ahead and determine if we are going to disable to     */
            /* Aggregate Characteristic.  We will simply override the   */
            /* value set in the AIOS Server information.                */
            AIOSServerInfo.Aggregate_Supported = Aggregate_Supported;

            /* Set the aggregate information first if it is supported.  */
            if(AIOSServerInfo.Aggregate_Supported)
            {
               /* Go ahead and determine if we are going to disable the */
               /* default indicate property or use the notify property. */
               /* * NOTE * Digital and Analog Characteristics should    */
               /*          have these properties cleared since they     */
               /*          CANNOT support indicate or notification if   */
               /*          the Aggregate Characteristic is supported.   */
               /*          This is why we set the aggregate first.      */
               if(Server_Configuration == 1)
               {
                  /* Change from the default indicate property.         */
                  AIOSServerInfo.Aggregate_Property_Flags = GATT_CHARACTERISTIC_PROPERTIES_NOTIFY;
               }
               else
               {
                  if(Server_Configuration == 2)
                  {
                     /* Disabled notify and indicate properties.  This  */
                     /* will excluded the Client Characteristic         */
                     /* Configuration for the Aggregate since it CANNOT */
                     /* exist without these properties.                 */
                     AIOSServerInfo.Aggregate_Property_Flags = 0;
                  }
               }
            }

            /* We will now setup the InitializeData parameter that will */
            /* be passed to AIOS_Initialize_Service() to initialize the */
            /* service with the configuration options we have specified.*/

            /* Set the InitializeData fields based on the AIOS Server   */
            /* information that we initialized previously.              */
            InitializeData.Number_Of_Entries        = (Byte_t)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS;
            InitializeData.Aggregate_Supported      = AIOSServerInfo.Aggregate_Supported;
            InitializeData.Aggregate_Property_Flags = AIOSServerInfo.Aggregate_Property_Flags;

            /* We need to allocate memory for the Entries field.        */
            if((InitializeData.Entries = (AIOS_Characteristic_Entry_t *)BTPS_AllocateMemory(AIOS_CHARACTERISTIC_ENTRY_SIZE * InitializeData.Number_Of_Entries)) != NULL)
            {
               /* We will loop through each AIOS Characteristic Entry.  */
               for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS; Index++)
               {
                  /* The Index will be used to indicate the AIOS        */
                  /* Characteristic type.                               */
                  Type = (AIOS_Characteristic_Type_t)Index;

                  /* Store a pointer to the Characteristic Entry        */
                  /* information in InitializeData.                     */
                  CharacteristicEntryPtr = &(InitializeData.Entries[Type]);

                  /* Store a pointer to the Characteristic information  */
                  /* on the AIOS Server.                                */
                  CharacteristicDataPtr = &(AIOSServerInfo.Characteristic[Type]);

                  /* Simply copy the Characteristic Entry information we*/
                  /* initialized earlier in the AIOS Server information.*/
                  /* * NOTE * The Instances field has not be set since  */
                  /*          each instance on the AIOS Server will hold*/
                  /*          its Characteristic Instance Entry         */
                  /*          information.  Whereas the                 */
                  /*          AIOS_Characteristic_Entry_t has this      */
                  /*          information stored contiguously.          */
                  *CharacteristicEntryPtr = CharacteristicDataPtr->Characteristic_Entry;

                  /* We need to allocate memory of the Instances field  */
                  /* since it has not been set.                         */
                  if((CharacteristicEntryPtr->Instances = (AIOS_Characteristic_Instance_Entry_t *)BTPS_AllocateMemory(AIOS_CHARACTERISTIC_ENTRY_SIZE * CharacteristicEntryPtr->Number_Of_Instances)) != NULL)
                  {
                     /* We will loop through each AIOS Characteristic   */
                     /* Instance Entry.                                 */
                     for(Index2 = 0; Index2 < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_INSTANCES; Index2++)
                     {
                        /* Store a pointer to the Characteristic        */
                        /* Instance Entry information in InitializeData,*/
                        /* in the Entries field based on the AIOS       */
                        /* Characteristic type.                         */
                        InstanceEntryPtr = &(CharacteristicEntryPtr->Instances[Index2]);

                        /* Store a pointer to the Characteristic        */
                        /* instance information.                        */
                        /* * NOTE * The Index2 will be used to identify */
                        /*          the AIOS Characteristic instance.   */
                        /*          This also directly corresponds to   */
                        /*          the ID field that the service will  */
                        /*          use for the                         */
                        /*          AIOS_Characteristic_Info_t          */
                        /*          structure.                          */
                        InstanceDataPtr = &(CharacteristicDataPtr->Instances[Index2]);

                        /* Simply copy the Characteristic Instance Entry*/
                        /* information we initialized earlier in the    */
                        /* AIOS Server information.                     */
                        *InstanceEntryPtr = InstanceDataPtr->Instance_Entry;

                        /* We need to go ahead and either disable the      */
                        /* default Indicate property or enable the notify  */
                        /* property.                                       */
                        if(Server_Configuration == 1)
                        {
                           /* Clear the indicate property and enable       */
                           /* notification.                                */
                           InstanceEntryPtr->Characteristic_Property_Flags &= ~(GATT_CHARACTERISTIC_PROPERTIES_INDICATE);
                           InstanceEntryPtr->Characteristic_Property_Flags |= GATT_CHARACTERISTIC_PROPERTIES_NOTIFY;
                        }
                        else
                        {
                           if(Server_Configuration == 2)
                           {
                              /* Disable the default indicate property.    */
                              InstanceEntryPtr->Characteristic_Property_Flags &= ~(GATT_CHARACTERISTIC_PROPERTIES_INDICATE);
                           }
                        }

                        /* Now let's configure the service.  We will       */
                        /* disable any default features that CANNOT be used*/
                        /* depending on what is specifed by the            */
                        /* application.                                    */

                        /* Check if the application wants to use a custom  */
                        /* trigger.                                        */
                        if(Use_Custom_Trigger)
                        {
                           /* Enable the custom trigger, which is disabled */
                           /* by default.                                  */
                           InstanceEntryPtr->Use_Custom_Trigger = TRUE;

                           /* Clear the default Value Trigger Setting and  */
                           /* Time Trigger Setting flags since they should */
                           /* be excluded if a custom trigger is used.     */
                           InstanceEntryPtr->Descriptor_Flags &= ~(AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING);
                        }

                        /* Check if the Instance has the read property.    */
                        if(InstanceEntryPtr->Characteristic_Property_Flags & GATT_CHARACTERISTIC_PROPERTIES_READ)
                        {
                           /* Check that the instance has the indicate or  */
                           /* notify properties.                           */
                           if(InstanceEntryPtr->Characteristic_Property_Flags & (GATT_CHARACTERISTIC_PROPERTIES_INDICATE | GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                           {
                              /* Check if the Aggregate Characteristic is  */
                              /* supported.                                */
                              if(InitializeData.Aggregate_Supported)
                              {
                                 /* Clear the bits for the Indicate and    */
                                 /* Notify properties since they may not be*/
                                 /* included if the Aggregate is supported.*/
                                 InstanceEntryPtr->Characteristic_Property_Flags &= ~(GATT_CHARACTERISTIC_PROPERTIES_INDICATE | GATT_CHARACTERISTIC_PROPERTIES_NOTIFY);

                                 /* Check if the Aggregate Characteristic  */
                                 /* does not have the indicate or notify   */
                                 /* properties we set earlier in this      */
                                 /* function.                              */
                                 if(!(InitializeData.Aggregate_Property_Flags & (GATT_CHARACTERISTIC_PROPERTIES_INDICATE | GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)))
                                 {
                                    /* Clear the default Value Trigger     */
                                    /* Setting and Time Trigger Setting    */
                                    /* flags since they should be excluded */
                                    /* if the Aggregate and the Instance   */
                                    /* does not have the indicate or notify*/
                                    /* property.                           */
                                    InstanceEntryPtr->Descriptor_Flags &= ~(AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING);

                                    /* Check if the application enabled the*/
                                    /* custom trigger.                     */
                                    if(InstanceEntryPtr->Use_Custom_Trigger)
                                    {
                                       /* Disable the custom trigger.      */
                                       InstanceEntryPtr->Use_Custom_Trigger = FALSE;
                                    }
                                 }
                              }
                           }
                           else
                           {
                              /* Check if the Aggregate Characteristic is  */
                              /* supported.                                */
                              if(InitializeData.Aggregate_Supported)
                              {
                                 /* Check if the Aggregate Characteristic  */
                                 /* does not have the indicate or notify   */
                                 /* properties we set earlier in this      */
                                 /* function.                              */
                                 if(!(InitializeData.Aggregate_Property_Flags & (GATT_CHARACTERISTIC_PROPERTIES_INDICATE | GATT_CHARACTERISTIC_PROPERTIES_NOTIFY)))
                                 {
                                    /* Clear the default Value Trigger     */
                                    /* Setting and Time Trigger Setting    */
                                    /* flags since they should be excluded */
                                    /* if the Aggregate and the Instance   */
                                    /* does not have the indicate or notify*/
                                    /* property.                           */
                                    InstanceEntryPtr->Descriptor_Flags &= ~(AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING);

                                    /* Check if the application enabled the*/
                                    /* custom trigger.                     */
                                    if(InstanceEntryPtr->Use_Custom_Trigger)
                                    {
                                       /* Disable the custom trigger.      */
                                       InstanceEntryPtr->Use_Custom_Trigger = FALSE;
                                    }
                                 }
                              }
                              else
                              {
                                 /* Clear the default Value Trigger Setting*/
                                 /* and Time Trigger Setting flags since   */
                                 /* they should be excluded if the         */
                                 /* Aggregate and the Instance does not    */
                                 /* have the indicate or notify property.  */
                                 InstanceEntryPtr->Descriptor_Flags &= ~(AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING);

                                 /* Check if the application enabled the   */
                                 /* custom trigger.                        */
                                 if(InstanceEntryPtr->Use_Custom_Trigger)
                                 {
                                    /* Disable the custom trigger.         */
                                    InstanceEntryPtr->Use_Custom_Trigger = FALSE;
                                 }
                              }
                           }
                        }
                        else
                        {
                           /* Clear the bits for the Indicate and Notify   */
                           /* properties since they may not be included if */
                           /* read is not supported.                       */
                           InstanceEntryPtr->Characteristic_Property_Flags &= ~(GATT_CHARACTERISTIC_PROPERTIES_INDICATE | GATT_CHARACTERISTIC_PROPERTIES_NOTIFY);

                           /* Clear the default Value Trigger Setting and  */
                           /* Time Trigger Setting flags since they should */
                           /* be excluded if read is not supported.        */
                           InstanceEntryPtr->Descriptor_Flags &= ~(AIOS_DESCRIPTOR_FLAGS_VALUE_TRIGGER_SETTING | AIOS_DESCRIPTOR_FLAGS_TIME_TRIGGER_SETTING);

                           /* Check if the application enabled the custom  */
                           /* trigger.                                     */
                           if(InstanceEntryPtr->Use_Custom_Trigger)
                           {
                              /* Disable the custom trigger.               */
                              InstanceEntryPtr->Use_Custom_Trigger = FALSE;
                           }
                        }

                        /* We will go ahead and update the AIOS Server's*/
                        /* copy of the AIOS Characteristic Instance     */
                        /* Entry since fields may have been updated and */
                        /* we want to retain this information for later */
                        /* use by the AIOS Server.                      */
                        InstanceDataPtr->Instance_Entry = *InstanceEntryPtr;
                     }
                  }
                  else
                  {
                     ret_val = UNABLE_TO_ALLOCATE_MEMORY;
                  }
               }
            }
            else
            {
               ret_val = UNABLE_TO_ALLOCATE_MEMORY;
            }

            /* If an error has not occured then we can register the     */
            /* service.                                                 */
            if(!ret_val)
            {
               /* Initialize the service.                               */
               ret_val = AIOS_Initialize_Service(BluetoothStackID, (unsigned int)AIOS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, AIOS_EventCallback, 0, &AIOSInstanceID);
               if((ret_val > 0) && (AIOSInstanceID > 0))
               {
                  /* Display success message.                           */
                  printf("Successfully registered AIOS Service, AIOSInstanceID = %u.\r\n", ret_val);

                  /* Save the ServiceID of the registered service.      */
                  AIOSInstanceID = (unsigned int)ret_val;

                  /* Simply return success to the caller.               */
                  ret_val = 0;
               }
               else
                  DisplayFunctionError("AIOS_Initialize_Service", ret_val);
            }

            /* Free the memory for the InitializeData structure if it   */
            /* was allocated.                                           */
            /* * NOTE * The AIOS Server retains a copy of this          */
            /*          information for future use.                     */
            for(Index = 0; Index < (unsigned int)AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS; Index++)
            {
               if((InitializeData.Entries[Index].Number_Of_Instances) && (InitializeData.Entries[Index].Instances))
               {
                  /* Free the Characteristic instance entries.          */
                  BTPS_FreeMemory(InitializeData.Entries[Index].Instances);
                  InitializeData.Entries[Index].Instances           = NULL;
                  InitializeData.Entries[Index].Number_Of_Instances = 0;
               }
            }

            if((InitializeData.Number_Of_Entries) && (InitializeData.Entries))
            {
               /* Free the Characteristic entries.                      */
               BTPS_FreeMemory(InitializeData.Entries);
               InitializeData.Entries           = NULL;
               InitializeData.Number_Of_Entries = 0;
            }

         }
         else
         {
            printf("AIOS is already registered.\r\n");
         }
      }
      else
      {
         printf("Connection currently active.\r\n");
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      printf("Usage: RegisterAIOS [Aggregate Supported (False=0, True=1)] [CC Properties (0 = Indicate, 1 = Notify, 2 = Disabled)]  [Custom Trigger (False=0, True=1)].\r\n");
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for unregistering a BAP     */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnRegisterAIOSCommand(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that a service is registered.                              */
   if(AIOSInstanceID)
   {
      /* If there is a connected device, then first disconnect it.      */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR);

      /* Unregister the AIOS Service with GATT.                         */
      ret_val = AIOS_Cleanup_Service(BluetoothStackID, AIOSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("Successfully unregistered AIOS Service InstanceID %u.\r\n",AIOSInstanceID);

         /* Clear the InstanceID.                                       */
         AIOSInstanceID = 0;
      }
      else
         DisplayFunctionError("AIOS_Cleanup_Service", ret_val);
   }
   else
      printf("AIOS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function is responsible for performing a BAP        */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverAIOSCommand(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!AIOSInstanceID)
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
               /* Configure the filter so that only the AIOS Service is */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               AIOS_ASSIGN_AIOS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdAIOS);
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
      printf("Only the client can discover AIOS.\r\n");

   return(ret_val);
}

   /* The following function is responsible for issuing the command to  */
   /* read an AIOS Characteristic or its descriptors.                   */
static int ReadAIOSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                           ret_val = 0;
   AIOS_Characteristic_Info_t    CharacteristicInfo;
   AIOP_Attribute_Handle_Type_t  AttributeHandleType;
   Boolean_t                     ReadLong;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type, ID, and Attribute Type will be checked by*/
         /*          the internal function.                             */
         CharacteristicInfo.Type = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;
         AttributeHandleType     = (AIOP_Attribute_Handle_Type_t)TempParam->Params[2].intParam;

         /* If the Attribute handle type is for the User Description we */
         /* need to determine how to process the command.               */
         if(AttributeHandleType == ahtUserDescription)
         {
            /* Make sure we have the next parameter.                    */
            if(TempParam->NumberofParameters >= 4)
            {
               /* Store the parameter.                                  */
               ReadLong = (Boolean_t)TempParam->Params[3].intParam;

               /* Check if this is a GATT Read Value request for the    */
               /* User Description.                                     */
               if(!ReadLong)
               {
                  /* Simply call the internal function to procaios the  */
                  /* request.                                           */
                  ret_val = ReadAIOSCharacteristic(&CharacteristicInfo, AttributeHandleType);
               }
               else
               {
                  /* Issue a request to read the User Description with a*/
                  /* GATT Read Long Value request.                      */
                  ret_val = ReadLongUserDescriptionRequest(&CharacteristicInfo);
               }
            }
            else
               DisplayReadAIOSCharacteristicUsage();
         }
         else
         {
            /* Simply call the internal function to process the request.*/
            ret_val = ReadAIOSCharacteristic(&CharacteristicInfo, AttributeHandleType);
         }
      }
      else
         DisplayReadAIOSCharacteristicUsage();
   }
   else
      printf("\r\nBluetoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing the             */
   /* Digital/Analog Characteristic's CCCD.  This function will return  */
   /* zero on successful execution and a negative value on errors.      */
static int WriteCCCDCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
   {
      /* Store the parameters.                                          */
      CharacteristicInfo.Type = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;
      CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

      /* Store the CCCD.                                                */
      if(TempParam->Params[2].intParam == 1)
      {
         WriteRequestData.Data.CCCD = (Word_t)AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE;
      }
      else
      {
         if(TempParam->Params[2].intParam == 2)
         {
            WriteRequestData.Data.CCCD = (Word_t)AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE;
         }
         else
            WriteRequestData.Data.CCCD = 0;
      }

      /* Simply call the internal function to send the request.         */
      ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtClientCharacteristicConfig, &WriteRequestData);
   }
   else
      DisplayWriteCCCDUsage();

   return(ret_val);
}

   /* The following function is responsible for writing a Digital/Analog*/
   /* Characteristic's User description descriptor.                     */
static int WriteUserDescriptionCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         CharacteristicInfo.Type = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Set the length of the User Description.                     */
         WriteRequestData.UserDescriptionLength = BTPS_StringLength(TempParam->Params[2].strParam);

         /* Make sure the size will fit in the buffer.                  */
         if(WriteRequestData.UserDescriptionLength <= (unsigned int)AIOP_USER_DESCRIPTION_LENGTH)
         {
            /* Simply copy the data.                                    */
            BTPS_StringCopy(WriteRequestData.Data.UserDescription, TempParam->Params[2].strParam);

            /* Simply call the internal function to send the request.      */
            ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtUserDescription, &WriteRequestData);
         }
         else
            printf("\r\nUser description is too big to store in the buffer.\r\n");
      }
      else
         printf("\r\nUsage: WriteUserDescription [Type (0-1)] [ID(UINT16)] [User Description(UTF-8)]\r\n");
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing a Digital/Analog*/
   /* Characteristic's Value Trigger Setting descriptor.                */
static int WriteValueTriggerSettingCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;
   Byte_t                      BitMask[AIOP_DIGITAL_ARRAY_SIZE];

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
   {
      /* Store the parameters.                                          */
      CharacteristicInfo.Type                             = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;
      CharacteristicInfo.ID                               = (unsigned int)TempParam->Params[1].intParam;
      WriteRequestData.Data.ValueTriggerSetting.Condition = (AIOS_Value_Trigger_Type_t)TempParam->Params[2].intParam;

      /* Switch on the condition.                                       */
      switch(WriteRequestData.Data.ValueTriggerSetting.Condition)
      {
         case vttStateChanged:
         case vttNoValueTrigger:
            /* No parameters.                                           */
            break;
         case vttCrossedBoundaryAnalogValue:
         case vttOnBoundaryAnalogValue:
         case vttStateChangedAnalogValue:
            if(TempParam->NumberofParameters == 4)
            {
               /* Analog value parameter.                               */
               WriteRequestData.Data.ValueTriggerSetting.ComparisonValue.AnalogValue = (Word_t)TempParam->Params[3].intParam;
            }
            else
            {
               DisplayWriteValueTriggerUsage();
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case vttDigitalStateChangedBitMask:
            if(TempParam->NumberofParameters == 5)
            {
               /* Digital Bit mask.                                     */
               BitMask[0] = (Byte_t)TempParam->Params[3].intParam;
               BitMask[1] = (Byte_t)TempParam->Params[4].intParam;
               WriteRequestData.Data.ValueTriggerSetting.ComparisonValue.BitMask.Length = (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
               WriteRequestData.Data.ValueTriggerSetting.ComparisonValue.BitMask.Buffer = BitMask;
            }
            else
            {
               DisplayWriteValueTriggerUsage();
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case vttCrossedBoundaryAnalogInterval:
         case vttOnBoundaryAnalogInterval:
            if(TempParam->NumberofParameters == 5)
            {
               /* Analog Interval.                                      */
               WriteRequestData.Data.ValueTriggerSetting.ComparisonValue.AnalogInterval.LowerBound = (Word_t)TempParam->Params[3].intParam;
               WriteRequestData.Data.ValueTriggerSetting.ComparisonValue.AnalogInterval.UpperBound = (Word_t)TempParam->Params[4].intParam;
            }
            else
            {
               DisplayWriteValueTriggerUsage();
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         default:
            DisplayWriteValueTriggerUsage();
            ret_val = INVALID_PARAMETERS_ERROR;
            break;
      }

      if(!ret_val)
      {
         /* Simply call the internal function to send the request.      */
         ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtValueTriggerSetting, &WriteRequestData);
      }
      else
      {
         /* Invalid parameter.  Return success since we displayed the   */
         /* usage.                                                      */
         ret_val = 0;
      }
   }
   else
      DisplayWriteValueTriggerUsage();

   return(ret_val);
}

   /* The following function is responsible for writing a Digital/Analog*/
   /* Characteristic's Time Trigger Setting descriptor.                 */
static int WriteTimeTriggerSettingCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
   {
     /* Store the parameters.                                           */
     CharacteristicInfo.Type                            = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;
     CharacteristicInfo.ID                              = (unsigned int)TempParam->Params[1].intParam;
     WriteRequestData.Data.TimeTriggerSetting.Condition = (AIOS_Time_Trigger_Type_t)TempParam->Params[2].intParam;

      /* Switch on the condition.                                       */
      switch(WriteRequestData.Data.TimeTriggerSetting.Condition)
      {
         case tttNoTimeBasedTrigger:
            /* No parameters.                                           */
            break;
         case tttTimeIntervalIgnoreValueTrigger:
         case tttTimeIntervalCheckValueTrigger:
            if(TempParam->NumberofParameters == 5)
            {
               /* Time interval parameter.                              */
               WriteRequestData.Data.TimeTriggerSetting.ComparisonValue.TimeInterval.Lower = (Word_t)TempParam->Params[3].intParam;
               WriteRequestData.Data.TimeTriggerSetting.ComparisonValue.TimeInterval.Upper = (Word_t)TempParam->Params[4].intParam;
            }
            else
            {
               DisplayWriteTimeTriggerUsage();
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         case tttCountChangedMoreOftenThan:
            /* Count parameter.                                         */
            if(TempParam->NumberofParameters == 4)
            {
               WriteRequestData.Data.TimeTriggerSetting.ComparisonValue.Count = (Word_t)TempParam->Params[3].intParam;
            }
            else
            {
               DisplayWriteTimeTriggerUsage();
               ret_val = INVALID_PARAMETERS_ERROR;
            }
            break;
         default:
            DisplayWriteTimeTriggerUsage();
            ret_val = INVALID_PARAMETERS_ERROR;
            break;
      }

      if(!ret_val)
      {
         /* Simply call the internal function to send the request.      */
         ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtTimeTriggerSetting, &WriteRequestData);
      }
      else
      {
         /* Invalid parameter.  Return success since we displayed the   */
         /* usage.                                                      */
         ret_val = 0;
      }
   }
   else
      DisplayWriteTimeTriggerUsage();

   return(ret_val);
}

   /* The following function is responsible for Writing a Digital       */
   /* Characteristc's value.                                            */
static int WriteDigitalCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;
   Byte_t                      Digital[AIOP_DIGITAL_ARRAY_SIZE];

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 3))
   {
      /* Store the parameters.                                          */
      CharacteristicInfo.Type                  = actDigital;
      CharacteristicInfo.ID                    = (Word_t)TempParam->Params[0].intParam;
      Digital[0]                               = (Byte_t)TempParam->Params[1].intParam;
      Digital[1]                               = (Byte_t)TempParam->Params[2].intParam;

      if(TempParam->NumberofParameters == 4)
      {
         WriteRequestData.UseWriteWithoutResponse = (Boolean_t)TempParam->Params[3].intParam;
      }
      else
         WriteRequestData.UseWriteWithoutResponse = FALSE;

      /* Store the Digital Characteristic.                              */
      WriteRequestData.Data.Characteristic.Digital.Length = (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
      WriteRequestData.Data.Characteristic.Digital.Buffer = Digital;

      /* Simply call the internal function to send the request.         */
      ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtCharacteristic, &WriteRequestData);
   }
   else
      printf("Usage: WriteDigital [ID (UINT16)] [Digital Byte 1 (UINT8)] [Digital Byte 2(UINT8)] [Use Write WO/Response (Client ONLY and optional BOOLEAN) ]  \r\n");

   return(ret_val);
}

   /* The following function is responsible for Writing an Analog       */
   /* Characteristc's value.                                            */
static int WriteAnalogCommand(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   AIOS_Characteristic_Info_t  CharacteristicInfo;
   AIOP_Write_Request_Data_t   WriteRequestData;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters >= 2))
   {
      /* Store the parameters.                                          */
      CharacteristicInfo.Type                     = actAnalog;
      CharacteristicInfo.ID                       = (Word_t)TempParam->Params[0].intParam;
      WriteRequestData.Data.Characteristic.Analog = (Word_t)TempParam->Params[1].intParam;

      if(TempParam->NumberofParameters == 3)
      {
         WriteRequestData.UseWriteWithoutResponse = (Boolean_t)TempParam->Params[2].intParam;
      }
      else
         WriteRequestData.UseWriteWithoutResponse = FALSE;

      /* Simply call the internal function to send the request.         */
      ret_val = WriteAIOSCharacteristic(&CharacteristicInfo, ahtCharacteristic, &WriteRequestData);

   }
   else
      printf("Usage: WriteAnalog [ID (UINT16)] [Analog Value (UINT16)] [Use Write WO/Response (Client ONLY and optional BOOLEAN) ]  \r\n");

   return(ret_val);
}

   /* The following function is responsible for indicating/notifying a  */
   /* Characteristic to a remote AIOS Client.                           */
static int IndicateNotifyCommand(ParameterList_t *TempParam)
{
   int                          ret_val = 0;
   AIOS_Characteristic_Info_t   CharacteristicInfo;

   /* Make sure we are the server.                                      */
   if(AIOSInstanceID)
   {
      /* Verify that the input parameters are semi-valid.               */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         CharacteristicInfo.Type = (AIOS_Characteristic_Type_t)TempParam->Params[0].intParam;;
         CharacteristicInfo.ID   = (Word_t)TempParam->Params[1].intParam;

         /* Simpy call the internal function to send the notification or*/
         /* indication.                                                 */
         ret_val = IndicateNotifyCharacteristic(&CharacteristicInfo, (Boolean_t)TempParam->Params[2].intParam);
      }
      else
         DisplayIndicateNotifyUsage();
   }
   else
      printf("\r\nOnly the AIOS Server may send indications and notifications.\r\n");

   return(ret_val);
}

   /* The following function is a helper function to display the usage  */
   /* for the ReadAIOSCharacteristicCommand().                          */
static void DisplayReadAIOSCharacteristicUsage(void)
{
   printf("\r\nUsage: Read [Type (0-2)] [ID (UINT16)] [Attribute Type (UINT8)] [OPT PARAMS]\r\n");
   printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Digital\r\n");
   printf("  1  = Analog\r\n");
   printf("  2  = Aggregate\r\n");
   printf("\r\n Where Attribute Type and (OPT PARAMS) are:\r\n");
   printf("  0 = Characteristic\r\n");
   printf("  1 = Extended Properties\r\n");
   printf("  2 = CCCD\r\n");
   printf("  3 = Presentation Format\r\n");
   printf("  4 = Value Trigger Setting\r\n");
   printf("  5 = Time Trigger Setting\r\n");
   printf("  6 = User Description [Read Long(0 = FALSE, 1 = TRUE)]\r\n");
   printf("  7 = Number Of Digitals (Digital Only)\r\n");
   printf("  8 = Valid Range (Analog Only)\r\n");
   printf("\r\nNOTE: The ID parameter is REQUIRED, but will be ignored for the Aggregate type.\r\n");
   printf("\r\nNOTE: To read a long User Description descriptor, the standard read MUST be used first, followed by read longs for subsequent requests.\r\n");
}

   /* The following function is responsible for displaying the          */
   /* WriteCCCDCommand() usage.                                         */
static void DisplayWriteCCCDUsage(void)
{
   printf("Usage: WriteCCCD [Type (0-2)] [ID (UINT16)] [Enable Notify/Indicate (0=Disable, 1=Notify, 2=Indicate)]\r\n");
   printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Digital\r\n");
   printf("  1  = Analog\r\n");
   printf("  2  = Aggregate\r\n");
}

   /* The following function is responsible for displaying the          */
   /* WriteValueTriggerSettingCommand() usage.                          */
static void DisplayWriteValueTriggerUsage(void)
{
   printf("Usage: WriteValueTrigger [Type (0-2)] [ID (UINT16] [Condition (UINT8)] [Opt. Params]\r\n");
    printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Digital\r\n");
   printf("  1  = Analog\r\n");
   printf("  2  = Aggregate\r\n");
   printf("\r\n Where (Conditon) and (Opt. Params if present) are:\r\n");
   printf("  0 = State Changed\r\n");
   printf("  1 = Crossed Boundary [Analog Value(UINT16)]\r\n");
   printf("  2 = On Boundary [Analog Value(UINT16)]\r\n");
   printf("  3 = State Changed [Analog Value(UINT16)]\r\n");
   printf("  4 = State Changed (2 octets for this sample) [Dig. Bitmask LSO (UINT8)] [Dig. Bitmask MSO (UINT8)]\r\n");
   printf("  5 = Crossed Boundary [Analog Interval Lower(UINT16)] [Analog Interval Upper(UINT16)]\r\n");
   printf("  6 = On Boundary [Analog Interval Lower(UINT16)] [Analog Interval Upper(UINT16)]\r\n");
   printf("  7 = No Value Trigger Setting\r\n");
}

   /* The following function is responsible for displaying the          */
   /* WriteTimeTriggerSettingDescriptorCommand() usage.                 */
static void DisplayWriteTimeTriggerUsage(void)
{
   printf("Usage: WriteTimeTrigger [Type (0-2)] [ID] [Condition(UINT8)] [Opt. Params]\r\n");
    printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Digital\r\n");
   printf("  1  = Analog\r\n");
   printf("  2  = Aggregate\r\n");
   printf("\r\n Where (Conditon) and (Opt. Params if present) are:\r\n");
   printf("  0 = No Time Based Interval\r\n");
   printf("  1 = Time Interval (Ignore Value Trigger) [Interval Lower(UINT16)] [Interval Upper(UINT16)]\r\n");
   printf("  2 = Time Interval (Check Value Trigger) [Interval Lower(UINT16)] [Interval Upper(UINT16)]\r\n");
   printf("  3 = Count [Seconds(UINT16)]\r\n");
}

   /* The following function is responsible for display the             */
   /* IndicationNotifyCommand() usage.                                  */
static void DisplayIndicateNotifyUsage(void)
{
   printf("Usage: Indicate/Notify [Type (0-2)] [ID (UINT16] [Indicate/Notify (Indicate=0, Notify=1)]\r\n");
    printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Digital\r\n");
   printf("  1  = Analog\r\n");
   printf("  2  = Aggregate\r\n");
   printf("\r\nNOTE: ID will be ignored for the Aggregate type\r\n");
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

   /* The following is a AIOS Server Event Callback.  This function will*/
   /* be called whenever an AIOS Server Profile Event occurs that is    */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the AIOS Event Data  */
   /* that occurred and the AIOS Event Callback Parameter that was      */
   /* specified when this Callback was The caller is free to installed. */
   /* use the contents of the AIOS Event Data ONLY in the context If the*/
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
   /* possible (this argument holds anyway because another AIOS Event   */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving AIOS Event Packets. */
   /*            A Deadlock WILL occur because NO AIOS Event Callbacks  */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI AIOS_EventCallback(unsigned int BluetoothStackID, AIOS_Event_Data_t *AIOS_Event_Data, unsigned long CallbackParameter)
{
   int                                  Result;
   BoardStr_t                           BoardStr;
   AIOP_Server_Instance_Data_t         *ServerInstancePtr;

   /* AIOS Common Event fields.                                         */
   unsigned int                         InstanceID;
   unsigned int                         ConnectionID;
   unsigned int                         TransactionID;
   BD_ADDR_t                            RemoteDevice;
   GATT_Connection_Type_t               ConnectionType;
   AIOS_Characteristic_Info_t           CharacteristicInfo;

   /* AIOS Event fields.                                                */
   AIOS_Characteristic_Data_t           Data;
   Word_t                               ClientConfiguration;
   Word_t                               UserDescriptionOffset;
   Word_t                               UserDescriptionLength;
   Byte_t                              *UserDescription;
   AIOS_Value_Trigger_Data_t           *ValueTriggerSetting;
   AIOS_Time_Trigger_Data_t            *TimeTriggerSetting;
   Byte_t                               Status;
   Word_t                               BytesWritten;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (AIOS_Event_Data))
   {
      /* Determine the AIOS Event type.                                 */
      switch(AIOS_Event_Data->Event_Data_Type)
      {
         case etAIOS_Server_Read_Characteristic_Request:
            printf("\r\netAIOS_Server_Read_Characteristic_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data)
            {
               /* Store event information.                              */
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Read_Characteristic_Request_Data->CharacteristicInfo;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Determine the Characteristic type so we know how to   */
               /* format the AIOS Characteristic for the response.      */
               switch(CharacteristicInfo.Type)
               {
                  case actDigital:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Get the Digital Characteristic.              */
                        Data.Digital.Length = (Byte_t)AIOP_DIGITAL_ARRAY_SIZE;
                        Data.Digital.Buffer = ServerInstancePtr->Data.Digital;

                        /* Send the response.                           */
                        if((Result = AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &Data)) != 0)
                           DisplayFunctionError("AIOS_Read_Characteristic_Request_Response", Result);
                     }
                     else
                     {
                        /* Send the error response.                     */
                        if((Result = AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                           DisplayFunctionError("AIOS_Read_Characteristic_Request_Response", Result);
                     }
                     break;
                  case actAnalog:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Get the Analog Characteristic.               */
                        Data.Analog = ServerInstancePtr->Data.Analog;

                        /* Send the response.                           */
                        if((Result = AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &Data)) != 0)
                           DisplayFunctionError("AIOS_Read_Characteristic_Request_Response", Result);
                     }
                     else
                     {
                        /* Send the error response.                     */
                        if((Result = AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                           DisplayFunctionError("AIOS_Read_Characteristic_Request_Response", Result);
                     }
                     break;
                  case actAggregate:
                        /* Simply call the internal function to format  */
                        /* the Aggregate data.                          */
                        if((Result = FormatAggregateCharacteristic(&(Data.Aggregate))) == 0)
                        {
                           /* Send the response.                        */
                           if((Result = AIOS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, ConnectionID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &Data)) != 0)
                              DisplayFunctionError("AIOS_Read_Characteristic_Request_Response", Result);

                           /* If we sent the Aggregate Characteristic we*/
                           /* need to free the memory for the buffer.   */
                           if((Data.Aggregate.Length) && (Data.Aggregate.Buffer))
                           {
                              /* Free the AggregateBuffer field of the  */
                              /* Aggregate Data just sent.              */
                              BTPS_FreeMemory(Data.Aggregate.Buffer);
                              Data.Aggregate.Buffer = NULL;
                              Data.Aggregate.Length = 0;
                           }
                        }
                     break;
                  default:
                     /* This CANNOT occur the event will not be         */
                     /* received.                                       */
                     break;
               }
            }
            break;
         case etAIOS_Server_Write_Characteristic_Request:
            printf("\r\netAIOS_Server_Write_Characteristic_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data)
            {
               /* Store event information.                              */
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->CharacteristicInfo;
               Data               = AIOS_Event_Data->Event_Data.AIOS_Write_Characteristic_Request_Data->Data;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Determine the Characteristic type so we know what to  */
               /* store and print the Characteristic data.              */
               switch(CharacteristicInfo.Type)
               {
                  case actDigital:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Make sure we have room to store the Digital  */
                        /* Characteristic.                              */
                        if(Data.Digital.Length == (Word_t)AIOP_DIGITAL_ARRAY_SIZE)
                        {
                           /* Simply copy the buffer that contains the  */
                           /* Digital Characteristic array to store the */
                           /* Digital Characteristic.                   */
                           BTPS_MemCopy(ServerInstancePtr->Data.Digital, Data.Digital.Buffer, Data.Digital.Length);

                           /* Display the Digital Characteristic.       */
                           DisplayDigitalCharacteristic(&(Data.Digital), CharacteristicInfo.ID);

                            /* Send the write request response.         */
                           if((Result = AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Characteristic_Request_Response", Result);
                        }
                        else
                        {
                           /* We will simply reject the write request   */
                           /* since the Digital Characteristic being    */
                           /* written has an invalid length.            */
                           if((Result = AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Characteristic_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* Send the error response.                     */
                        if((Result = AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                           DisplayFunctionError("AIOS_Write_Characteristic_Request_Response", Result);
                     }
                     break;
                  case actAnalog:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Store the Analog Characteristic.             */
                        ServerInstancePtr->Data.Analog = Data.Analog;

                        /* Display the Analog Characteristic.           */
                        DisplayAnalogCharacteristic(ServerInstancePtr->Data.Analog, CharacteristicInfo.ID);

                        /* Send the write request response.             */
                        if((Result = AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                           DisplayFunctionError("AIOS_Write_Characteristic_Request_Response", Result);
                     }
                     else
                     {
                        /* Send the error response.                     */
                        if((Result = AIOS_Write_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                           DisplayFunctionError("AIOS_Write_Characteristic_Request_Response", Result);
                     }
                     break;
                  case actAggregate:
                        /* Intentional fall through since the Aggregate */
                        /* Characteristic CANNOT be written.            */
                  default:
                     /* This CANNOT occur the event will not be         */
                     /* received.                                       */
                     break;
               }
            }
            break;
         case etAIOS_Server_Read_CCCD_Request:
            printf("\r\netAIOS_Server_Read_CCCD_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Read_CCCD_Request_Data->CharacteristicInfo;

               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Determine the Characteristic type so we know what CCCD*/
               /* to send in the response.                              */
               switch(CharacteristicInfo.Type)
               {
                  case actDigital:
                  case actAnalog:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Intentional fall through.                    */
                        ClientConfiguration = ServerInstancePtr->Client_Configuration;

                        /* Send the response.                           */
                        if((Result = AIOS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, ClientConfiguration)) != 0)
                           DisplayFunctionError("AIOS_Read_CCCD_Request_Response", Result);
                     }
                     else
                     {
                        /* Send the error response.                     */
                        /* * NOTE * We set the CCCD to zero since it    */
                        /*          will be ignored.                    */
                        if((Result = AIOS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, 0)) != 0)
                           DisplayFunctionError("AIOS_Read_CCCD_Request_Response", Result);
                     }
                     break;
                  case actAggregate:
                     ClientConfiguration = AIOSServerInfo.Aggregate_CC;

                     /* Send the response.                              */
                     if((Result = AIOS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, ClientConfiguration)) != 0)
                        DisplayFunctionError("AIOS_Read_CCCD_Request_Response", Result);
                     break;
                  default:
                     /* This CANNOT occur the event will not be         */
                     /* received.                                       */
                     break;
               }
            }
            break;
         case etAIOS_Server_Write_CCCD_Request:
            printf("\r\netAIOS_Server_Write_CCCD_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->InstanceID;
               ConnectionID        = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->ConnectionID;
               TransactionID       = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->TransactionID;
               ConnectionType      = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->ConnectionType;
               RemoteDevice        = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->RemoteDevice;
               CharacteristicInfo  = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->CharacteristicInfo;
               ClientConfiguration = AIOS_Event_Data->Event_Data.AIOS_Write_CCCD_Request_Data->ClientConfiguration;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Display the received configuration.                   */
               printf("   Client Configuration: 0x%04X.\r\n", ClientConfiguration);

               /* Determine the Characteristic type so we know how to   */
               /* verify the CCCD and where to store it.  where to store*/
               /* the new CCCD.                                         */
               switch(CharacteristicInfo.Type)
               {
                  case actDigital:
                  case actAnalog:
                     /* Store a pointer to the Characteristic instance  */
                     /* data for readability.                           */
                     /* * NOTE * We will use the Characteristic         */
                     /*          information to access it.              */
                     if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
                     {
                        /* Intentional fall through.                    */
                        if(ServerInstancePtr->Instance_Entry.Characteristic_Property_Flags & AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_INDICATE)
                        {
                           /* We will only accept indicate or disabled  */
                           /* (0).                                      */
                           if((!ClientConfiguration) || (ClientConfiguration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE))
                           {
                              /* Store the Client Configuration.        */
                              ServerInstancePtr->Client_Configuration = ClientConfiguration;

                              /* Send the response.                     */
                              if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                                 DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);

                              /* If indications were enabled then we    */
                              /* need to immediately indcate this       */
                              /* Characteristic to the AIOS Client.     */
                              /* * NOTE * We will assume indications    */
                              /*          were previously disabled.     */
                              if(ServerInstancePtr->Client_Configuration == AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                              {
                                 /* Simply call the internal function to*/
                                 /* format and send the indication.     */
                                 if((Result = IndicateNotifyCharacteristic(&CharacteristicInfo, FALSE)) != 0)
                                    DisplayFunctionError("IndicateNotifyCharacteristic", Result);
                              }
                           }
                           else
                           {
                              /* Reject the write request since the CCCD*/
                              /* is improperly configured.              */
                              if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED, &CharacteristicInfo)) != 0)
                                 DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);
                           }
                        }
                        else
                        {
                           /* We will only accept notify or disabled    */
                           /* (0).                                      */
                           if((!ClientConfiguration) || (ClientConfiguration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE))
                           {
                              /* Store the Client Configuration.        */
                              ServerInstancePtr->Client_Configuration = ClientConfiguration;

                              /* Send the response.                     */
                              if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                                 DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);

                              /* If notifications were enabled then we  */
                              /* need to immediately indcate this       */
                              /* Characteristic to the AIOS Client.     */
                              /* * NOTE * We will assume notifications  */
                              /*          were previously disabled.     */
                              if(ServerInstancePtr->Client_Configuration == AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                              {
                                 /* Simply call the internal function to*/
                                 /* format and send the notification.   */
                                 if((Result = IndicateNotifyCharacteristic(&CharacteristicInfo, TRUE)) != 0)
                                    DisplayFunctionError("IndicateNotifyCharacteristic", Result);
                              }
                           }
                           else
                           {
                              /* Reject the write request since the CCCD*/
                              /* is improperly configured.              */
                              if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED, &CharacteristicInfo)) != 0)
                                 DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);
                           }
                        }
                     }
                     else
                     {
                        /* Send the error response.                     */
                        if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                           DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);
                     }
                     break;
                  case actAggregate:
                     /* If the Aggregate supports Indicate.             */
                     if(AIOSServerInfo.Aggregate_Property_Flags & AIOS_AGGREGATE_PROPERTY_FLAGS_INDICATE)
                     {
                        /* We will only accept indicate or disabled (0).*/
                        if((!ClientConfiguration) || (ClientConfiguration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE))
                        {
                           /* Store the Client Configuration.           */
                           AIOSServerInfo.Aggregate_CC = ClientConfiguration;

                           /* Send the response.                        */
                           if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);

                           /* If indications were enabled then we need  */
                           /* to immediately indcate this Characteristic*/
                           /* to the AIOS Client.                       */
                           /* * NOTE * We will assume indications were  */
                           /*          previously disabled.             */
                           if(AIOSServerInfo.Aggregate_CC == AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
                           {
                              /* Simply call the internal function to   */
                              /* format and send the indication.        */
                              if((Result = IndicateNotifyCharacteristic(&CharacteristicInfo, FALSE)) != 0)
                                 DisplayFunctionError("IndicateNotifyCharacteristic", Result);
                           }
                        }
                        else
                        {
                           /* Reject the write request since the CCCD is*/
                           /* improperly configured.                    */
                           if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);
                        }
                     }
                     else
                     {
                        /* We will only accept notify or disabled (0).  */
                        if((!ClientConfiguration) || (ClientConfiguration & AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE))
                        {
                           /* Store the Client Configuration.           */
                           AIOSServerInfo.Aggregate_CC = ClientConfiguration;

                           /* Send the response.                        */
                           if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);

                           /* If notifications were enabled then we need*/
                           /* to immediately indcate this Characteristic*/
                           /* to the AIOS Client.                       */
                           /* * NOTE * We will assume notifications were*/
                           /*          previously disabled.             */
                           if(AIOSServerInfo.Aggregate_CC == AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                           {
                              /* Simply call the internal function to   */
                              /* format and send the notification.      */
                              if((Result = IndicateNotifyCharacteristic(&CharacteristicInfo, TRUE)) != 0)
                                 DisplayFunctionError("IndicateNotifyCharacteristic", Result);
                           }
                        }
                        else
                        {
                           /* Reject the write request since the CCCD is*/
                           /* improperly configured.                    */
                           if((Result = AIOS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_CCCD_Request_Response", Result);
                        }
                     }
                     break;
                  default:
                     /* This CANNOT occur the event will not be         */
                     /* received.                                       */
                     break;
               }
            }
            break;
         case etAIOS_Server_Read_Presentation_Format_Request:
            printf("\r\netAIOS_Server_Read_Presentation_Format_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data)
            {
               /* Store event information.                              */
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Read_Presentation_Format_Request_Data->CharacteristicInfo;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  /* * NOTE * This MUST be a Presentation Format        */
                  /*          descriptor for a Digital/Analog           */
                  /*          Characteristic since the Aggregate CANNOT */
                  /*          have one.                                 */
                  if((Result = AIOS_Read_Presentation_Format_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &(ServerInstancePtr->Presentation_Format))) != 0)
                     DisplayFunctionError("AIOS_Read_Presentation_Format_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Read_Presentation_Format_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     DisplayFunctionError("AIOS_Read_Presentation_Format_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Read_User_Description_Request:
            printf("\r\netAIOS_Server_Read_User_Description_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data)
            {
               /* Store event information.                              */
               InstanceID            = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->InstanceID;
               ConnectionID          = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->ConnectionID;
               TransactionID         = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->TransactionID;
               ConnectionType        = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->ConnectionType;
               RemoteDevice          = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->RemoteDevice;
               CharacteristicInfo    = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->CharacteristicInfo;
               UserDescriptionOffset = AIOS_Event_Data->Event_Data.AIOS_Read_User_Description_Request_Data->Offset;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Offset:               %u.\r\n", UserDescriptionOffset);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  /* * NOTE * This MUST be a User Description descriptor*/
                  /*          for a Digital/Analog Characteristic since */
                  /*          the Aggregate CANNOT have one.            */
                  /* * NOTE * We will simply set the start of the User  */
                  /*          Description to the offset that has been   */
                  /*          requested.  This is for GATT Read Long    */
                  /*          requests.                                 */
                  if((Result = AIOS_Read_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, (ServerInstancePtr->User_Description + UserDescriptionOffset))) != 0)
                     DisplayFunctionError("AIOS_Read_Presentation_Format_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Read_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     DisplayFunctionError("AIOS_Read_User_Description_Request_Response", Result);
               }

            }
            break;
         case etAIOS_Server_Write_User_Description_Request:
            printf("\r\netAIOS_Server_Write_User_Description_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data)
            {
               /* Store event information.                              */
               InstanceID            = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->InstanceID;
               ConnectionID          = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->ConnectionID;
               TransactionID         = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->TransactionID;
               ConnectionType        = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->ConnectionType;
               RemoteDevice          = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->RemoteDevice;
               CharacteristicInfo    = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->CharacteristicInfo;
               UserDescriptionLength = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->UserDescriptionLength;
               UserDescription       = AIOS_Event_Data->Event_Data.AIOS_Write_User_Description_Request_Data->UserDescription;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Make sure we have enough room to hold the new user */
                  /* description.                                       */
                  /* * NOTE * We will make sure we have room for the    */
                  /*          NULL terminator.                          */
                  if((UserDescriptionLength + 1) <= (Word_t)AIOP_USER_DESCRIPTION_LENGTH)
                  {
                     /* Let's store the new User Description.           */
                     BTPS_MemCopy(ServerInstancePtr->User_Description, UserDescription, UserDescriptionLength);

                     /* Make sure the NULL terminator is set so we don't*/
                     /* display past the end of the string.             */
                     ServerInstancePtr->User_Description[UserDescriptionLength] = '\0';

                     /* Display the User Description.                   */
                     printf("      User Description:  %s.\r\n", ServerInstancePtr->User_Description);

                     /* Send the response.                              */
                     if((Result = AIOS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                        DisplayFunctionError("AIOS_Write_User_Description_Request_Response", Result);
                  }
                  else
                  {
                     /* Send the error response since the length is too */
                     /* big to store.                                   */
                     if((Result = AIOS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH, &CharacteristicInfo)) != 0)
                        DisplayFunctionError("AIOS_Write_User_Description_Request_Response", Result);
                  }
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                     DisplayFunctionError("AIOS_Write_User_Description_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Read_Value_Trigger_Setting_Request:
            printf("\r\netAIOS_Server_Read_Value_Trigger_Setting_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->InstanceID;
               ConnectionID        = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->ConnectionID;
               TransactionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->TransactionID;
               ConnectionType      = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->ConnectionType;
               RemoteDevice        = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->RemoteDevice;
               CharacteristicInfo  = AIOS_Event_Data->Event_Data.AIOS_Read_Value_Trigger_Setting_Request_Data->CharacteristicInfo;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  if((Result = AIOS_Read_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &(ServerInstancePtr->Value_Trigger_Setting))) != 0)
                     DisplayFunctionError("AIOS_Read_Value_Trigger_Setting_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Read_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     DisplayFunctionError("AIOS_Read_Value_Trigger_Setting_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Write_Value_Trigger_Setting_Request:
            printf("\r\netAIOS_Server_Write_Value_Trigger_Setting_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->InstanceID;
               ConnectionID        = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->ConnectionID;
               TransactionID       = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->TransactionID;
               ConnectionType      = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->ConnectionType;
               RemoteDevice        = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->RemoteDevice;
               CharacteristicInfo  = AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->CharacteristicInfo;
               ValueTriggerSetting = &(AIOS_Event_Data->Event_Data.AIOS_Write_Value_Trigger_Setting_Request_Data->ValueTriggerSetting);

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Display the Value Trigger Setting.                    */
               DisplayValueTriggerSetting(ValueTriggerSetting);

                /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* We need to verify the condition based on the       */
                  /* Characteristic type.                               */
                  switch(CharacteristicInfo.Type)
                  {
                     case actDigital:
                        /* We will only accept valid conditions for the */
                        /* Digital Characteristic.                      */
                        if((ValueTriggerSetting->Condition == vttStateChanged) ||
                           (ValueTriggerSetting->Condition == vttDigitalStateChangedBitMask) ||
                           (ValueTriggerSetting->Condition == vttNoValueTrigger))
                        {
                           /* Store the Value Trigger Setting.          */
                           ServerInstancePtr->Value_Trigger_Setting = *ValueTriggerSetting;

                           /* Send the response.                        */
                           if((Result = AIOS_Write_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Value_Trigger_Setting_Request_Response", Result);
                        }
                        else
                        {
                           /* Send the error response.                  */
                           if((Result = AIOS_Write_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_TRIGGER_CONDITION_VALUE_NOT_SUPPORTED, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Value_Trigger_Setting_Request_Response", Result);
                        }
                        break;
                     case actAnalog:
                        /* We will only accept valid conditions for the */
                        /* Digital Characteristic.                      */
                        if((ValueTriggerSetting->Condition == vttStateChanged) ||
                           (ValueTriggerSetting->Condition == vttCrossedBoundaryAnalogValue) ||
                           (ValueTriggerSetting->Condition == vttOnBoundaryAnalogValue) ||
                           (ValueTriggerSetting->Condition == vttStateChangedAnalogValue) ||
                           (ValueTriggerSetting->Condition == vttCrossedBoundaryAnalogInterval) ||
                           (ValueTriggerSetting->Condition == vttOnBoundaryAnalogInterval) ||
                           (ValueTriggerSetting->Condition == vttNoValueTrigger))
                        {
                           /* Store the Value Trigger Setting.          */
                           ServerInstancePtr->Value_Trigger_Setting = *ValueTriggerSetting;

                           /* Send the response.                        */
                           if((Result = AIOS_Write_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Value_Trigger_Setting_Request_Response", Result);
                        }
                        else
                        {
                           /* Send the error response.                  */
                           if((Result = AIOS_Write_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_TRIGGER_CONDITION_VALUE_NOT_SUPPORTED, &CharacteristicInfo)) != 0)
                              DisplayFunctionError("AIOS_Write_Value_Trigger_Setting_Request_Response", Result);
                        }
                        break;
                     default:
                        /* This CANNOT occur the event will not be      */
                        /* received.                                    */
                        break;
                  }
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Write_Value_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                     DisplayFunctionError("AIOS_Write_Value_Trigger_Setting_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Read_Time_Trigger_Setting_Request:
            printf("\r\netAIOS_Server_Read_ValetAIOS_Server_Read_Time_Trigger_Setting_Requestue_Trigger_Setting_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->InstanceID;
               ConnectionID        = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->ConnectionID;
               TransactionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->TransactionID;
               ConnectionType      = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->ConnectionType;
               RemoteDevice        = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->RemoteDevice;
               CharacteristicInfo  = AIOS_Event_Data->Event_Data.AIOS_Read_Time_Trigger_Setting_Request_Data->CharacteristicInfo;

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  if((Result = AIOS_Read_Time_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &(ServerInstancePtr->Time_Trigger_Setting))) != 0)
                     DisplayFunctionError("AIOS_Read_Time_Trigger_Setting_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Read_Time_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     DisplayFunctionError("AIOS_Read_Time_Trigger_Setting_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Write_Time_Trigger_Setting_Request:
            printf("\r\netAIOS_Server_Write_Time_Trigger_Setting_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data)
            {
               /* Store event information.                              */
               InstanceID          = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->InstanceID;
               ConnectionID        = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->ConnectionID;
               TransactionID       = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->TransactionID;
               ConnectionType      = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->ConnectionType;
               RemoteDevice        = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->RemoteDevice;
               CharacteristicInfo  = AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->CharacteristicInfo;
               TimeTriggerSetting = &(AIOS_Event_Data->Event_Data.AIOS_Write_Time_Trigger_Setting_Request_Data->TimeTriggerSetting);

               /* Print event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the Characteristic Information.               */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Display the Time Trigger Setting.                     */
               DisplayTimeTriggerSetting(TimeTriggerSetting);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Store the Time Trigger Setting.                    */
                  ServerInstancePtr->Time_Trigger_Setting = *TimeTriggerSetting;

                  /* Send the response.                                 */
                  if((Result = AIOS_Write_Time_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                     DisplayFunctionError("AIOS_Write_Time_Trigger_Setting_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Write_Time_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                     DisplayFunctionError("AIOS_Write_Time_Trigger_Setting_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Read_Number_Of_Digitals_Request:
            printf("\r\netAIOS_Server_Read_Number_Of_Digitals_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data)
            {
               /* Print event information.                              */
               BD_ADDRToStr(AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->RemoteDevice, BoardStr);
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Read_Number_Of_Digitals_Request_Data->CharacteristicInfo;

               /* Store event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the characteristic Information.               */
               /* * NOTE * The Type field is basically information to   */
               /*          tell whether this is a digital input/output. */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  if((Result = AIOS_Read_Number_Of_Digitals_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, ServerInstancePtr->Number_Of_Digitals)) != 0)
                     DisplayFunctionError("AIOS_Read_Number_Of_Digitals_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  /* * NOTE * We pass zero in for the Number Of Digitals*/
                  /*          since it will be ignored.                 */
                  if((Result = AIOS_Read_Number_Of_Digitals_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, 0)) != 0)
                     DisplayFunctionError("AIOS_Read_Number_Of_Digitals_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Read_Valid_Range_Request:
            printf("\r\netAIOS_Server_Read_Valid_Range_Request with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data)
            {
               /* Print event information.                              */
               BD_ADDRToStr(AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->RemoteDevice, BoardStr);
               InstanceID         = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->InstanceID;
               ConnectionID       = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->ConnectionID;
               TransactionID      = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->TransactionID;
               ConnectionType     = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->ConnectionType;
               RemoteDevice       = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->RemoteDevice;
               CharacteristicInfo = AIOS_Event_Data->Event_Data.AIOS_Read_Valid_Range_Request_Data->CharacteristicInfo;

               /* Store event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);

               /* Display the characteristic Information.               */
               /* * NOTE * The Type field is basically information to   */
               /*          tell whether this is a digital input/output. */
               DisplayCharacteristicInfo(&CharacteristicInfo);

               /* Store a pointer to the Characteristic instance data   */
               /* for readability.                                      */
               /* * NOTE * We will use the Characteristic information to*/
               /*          access it.                                   */
               if((ServerInstancePtr = GetServerInstanceInfoPtr(&CharacteristicInfo)) != NULL)
               {
                  /* Send the response.                                 */
                  if((Result = AIOS_Read_Valid_Range_Request_Response(BluetoothStackID, InstanceID, TransactionID, AIOS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &(ServerInstancePtr->Valid_Range))) != 0)
                     DisplayFunctionError("AIOS_Read_Valid_Range_Request_Response", Result);
               }
               else
               {
                  /* Send the error response.                           */
                  if((Result = AIOS_Read_Valid_Range_Request_Response(BluetoothStackID, InstanceID, ConnectionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     DisplayFunctionError("AIOS_Read_Valid_Range_Request_Response", Result);
               }
            }
            break;
         case etAIOS_Server_Confirmation:
            printf("\r\netAIOS_Server_Confirmation_Data with size %u.\r\n", AIOS_Event_Data->Event_Data_Size);
            if(AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data)
            {
               /* Print event information.                              */
               InstanceID     = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->InstanceID;
               ConnectionID   = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->ConnectionID;
               TransactionID  = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->TransactionID;
               ConnectionType = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->ConnectionType;
               RemoteDevice   = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->RemoteDevice;
               Status         = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->Status;
               BytesWritten   = AIOS_Event_Data->Event_Data.AIOS_Confirmation_Data->BytesWritten;

               /* Store event information.                              */
               printf("   Instance ID:          %u.\r\n", InstanceID);
               printf("   Connection ID:        %u.\r\n", ConnectionID);
               printf("   Transaction ID:       %u.\r\n", TransactionID);
               printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
               BD_ADDRToStr(RemoteDevice, BoardStr);
               printf("   Remote Device:        %s.\r\n", BoardStr);
               printf("   Status:               0x%02X.\r\n", Status);
               printf("   Bytes Written:        %u.\r\n", BytesWritten);
            }
            break;
         default:
            printf("\r\nUnknown AIOS Event\r\n");
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
static void BTPSAPI GATT_ClientEventCallback_AIOS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   int                                 Result;
   BoardStr_t                          BoardStr;
   DeviceInfo_t                       *DeviceInfo;
   unsigned int                        Index;
   Word_t                              AttributeHandle = 0;
   AIOP_Client_Instance_Info_t        *InstanceInfoPtr = NULL;

   /* GATT AIOS Client Event Callback data types.                       */

   unsigned int                        ConnectionID;
   unsigned int                        TransactionID;
   GATT_Connection_Type_t              ConnectionType;
   BD_ADDR_t                           RemoteDevice;

   Word_t                              ValueOffset;
   Word_t                              ValueLength;
   Byte_t                             *Value;
   Word_t                              BytesWritten;
   GATT_Request_Error_Type_t           ErrorType;
   Word_t                              MTU;
   Word_t                              CurrentOffset;
   Word_t                              ExpectedLength;
   Word_t                              PreparedWriteLength;

   union
   {
      AIOS_Characteristic_Data_t       Data;
      Word_t                           CCCD;
      AIOS_Presentation_Format_Data_t  PresentationFormat;
      Word_t                           ExtendedProperties;
      char                            *UserDescription;
      AIOS_Value_Trigger_Data_t        ValueTrigger;
      AIOS_Time_Trigger_Data_t         TimeTrigger;
      AIOS_Valid_Range_Data_t          ValidRange;
   } ReadResponseData;

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

                  /* Print common error codes.                          */
                  switch(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode)
                  {
                     case AIOS_ERROR_CODE_TRIGGER_CONDITION_VALUE_NOT_SUPPORTED:
                        printf("   Error Mesg:       AIOS_ERROR_CODE_TRIGGER_CONDITION_VALUE_NOT_SUPPORTED\r\n");
                        break;
                     case ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED:
                        printf("   Error Mesg:       ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED\r\n");
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
                     /* Process the request depending on the attribute  */
                     /* handle type we set before issuing the read      */
                     /* request.                                        */
                     switch(DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType)
                     {
                        case ahtCharacteristic:
                           /* We need to determine the type of AIOS     */
                           /* Characteristic to properly decode the     */
                           /* value.                                    */
                           switch(DeviceInfo->AIOPClientInfo.Client_Request_Info.Type)
                           {
                              case actDigital:
                                 /* Call the function to decode and     */
                                 /* display the Digital Characteristic. */
                                 DecodeDisplayDigitalCharacteristic(ValueLength, Value, DeviceInfo->AIOPClientInfo.Client_Request_Info.ID);
                                 break;
                              case actAnalog:
                                 /* Call the function to decode and     */
                                 /* display the Analog Characteristic.  */
                                 DecodeDisplayAnalogCharacteristic(ValueLength, Value, DeviceInfo->AIOPClientInfo.Client_Request_Info.ID);
                                 break;
                              case actAggregate:
                                 /* Call the function to decode and     */
                                 /* display the Aggregate               */
                                 /* Charactersistic.                    */
                                 DecodeDisplayAggregateCharacteristic(&(DeviceInfo->AIOPClientInfo), ValueLength, Value);
                                 break;
                              default:
                                 printf("\r\nFailure - Invalid Characteristic type.\r\n");
                                 break;
                           }
                           break;
                        case ahtExtendedProperties:
                           /* Display the descriptor type.              */
                           printf("\r\nExtended Properties:\r\n");

                           /* Verify the length of the Extended         */
                           /* Properties.                               */
                           if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                           {
                              /* Decode the value.                      */
                              ReadResponseData.ExtendedProperties = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                              /* Display the value.                     */
                              printf("   Value:  0x%04X\r\n", ReadResponseData.ExtendedProperties);
                              break;
                           }
                           else
                              printf("   Value:  Invalid length.\r\n");
                           break;
                        case ahtClientCharacteristicConfig:
                           /* Display the descriptor type.              */
                              printf("\r\nCCCD:\r\n");

                           /* Verify the length of the CCCD.            */
                           if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                           {
                              /* Decode the value.                      */
                              ReadResponseData.CCCD = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);

                              /* Display the value.                     */
                              printf("   Value:  0x%04X\r\n", ReadResponseData.CCCD);
                              break;
                           }
                           else
                              printf("   Value:  Invalid length.\r\n");
                           break;
                        case ahtPresentationFormat:
                           /* Simply call the AIOS Client API to decode */
                           /* the Presentation Format.                  */
                           if((Result = AIOS_Decode_Presentation_Format(ValueLength, Value, &ReadResponseData.PresentationFormat)) == 0)
                           {
                              /* Simply call the internal function to   */
                              /* display the Presentation Format        */
                              /* descriptor.                            */
                              DisplayPresentationFormatData(&ReadResponseData.PresentationFormat);
                           }
                           else
                              DisplayFunctionError("AIOS_Decode_Presentation_Format",  Result);
                           break;
                        case ahtValueTriggerSetting:
                           /* Simply call the AIOS Client API to decode */
                           /* the Value Trigger Setting.                */
                           if((Result = AIOS_Decode_Value_Trigger_Setting(ValueLength, Value, &ReadResponseData.ValueTrigger)) == 0)
                           {
                              /* Simply call the internal function to   */
                              /* display the Value Trigger Setting      */
                              /* descriptor.                            */
                              DisplayValueTriggerSetting(&ReadResponseData.ValueTrigger);
                           }
                           else
                              DisplayFunctionError("AIOS_Decode_Value_Trigger_Setting",  Result);
                           break;
                        case ahtTimeTriggerSetting:
                           /* Simply call the AIOS Client API to decode */
                           /* the Time Trigger Setting.                 */
                           if((Result = AIOS_Decode_Time_Trigger_Setting(ValueLength, Value, &ReadResponseData.TimeTrigger)) == 0)
                           {
                              /* Simply call the internal function to   */
                              /* display the Time Trigger Setting       */
                              /* descriptor.                            */
                              DisplayTimeTriggerSetting(&ReadResponseData.TimeTrigger);

                           }
                           else
                              DisplayFunctionError("AIOS_Decode_Time_Trigger_Setting",  Result);
                           break;
                        case ahtUserDescription:
                           /* Display the descriptor type.              */
                           printf("\r\nUser Description:\r\n");

                           /* Make sure the length is valid for the     */
                           /* buffer size.                              */
                           /* * NOTE * Add one to make sure there is    */
                           /*          room so we can insert the NULL   */
                           /*          terminatord.                     */
                           if((ValueLength + 1) <= ((Word_t)AIOP_USER_DESCRIPTION_LENGTH))
                           {
                              /* Set the pointer to the User            */
                              /* Description.                           */
                              /* * NOTE * We will always assume zero for*/
                              /*          the offset for a read request */
                              /*          (not long).  Offset should be */
                              /*          set to zero when the request  */
                              /*          is made.                      */
                              BTPS_MemCopy(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription, Value, ValueLength);

                              /* Insert the NULL terminator.            */
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription[ValueLength] = '\0';

                              /* Display the User Description.          */
                              /* * NOTE * This may be incomplete if we  */
                              /*          could not read the entire User*/
                              /*          Description in a read request.*/

                              printf("   Value:   %s.\r\n", DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription);

                              /* Update the offset.                     */
                              /* * NOTE * We will update the offset in  */
                              /*          case we did not read the      */
                              /*          entire User description.  If  */
                              /*          this is the case we need to   */
                              /*          issue a read long request and */
                              /*          the offset will already be set*/
                              /*          to the correct position.      */
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset = ValueLength;
                           }
                           else
                              printf("\r\nError - ValueLength is greater than User Description buffer.\r\n");
                           break;
                        case ahtNumberOfDigitals:
                           /* Display the descriptor type.              */
                           printf("\r\nNumber Of Digitals:\r\n");

                           /* Verify the length of the number of        */
                           /* digitals.                                 */
                           if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
                           {
                              /* Let's get the instance information     */
                              /* based on the Type and ID we previously */
                              /* stored when the request was sent so we */
                              /* can store the Number of Digitals value */
                              /* in the instance's information.         */
                              InstanceInfoPtr = GetClientInstanceInfoPtr(&(DeviceInfo->AIOPClientInfo));
                              if(InstanceInfoPtr)
                              {
                                 /* Decode the value.                   */
                                 InstanceInfoPtr->Number_Of_Digitals = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);

                                 /* Display the value.                  */
                                 printf("   Value:  %u\r\n", InstanceInfoPtr->Number_Of_Digitals);
                              }
                              else
                                 printf("\r\nFailure - Could not find the Characteristic instance's information.\r\n");
                           }
                           else
                              printf("   Value:  Invalid length.\r\n");

                           /* Check if we are automatically reading the */
                           /* Number Of Digital descriptors for Digital */
                           /* Characteristics that are included in the  */
                           /* Aggregate Characteristic.                 */
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_READING_NUMBER_OF_DIGITALS)
                           {
                              /* We will flag that we are done unless   */
                              /* another request is sent.               */
                              DeviceInfo->Flags &= ~(DEVICE_INFO_FLAGS_READING_NUMBER_OF_DIGITALS);

                              /* We will simply loop to find the first  */
                              /* Digital Characteristic included in the */
                              /* Aggregate whose Number Of Digitals     */
                              /* descriptor has not been read.          */
                              /* * NOTE * We will hardcode the Digital  */
                              /*          Characteristic type to access */
                              /*          its instances.                */
                              for(Index = 0; Index < DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances; Index++)
                              {
                                 /* Store a pointer to the Attribute    */
                                 /* Handle instance to aid in           */
                                 /* readability.                        */
                                 InstanceInfoPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances[Index]);

                                 /* Check if the Digital Characteristic */
                                 /* is included in the Aggregate        */
                                 /* Characteristic.                     */
                                 if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
                                 {
                                    /* Check to make sure the Number Of */
                                    /* Digitals for this descriptor has */
                                    /* been discovered.                 */
                                    if(InstanceInfoPtr->Number_Of_Digitals_Handle)
                                    {
                                       /* Check if we have read the     */
                                       /* Number Of Digitals already.   */
                                       /* Otherwise check the next      */
                                       /* Digital Characteristic.       */
                                       if(!InstanceInfoPtr->Number_Of_Digitals)
                                       {
                                          /* Flag that we are reading   */
                                          /* Number Of Digitals for a   */
                                          /* Digital Characteristics    */
                                          /* that is included in the    */
                                          /* Aggregate.                 */
                                          DeviceInfo->Flags |= DEVICE_INFO_FLAGS_READING_NUMBER_OF_DIGITALS;

                                          /* Store the attribute handle */
                                          /* for the request.           */
                                          AttributeHandle = InstanceInfoPtr->Number_Of_Digitals_Handle;

                                          if(AttributeHandle)
                                          {
                                             /* Set the client request  */
                                             /* information so we can   */
                                             /* handle the response when*/
                                             /* it is received.         */
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType = ahtNumberOfDigitals;
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.Type                = actDigital;
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.ID                  = Index;

                                             /* Send a GATT read request*/
                                             /* for the Number Of       */
                                             /* Digitals descriptor.    */
                                             /* Subsequent requests will*/
                                             /* be sent when the        */
                                             /* response has been       */
                                             /* received.               */
                                             if((Result = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                             {
                                                printf("\r\nGATT Read Value Request sent:\r\n");
                                                printf("   TransactionID:     %d\r\n", Result);
                                                printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                             }
                                             else
                                                DisplayFunctionError("GATT_Read_Value_Request", Result);
                                          }

                                          /* Terminate from the loop    */
                                          /* since we are done.         */
                                          break;
                                       }
                                    }
                                    else
                                       printf("\r\nError - The Number of Digitals handle is invalid.\r\n");
                                 }
                              }
                           }
                           break;
                        case ahtValidRange:
                           /* Display the descriptor type.              */
                           printf("\r\nValid Range:\r\n");

                           /* Simply call the AIOS Client API to decode */
                           /* the Valid Range.                          */
                           if((Result = AIOS_Decode_Valid_Range(ValueLength, Value, &ReadResponseData.ValidRange)) == 0)
                           {
                              /* Display the Valid Range descriptor.    */
                              /* * NOTE * This MUST be an Analog        */
                              /*          Charactersitic.               */
                              printf("   Lower:  0x%04X\r\n", ReadResponseData.ValidRange.LowerBound);
                              printf("   Upper:  0x%04X\r\n", ReadResponseData.ValidRange.UpperBound);
                           }
                           else
                              DisplayFunctionError("AIOS_Decode_Valid_Range",  Result);
                           break;
                        default:
                           printf("\r\nFailure - Invalid attribute handle type.\r\n");
                           break;
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
                     /* Process the request depending on the attribute  */
                     /* handle type we set before issuing the read      */
                     /* request.                                        */
                     switch(DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType)
                     {
                        case ahtUserDescription:
                           /* Make sure the length is valid for the     */
                           /* current buffer size.                      */
                           /* * NOTE * Add one to make sure there is    */
                           /*          room so we can insert the NULL   */
                           /*          terminator.                      */
                           CurrentOffset = DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset;
                           if((CurrentOffset + ValueLength + 1) <= ((Word_t)AIOP_USER_DESCRIPTION_LENGTH))
                           {
                              /* Simply copy the value into the User    */
                              /* Description at the current offset from */
                              /* the previous read request.             */
                              BTPS_MemCopy(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription + CurrentOffset, Value , ValueLength);

                              /* Make sure we don't go past the end of  */
                              /* the string.                            */
                              (DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription+CurrentOffset)[ValueLength] = '\0';

                              /* * NOTE * If the User Description is    */
                              /*          incomplete then a read long   */
                              /*          request should be performed to*/
                              /*          get the remaining data since  */
                              /*          the read response couldn't fit*/
                              /*          the entire User Description in*/
                              /*          the response.                 */
                              printf("\r\nUser Description:\r\n");
                              printf("   Value:   %s.\r\n", DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription);

                              /* Update the offset.                     */
                              DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset += ValueLength;
                           }
                           else
                              printf("\r\nError - ValueLength is greater than User Description buffer.\r\n");
                           break;
                        default:
                           printf("\r\nFailure - Invalid attribute handle type.\r\n");
                           break;
                     }
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

               /* * NOTE * We could verify the data that was sent in the*/
               /*          request since it should be sent back to us in*/
               /*          this response, but we will not do that here. */
               /*          If the data is valid we could also send the  */
               /*          execute write request to cancel the data we  */
               /*          prepared.                                    */

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Increment the User Description offset that we have */
                  /* written thus far.                                  */
                  DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset += ValueLength;
                  CurrentOffset = DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset;

                  /* Determine the size of the User Description we are  */
                  /* writing.                                           */
                  ExpectedLength = BTPS_StringLength(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription);

                  /* If the User Description offset reaches the expected*/
                  /* length then we have written all the data.          */
                  if(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset >= ExpectedLength)
                  {
                     /* If have sent the entire User Description.       */
                     if(DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescriptionOffset == ExpectedLength)
                     {
                        /* Inform the user the User description has been*/
                        /* prepared.                                    */
                        printf("\r\nUser Description has been prepared.\r\n");

                        /* Execute the write request.                   */
                        /* * NOTE * We will not save the TransactionID  */
                        /*          for this request.                   */
                        if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, FALSE, GATT_ClientEventCallback_AIOS, 0)) <= 0)
                           DisplayFunctionError("GATT_Execute_Write_Request", Result);
                     }
                     else
                     {
                        printf("\r\nFailure - User Description offset has exceeded the expected length.\r\n");

                        /* Execute the write request to cancel the      */
                        /* request.                                     */
                        /* * NOTE * We will not save the TransactionID  */
                        /*          for this request.                   */
                        if((Result = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, TRUE, GATT_ClientEventCallback_AIOS, 0)) <= 0)
                           DisplayFunctionError("GATT_Execute_Write_Request", Result);
                     }
                  }
                  else
                  {
                     /* Query the GATT Connection MTU to determine the  */
                     /* amount of data we can send in the prepare write.*/
                     if((Result = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                     {
                        /* Determine the length to send.                */
                        /* * NOTE * The expected length minus the offset*/
                        /*          is the length of User Description   */
                        /*          that we have sent thus far.  If that*/
                        /*          size is under the maximum size we   */
                        /*          can send for the prepare write      */
                        /*          (MTU-5), then we will send the      */
                        /*          remaining data, otherwise we will   */
                        /*          send the max.                       */
                        PreparedWriteLength = ((ExpectedLength-CurrentOffset) <= (MTU-5)) ? (ExpectedLength-CurrentOffset) : (MTU-5);

                        /* We need to send the next prepare write       */
                        /* request since we have not sent all the data. */
                        /* * NOTE * We will again try to send the       */
                        /*          maximimum amount of data based on   */
                        /*          MTU.                                */
                        /* * NOTE * We will not save the transactionID  */
                        /*          returned by this function, which we */
                        /*          could use to cancel the request.    */
                        if((Result = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, PreparedWriteLength, CurrentOffset, (Byte_t *)((DeviceInfo->AIOPClientInfo.Client_Request_Info.UserDescription + CurrentOffset)), GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                        {
                           printf("\r\nGATT Prepare Write Request sent:\r\n");
                           printf("   TransactionID:     %d\r\n", Result);
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
      printf("\r\nAIOS Callback Data: Event_Data = NULL.\r\n");
   }

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
   int                             Result;
   DeviceInfo_t                   *DeviceInfo;
   GAP_Encryption_Mode_t           EncryptionMode;

   unsigned int                    Index;
   Boolean_t                       FoundMatch = FALSE;
   AIOP_Client_Instance_Info_t    *ClientInstancePtr;

   /* GATT Connection Event data types.                                 */
   BoardStr_t                      BoardStr;
   BD_ADDR_t                       RemoteBDADDR;
   unsigned int                    GATTConnectionID;
   unsigned int                    TransactionID;
   GATT_Connection_Type_t          ConnectionType;
   Word_t                          MTU;
   Word_t                          AttributeHandle;
   Word_t                          ValueLength;
   Byte_t                         *Value;

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
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_AIOS, 0);
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

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Only send an indication response if this is a known*/
                  /* remote device.                                     */
                  if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, GATTConnectionID, TransactionID)) != 0)
                  {
                     DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);
                  }

                  /* Check if this is an indication of the Aggregate    */
                  /* Characteristic.                                    */
                  if(DeviceInfo->AIOPClientInfo.Aggregate_Handle == AttributeHandle)
                  {
                     /* Decode and display the Aggregate Characteristic.*/
                     DecodeDisplayAggregateCharacteristic(&(DeviceInfo->AIOPClientInfo), ValueLength, Value);
                  }
                  else
                  {
                     /* This MUST be a Digital/Analog Characteristic.   */
                     /* Unfortunately we CANNOT know before hand what   */
                     /* the notification/indication was for so we will  */
                     /* need to explicitly find the attribute handle    */
                     /* that was previously discovered.                 */
                     /* * NOTE * We cannot decode and display if service*/
                     /*          discovery has not been performed since */
                     /*          we cannot determine the type based on  */
                     /*          the attribute handle.                  */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
                     {
                        /* We will first loop through the Digital       */
                        /* Characteristic instances for a matching      */
                        /* attribute handle.                            */
                        for(Index = 0; (!FoundMatch) && (Index < DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances); Index++)
                        {
                           /* Store a pointer to the AIOS Client's      */
                           /* instance information.                     */
                           ClientInstancePtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances[Index]);

                           /* If we find a match.                       */
                           if(ClientInstancePtr->Digital_Characteristic_Handle == AttributeHandle)
                           {
                              /* Flag that we found a match.            */
                              FoundMatch = TRUE;

                              /* Decode and display the digital         */
                              /* characteristic.                        */
                              DecodeDisplayDigitalCharacteristic(ValueLength, Value, Index);
                              break;
                           }
                        }

                        /* Loop through the Analog Characteristic       */
                        /* instances for a matching attribute handle.   */
                        for(Index = 0; (!FoundMatch) && (Index < DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances); Index++)
                        {
                           /* Store a pointer to the AIOS Client's      */
                           /* instance information.                     */
                           ClientInstancePtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances[Index]);

                           /* If we find a match.                       */
                           if(ClientInstancePtr->Analog_Charactersitic_Handle == AttributeHandle)
                           {
                               /* Flag that we found a match.           */
                              FoundMatch = TRUE;

                              /* Decode and display the analog          */
                              /* characteristic.                        */
                              DecodeDisplayAnalogCharacteristic(ValueLength, Value, Index);
                              break;
                           }
                        }

                        if(FoundMatch == FALSE)
                        {
                           /* Display an error since could not find a   */
                           /* matching attribute handle.                */
                           printf("\r\nError - No Characteristic instance information found for the attribute handle.\r\n");
                        }
                     }
                     else
                        printf("\r\nError - Service discovery for AIOS has not been performed.\r\n");
                  }
               }
               else
               {
                  printf("\r\nError - Unknown remote device.\r\n");
               }
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

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteBDADDR)) != NULL)
               {
                  /* Check if this is an indication of the Aggregate    */
                  /* Characteristic.                                    */
                  if(DeviceInfo->AIOPClientInfo.Aggregate_Handle == AttributeHandle)
                  {
                     /* Decode and display the aggregate characteristic.*/
                     DecodeDisplayAggregateCharacteristic(&(DeviceInfo->AIOPClientInfo), ValueLength, Value);
                  }
                  else
                  {
                     /* This MUST be a Digital/Analog Characteristic.   */
                     /* Unfortunately we CANNOT know before hand what   */
                     /* the notification/indication was for so we will  */
                     /* need to explicitly find the attribute handle    */
                     /* that was previously discovered.                 */
                     /* * NOTE * We cannot decode and display if service*/
                     /*          discovery has not been performed since */
                     /*          we cannot determine the type based on  */
                     /*          the attribute handle.                  */
                     if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
                     {
                        /* We will first loop through the Digital       */
                        /* Characteristic instances for a matching      */
                        /* attribute handle.                            */
                        for(Index = 0; (!FoundMatch) && (Index < DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances); Index++)
                        {
                           /* Store a pointer to the AIOS Client's      */
                           /* instance information.                     */
                           ClientInstancePtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances[Index]);

                           /* If we find a match.                       */
                           if(ClientInstancePtr->Digital_Characteristic_Handle == AttributeHandle)
                           {
                              /* Flag that we found a match.            */
                              FoundMatch = TRUE;

                              /* Decode and display the digital         */
                              /* characteristic.                        */
                              DecodeDisplayDigitalCharacteristic(ValueLength, Value, Index);
                              break;
                           }
                        }

                        /* Loop through the Analog Characteristic       */
                        /* instances for a matching attribute handle.   */
                        for(Index = 0; (!FoundMatch) && (Index < DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances); Index++)
                        {
                           /* Store a pointer to the AIOS Client's      */
                           /* instance information.                     */
                           ClientInstancePtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances[Index]);

                           /* If we find a match.                       */
                           if(ClientInstancePtr->Analog_Charactersitic_Handle == AttributeHandle)
                           {
                               /* Flag that we found a match.           */
                              FoundMatch = TRUE;

                              /* Decode and display the analog          */
                              /* characteristic.                        */
                              DecodeDisplayAnalogCharacteristic(ValueLength, Value, Index);
                              break;
                           }
                        }

                        if(FoundMatch == FALSE)
                        {
                           /* Display an error since could not find a   */
                           /* matching attribute handle.                */
                           printf("\r\nError - No Characteristic instance information found for the attribute handle.\r\n");
                        }
                     }
                     else
                        printf("\r\nError - Service discovery for AIOS has not been performed.\r\n");
                  }
               }
               else
                  printf("\r\nError - Unknown remote device.\r\n");
            }
            else
               printf("\r\nError - Null Server Notification Data.\r\n");
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
      printf("\r\nGATT Connection Callback Data: Event_Data = NULL.\r\n");

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
   int                          Result;
   unsigned int                 Index;
   DeviceInfo_t                *DeviceInfo;
   GATT_Service_Information_t  *IncludedServiceInfo;
   AIOP_Client_Instance_Info_t *InstanceInfoPtr;
   Word_t                       AttributeHandle = 0;

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
                     case sdAIOS:
                        /* If service discovery has been completed      */
                        /* previously and we are re-discovering AIOS, we*/
                        /* will free the memory that has been allocated */
                        /* for all the Digital/Analog Characteristic    */
                        /* instance's information.  This way if the     */
                        /* service has changed since the last time we   */
                        /* performed service discovery we will populate */
                        /* the handles correctly.                       */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* If there were Digital Characteristic      */
                           /* instances discovered we need to free their*/
                           /* information that was allocated during     */
                           /* service discovery.                        */
                           if((DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances) && (DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances))
                           {
                              BTPS_FreeMemory(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances);
                              DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances           = NULL;
                              DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances = 0;
                           }

                           /* If there were Analog Characteristic       */
                           /* instances discovered we need to free their*/
                           /* information that was allocated during     */
                           /* service discovery.                        */
                           if((DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances) && (DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances))
                           {
                              BTPS_FreeMemory(DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances);
                              DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances           = NULL;
                              DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances = 0;
                           }
                        }

                        /* Attempt to populate the handles for the AIOS */
                        /* Service.                                     */
                        AIOSPopulateHandles(&(DeviceInfo->AIOPClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                     case sdAIOS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_AIOS_SERVICE_DISCOVERY_COMPLETE;

                        printf("\r\nAIOS Service Discovery Summary\r\n\r\n");

                        /* Print the Digital Characteristic instances   */
                        /* attribute handle information that was        */
                        /* discovered.                                  */
                        /* * NOTE * We will hardcode the Digital type to*/
                        /*          access the Digital Characteristic   */
                        /*          instances information.              */
                        for(Index = 0; Index < DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances; Index++)
                        {
                           /* Store a pointer to the Attribute Handle   */
                           /* instance to aid in readability.           */
                           InstanceInfoPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances[Index]);

                           printf("\r\nDigital Characteristic (ID = %u):\r\n", (Index));
                           printf("   IO Type:                 %s\r\n", (InstanceInfoPtr->IOType == ioInput) ? "ioInput" : "ioOutput");
                           printf("   Descriptors:\r\n");
                           printf("      Extended Properties:  %s\r\n", (InstanceInfoPtr->Extended_Properties_Handle   ? "Supported" : "Not Supported"));
                           printf("      CCCD:                 %s\r\n", (InstanceInfoPtr->CCCD_Handle                  ? "Supported" : "Not Supported"));
                           printf("      Presentation Format:  %s\r\n", (InstanceInfoPtr->Presentation_Format_Handle   ? "Supported" : "Not Supported"));
                           printf("      Value Trigger:        %s\r\n", (InstanceInfoPtr->Value_Trigger_Setting_Handle ? "Supported" : "Not Supported"));
                           printf("      Time Trigger:         %s\r\n", (InstanceInfoPtr->Time_Trigger_Setting_Handle  ? "Supported" : "Not Supported"));
                           printf("      User Description:     %s\r\n", (InstanceInfoPtr->User_Description_Handle      ? "Supported" : "Not Supported"));
                           printf("      Number of Digitals:   %s\r\n", (InstanceInfoPtr->Number_Of_Digitals_Handle    ? "Supported" : "Not Supported"));
                        }

                        /* Print the Ananlog Characteristic instances   */
                        /* attribute handle information that was        */
                        /* discovered.                                  */
                        /* * NOTE * We will hardcode the Analog type to */
                        /*          access the Analog Characteristic    */
                        /*          instances information.              */
                        for(Index = 0; Index < DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Number_Of_Instances; Index++)
                        {
                           /* Store a pointer to the Attribute Handle   */
                           /* instance to aid in readability.           */
                           InstanceInfoPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actAnalog].Instances[Index]);

                           printf("\r\nAnalog Characteristic (ID = %u):\r\n", (Index));
                           printf("   IO Type:                 %s\r\n", (InstanceInfoPtr->IOType == ioInput) ? "ioInput" : "ioOutput");
                           printf("   Descriptors:\r\n");
                           printf("      Extended Properties:  %s\r\n", (InstanceInfoPtr->Extended_Properties_Handle   ? "Supported" : "Not Supported"));
                           printf("      CCCD:                 %s\r\n", (InstanceInfoPtr->CCCD_Handle                  ? "Supported" : "Not Supported"));
                           printf("      Presentation Format:  %s\r\n", (InstanceInfoPtr->Presentation_Format_Handle   ? "Supported" : "Not Supported"));
                           printf("      Value Trigger:        %s\r\n", (InstanceInfoPtr->Value_Trigger_Setting_Handle ? "Supported" : "Not Supported"));
                           printf("      Time Trigger:         %s\r\n", (InstanceInfoPtr->Time_Trigger_Setting_Handle  ? "Supported" : "Not Supported"));
                           printf("      User Description:     %s\r\n", (InstanceInfoPtr->User_Description_Handle      ? "Supported" : "Not Supported"));
                           printf("      Valid Range:          %s\r\n", (InstanceInfoPtr->Valid_Range_Handle           ? "Supported" : "Not Supported"));
                        }

                        /* Print the Aggregate Characteristic.          */
                        printf("\r\nAggregate Characteristic:  %s\r\n", (DeviceInfo->AIOPClientInfo.Aggregate_Handle      ? "Supported" : "Not Supported"));
                        printf("   Descriptors:\r\n");
                        printf("      CCCD:                %s\r\n", (DeviceInfo->AIOPClientInfo.Aggregate_CCCD_Handle ? "Supported" : "Not Supported"));

                        /* If the Aggregate Characteristic was          */
                        /* discovered we will issue read requests for   */
                        /* the Number Of Digitals descriptor for each   */
                        /* Digital Characteristic included in the       */
                        /* Aggregate Characteristic.                    */
                        /* *** NOTE *** THIS MUST BE DONE OR DECODING   */
                        /*            THE AGGREGATE CHARACTERISTIC WILL */
                        /*            FAIL SINCE DIGITAL CHARACTERISTICS*/
                        /*            ARE VARIABLE LENGTH AND DETERMINED*/
                        /*            BY THE NUMBER OF DIGITALS         */
                        /*            DESCRIPTOR.                       */
                        if(DeviceInfo->AIOPClientInfo.Aggregate_Handle)
                        {
                           /* Make sure there are Digital               */
                           /* Characteristics in the Aggregate.         */
                           if(DeviceInfo->AIOPClientInfo.Num_Digital_Characteristics_In_Aggregate)
                           {
                              /* We will simply loop to find the first  */
                              /* Digital Characteristic included in the */
                              /* Aggregate whose Number Of Digitals     */
                              /* descriptor has not been read.          */
                              /* * NOTE * We will hardcode the Digital  */
                              /*          Characteristic type to access */
                              /*          its instances.                */
                              for(Index = 0; Index < DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Number_Of_Instances; Index++)
                              {
                                 /* Store a pointer to the Attribute    */
                                 /* Handle instance to aid in           */
                                 /* readability.                        */
                                 InstanceInfoPtr = &(DeviceInfo->AIOPClientInfo.Characteristics[actDigital].Instances[Index]);

                                 /* Check if the Digital Characteristic */
                                 /* is included in the Aggregate        */
                                 /* Characteristic.                     */
                                 if(InstanceInfoPtr->Properties & GATT_CHARACTERISTIC_PROPERTIES_READ)
                                 {
                                    /* Check to make sure the Number Of */
                                    /* Digitals for this descriptor has */
                                    /* been discovered.                 */
                                    if(InstanceInfoPtr->Number_Of_Digitals_Handle)
                                    {
                                       /* Check if we have read the     */
                                       /* Number Of Digitals already.   */
                                       /* Otherwise check the next      */
                                       /* Digital Characteristic.       */
                                       if(!InstanceInfoPtr->Number_Of_Digitals)
                                       {
                                          /* Store the attribute handle */
                                          /* for the request.           */
                                          AttributeHandle = InstanceInfoPtr->Number_Of_Digitals_Handle;

                                          if(AttributeHandle)
                                          {
                                             /* Flag that we are reading*/
                                             /* Number Of Digitals for a*/
                                             /* Digital Characteristics */
                                             /* that is included in the */
                                             /* Aggregate.              */
                                             DeviceInfo->Flags |= DEVICE_INFO_FLAGS_READING_NUMBER_OF_DIGITALS;

                                             /* Set the client request  */
                                             /* information so we can   */
                                             /* handle the response when*/
                                             /* it is received.         */
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.AttributeHandleType = ahtNumberOfDigitals;
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.Type                = actDigital;
                                             DeviceInfo->AIOPClientInfo.Client_Request_Info.ID                  = Index;

                                             /* Send a GATT read request*/
                                             /* for the Number Of       */
                                             /* Digitals descriptor.    */
                                             /* Subsequent requests will*/
                                             /* be sent when the        */
                                             /* response has been       */
                                             /* received.               */
                                             if((Result = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_AIOS, AttributeHandle)) > 0)
                                             {
                                                printf("\r\nGATT Read Value Request sent:\r\n");
                                                printf("   TransactionID:     %d\r\n", Result);
                                                printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                                             }
                                             else
                                                DisplayFunctionError("GATT_Read_Value_Request", Result);
                                          }

                                          /* Terminate from the loop    */
                                          /* since we are done.         */
                                          break;
                                       }
                                    }
                                    else
                                       printf("\r\nError - The Number of Digitals handle is invalid.\r\n");
                                 }
                              }
                           }
                        }
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
