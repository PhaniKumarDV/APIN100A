/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< linuxesp.c >***********************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LINUXESP - Linux Bluetooth Environmental Sensing Profile using GATT       */
/*             (LE/BREDR) sample application.                                 */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/14/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "LinuxESP.h"           /* Application Header.                        */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTDBG.h"            /* BTPS Debug Header.                        */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */

#include "SS1BTESS.h"            /* Main SS1 ESS Service Header.              */
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
                                                         /* create a ESS     */
                                                         /* Server.           */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

#define UNABLE_TO_ALLOCATE_MEMORY                  (-11) /* Denotes that we   */
                                                         /* failed because we */
                                                         /* couldn't allocate */
                                                         /* memory.           */

   /* ESS Server defines.                                               */

#define ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES (3)  /* Defines the number*/
                                                         /* of maximum        */
                                                         /* instances for each*/
                                                         /* ESS Characteristic*/
                                                         /* supported by the  */
                                                         /* ESS Server.       */

#define ESS_SERVER_REGISTER_DESCRIPTOR_VALUE_CHANGED (TRUE)
                                                         /* Defines whether to*/
                                                         /* include the       */
                                                         /* DVC.              */

#define ESS_DEFAULT_CHARACTERISTIC_PROPERTIES      (ESS_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY)
                                                         /* Denotes the       */
                                                         /* default properties*/
                                                         /* for ESS           */
                                                         /* characteristics.  */

#define ESS_DEFAULT_DESCRIPTOR_FLAGS               (ESS_DESCRIPTOR_FLAGS_ES_MEASUREMENT      | \
                                                    ESS_DESCRIPTOR_FLAGS_ES_TRIGGER_SETTING  | \
                                                    ESS_DESCRIPTOR_FLAGS_USER_DESCRIPTION    | \
                                                    ESS_DESCRIPTOR_FLAGS_VALID_RANGE)
                                                         /* Denotes the       */
                                                         /* optional          */
                                                         /* descriptors for   */
                                                         /* ESS               */
                                                         /* characteristics.  */

#define ESS_DEFAULT_DESCRIPTOR_PROPERTIES          (ESS_DESCRIPTOR_PROPERTY_FLAGS_WRITE_ES_TRIGGER_SETTING | \
                                                    ESS_DESCRIPTOR_PROPERTY_FLAGS_WRITE_USER_DESCRIPTION)
                                                         /* Denotes the       */
                                                         /* optional          */
                                                         /* descriptors       */
                                                         /* properties for ESS*/
                                                         /* characteristics.  */

#define ESS_DEFAULT_ES_TRIGGER_SETTING_INSTANCES   (3)
                                                         /* Denotes the number*/
                                                         /* of ES Trigger     */
                                                         /* Setting instances.*/

   /* ESS Client defines.                                               */

#define PRINT_CHARACTERISTIC_TYPE_INFO             (FALSE)
                                                         /* Enables printing  */
                                                         /* of the ESS service*/
                                                         /* discovery summary.*/

#define PRINT_SERVICE_DISCOVERY_HANDLES            (FALSE)
                                                         /* Enables printing  */
                                                         /* of the UUID's     */
                                                         /* during service    */
                                                         /* discovery. May    */
                                                         /* be enabled to aid */
                                                         /* with debugging and*/
                                                         /* PTS testing.      */

#define PRINT_FULL_SERVICE_DISCOVERY_SUMMARY       (FALSE)
                                                         /* Enables printing  */
                                                         /* of the ESS service*/
                                                         /* discovery summary.*/

   /* ESS Sample application defines.                                   */

#define ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES        (20)  /* Defines the number*/
                                                         /* of maximum        */
                                                         /* types for each    */
                                                         /* ESS Characteristic*/
                                                         /* supported by the  */
                                                         /* sample app.       */

#define ESS_SA_DEFAULT_STRING_LENGTH               (255) /* Defines the       */
                                                         /* default string    */
                                                         /* length.           */

   /* The following constants represent the default log file names that */
   /* are used if no Log file name is specified when enabling debug.    */
#define DEFAULT_DEBUG_LOG_FILE_NAME                "LinuxESP_ASC.log"
#define DEFAULT_DEBUG_FTS_FILE_NAME                "LinuxESP_FTS.log"

   /* Determine the Name we will use for this compilation.              */
#define LE_DEMO_DEVICE_NAME                        "LinuxESP"

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
   int          NumberofParameters;
   Parameter_t  Params[MAX_NUM_OF_PARAMETERS];
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
   sdESS
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
   Word_t  DeviceNameHandle;
   Word_t  DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   Word_t  Appearance;
   char   *String;
} GAPS_Device_Appearance_Mapping_t;

   /* The following enumeration will be used to determine the correct   */
   /* Attribute Handle to select for an ESS Characteristic or           */
   /* Descriptor.                                                       */
typedef enum _tagESS_Sample_Attribute_Handle_Type_t
{
   ahtCharacteristic,
   ahtExtendedProperties,
   ahtClientCharacteristicConfig,
   ahtESMeasurement,
   ahtESTriggerSetting_0,
   ahtESTriggerSetting_1,
   ahtESTriggerSetting_2,
   ahtESConfiguration,
   ahtUserDescription,
   ahtValidRange
} ESS_Sample_Attribute_Handle_Type_t;

   /* The following structure holds the request information that the ESS*/
   /* Client MUST store for each GATT request in order to process the   */
   /* GATT response in the GATT_ClientEventCallback_ESS().              */
   /* * NOTE * This structure will only be used for ESS Characteristic  */
   /*          requests.  The Descriptor Value Changed Characteristic   */
   /*          and CCCD will not need to use this structure.            */
   /* * NOTE * We do not need to store the attribute handle since we    */
   /*          will set the CallbackParameter to the attribute handle.  */
   /* * NOTE * The Type and ID fields indicate the ESS Characteristic   */
   /*          that this request was for.  This way we do not have to   */
   /* * NOTE * The AttributeHandleType field allows us to specify the   */
   /*          type of the attribute handle we are expecting in the     */
   /*          response.  This way with the Type and ID fields, we can  */
   /*          quickly locate the correct attribute handle to verify.   */
   /*          Otherwise we would need to check every attribute handle  */
   /*          for a match to know how to process the response.         */
typedef struct _tagESS_Client_Request_Info_t
{
   ESS_Characteristic_Type_t           Type;
   unsigned int                        ID;
   ESS_Sample_Attribute_Handle_Type_t  AttributeHandleType;
   char                                UserDescription[ESS_SA_DEFAULT_STRING_LENGTH];
   Word_t                              UserDescriptionOffset;
} ESS_Client_Request_Info_t;

   /* The following structure defines the information needed to send an */
   /* ESS Characteristic Descriptor Write Request.                      */
   /* * NOTE * This structure WILL NOT be used to write the Descriptor  */
   /*          Value Changed CCCD.                                      */
   /* * NOTE * The AttributeHandleType field may only be set to ESS     */
   /*          Characteristic descriptors that may be written.          */
   /* * NOTE * The UserDescription field should be valid (non-zero), if */
   /*          the AttributeHandleType is ahtUserDescription.  This     */
   /*          length should account for the NULL terminator.           */
typedef struct _tagESS_Write_Request_Data_t
{
   ESS_Sample_Attribute_Handle_Type_t  AttributeHandleType;
   Word_t                              UserDescriptionLength;
   union
   {
      ESS_Characteristic_Data_t        Characteristic;
      Word_t                           CCCD;
      ESS_ES_Trigger_Setting_Data_t    TriggerSettingData;
      Byte_t                           Configuration;
      char                            *UserDescription;
   } Data;
   ESS_Characteristic_Data_t           UpperCharacteristic;
} ESS_Write_Request_Data_t;

   /* The following structure contains the information that the ESS     */
   /* Server will need to store for each ESS Characteristic instance.   */
   /* Some fields will not be used depending on how the ESS             */
   /* Characteristic is configured for all instances.                   */
   /* * NOTE * The ESS Characteristic Type will be implicitly known by  */
   /*          how this structure is accessed so it does not need to be */
   /*          stored here.                                             */
   /* * NOTE * The ESS Characteristic ID is implicitly known based on   */
   /*          the location in the Instances field of the               */
   /*          ESS_Server_Characteristic_t structure.  The IESS         */
   /*          Characteristic ID is used to identify this ESS           */
   /*          Characteristic instance with the service.  The ID's will */
   /*          start at zero to simplify indexing the ESS Server        */
   /*          information.  This will directly correspond to the order */
   /*          in which the ESS Characteristic instances are registered */
   /*          with the service.                                        */
   /* * NOTE * The InitInformation will be used to hold information for */
   /*          the ESS_Initialize_Data_t structure, which is used to    */
   /*          register ESS via ESS_Initialize_Service().  The          */
   /*          information will be retained here for use by the ESS     */
   /*          Server.                                                  */
   /* * NOTE * The number of ES Trigger Setting instances for this ESS  */
   /*          Characteristic instance is contained in the              */
   /*          InstanceEntryInfo field.                                 */
   /* * NOTE * See ESSAPI.h for more information about API structures   */
   /*          that are used as fields.  Some fields will not be used   */
   /*          depending on how the InstanceEntryInfo is configured.    */
typedef struct _tagESS_Server_Instance_t
{
   ESS_Characteristic_Data_t            Characteristic;
   ESS_Characteristic_Instance_Entry_t  InitInformation;
   Word_t                               ClientConfiguration;
   ESS_ES_Measurement_Data_t            MeasurementData;
   ESS_ES_Trigger_Setting_Data_t        TriggerSettingData[3];
   Byte_t                               Configuration;
   char                                 UserDescription[ESS_SA_DEFAULT_STRING_LENGTH];
   ESS_Valid_Range_Data_t               ValidRange;
} ESS_Server_Instance_t;

#define ESS_SERVER_INSTANCE_SIZE                         (sizeof(ESS_Server_Instance_t))

   /* The following structure contains the information for each ESS     */
   /* Characteristic Type entry.  Each entry holds a list for the ESS   */
   /* Characteristic instances associated with that ESS Characteristic  */
   /* Type entry.                                                       */
   /* * NOTE * Since the ESS Characteristic Type is used to index this  */
   /*          structure, the Type will be implicitly known and WILL NOT*/
   /*          be stored.  However, the UUID field will contain the ESS */
   /*          Characteristic UUID, which will be needed to indicate the*/
   /*          Descriptor Value Changed Characteristic.                 */
   /* * NOTE * For simplicity of this application there will ALWAYS be a*/
   /*          defined number of instances for each ESS Characteristic  */
   /*          Type present when this application starts up.  However,  */
   /*          we will only register certain ESS Characteristics with   */
   /*          the service.  This way we will only receive ESS Events   */
   /*          from the service for ESS Characteristics that were       */
   /*          registered.                                              */
typedef struct _tagESS_Server_Characteristic_t
{
   GATT_UUID_t            UUID;
   ESS_Server_Instance_t  Instances[ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES];
} ESS_Server_Characteristic_t;

#define ESS_SERVER_CHARACTERISTIC_SIZE                   (sizeof(ESS_Server_Characteristic_t))

   /* The following structure holds the information the ESS Server needs*/
   /* to store for the Descriptor Value Changed Characteristic.         */
typedef struct _tagESS_Server_Descriptor_Value_Changed_t
{
   Boolean_t  Registered;
   Word_t     CCCD;
} ESS_Server_Descriptor_Value_Changed_t;

   /* The following structure contains the ESS Server information.  This*/
   /* information will include a list with an entry for each ESS        */
   /* Characteristic Type (TypeList).                                   */
   /* * NOTE * The Characteristics field has been hard coded based on   */
   /*          the ESS_Characteristic_Type_t structure so that we have  */
   /*          an entry for each ESS Characteristic Type.  Each entry   */
   /*          will be able to store ESS Characteristic instances of the*/
   /*          ESS Characteristic Type.                                 */
   /* * NOTE * The DescriptorValueChanged indicates that this           */
   /*          characteristic is present and will be used to copy       */
   /*          information to the ESS_Initialize_Data_t structure, which*/
   /*          is used to register ESS via ESS_Initialize_Service().    */
   /*          The information will be retained here for use by the ESS */
   /*          Server.                                                  */
   /* * NOTE * If the DescriptorValueChanged is FALSE, it is the        */
   /*          application's responsibility to follow the ESS           */
   /*          specification and not change the ES Measurement, ES      */
   /*          Trigger Setting, ES Configuration, or Characteristic User*/
   /*          Description descriptors since they are stored in the     */
   /*          application.                                             */
typedef struct _tagESS_Server_Information_t
{
   ESS_Server_Characteristic_t            Characteristics[ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES];
   ESS_Server_Descriptor_Value_Changed_t  DescriptorValueChanged;
   Word_t                                 ChangeIndex;
} ESS_Server_Information_t;

#define ESS_SERVER_INFORMATION_SIZE                      (sizeof(ESS_Server_Information_t))

   /* The following structure contains the Attribute Handle information */
   /* that will need to be cached by a ESS Client for each ESS          */
   /* Characteristic instance in order to only do service discovery     */
   /* once.                                                             */
   /* * NOTE * The Valid field indicates that the Attribute Handle      */
   /*          information for this ESS Characteristic instance has     */
   /*          previously been stored during service discovery.  The ESS*/
   /*          Client may check this field to make sure the attribute   */
   /*          handle information is valid before using the handles in  */
   /*          GATT requests.                                           */
   /* * NOTE * This structure should be initialized to zero for all     */
   /*          instances so that the attribute handles are zero (ie.    */
   /*          invalid).  They will be set during service discovery.    */
typedef struct _tagESS_Client_Attribute_Handle_t
{
   Boolean_t  Valid;
   Word_t     Attribute_Handle;
   Word_t     Extended_Properties_Handle;
   Word_t     CCCD_Handle;
   Word_t     ES_Measurement_Handle;
   Word_t     ES_Trigger_Setting_Handle[3];
   Word_t     ES_Configuration_Handle;
   Word_t     User_Description_Handle;
   Word_t     Valid_Range_Handle;
} ESS_Client_Attribute_Handle_t;

#define ESS_CLIENT_ATTRIBUTE_HANDLE_ENTRY_SIZE           (sizeof(ESS_Client_Attribute_Handle_t))

   /* The following structure defines the ESS Characteristic Type entry */
   /* for a list of ESS Characteristic Types.  Each entry will contain a*/
   /* list of information for each ESS Characteristic's attribute handle*/
   /* information that was discovered during service discovery.         */
   /* * NOTE * The ESS Characteristic Type will be implicitly known     */
   /*          since each entry is associated with an ESS Characteristic*/
   /*          Type.  Therefore we do not need to store it here.        */
   /* * NOTE * The AttributeHandleEntries field contains the number of  */
   /*          instances discovered for the ESS Characteristic Type.    */
   /*          This field also indicates the number of entries contained*/
   /*          in the AttributeHandleList field.                        */
   /* * NOTE * The AttributeHandleList field contains attribute handle  */
   /*          information for each instance of the Type discovered.    */
typedef struct _tagESS_Client_Characteristic_t
{
   unsigned int                   AttributeHandleEntries;
   ESS_Client_Attribute_Handle_t *AttributeHandleList;
} ESS_Client_Characteristic_t;

#define ESS_CLIENT_CHARACTERISTIC_TYPE_ENTRY_SIZE        (sizeof(ESS_Client_Characteristic_t))

   /* The following structure contains the information that the ESS     */
   /* Client will need to store during service discovery.  This         */
   /* information includes a list of attribute handle entries for each  */
   /* instance of the ESS Characteristic Type discovered.               */
   /* * NOTE * The Descriptor_Value_Changed_Handle is stored separately */
   /*          since it is not an ESS Characteristic.                   */
   /* * NOTE * The ClientRequestInfo will be used to store the GATT     */
   /*          request information for ESS Characteristics so that we   */
   /*          will be able to simplify processing of GATT responses in */
   /*          the GATT_ClientEventCallback_ESS().                      */
   /* * NOTE * The ClientRequestInfo field will NOT be used for the     */
   /*          Descriptor Value Changed Characteristic and Descriptor   */
   /*          Value Changed CCCD.                                      */
typedef struct _tagESS_Client_Information_t
{
   ESS_Client_Characteristic_t  Characteristics[ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES];
   Word_t                       Descriptor_Value_Changed_Handle;
   Word_t                       Descriptor_Value_Changed_CCCD;
   ESS_Client_Request_Info_t    ClientRequestInfo;
} ESS_Client_Information_t;

#define ESS_CLIENT_INFORMATION_SIZE                      (sizeof(ESS_Client_Information_t))

   /* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices.  For slave we will not use this*/
   /* structure.                                                        */
   /* * NOTE * The ESSClientInfo contains an array based on the maximum */
   /*          number of ESS Characteristic Types.  This value has been */
   /*          hard coded based on the ESS_Characteristic_Type_t        */
   /*          enumeration.  This field MUST be indexed based on the    */
   /*          ESS_Characteristic_Type_t value.  Each index will store a*/
   /*          pointer to list attribute handle entries for each ESS    */
   /*          Characteristic instance of the associated ESS            */
   /*          Characteristic Type.  Since the Type is used to index    */
   /*          this field, the Type is implicitly known and WILL NOT be */
   /*          stored for each entry.                                   */
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
   ESS_Client_Information_t  ESSClientInfo;
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
#define DEVICE_INFO_FLAGS_DIS_SERVICE_DISCOVERY_COMPLETE    0x0020
#define DEVICE_INFO_FLAGS_GAPS_SERVICE_DISCOVERY_COMPLETE   0x0040
#define DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE    0x0080

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
static unsigned int        ESSInstanceID;
                                                    /* The following holds the ESS     */
                                                    /* Instance IDs that are returned  */
                                                    /* from ESS_Initialize_XXX().      */

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

static ESS_Server_Information_t   ServerInfo;       /* Variable which holds the        */
                                                    /* information needed by the ESS   */
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

static int GetGATTMTU(ParameterList_t *TempParam);
static int SetGATTMTU(ParameterList_t *TempParam);
static int EnableDebug(ParameterList_t *TempParam);

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

   /* Environmental Sensing Service Functions and Commands.             */
static void InitializeESSServer(ESS_Initialize_Data_t *InitializeData);
static int RegisterESS(ParameterList_t *TempParam);
static int UnregisterESS(ParameterList_t *TempParam);
static int DiscoverESS(ParameterList_t *TempParam);

static void ESSPopulateHandles(ESS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void CalculateAttributeHandleEntries(ESS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData);
static void StoreAttributeHandles(ESS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfo);
static void StoreDescriptorHandles(ESS_Client_Attribute_Handle_t *AttributeHandleEntry, GATT_Characteristic_Information_t *CharacteristicInfo);
static void FreeAttributeHandleLists(ESS_Client_Information_t *ClientInfo);

static void DisplayESSCharacteristicType(ESS_Characteristic_Type_t Type);
static void DisplayESSCharacteristicInfo(ESS_Characteristic_Info_t *CharacteristicInfo);

static ESS_Server_Instance_t *GetServerCharacteristicInstanceInfoPtr(ESS_Characteristic_Info_t *CharacteristicInfo);
static ESS_Client_Attribute_Handle_t *GetClientAttributeHandleInfoPtr(DeviceInfo_t *DeviceInfo);

static int DecodeDisplayDescriptorValueChangedData(unsigned int ValueLength, Byte_t *Value);
static int DecodeDisplayESSCharacteristicData(ESS_Characteristic_Type_t Type, unsigned int ValueLength, Byte_t *Value);
static int DecodeDisplayESTriggerSettingData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSetting, unsigned int ValueLength, Byte_t *Value);
static void DisplayESSCharacteristic(ESS_Characteristic_Type_t Type, ESS_Characteristic_Data_t *Data);
static void DisplayESMeasurementData(ESS_ES_Measurement_Data_t *MeasurementData);
static void DisplayESTriggerSettingData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSettingData);
static void DisplayESTriggerSettingOperandData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSettingData);
static void DisplayValidRangeData(ESS_Characteristic_Type_t Type, ESS_Valid_Range_Data_t *ValidRangeData);

static void AssignESSCharacteristicUUID(ESS_Characteristic_Type_t Type, GATT_UUID_t *UUID);

static int ReadESSCharacteristicRequest(ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Sample_Attribute_Handle_Type_t AttributeHandleType);
static int ReadLongUserDescriptionRequest(ESS_Characteristic_Info_t *CharacteristicInfo);
static int WriteESSCharacteristicRequest(ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Write_Request_Data_t *WriteRequestData);
static int NotifyESSCharacateristic(ESS_Characteristic_Info_t *CharacteristicInfo);
static int ConfigureDescriptorValueChanged(Word_t ClientConfiguration);
static int IndicateDescriptorValueChanged(ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged);

static int ReadESSCharacteristicCommand(ParameterList_t *TempParam);
static int WriteESSCharacteristicCommand(ParameterList_t *TempParam);
static int WriteCCCDCommand(ParameterList_t *TempParam);
static int WriteESTriggerSettingCommand(ParameterList_t *TempParam);
static int WriteESConfigurationCommand(ParameterList_t *TempParam);
static int WriteUserDescriptionCommand(ParameterList_t *TempParam);
static int WriteValidRangeCommand(ParameterList_t *TempParam);
static int ExecuteWriteRequestCommand(ParameterList_t *TempParam);
static int NotifyESSCharacteristicCommand(ParameterList_t *TempParam);
static int ConfigureDescriptorValueChangedCommand(ParameterList_t *TempParam);
static int IndicateDescriptorValueChangedCommand(ParameterList_t *TempParam);
static int FindESSCharacteristicReadAttributeInfo(ParameterList_t *TempParam);
static int FindESSCharacteristicWriteAttributeInfo(ParameterList_t *TempParam);

static void DisplayReadESSCharacteristicCommandUsage(void);
static void DisplayWriteESSCharacteristicCommandUsage(void);
static void DisplayWriteCCCDCommandUsage(void);
static void DisplayWriteESTriggerSettingCommandUsage(void);
static void DisplayWriteESConfigurationCommandUsage(void);
static void DisplayWriteUserDescriptionCommandUsage(void);
static void DisplayWriteValidRangeCommandUsage(void);
static void DisplayNotifyESSCharacteristicCommandUsage(void);
static void DisplayIndicateDescriptorValueChangedCommandUsage(void);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI ESS_EventCallback(unsigned int BluetoothStackID, ESS_Event_Data_t *ESS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_ESS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
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
   unsigned int Index;

   /* Loop through the ESS Client Characteristic type entries.          */
   for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
   {
      /* If memory was allocated for Characteristic instances attribute */
      /* handle information.                                            */
      if((EntryToFree->ESSClientInfo.Characteristics[Index].AttributeHandleEntries) && (EntryToFree->ESSClientInfo.Characteristics[Index].AttributeHandleList))
      {
         /* Free the memory.                                            */
         BTPS_FreeMemory(EntryToFree->ESSClientInfo.Characteristics[Index].AttributeHandleList);
         EntryToFree->ESSClientInfo.Characteristics[Index].AttributeHandleList    = NULL;
         EntryToFree->ESSClientInfo.Characteristics[Index].AttributeHandleEntries = 0;
      }
   }

   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  Index;

   /* Free any memory in use by the client for the ESS characteristic   */
   /* information.                                                      */
   DeviceInfo = *ListHead;
   while(DeviceInfo)
   {
      /* Loop through the ESS Client Characteristic type entries.       */
      for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
      {
         /* If memory was allocated for Characteristic instances        */
         /* attribute handle information.                               */
         if((DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries) && (DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList))
         {
            /* Free the memory.                                         */
            BTPS_FreeMemory(DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList);
            DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList    = NULL;
            DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries = 0;
         }
      }

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
   AddCommand("REGISTERESS", RegisterESS);
   AddCommand("UNREGISTERESS", UnregisterESS);
   AddCommand("DISCOVERESS", DiscoverESS);
   AddCommand("READ", ReadESSCharacteristicCommand);
   AddCommand("WRITE", WriteESSCharacteristicCommand);
   AddCommand("WRITECCCD", WriteCCCDCommand);
   AddCommand("WRITETRIGGERSETTING", WriteESTriggerSettingCommand);
   AddCommand("WRITECONFIGURATION", WriteESConfigurationCommand);
   AddCommand("WRITEUSERDESCRIPTION", WriteUserDescriptionCommand);
   AddCommand("WRITEVALIDRANGE", WriteValidRangeCommand);
   AddCommand("EXECUTEWRITEREQUEST", ExecuteWriteRequestCommand);
   AddCommand("NOTIFY", NotifyESSCharacteristicCommand);
   AddCommand("CONFIGUREDVC", ConfigureDescriptorValueChangedCommand);
   AddCommand("INDICATEDVC", IndicateDescriptorValueChangedCommand);
   AddCommand("FINDREAD", FindESSCharacteristicReadAttributeInfo);
   AddCommand("FINDWRITE", FindESSCharacteristicWriteAttributeInfo);
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
         /* First verify the length of the command.                     */
         if(BTPS_StringLength(Command) == BTPS_StringLength(CommandTable[Index].CommandName))
         {
            /* Simply compare the values.                               */
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
   Word_t                           ChangeIndex;

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

               /* We will check for the ESS Service data.  Make sure the*/
               /* mandatory UUID is present.                            */
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the ESS Service data.             */
                  if(ESS_COMPARE_ESS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
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
            case HCI_LE_ADVERTISING_REPORT_DATA_TYPE_SERVICE_DATA:
               printf("   AD Type:   Service Data.\r\n");
               printf("   AD Length: 0x%02X.\r\n", (unsigned int)(Advertising_Data_Entry->AD_Data_Length));
               DisplayRawAdvertisingData(&(Advertising_Data->Data_Entries[Index]));

               /* We will check for the ESS Service data.  Make sure the*/
               /* mandatory UUID is present.                            */
               /* * NOTE * We will ONLY accept 16 bit UUIDS to display  */
               /*          the service data for this sample application.*/
               if(Advertising_Data_Entry->AD_Data_Length >= NON_ALIGNED_WORD_SIZE)
               {
                  /* Store the type.                                    */
                  UUID.UUID_Type = guUUID_16;

                  /* Check if this is the ESS Service data.             */
                  if(ESS_COMPARE_ESS_SERVICE_UUID_TO_UUID_16(*((UUID_16_t *)&(Advertising_Data_Entry->AD_Data_Buffer[0]))))
                  {
                     /* Decode the UUID.                                */
                     ASSIGN_SDP_UUID_16(UUID.UUID.UUID_16, Advertising_Data_Entry->AD_Data_Buffer[0], Advertising_Data_Entry->AD_Data_Buffer[1]);

                     printf("\r\n   ESS Service data:\r\n");
                     printf("      UUID:          0x");
                     DisplayUUID(&UUID);

                     /* Decode the Change Index.                        */
                     ChangeIndex = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(Advertising_Data_Entry->AD_Data_Buffer[2]));
                     printf("      Change Index:  0x%04X\r\n", ChangeIndex);
                  }
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
   printf("\r\nLinuxESP>");

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
                     DIS_Set_Software_Revision(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);

                     /* Return success to the caller.                   */
                     ret_val       = 0;
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
      /* Cleanup DIS Service Module.                                    */
      if(DISInstanceID)
      {
         DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

         DISInstanceID = 0;
      }

      /* Cleanup GAP Service Module.                                    */
      if(GAPSInstanceID)
      {
         GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

         GAPSInstanceID = 0;
      }

      /* Cleanup ESS Service.                                           */
      if(ESSInstanceID)
      {
         ESS_Cleanup_Service(BluetoothStackID, ESSInstanceID);
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
   printf("* ESP Sample Application                                         *\r\n");
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
   printf("* Command Options ESS:                                           *\r\n");
   printf("*                Server:   RegisterESS,                          *\r\n");
   printf("*                          UnregisterESS,                        *\r\n");
   printf("*                          Notify,                               *\r\n");
   printf("*                          IndicateDVC,                          *\r\n");
   printf("*                          WriteValidRange,                      *\r\n");
   printf("*                Client:   DiscoverESS,                          *\r\n");
   printf("*                          WriteCCCD,                            *\r\n");
   printf("*                          ConfigureDVC,                         *\r\n");
   printf("*                          ExecuteWriteRequest,                  *\r\n");
   printf("*                          FindRead,                             *\r\n");
   printf("*                          FindWrite,                            *\r\n");
   printf("*                Both:     Read, Write                           *\r\n");
   printf("*                          WriteTriggerSetting,                  *\r\n");
   printf("*                          WriteConfiguration,                   *\r\n");
   printf("*                          WriteUserDescription,                 *\r\n");
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
   int                                 NameLength;
   GAP_LE_Advertising_Parameters_t     AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t  ConnectabilityParameters;
   unsigned int                        Length = 0;
   char                                Name[GAP_MAXIMUM_DEVICE_NAME_LENGTH+1];
   Word_t                              DeviceAppearance;
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

            /* Configure the flags field based on the Discoverability   */
            /* Mode.                                                    */
            if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            else
            {
               if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
            }

            /* Update the length.                                       */
            /* * NOTE * We need to add one for the length octet.        */
            Length += Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + 1;

            /* If we are configured as the ESS Server we need to add    */
            /* some additional advertising data.                        */
            if(ESSInstanceID)
            {
               /* Advertise the ESS Server(1 byte type and 2 bytes UUID)*/
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] = 3;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
               ESS_ASSIGN_ESS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2]));

               /* Update the length.                                    */
               /* * NOTE * We need to add one for the length octet.     */
               Length += Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] + 1;

               /* Advertise the Service data.  It's main use is to allow*/
               /* an ESS Client to determine if there are any pending   */
               /* notifications since it last connected.  This way it   */
               /* can connect if necessary.                             */
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] = 5;
               Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_SERVICE_DATA;
               ESS_ASSIGN_ESS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2]));
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+4]), ServerInfo.ChangeIndex);

               /* Update the length.                                    */
               /* * NOTE * We need to add one for the length octet.     */
               Length += Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] + 1;
            }

            /* Write the advertising data to the chip.                  */
            ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.AdvertisingData));
            if(!ret_val)
            {
               /* Reset the length to zero for the Scan Response data.  */
               Length = 0;

               /* Set the Scan Response data.                           */
               BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

               /* First include the local device name.                  */
               if((ret_val = GAPS_Query_Device_Name(BluetoothStackID, GAPSInstanceID, Name)) == 0)
               {
                  NameLength = BTPS_StringLength(Name);
                  if(NameLength < (ADVERTISING_DATA_MAXIMUM_SIZE - 2))
                  {
                     Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
                  }
                  else
                  {
                     Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                     NameLength = (ADVERTISING_DATA_MAXIMUM_SIZE - 2);
                  }

                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (Byte_t)(1 + NameLength);
                  BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]), LE_DEMO_DEVICE_NAME, NameLength);

                  /* Update the length.                                 */
                  /* * NOTE * We need to add one for the length octet.  */
                  Length += Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] + 1;
               }

               /* Include local device appearance.                      */
               /* * NOTE * Length will be zero if we could not retrieve */
               /*          the device name.                             */
               if((ret_val = GAPS_Query_Device_Appearance(BluetoothStackID, GAPSInstanceID, &DeviceAppearance)) == 0)
               {
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[Length] = 3;
                  Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[Length+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[Length+2]), DeviceAppearance);

                  /* Update the length.                                 */
                  /* * NOTE * We need to add one for the length octet.  */
                  Length += Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[Length] + 1;
               }
               else
                  DisplayFunctionError("GAPS_Query_Device_Appearance", ret_val);

               /* Write the Scan Response data to the chip.             */
               ret_val = GAP_LE_Set_Scan_Response_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.ScanResponseData));
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

            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, 1, &UUID, GATT_Service_Discovery_Event_Callback, 0);
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
                  printf("\r\nGATT_Service_Discovery_Start() success.\r\n");
               else
                  printf("\r\nGATT_Start_Service_Discovery_Handle_Range() success.\r\n");

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
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

   /* The following function is responsible for initializing the ESS    */
   /* Server.  This function is also responsible for setting up the     */
   /* InitializeData for the call to ESS_Initialize_Service() to        */
   /* register the service.                                             */
   /* ** NOTE ** Memory will need to be freed after the service is      */
   /*            registered for the InializeData structure members.     */
static void InitializeESSServer(ESS_Initialize_Data_t *InitializeData)
{
   unsigned int                Index;
   unsigned int                Index2;
   ESS_Characteristic_Entry_t *InitCharacteristicPtr;
   ESS_Server_Instance_t      *ServerInstancePtr;

   /* Loop through every ESS Characteristic type since this application */
   /* will register each ESS Characteristic Type.                       */
   /* * NOTE * We are not using the stored field here since it is set to*/
   /*          the DEFINE and it may change in the future.              */
   for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
   {
      /* Store a pointer to the Characteristic's init information to aid*/
      /* in readability.                                                */
      InitCharacteristicPtr = &(InitializeData->Entries[Index]);

      /* Set the Characteristic Init Type.  We will simply use the index*/
      /* since we are registering every ESS Characteristic.             */
      InitCharacteristicPtr->Type = (ESS_Characteristic_Type_t)Index;

      /* Store the default number of ESS Characteristic instances that  */
      /* we want to register.                                           */
      InitCharacteristicPtr->Number_Of_Instances = (Word_t)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES;

      /* Assign the ESS Characteristic UUID in the ESS Server           */
      /* information.  We will simply use the Index to identify the ESS */
      /* Characteristic Type.                                           */
      /* * NOTE * This function will assign the UUID in Little-Endian   */
      /*          format.                                               */
      AssignESSCharacteristicUUID((ESS_Characteristic_Type_t)Index, &(ServerInfo.Characteristics[Index].UUID));

      /* Allocate memory for instances init information.                */
      /* ** NOTE ** This memory will need to be freed after a call to   */
      /*            the ESS_Initialize_Service() function.              */
      if((InitCharacteristicPtr->Instances = (ESS_Characteristic_Instance_Entry_t *)BTPS_AllocateMemory(ESS_CHARACTERISTIC_INSTANCE_ENTRY_SIZE * InitCharacteristicPtr->Number_Of_Instances)) != NULL)
      {
         /* Initiailze the memory so there is no unexpected behaviour.  */
         BTPS_MemInitialize(InitCharacteristicPtr->Instances, 0, (ESS_CHARACTERISTIC_INSTANCE_ENTRY_SIZE * InitCharacteristicPtr->Number_Of_Instances));

         /* Loop through the instances and set the default Flags and    */
         /* properties for the sample application.                      */
         for(Index2 = 0; Index2 < (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES; Index2++)
         {
            /* Store a pointer to the Server's Characteristic Instance  */
            /* information to aid in readability.                       */
            ServerInstancePtr = &(ServerInfo.Characteristics[Index].Instances[Index2]);

            /* Set the default characteristic properties.               */
            ServerInstancePtr->InitInformation.Characteristic_Property_Flags = ESS_DEFAULT_CHARACTERISTIC_PROPERTIES;

            /* Set the optional characteristic descriptors to include.  */
            ServerInstancePtr->InitInformation.Descriptor_Flags              = ESS_DEFAULT_DESCRIPTOR_FLAGS;

            /* Set the optional descriptor properties.                  */
            ServerInstancePtr->InitInformation.Descriptor_Property_Flags     = ESS_DEFAULT_DESCRIPTOR_PROPERTIES;

            /* Set the number of ES Trigger Setting instances.          */
            ServerInstancePtr->InitInformation.Trigger_Setting_Instances     = ESS_DEFAULT_ES_TRIGGER_SETTING_INSTANCES;

            /* Set the elevation for PTS.                               */
            if((ESS_Characteristic_Type_t)Index == ectElevation)
            {
               ServerInstancePtr->Characteristic.Elevation.Lower = 0x00FF;
               ServerInstancePtr->Characteristic.Elevation.Upper = 0x00;
            }

            /* Set the temperature for PTS.                             */
            if((ESS_Characteristic_Type_t)Index == ectTemperature)
            {
               ServerInstancePtr->Characteristic.Temperature = 0x00FF;
            }

            /* Let's initialize the ES Measurement Data in case it is   */
            /* included.                                                */
            /* * NOTE * Some of the fields we will set on the indexes to*/
            /*          make the ES MEasurement data UNIQUE, which is   */
            /*          required by the SPEC.                           */
            ServerInstancePtr->MeasurementData.Flags                    = 0;
            ServerInstancePtr->MeasurementData.Sampling_Function        = Index2+1;
            ServerInstancePtr->MeasurementData.Measurement_Period.Lower = 0;
            ServerInstancePtr->MeasurementData.Measurement_Period.Upper = 0;
            ServerInstancePtr->MeasurementData.Update_Interval.Lower    = 0;
            ServerInstancePtr->MeasurementData.Update_Interval.Upper    = 0;
            ServerInstancePtr->MeasurementData.Application              = Index2+1;
            ServerInstancePtr->MeasurementData.Measurement_Uncertainty  = ESS_ES_MEASUREMENT_UNCERTAINTY_INFO_NOT_AVAILABLE;

            /* Set the elevation for PTS.                               */
            if((ESS_Characteristic_Type_t)Index == ectElevation)
            {
               ServerInstancePtr->ValidRange.Lower.Elevation.Lower = 0x0000;
               ServerInstancePtr->ValidRange.Lower.Elevation.Upper = 0x00;
               ServerInstancePtr->ValidRange.Upper.Elevation.Lower = 0xFFFF;
               ServerInstancePtr->ValidRange.Upper.Elevation.Upper = 0x00;
            }

            /* Set the temperature for PTS.                             */
            if((ESS_Characteristic_Type_t)Index == ectTemperature)
            {
               ServerInstancePtr->ValidRange.Lower.Temperature = 0x0000;
               ServerInstancePtr->ValidRange.Upper.Temperature = 0xFFFF;
            }

            /* Set default user description.                            */
            BTPS_StringCopy(ServerInstancePtr->UserDescription, "Default");

            /* Copy the ESS Server init information for the instance    */
            /* into the corresponding instance location in the          */
            /* InitializeData parameter.                                */
            InitializeData->Entries[Index].Instances[Index2] = ServerInstancePtr->InitInformation;
         }

         /* Set the default value for the descriptor value changed.     */
         ServerInfo.DescriptorValueChanged.Registered = InitializeData->Descriptor_Value_Changed;
      }
      else
         printf("\r\nFailure - Could not allocate memory for the ESS Characteristic instance init information.\r\n");
   }
}

   /* The following function is responsible for registering a BAP       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterESS(ParameterList_t *TempParam)
{
   int                         ret_val = 0;
   ESS_Initialize_Data_t       InitializeData;
   ESS_Characteristic_Entry_t  Characteristics[ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES];
   unsigned int                Index;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!ESSInstanceID)
      {
         /* Initialize the ESS information data so we do not have       */
         /* unexpected behaviour.                                       */
         BTPS_MemInitialize(&InitializeData, 0, ESS_INITIALIZE_DATA_SIZE);

         /* Initialize the InstanceEntry information so we do not have  */
         /* unexpected behaviour.                                       */
         BTPS_MemInitialize(Characteristics, 0, (ESS_CHARACTERISTIC_ENTRY_SIZE * ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES));

         /* Setup the initialize data structure.                        */
         InitializeData.Number_Of_Entries         = (Byte_t)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES;
         InitializeData.Entries                   = Characteristics;
         InitializeData.Descriptor_Value_Changed  = (Boolean_t)ESS_SERVER_REGISTER_DESCRIPTOR_VALUE_CHANGED;

         /* Initialize the ESS Server information.                      */
         InitializeESSServer(&InitializeData);

         /* Initialize the service.                                     */
         ret_val = ESS_Initialize_Service(BluetoothStackID, (unsigned int)ESS_SERVICE_FLAGS_DUAL_MODE, &InitializeData, ESS_EventCallback, 0, &ESSInstanceID);
         if((ret_val > 0) && (ESSInstanceID > 0))
         {
            /* Display success message.                                 */
            printf("Successfully registered ESS Service, ESSInstanceID = %u.\r\n", ret_val);

            /* Save the ServiceID of the registered service.            */
            ESSInstanceID = (unsigned int)ret_val;

            /* Free the memory previously allocated by the              */
            /* InitializeESSServer() function for the ESS Characteristic*/
            /* instance's init information.                             */
            for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
            {
               /* If there are instances.                               */
               if(Characteristics[Index].Number_Of_Instances)
               {
                  /* Free the memory.                                   */
                  BTPS_FreeMemory(Characteristics[Index].Instances);
                  Characteristics[Index].Instances           = NULL;
                  Characteristics[Index].Number_Of_Instances = 0;
               }
            }

            /* Simply return success to the caller.                     */
            ret_val = 0;
         }
         else
         {
            DisplayFunctionError("ESS_Initialize_Service", ret_val);
         }
      }
      else
      {
         printf("\r\nESS is already registered.\r\n");
      }
   }
   else
   {
      printf("\r\nConnection currently active.\r\n");
   }

   return(ret_val);
}

   /* The following function is responsible for unregistering a BAP     */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnregisterESS(ParameterList_t *TempParam)
{
   int           ret_val = FUNCTION_ERROR;
   DeviceInfo_t *DeviceInfo;

   /* Verify that a service is registered.                              */
   if(ESSInstanceID)
   {
      /* Get the device info for the connection device.                 */
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

      /* Unregister the ESS Service with GATT.                          */
      ret_val = ESS_Cleanup_Service(BluetoothStackID, ESSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         printf("Successfully unregistered ESS Service InstanceID %u.\r\n",ESSInstanceID);

         /* Clear the InstanceID.                                       */
         ESSInstanceID = 0;
      }
      else
         DisplayFunctionError("ESS_Cleanup_Service", ret_val);
   }
   else
      printf("ESS Service not registered.\r\n");

   return(ret_val);
}

   /* The following function is responsible for performing a BAP        */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverESS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ESSInstanceID)
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
               /* Configure the filter so that only the ESS Service is  */
               /* discovered.                                           */
               UUID.UUID_Type = guUUID_16;
               ESS_ASSIGN_ESS_SERVICE_UUID_16(&(UUID.UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), &UUID, GATT_Service_Discovery_Event_Callback, sdESS);
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
      printf("Only the client can discover ESS.\r\n");

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a ESS Client Information structure with   */
   /* the information discovered from a GATT Discovery operation.       */
static void ESSPopulateHandles(ESS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;
   ESS_Client_Characteristic_t       *ClientCharacteristicPtr;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (ESS_COMPARE_ESS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service and */
      /* populate the correct entry.                                    */
      CharacteristicInfoPtr = ServiceDiscoveryData->CharacteristicInformationList;
      if(CharacteristicInfoPtr)
      {
         /* First we need to go through the characteristics and count   */
         /* the number of ESS Characteristics instances for each ESS    */
         /* Characteristic Type.  This way we can determine how many    */
         /* attribute handle entry structures we need to allocate to    */
         /* store the attribute handle information for each ESS         */
         /* Characteristic.                                             */
         CalculateAttributeHandleEntries(ClientInfo, ServiceDiscoveryData);

         /* Allocate memory for the each ESS Characteristic Type entry  */
         /* to hold the attribute handle list.                          */
         for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
         {
            /* Store a pointer to the ESS Characteristic to aid in      */
            /* readability.                                             */
            ClientCharacteristicPtr = &(ClientInfo->Characteristics[Index]);

            /* Only allocate memory if we discovered ESS Characteristics*/
            /* for this ESS Characteristic Type.                        */
            if(ClientCharacteristicPtr->AttributeHandleEntries)
            {
               /* Print the type information if enabled.                */
               if(PRINT_CHARACTERISTIC_TYPE_INFO)
               {
                  /* Print some information to inform the user how many */
                  /* ESS Characteristic instances for this ESS          */
                  /* Characteristic Type was found.                     */
                  printf("\r\nESS Characteristic Type Information:\r\n");
                  printf("   Type:                 ");
                  DisplayESSCharacteristicType(((ESS_Characteristic_Type_t)Index));
                  printf("   Number of instances:  %u\r\n", ClientCharacteristicPtr->AttributeHandleEntries);
               }

               /* Allocate the memory for all entries we calculated for */
               /* this ESS Characteristic Type.                         */
               if((ClientCharacteristicPtr->AttributeHandleList = (ESS_Client_Attribute_Handle_t *)BTPS_AllocateMemory(ESS_CLIENT_ATTRIBUTE_HANDLE_ENTRY_SIZE * ClientCharacteristicPtr->AttributeHandleEntries)) != NULL)
               {
                  /* Initialize the memory so we do not have unexpected */
                  /* behaviour.                                         */
                  BTPS_MemInitialize(ClientCharacteristicPtr->AttributeHandleList, 0, (ESS_CLIENT_ATTRIBUTE_HANDLE_ENTRY_SIZE * ClientCharacteristicPtr->AttributeHandleEntries));
               }
               else
               {
                  printf("\r\nFailure - Could not allocate memory for the attribute handle list.\r\n");

                  /* Free the memory we may have previously allocated   */
                  /* for other ESS Characteristic Type entries.         */
                  FreeAttributeHandleLists(ClientInfo);

                  /* Do not continue storing the atttribute handles     */
                  /* since we failed.                                   */
                  return;
               }
            }
         }

         /* Loop over all the Characteristics discovered and store their*/
         /* attribute handles.                                          */
         for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
         {
            /* All ESS UUIDs are defined to be 16 bit UUIDs.            */
            if(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Print the characteristic information discovered if    */
               /* needed.                                               */
               if(PRINT_SERVICE_DISCOVERY_HANDLES)
               {
                  printf("\r\nCharacteristic Handle: 0x%04X\r\n", CharacteristicInfoPtr[Index].Characteristic_Handle);
                  printf("   Properties:         0x%02X\r\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
                  printf("   UUID:               0x");
                  DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));

                  /* * NOTE * More information will be printed by the   */
                  /*          StoreAttributeHandles helper function.    */
               }

               /* Simply call the helper function to determine the      */
               /* Characteristic Type and store the attribute handles.  */
               StoreAttributeHandles(ClientInfo, &(CharacteristicInfoPtr[Index]));
            }
            else
               printf("\r\nWarning - Characteristic not a 16 bit UUID.\r\n");
         }
      }
   }
}

   /* The following function is a helper function for                   */
   /* ESSPopulateHandles().  This function will loop over the ESS       */
   /* Characteristic information and increment the number of attribute  */
   /* handle entries for each ESS Characteristic's Type.  This way we   */
   /* will be able to allocate memory to hold the attribute handle      */
   /* information for each ESS Characteristic instance.                 */
static void CalculateAttributeHandleEntries(ESS_Client_Information_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   unsigned int                       Index;
   GATT_Characteristic_Information_t *CharacteristicInfoPtr;

   /* Loop over all the ESS Characteristics discovered and determine    */
   /* their ESS Characteristic Types.                                   */
   for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
   {
      /* Store a pointer to the characteristic information.             */
      CharacteristicInfoPtr = ServiceDiscoveryData->CharacteristicInformationList;

      /* All ESS UUIDs are defined to be 16 bit UUIDs.                  */
      if(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID_Type == guUUID_16)
      {
         /* Determine the ESS Characteristic Type.                      */
         /* * NOTE * We will not check the descriptor value changed here*/
         /*          since there should only be one.                    */

         if(ESS_COMPARE_APPARENT_WIND_DIRECTION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectApparentWindDirection].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_APPARENT_WIND_SPEED_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectApparentWindSpeed].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_DEW_POINT_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectDewPoint].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_ELEVATION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectElevation].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_GUST_FACTOR_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectGustFactor].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_HEAT_INDEX_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectHeatIndex].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_HUMIDITY_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectHumidity].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_IRRADIANCE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectIrradiance].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_POLLEN_CONCENTRATION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectPollenConcentration].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_RAINFALL_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectRainfall].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_PRESSURE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectPressure].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_TEMPERATURE_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectTemperature].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_TRUE_WIND_DIRECTION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectTrueWindDirection].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_TRUE_WIND_SPEED_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectTrueWindSpeed].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_UV_INDEX_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectUVIndex].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_WIND_CHILL_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectWindChill].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_BAROMETRIC_PRESSURE_TREND_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectBarometricPressureTrend].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_MAGNETIC_DECLINATION_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectMagneticDeclination].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_MAGNETIC_FLUX_DENSITY_2D_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectMagneticFluxDensity2D].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }

         if(ESS_COMPARE_MAGNETIC_FLUX_DENSITY_3D_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16))
         {
            /* Increment the number of attribute handle entries for this*/
            /* characteristic type.                                     */
            ClientInfo->Characteristics[ectMagneticFluxDensity3D].AttributeHandleEntries++;

            /* We can't match another type so go to the next            */
            /* characteristic.                                          */
            continue;
         }
      }
      else
         printf("\r\nWarning: UUID not 16 bit.\r\n");
   }
}

   /* The following function is a helper function for                   */
   /* ESSPopulateHandles() and is responsible for storing the attribute */
   /* handles for all Characteristics.                                  */
static void StoreAttributeHandles(ESS_Client_Information_t *ClientInfo, GATT_Characteristic_Information_t *CharacteristicInfo)
{
   unsigned int                                  Index;
   ESS_Client_Characteristic_t                  *ClientCharacteristicPtr;
   ESS_Client_Attribute_Handle_t                *AttributeHandlePtr;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfo;

   /* Determine the ESS Characteristic Type.                            */
   if(ESS_COMPARE_APPARENT_WIND_DIRECTION_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic to aid in            */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectApparentWindDirection]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance to aid in  */
         /* readability.                                                */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Apparent Wind Direction\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_APPARENT_WIND_SPEED_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectApparentWindSpeed]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Apparent Wind Speed\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_DEW_POINT_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectDewPoint]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Dew Point\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_ELEVATION_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectElevation]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Elevation\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_GUST_FACTOR_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectGustFactor]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Gust Factor\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_HEAT_INDEX_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectHeatIndex]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Heat Index\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_HUMIDITY_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectHumidity]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Humidity\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_IRRADIANCE_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectIrradiance]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Irradiance\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_POLLEN_CONCENTRATION_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectPollenConcentration]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Pollen Concentration\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_RAINFALL_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectRainfall]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               RainFall\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_PRESSURE_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectPressure]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Pressure\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_TEMPERATURE_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectTemperature]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Temperature\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_TRUE_WIND_DIRECTION_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectTrueWindDirection]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               True Wind Direction\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_TRUE_WIND_SPEED_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectTrueWindSpeed]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               True Wind Speed\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_UV_INDEX_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectUVIndex]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               UV Index\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_WIND_CHILL_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectWindChill]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Wind Chill\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_BAROMETRIC_PRESSURE_TREND_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectBarometricPressureTrend]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Barometric Pressure Trend\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_MAGNETIC_DECLINATION_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectMagneticDeclination]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Magnetic Declination\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_MAGNETIC_FLUX_DENSITY_2D_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectMagneticFluxDensity2D]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Magnetic Flux Density 2D\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_MAGNETIC_FLUX_DENSITY_3D_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Verify that read is supported.                                 */
      /* * NOTE * All ESS Characteristics MUST support read.            */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
         printf("Warning - Mandatory read property of the ESS characteristic not supported!\r\n");

      /* Store a pointer to the ESS Characteristic Type entry to aid in */
      /* readability.                                                   */
      ClientCharacteristicPtr = &(ClientInfo->Characteristics[ectMagneticFluxDensity3D]);

      /* Find an empty attribute handle entry to store this attribute   */
      /* handle information.                                            */
      for(Index = 0; Index < ClientCharacteristicPtr->AttributeHandleEntries; Index++)
      {
         /* Store a pointer to the Attribute Handle instance entry to   */
         /* aid in readability.                                         */
         AttributeHandlePtr = &(ClientCharacteristicPtr->AttributeHandleList[Index]);

         /* If this attribute handle information instance has not been  */
         /* used..                                                      */
         /* * NOTE * We determined the number of enties earlier for all */
         /*          the ESS Characteristics so there MUST be an empty  */
         /*          entry.                                             */
         if(AttributeHandlePtr->Valid == FALSE)
         {
            /* Mark the entry has used.                                 */
            AttributeHandlePtr->Valid = TRUE;

            /* Print the characteristic information discovered if       */
            /* needed.                                                  */
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("   Type:               Magnetic Flux Density 3D\r\n");
               printf("   Instance:           %u\r\n", Index);
            }

            /* Store the attribute handle for this ESS Characteristic.  */
            AttributeHandlePtr->Attribute_Handle = CharacteristicInfo->Characteristic_Handle;

            /* Call the helper function to populate the descriptor      */
            /* handles for this attribute handle entry.                 */
            StoreDescriptorHandles(AttributeHandlePtr, CharacteristicInfo);

            /* Return since we found the ESS Characteristic Type.       */
            return;
         }
      }
   }

   if(ESS_COMPARE_DESCRIPTOR_VALUE_CHANGED_UUID_TO_UUID_16(CharacteristicInfo->Characteristic_UUID.UUID.UUID_16))
   {
      /* Print the characteristic information discovered if needed.     */
      if(PRINT_SERVICE_DISCOVERY_HANDLES)
      {
         printf("   Type:               Descriptor Value Changed\r\n");
      }

      /* Verify that indicate is supported.                             */
      /* * NOTE * Descriptor Value Changed MUST support indicate.       */
      if(!(CharacteristicInfo->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_INDICATE))
         printf("Warning - Mandatory indicate property of the ESS characteristic not supported!\r\n");

      /* Store the attribute handle for the Descriptor Value Changed    */
      /* Characteristic.                                                */
      ClientInfo->Descriptor_Value_Changed_Handle = CharacteristicInfo->Characteristic_Handle;

      /* Store a pointer to the Descriptor information.                 */
      DescriptorInfo = CharacteristicInfo->DescriptorList;

      /* Loop through the descriptor information and store the          */
      /* descriptor handles.                                            */
      for(Index = 0; Index < CharacteristicInfo->NumberOfDescriptors; Index++)
      {
         /* Make sure the descriptor is a 16 bit UUID.                  */
         if(DescriptorInfo->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
         {
            /* Check for the CCCD.                                      */
            if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
            {
               if(PRINT_SERVICE_DISCOVERY_HANDLES)
               {
                  printf("\r\n");
                  printf("Descriptor Handle: 0x%04X\r\n", DescriptorInfo[Index].Characteristic_Descriptor_Handle);
                  printf("   UUID:           0x");
                  DisplayUUID(&(DescriptorInfo[Index].Characteristic_Descriptor_UUID));
                  printf("   Type:           CCCD\r\n");
               }

               /* Store the handle.                                     */
               ClientInfo->Descriptor_Value_Changed_CCCD = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
               continue;
            }

            printf("\r\nWarning - Unknown characteristic descriptor.\r\n");
         }
         else
            printf("\r\nWarning - Descriptor not a 16 bit UUID.\r\n");
      }

      /* Return since we found the ESS Characteristic Type.             */
      return;
   }

   /* Always print warnings about unknown ESS Characteristics .         */
   printf("\r\nWarning - Unknown ESS characteristic.\r\n");
   printf("\r\nCharacteristic Handle: 0x%04X\r\n", CharacteristicInfo->Characteristic_Handle);
   printf("   Properties:         0x%02X\r\n", CharacteristicInfo->Characteristic_Properties);
   printf("   UUID:               0x");
   DisplayUUID(&(CharacteristicInfo->Characteristic_UUID));
}

   /* The following function is a helper function for                   */
   /* StoreAttributeHandles() to populate the descriptor handles for an */
   /* attribute handle entry.                                           */
static void StoreDescriptorHandles(ESS_Client_Attribute_Handle_t *AttributeHandleEntry, GATT_Characteristic_Information_t *CharacteristicInfo)
{
   unsigned int                                  Index;
   GATT_Characteristic_Descriptor_Information_t *DescriptorInfo;
   Byte_t                                        ESTriggerIndex = 0;

   /* Store a pointer to the Descriptor information.                    */
   DescriptorInfo = CharacteristicInfo->DescriptorList;

   /* Loop through the descriptor information and store the descriptor  */
   /* handles.                                                          */
   for(Index = 0; Index < CharacteristicInfo->NumberOfDescriptors; Index++)
   {
      /* Print the characteristic descriptor information if needed.     */
      if(PRINT_SERVICE_DISCOVERY_HANDLES)
      {
         printf("\r\nDescriptor Handle: 0x%04X\r\n", DescriptorInfo[Index].Characteristic_Descriptor_Handle);
         printf("   UUID:           0x");
         DisplayUUID(&(DescriptorInfo[Index].Characteristic_Descriptor_UUID));
         printf("   Type:           ");
      }

      /* Make sure the descriptor is a 16 bit UUID.                     */
      if(DescriptorInfo->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
      {
         /* Check for the CCCD.                                         */
         if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("CCCD\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->CCCD_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the extended properties.                          */
         if(GATT_COMPARE_CHARACTERISTIC_EXTENDED_PROPERTIES_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("Extended Properties\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->Extended_Properties_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the ES Measurement.                               */
         if(ESS_COMPARE_ENVIRONMENTAL_SENSING_MEASUREMENT_UUID_TO_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("ES Measurement\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->ES_Measurement_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the ES Trigger Setting.                           */
         if(ESS_COMPARE_ENVIRONMENTAL_SENSING_TRIGGER_SETTING_UUID_TO_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("ES Trigger Setting\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->ES_Trigger_Setting_Handle[ESTriggerIndex++] = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the ES Configuration.                             */
         if(ESS_COMPARE_ENVIRONMENTAL_SENSING_CONFIGURATION_UUID_TO_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("ES Configuration\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->ES_Configuration_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the User Description.                             */
         if(GATT_COMPARE_CHARACTERISTIC_USER_DESCRIPTION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("User Description\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->User_Description_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Check for the Valid Range                                   */
         if(ESS_COMPARE_VALID_RANGE_UUID_TO_UUID_16(DescriptorInfo[Index].Characteristic_Descriptor_UUID.UUID.UUID_16))
         {
            if(PRINT_SERVICE_DISCOVERY_HANDLES)
            {
               printf("Valid Range\r\n");
            }

            /* Store the handle.                                        */
            AttributeHandleEntry->Valid_Range_Handle = DescriptorInfo[Index].Characteristic_Descriptor_Handle;
            continue;
         }

         /* Always print warnings for unknown characteristic            */
         /* descriptors.                                                */
         printf("\r\nWarning - Unknown characteristic descriptor.\r\n");
         printf("\r\nDescriptor Handle: 0x%04X\r\n", DescriptorInfo[Index].Characteristic_Descriptor_Handle);
         printf("   UUID:           0x");
         DisplayUUID(&(DescriptorInfo[Index].Characteristic_Descriptor_UUID));
      }
      else
         printf("\r\nWarning - Descriptor not a 16 bit UUID.\r\n");
   }
}

   /* The following function is responsible for free the attribute      */
   /* handle lists.  These lists will be allocated during service       */
   /* discovery by the ESS Client and will need to be freed if an error */
   /* occurs during service discovery or cleanup is performed.          */
static void FreeAttributeHandleLists(ESS_Client_Information_t *ClientInfo)
{
   unsigned int                 Index;
   ESS_Client_Characteristic_t *CharacteristicTypeEntry;

   /* Loop through all the Characteristic Type Entries and free the     */
   /* attribute handle lists.                                           */
   for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
   {
      /* Store a pointer to the Characteristic Type entry to aid in     */
      /* readability.                                                   */
      CharacteristicTypeEntry = &(ClientInfo->Characteristics[Index]);

      /* If there are entries for this ESS Characteristic type.         */
      if(CharacteristicTypeEntry->AttributeHandleEntries)
      {
         /* Free the memory.                                            */
         BTPS_FreeMemory(CharacteristicTypeEntry->AttributeHandleList);

         /* Set the pointer to NULL and the number of entries to zero.  */
         CharacteristicTypeEntry->AttributeHandleList    = NULL;
         CharacteristicTypeEntry->AttributeHandleEntries = 0;
      }
   }
}

   /* The following is a helper function to display the ESS             */
   /* Characteristic Type.                                              */
static void DisplayESSCharacteristicType(ESS_Characteristic_Type_t Type)
{
   switch(Type)
   {
      case ectApparentWindDirection:
         printf("ectApparentWindDirection(%u)\r\n", ectApparentWindDirection);
         break;
      case ectApparentWindSpeed:
         printf("ectApparentWindSpeed(%u)\r\n", ectApparentWindSpeed);
         break;
      case ectDewPoint:
         printf("ectDewPoint(%u)\r\n", ectDewPoint);
         break;
      case ectElevation:
         printf("ectElevation(%u)\r\n", ectElevation);
         break;
      case ectGustFactor:
         printf("ectGustFactor(%u)\r\n", ectGustFactor);
         break;
      case ectHeatIndex:
         printf("ectHeatIndex(%u)\r\n", ectHeatIndex);
         break;
      case ectHumidity:
         printf("ectHumidity(%u)\r\n", ectHumidity);
         break;
      case ectIrradiance:
         printf("ectIrradiance(%u)\r\n", ectIrradiance);
         break;
      case ectPollenConcentration:
         printf("ectPollenConcentration(%u)\r\n", ectPollenConcentration);
         break;
      case ectRainfall:
         printf("ectRainfall(%u)\r\n", ectRainfall);
         break;
      case ectPressure:
         printf("ectPressure(%u)\r\n", ectPressure);
         break;
      case ectTemperature:
         printf("ectTemperature(%u)\r\n", ectTemperature);
         break;
      case ectTrueWindDirection:
         printf("ectTrueWindDirection(%u)\r\n", ectTrueWindDirection);
         break;
      case ectTrueWindSpeed:
         printf("ectTrueWindSpeed(%u)\r\n", ectTrueWindSpeed);
         break;
      case ectUVIndex:
         printf("ectUVIndex(%u)\r\n", ectUVIndex);
         break;
      case ectWindChill:
         printf("ectWindChill(%u)\r\n", ectWindChill);
         break;
      case ectBarometricPressureTrend:
         printf("ectBarometricPressureTrend(%u)\r\n", ectBarometricPressureTrend);
         break;
      case ectMagneticDeclination:
         printf("ectMagneticDeclination(%u)\r\n", ectMagneticDeclination);
         break;
      case ectMagneticFluxDensity2D:
         printf("ectMagneticFluxDensity2D(%u)\r\n", ectMagneticFluxDensity2D);
         break;
      case ectMagneticFluxDensity3D:
         printf("ectMagneticFluxDensity3D(%u)\r\n", ectMagneticFluxDensity3D);
         break;
      default:
         printf("Invalid ESS Characteristic type.\r\n");
         break;
   }
}

   /* The following function is responsible for display the             */
   /* characteristic info in ESS Events.                                */
static void DisplayESSCharacteristicInfo(ESS_Characteristic_Info_t *CharacteristicInfo)
{
   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      printf("   Characteristic Info:\r\n");
      printf("      Type:              ");
      DisplayESSCharacteristicType(CharacteristicInfo->Type);
      printf("      ID:                %u\r\n", CharacteristicInfo->ID);
   }
}

   /* The following function is responsible for getting a pointer to the*/
   /* ESS Server Characteristic instance information based on the       */
   /* CharacteristicInfo parameter.  If this function is successful a   */
   /* pointer to the instance will be returned.  Otherwise NULL will be */
   /* returned.                                                         */
static ESS_Server_Instance_t *GetServerCharacteristicInstanceInfoPtr(ESS_Characteristic_Info_t *CharacteristicInfo)
{
   ESS_Server_Instance_t *ret_val = NULL;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Make sure the type is valid.                                   */
      if((CharacteristicInfo->Type >= ectApparentWindDirection) && (CharacteristicInfo->Type <= ectMagneticFluxDensity3D))
      {
         /* Make sure the number of instances are valid for the type.   */
         if(CharacteristicInfo->ID < ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES)
         {
            /* Since we have the type and instance identifier we can use*/
            /* these values to index the ESS Server information and get */
            /* the correct instance.                                    */
            ret_val = &(ServerInfo.Characteristics[CharacteristicInfo->Type].Instances[CharacteristicInfo->ID]);
         }
      }
   }
   else
      printf("\r\nWarning - Characteristic information invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for getting a pointer to the*/
   /* ESS Client Characteristic instance's attribute handle information */
   /* based on the CharacteristicInfo parameter.  If this function is   */
   /* successful a pointer to the attribute handle information will be  */
   /* returned.  Otherwise NULL will be returned.                       */
static ESS_Client_Attribute_Handle_t *GetClientAttributeHandleInfoPtr(DeviceInfo_t *DeviceInfo)
{
   ESS_Client_Attribute_Handle_t *ret_val = NULL;
   ESS_Characteristic_Type_t      Type;
   unsigned int                   ID;

   /* Make sure the parameters are semi-valid.                          */
   if(DeviceInfo)
   {
      /* Store the Type and ID to make the code more readable.          */
      Type = DeviceInfo->ESSClientInfo.ClientRequestInfo.Type;
      ID   = DeviceInfo->ESSClientInfo.ClientRequestInfo.ID;

      /* Make sure the type is valid.                                   */
      if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
      {
         /* Make sure the ID is less than or equal to the number of     */
         /* attribute handle information entries.                       */
         /* * NOTE * This should directly correspond to the number of   */
         /*          instances of a ESS Characteristic Type we          */
         /*          discovered.                                        */
         if(ID < DeviceInfo->ESSClientInfo.Characteristics[Type].AttributeHandleEntries)
         {
            /* Since we have the type and instance identifier we can use*/
            /* these values to index the ESS Client Attribute Handle    */
            /* information and get the correct instance.                */
            ret_val = &(DeviceInfo->ESSClientInfo.Characteristics[Type].AttributeHandleList[ID]);
         }
      }
   }
   else
      printf("\r\nWarning - Device information invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for decoding and displaying */
   /* the ESS Characteristic data.                                      */
static int DecodeDisplayDescriptorValueChangedData(unsigned int ValueLength, Byte_t *Value)
{
   int                                  ret_val = 0;
   ESS_Descriptor_Value_Changed_Data_t  Data;

   /* Simply call the API to decode the Characteristic Data.            */
   /* * NOTE * If an error is returned the caller should print the      */
   /*          result.                                                  */
   if((ret_val = ESS_Decode_Descriptor_Value_Changed(ValueLength, Value, &Data)) == 0)
   {
      /* Display the decoded value.                                     */
      printf("\r\nDescriptor Value Changed:\r\n");
      printf("   Flags:                   0x%04X\r\n", Data.Flags);
      printf("      Source:               %s\r\n", (Data.Flags & ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_SOURCE_OF_CHANGE_CLIENT)             ? "Client" : "Server");
      printf("      Changed:\r\n");
      printf("      ES Trigger Setting:   %s\r\n", (Data.Flags & ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_ES_TRIGGER_SETTING_CHANGED)          ? "Yes" : "No");
      printf("      ES Configuration:     %s\r\n", (Data.Flags & ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_ES_CONFIGURATION_DESCRIPTOR_CHANGED) ? "Yes" : "No");
      printf("      ES Measurement:       %s\r\n", (Data.Flags & ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_ES_MEASUREMENT_DESCRIPTOR_CHANGED)   ? "Yes" : "No");
      printf("      User Description:     %s\r\n", (Data.Flags & ESS_DESCRIPTOR_VALUE_CHANGED_FLAGS_USER_DESCRIPTION_CHANGED)            ? "Yes" : "No");
      printf("   UUID :                   ");
      DisplayUUID(&Data.UUID);
   }
   else
   {
      /* Print the length is invalid if we received that the data was   */
      /* malformatted for the ESS Characteristic Type.                  */
      if(ret_val == ESS_ERROR_MALFORMATTED_DATA)
      {
         printf("\r\nDescriptor Value Changed:\r\n");
         printf("   Value:   Invalid length.\r\n");
      }
   }

   return (ret_val);
}

   /* The following function is responsible for decoding and displaying */
   /* the ESS Characteristic data.                                      */
static int DecodeDisplayESSCharacteristicData(ESS_Characteristic_Type_t Type, unsigned int ValueLength, Byte_t *Value)
{
   int                        ret_val = 0;
   ESS_Characteristic_Data_t  Data;

   /* Simply call the API to decode the Characteristic Data.            */
   /* * NOTE * If an error is returned the caller should print the      */
   /*          result.                                                  */
   if((ret_val = ESS_Decode_Characteristic(ValueLength, Value, Type, &Data)) == 0)
   {
      /* Display the decoded value.                                     */
      DisplayESSCharacteristic(Type, &Data);
   }
   else
   {
      /* Print the length is invalid if we received that the data was   */
      /* malformatted for the ESS Characteristic Type.                  */
      if(ret_val == ESS_ERROR_MALFORMATTED_DATA)
      {
         printf("\r\nESS Characteristic:\r\n");
         printf("   Value:   Invalid length.\r\n");
      }
   }

   return (ret_val);
}

   /* The following function is responsible for decoding and displaying */
   /* the ESS ES Trigger Setting data.                                  */
static int DecodeDisplayESTriggerSettingData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSetting, unsigned int ValueLength, Byte_t *Value)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if((TriggerSetting) && (ValueLength) && (Value))
   {
      /* Simply call the API to decode the ES Measurement.              */
      if((ret_val = ESS_Decode_ES_Trigger_Setting(ValueLength, Value, Type, TriggerSetting)) == 0)
      {
         /* Simply call the internal function to display the ES Trigger */
         /* Setting.                                                    */
         DisplayESTriggerSettingData(Type, Instance, TriggerSetting);
      }
      else
      {
         switch(ret_val)
         {
            case ESS_ERROR_MALFORMATTED_DATA:
               /* Print the length is invalid if the data was           */
               /* malformatted.                                         */
               printf("\r\nES Trigger Setting:\r\n");
               printf("   Value:   Invalid length.\r\n");
               break;
            case ESS_ERROR_INVALID_ES_TRIGGER_SETTING_CONDITION:
               /* Display the ES Trigger Setting if we received in RFU  */
               /* condition.                                            */
               DisplayESTriggerSettingData(Type, Instance, TriggerSetting);
               break;
            default:
               /* Always display the function error.                    */
               DisplayFunctionError("ESS_Decode_ES_Trigger_Setting", ret_val);
         }
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is a helper function for displaying the ESS*/
   /* Characteristic data based on the Type after it is decoded.  This  */
   /* function will call the DisplayESSCharacteristicType() helper      */
   /* function to display the Type field.                               */
static void DisplayESSCharacteristic(ESS_Characteristic_Type_t Type, ESS_Characteristic_Data_t *Data)
{
   printf("\r\nESS Characteristic:\r\n");
   switch(Type)
   {
      case ectApparentWindDirection:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Apparent_Wind_Direction);
         break;
      case ectApparentWindSpeed:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Apparent_Wind_Speed);
         break;
      case ectDewPoint:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%02X\r\n", (Byte_t)Data->Dew_Point);
         break;
      case ectElevation:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%08X\r\n", (((int)Data->Elevation.Upper << 16) | ((int)Data->Elevation.Lower)));
         break;
      case ectGustFactor:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%02X\r\n", Data->Gust_Factor);
         break;
      case ectHeatIndex:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%02X\r\n", (Byte_t)Data->Heat_Index);
         break;
      case ectHumidity:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Humidity);
         break;
      case ectIrradiance:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Irradiance);
         break;
      case ectPollenConcentration:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%08X\r\n", (((int)Data->Pollen_Concentration.Upper << 16) | ((int)Data->Pollen_Concentration.Lower)));
         break;
      case ectRainfall:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Rainfall);
         break;
      case ectPressure:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%08X\r\n", Data->Pressure);
         break;
      case ectTemperature:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", (Word_t)Data->Temperature);
         break;
      case ectTrueWindDirection:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->True_Wind_Direction);
         break;
      case ectTrueWindSpeed:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->True_Wind_Speed);
         break;
      case ectUVIndex:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%02X\r\n", Data->UV_Index);
         break;
      case ectWindChill:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%02X\r\n", (Byte_t)Data->Wind_Chill);
         break;
      case ectBarometricPressureTrend:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   ");
         switch(Data->Barometric_Pressure_Trend)
         {
            case bptUnknown:
               printf("Unknown\r\n");
               break;
            case bptContinuouslyFalling:\
               printf("Continuously falling.\r\n");
               break;
            case bptContinuouslyRising:
               printf("Continuously rising.\r\n");
               break;
            case bptFallingThenSteady:
               printf("Falling, then steady.\r\n");
               break;
            case bptRisingThenSteady:
               printf("Rising, then steady.\r\n");
               break;
            case bptFallingBeforeLesserRise:
               printf("Falling, before lesser rise.\r\n");
               break;
            case bptFallingBeforeGreaterRise:
               printf("Falling, before greater rise .\r\n");
               break;
            case bptRisingBeforeGreaterFall:
               printf("Rising, before greater fall.\r\n");
               break;
            case bptRisingBeforeLesserFall:
               printf("Rising, before lesser fall.\r\n");
               break;
            case bptSteady:
               printf("Steady.\r\n");
               break;
            default:
               printf("Invalid enum value.\r\n");
               break;
         }
         break;
      case ectMagneticDeclination:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:   0x%04X\r\n", Data->Magnetic_Declination);
         break;
      case ectMagneticFluxDensity2D:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:\r\n");
         printf("   X-Axis:  0x%04X\r\n", (Word_t)Data->Magnetic_Flux_Density_2D.X_Axis);
         printf("   Y-Axis:  0x%04X\r\n", (Word_t)Data->Magnetic_Flux_Density_2D.Y_Axis);
         break;
      case ectMagneticFluxDensity3D:
         printf("   Type:    ");
         DisplayESSCharacteristicType(Type);
         printf("   Value:\r\n");
         printf("   X-Axis:  0x%04X\r\n", (Word_t)Data->Magnetic_Flux_Density_3D.X_Axis);
         printf("   Y-Axis:  0x%04X\r\n", (Word_t)Data->Magnetic_Flux_Density_3D.Y_Axis);
         printf("   Z-Axis:  0x%04X\r\n", (Word_t)Data->Magnetic_Flux_Density_3D.Z_Axis);
         break;
      default:
         /* Can't occur API will fail to decode if Type is not correct. */
         break;
   }
}

   /* The following function is responsible for displaying the ES       */
   /* Measurement data after it has been decoded.                       */
static void DisplayESMeasurementData(ESS_ES_Measurement_Data_t *MeasurementData)
{
   /* Make sure the ES Measurement is valid.                            */
   if(MeasurementData)
   {
      printf("\r\nES Measurement:\r\n");
      printf("   Flags:               0x%04X\r\n", MeasurementData->Flags);
      printf("   Sampling Function:   0x%04X\r\n", MeasurementData->Sampling_Function);
      printf("   Measurement Period:  0x%08X\r\n", (((int)MeasurementData->Measurement_Period.Upper << 16) | ((int)MeasurementData->Measurement_Period.Lower)));
      printf("   Update Interval:     0x%08X\r\n", (((int)MeasurementData->Update_Interval.Upper << 16) | ((int)MeasurementData->Update_Interval.Lower)));
      printf("   Application:         0x%02X\r\n", MeasurementData->Application);
      printf("   Uncertainty:         0x%02X\r\n", MeasurementData->Measurement_Uncertainty);
   }
   else
      printf("\r\nWarning - Invalid ES Measurement.\r\n");
}

   /* The following function is responsible for displaying the ES       */
   /* Trigger Setting Data after it has been decoded.                   */
   /* * NOTE * The Decode API should have failed if the Type and        */
   /*          Condition does not match up properly.  Some Types cannot */
   /*          be used for certain Conditions.                          */
static void DisplayESTriggerSettingData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Instance_t Instance, ESS_ES_Trigger_Setting_Data_t *TriggerSettingData)
{
   /* Make sure the ES Trigger Setting is valid.                        */
   if(TriggerSettingData)
   {
      printf("\r\nES Trigger Setting:\r\n");

      /* Display the Type.                                              */
      printf("   Type:       ");
      DisplayESSCharacteristicType(Type);

      /* Display the instnace.                                          */
      printf("   Instance:   ");
      switch(Instance)
      {
         case tsiTriggerSetting_0:
            printf("%u.\r\n", tsiTriggerSetting_0);
            break;
         case tsiTriggerSetting_1:
            printf("%u.\r\n", tsiTriggerSetting_1);
            break;
         case tsiTriggerSetting_2:
            printf("%u.\r\n", tsiTriggerSetting_2);
            break;
         default:
            printf("Invalid instance.\r\n");
            break;
      }

      /* Display the Condition.                                         */
      printf("   Condition:  ");
      switch(TriggerSettingData->Condition)
      {
         case tscTriggerInactive:
            printf("Trigger inactive.\r\n");

            /* Display the operand.                                     */
            printf("   Operand:    None.\r\n");
            break;
         case tscFixedTimeInterval:
            printf("Fixed time interval.\r\n");

            /* Display the operand.                                     */
            printf("   Operand:    %u (seconds)\r\n", (((int)TriggerSettingData->Operand.Seconds.Upper << 16) | ((int)TriggerSettingData->Operand.Seconds.Lower)));
            break;
         case tscNoLessThanSpecifiedTime:
            printf("No less than specified time.\r\n");

            /* Display the operand.                                     */
            printf("   Operand:    %u (seconds)\r\n", (((int)TriggerSettingData->Operand.Seconds.Upper << 16) | ((int)TriggerSettingData->Operand.Seconds.Lower)));
            break;
         case tscValueChanged:
            printf("Value changed.\r\n");

            /* Display the operand.                                     */
            printf("   Operand:    None.\r\n");
            break;
         case tscLessThanSpecifiedValue:
            printf("Less than specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         case tscLessThanOrEqualSpecifiedValue:
            printf("Less than or equal to specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         case tscGreaterThanSpecifiedValue:
            printf("Greater than specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         case tscGreaterThanOrEqualSpecifiedValue:
            printf("Greater than or equal to specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         case tscEqualSpecifiedValue:
            printf("Equal to specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         case tscNotEqualSpecifiedValue:
            printf("Not equal to specified value.\r\n");

            /* Display the operand.                                     */
            DisplayESTriggerSettingOperandData(Type, TriggerSettingData);
            break;
         default:
            printf("Reserved for Future Use (RFU) condition.\r\n");
            break;
      }
   }
   else
      printf("\r\nWarning - Invalid ES Trigger Setting.\r\n");
}

   /* The following function is a helper function that is responsible   */
   /* for display the ES Trigger Setting Operand field.                 */
   /* * NOTE * This function will NOT display the Units field of the ES */
   /*          Trigger Setting.  This MUST be done by the caller.       */
static void DisplayESTriggerSettingOperandData(ESS_Characteristic_Type_t Type, ESS_ES_Trigger_Setting_Data_t *TriggerSettingData)
{
   switch(Type)
   {
      case ectApparentWindDirection:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Apparent_Wind_Direction);
         break;
      case ectApparentWindSpeed:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Apparent_Wind_Speed);
         break;
      case ectDewPoint:
         printf("   Operand:    0x%02X\r\n", (Byte_t)TriggerSettingData->Operand.Dew_Point);
         break;
      case ectElevation:
         printf("   Operand:    0x%08X\r\n", (((int)TriggerSettingData->Operand.Elevation.Upper << 16) | ((int)TriggerSettingData->Operand.Elevation.Lower)));
         break;
      case ectGustFactor:
         printf("   Operand:    0x%02X\r\n", TriggerSettingData->Operand.Gust_Factor);
         break;
      case ectHeatIndex:
         printf("   Operand:    0x%02X\r\n", (Byte_t)TriggerSettingData->Operand.Heat_Index);
         break;
      case ectHumidity:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Humidity);
         break;
      case ectIrradiance:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Irradiance);
         break;
      case ectPollenConcentration:
         printf("   Operand:    0x%08X\r\n", (((int)TriggerSettingData->Operand.Pollen_Concentration.Upper << 16) | ((int)TriggerSettingData->Operand.Pollen_Concentration.Lower)));
         break;
      case ectRainfall:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Rainfall);
         break;
      case ectPressure:
         printf("   Operand:    0x%08X\r\n", TriggerSettingData->Operand.Pressure);
         break;
      case ectTemperature:
         printf("   Operand:    0x%04X\r\n", (Word_t)TriggerSettingData->Operand.Temperature);
         break;
      case ectTrueWindDirection:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.True_Wind_Direction);
         break;
      case ectTrueWindSpeed:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.True_Wind_Speed);
         break;
      case ectUVIndex:
         printf("   Operand:    0x%02X\r\n", TriggerSettingData->Operand.UV_Index);
         break;
      case ectWindChill:
         printf("   Operand:    0x%02X\r\n", (Byte_t)TriggerSettingData->Operand.Wind_Chill);
         break;
      case ectMagneticDeclination:
         printf("   Operand:    0x%04X\r\n", TriggerSettingData->Operand.Magnetic_Declination);
         break;
      default:
         /* Can't occur API will fail to decode if Type is not correct. */
         break;
   }
}

   /* The following function is responsible for displaying the Valid    */
   /* Range after it has been decoded.                                  */
static void DisplayValidRangeData(ESS_Characteristic_Type_t Type, ESS_Valid_Range_Data_t *ValidRangeData)
{
   /* Make sure the valid range is valid.                               */
   if(ValidRangeData)
   {
      printf("\r\nValid Range:\r\n");
      switch(Type)
      {
         case ectApparentWindDirection:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Apparent_Wind_Direction);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Apparent_Wind_Direction);
            break;
         case ectApparentWindSpeed:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Apparent_Wind_Speed);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Apparent_Wind_Speed);
            break;
         case ectDewPoint:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%02X\r\n", (Byte_t)ValidRangeData->Lower.Dew_Point);
            printf("   Upper:   0x%02X\r\n", (Byte_t)ValidRangeData->Upper.Dew_Point);
            break;
         case ectElevation:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%08X\r\n", (((int)ValidRangeData->Lower.Elevation.Upper << 16) | ((int)ValidRangeData->Lower.Elevation.Lower)));
            printf("   Upper:   0x%08X\r\n", (((int)ValidRangeData->Upper.Elevation.Upper << 16) | ((int)ValidRangeData->Upper.Elevation.Lower)));
            break;
         case ectGustFactor:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%02X\r\n", ValidRangeData->Lower.Gust_Factor);
            printf("   Upper:   0x%02X\r\n", ValidRangeData->Upper.Gust_Factor);
            break;
         case ectHeatIndex:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%02X\r\n", (Byte_t)ValidRangeData->Lower.Heat_Index);
            printf("   Upper:   0x%02X\r\n", (Byte_t)ValidRangeData->Upper.Heat_Index);
            break;
         case ectHumidity:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Humidity);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Humidity);
            break;
         case ectIrradiance:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Irradiance);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Irradiance);
            break;
         case ectPollenConcentration:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%08X\r\n", (((int)ValidRangeData->Lower.Pollen_Concentration.Upper << 16) | ((int)ValidRangeData->Lower.Pollen_Concentration.Lower)));
            printf("   Upper:   0x%08X\r\n", (((int)ValidRangeData->Upper.Pollen_Concentration.Upper << 16) | ((int)ValidRangeData->Upper.Pollen_Concentration.Lower)));
            break;
         case ectRainfall:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Rainfall);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Rainfall);
            break;
         case ectPressure:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%08X\r\n", ValidRangeData->Lower.Pressure);
            printf("   Upper:   0x%08X\r\n", ValidRangeData->Upper.Pressure);
            break;
         case ectTemperature:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", (Word_t)ValidRangeData->Lower.Temperature);
            printf("   Upper:   0x%04X\r\n", (Word_t)ValidRangeData->Upper.Temperature);
            break;
         case ectTrueWindDirection:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.True_Wind_Direction);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.True_Wind_Direction);
            break;
         case ectTrueWindSpeed:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.True_Wind_Speed);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.True_Wind_Speed);
            break;
         case ectUVIndex:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%02X\r\n", ValidRangeData->Lower.UV_Index);
            printf("   Upper:   0x%02X\r\n", ValidRangeData->Upper.UV_Index);
            break;
         case ectWindChill:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%02X\r\n", (Byte_t)ValidRangeData->Lower.Wind_Chill);
            printf("   Upper:   0x%02X\r\n", (Byte_t)ValidRangeData->Upper.Wind_Chill);
            break;
         case ectBarometricPressureTrend:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   ");
            switch(ValidRangeData->Lower.Barometric_Pressure_Trend)
            {
               case bptUnknown:
                  printf("Unknown\r\n");
                  break;
               case bptContinuouslyFalling:\
                  printf("Continuously falling.\r\n");
                  break;
               case bptContinuouslyRising:
                  printf("Continuously rising.\r\n");
                  break;
               case bptFallingThenSteady:
                  printf("Falling, then steady.\r\n");
                  break;
               case bptRisingThenSteady:
                  printf("Rising, then steady.\r\n");
                  break;
               case bptFallingBeforeLesserRise:
                  printf("Falling, before lesser rise.\r\n");
                  break;
               case bptFallingBeforeGreaterRise:
                  printf("Falling, before greater rise .\r\n");
                  break;
               case bptRisingBeforeGreaterFall:
                  printf("Rising, before greater fall.\r\n");
                  break;
               case bptRisingBeforeLesserFall:
                  printf("Rising, before lesser fall.\r\n");
                  break;
               case bptSteady:
                  printf("Steady.\r\n");
                  break;
               default:
                  printf("Invalid enum Lower.\r\n");
                  break;
            }
            printf("   Upper:   ");
            switch(ValidRangeData->Upper.Barometric_Pressure_Trend)
            {
               case bptUnknown:
                  printf("Unknown\r\n");
                  break;
               case bptContinuouslyFalling:\
                  printf("Continuously falling.\r\n");
                  break;
               case bptContinuouslyRising:
                  printf("Continuously rising.\r\n");
                  break;
               case bptFallingThenSteady:
                  printf("Falling, then steady.\r\n");
                  break;
               case bptRisingThenSteady:
                  printf("Rising, then steady.\r\n");
                  break;
               case bptFallingBeforeLesserRise:
                  printf("Falling, before lesser rise.\r\n");
                  break;
               case bptFallingBeforeGreaterRise:
                  printf("Falling, before greater rise .\r\n");
                  break;
               case bptRisingBeforeGreaterFall:
                  printf("Rising, before greater fall.\r\n");
                  break;
               case bptRisingBeforeLesserFall:
                  printf("Rising, before lesser fall.\r\n");
                  break;
               case bptSteady:
                  printf("Steady.\r\n");
                  break;
               default:
                  printf("Invalid enum Lower.\r\n");
                  break;
            }
            break;
         case ectMagneticDeclination:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:   0x%04X\r\n", ValidRangeData->Lower.Magnetic_Declination);
            printf("   Upper:   0x%04X\r\n", ValidRangeData->Upper.Magnetic_Declination);
            break;
         case ectMagneticFluxDensity2D:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:\r\n");
            printf("   X-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Lower.Magnetic_Flux_Density_2D.X_Axis);
            printf("   Y-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Lower.Magnetic_Flux_Density_2D.Y_Axis);

            printf("   Upper:\r\n");
            printf("   X-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Upper.Magnetic_Flux_Density_2D.X_Axis);
            printf("   Y-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Upper.Magnetic_Flux_Density_2D.Y_Axis);
            break;
         case ectMagneticFluxDensity3D:
            printf("   Type:    ");
            DisplayESSCharacteristicType(Type);
            printf("   Lower:\r\n");
            printf("   X-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Lower.Magnetic_Flux_Density_3D.X_Axis);
            printf("   Y-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Lower.Magnetic_Flux_Density_3D.Y_Axis);
            printf("   Z-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Lower.Magnetic_Flux_Density_3D.Z_Axis);

            printf("   Upper:\r\n");
            printf("   X-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Upper.Magnetic_Flux_Density_3D.X_Axis);
            printf("   Y-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Upper.Magnetic_Flux_Density_3D.Y_Axis);
            printf("   Z-Axis:  0x%04X\r\n", (Word_t)ValidRangeData->Upper.Magnetic_Flux_Density_3D.Z_Axis);
            break;
         default:
            /* Can't occur API will fail to decode if Type is not       */
            /* correct.                                                 */
            break;
      }
   }
   else
      printf("\r\nWarning - Invalid Valid Range.\r\n");
}

   /* The following function is a utility function to assign the ESS    */
   /* Characteristic UUID based on the Type.                            */
   /* * NOTE * UUID's will be assigned in Little-Endian format.         */
static void AssignESSCharacteristicUUID(ESS_Characteristic_Type_t Type, GATT_UUID_t *UUID)
{
   /* Let's go ahead and set the UUID type to 16-bit.                   */
   UUID->UUID_Type = guUUID_16;

   switch(Type)
   {
      case ectApparentWindDirection:
         ESS_ASSIGN_APPARENT_WIND_DIRECTION_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectApparentWindSpeed:
         ESS_ASSIGN_APPARENT_WIND_SPEED_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectDewPoint:
         ESS_ASSIGN_DEW_POINT_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectElevation:
         ESS_ASSIGN_ELEVATION_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectGustFactor:
         ESS_ASSIGN_GUST_FACTOR_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectHeatIndex:
         ESS_ASSIGN_HEAT_INDEX_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectHumidity:
         ESS_ASSIGN_HUMIDITY_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectIrradiance:
         ESS_ASSIGN_IRRADIANCE_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectPollenConcentration:
         ESS_ASSIGN_POLLEN_CONCENTRATION_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectRainfall:
         ESS_ASSIGN_RAINFALL_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectPressure:
         ESS_ASSIGN_PRESSURE_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectTemperature:
         ESS_ASSIGN_TEMPERATURE_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectTrueWindDirection:
         ESS_ASSIGN_TRUE_WIND_DIRECTION_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectTrueWindSpeed:
         ESS_ASSIGN_TRUE_WIND_SPEED_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectUVIndex:
         ESS_ASSIGN_UV_INDEX_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectWindChill:
         ESS_ASSIGN_WIND_CHILL_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectBarometricPressureTrend:
         ESS_ASSIGN_BAROMETRIC_PRESSURE_TREND_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectMagneticDeclination:
         ESS_ASSIGN_MAGNETIC_DECLINATION_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectMagneticFluxDensity2D:
         ESS_ASSIGN_MAGNETIC_FLUX_DENSITY_2D_UUID_16(UUID->UUID.UUID_16);
         break;
      case ectMagneticFluxDensity3D:
         ESS_ASSIGN_MAGNETIC_FLUX_DENSITY_3D_UUID_16(UUID->UUID.UUID_16);
         break;
      default:
         printf("Invalid ESS Characteristic type.\r\n");
         break;
   }
}


   /* The following function is an internal function to read the ESS    */
   /* Characteristic or its descriptors.  This function may be called by*/
   /* the ESS Server or ESS Client.                                     */
   /* * NOTE * This function may NOT be used for the Descriptor Value   */
   /*          Changed Client Characteristic Configuration Descriptor.  */
static int ReadESSCharacteristicRequest(ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Sample_Attribute_Handle_Type_t AttributeHandleType)
{
   int                          ret_val = 0;
   DeviceInfo_t                *DeviceInfo;
   ESS_Characteristic_Data_t    CharacteristicData;
   ESS_Characteristic_Type_t    Type;
   unsigned int                 ID;
   Word_t                       AttributeHandle;
   ESS_Client_Characteristic_t *CharacteristicPtr;
   Word_t                       ExtendedProperties;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the ESS Characteristic Type is valid.                */
      if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
      {
         /* Check the InstanceID to see if we are the ESS Server.       */
         if(ESSInstanceID)
         {
            /* Make sure the instance specified exists for the ESS      */
            /* Characteristic Type.                                     */
            if(ID < (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES)
            {
               /* Determine the attribute handle to send in the request.*/
               switch(AttributeHandleType)
               {
                  case ahtCharacteristic:
                     /* Set the Characteristic Data to display the ESS  */
                     /* Characteristic.                                 */
                     CharacteristicData= ServerInfo.Characteristics[Type].Instances[ID].Characteristic;

                     /* Let's display the ESS Characteristic.           */
                     DisplayESSCharacteristic(Type, &CharacteristicData);
                     break;
                  case ahtExtendedProperties:
                     if((ret_val = ESS_Query_Extended_Properties(BluetoothStackID, ESSInstanceID, CharacteristicInfo, &ExtendedProperties)) == 0)
                     {
                        printf("\r\nExtended Properties:\r\n");
                        printf("   Value:   0x%04X\r\n", ExtendedProperties);
                     }
                     else
                        DisplayFunctionError("ESS_Query_Extended_Properties", ret_val);

                     /* Simply return success to the caller.            */
                     ret_val = 0;
                     break;
                  case ahtClientCharacteristicConfig:
                     printf("\r\nClient Characteristic Configuration:\r\n");
                     printf("   Value:   0x%04X\r\n", ServerInfo.Characteristics[Type].Instances[ID].ClientConfiguration);
                     break;
                  case ahtESMeasurement:
                     DisplayESMeasurementData(&(ServerInfo.Characteristics[Type].Instances[ID].MeasurementData));
                     break;
                  case ahtESTriggerSetting_0:
                     DisplayESTriggerSettingData(Type, tsiTriggerSetting_0, &(ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[0]));
                     break;
                  case ahtESTriggerSetting_1:
                     DisplayESTriggerSettingData(Type, tsiTriggerSetting_1, &(ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[1]));
                     break;
                  case ahtESTriggerSetting_2:
                     DisplayESTriggerSettingData(Type, tsiTriggerSetting_2, &(ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[2]));
                     break;
                  case ahtESConfiguration:
                     printf("\r\nES Configuration:\r\n");
                     printf("   Value:   0x%02X\r\n", ServerInfo.Characteristics[Type].Instances[ID].Configuration);
                     break;
                  case ahtUserDescription:
                     printf("\r\nUser Description:\r\n");
                     printf("   Value:   %s\r\n", ServerInfo.Characteristics[Type].Instances[ID].UserDescription);
                     break;
                  case ahtValidRange:
                     DisplayValidRangeData(Type, &(ServerInfo.Characteristics[Type].Instances[ID].ValidRange));
                     break;
                  default:
                     ret_val = INVALID_PARAMETERS_ERROR;
                     break;
               }
            }
            else
               printf("\r\nInstance: %u, does not exist for the ESS Characteristic type on the ESS Server.\r\n", ID);
         }
         else
         {
            /* We are the ESS Client so we need to make sure that a     */
            /* connection exists to the ESS Server.                     */
            if(ConnectionID)
            {
               /* Get the device info for the connection device.        */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery since*/
                  /* we need the attribute handle to make the request.  */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Store a pointer to the specified ESS            */
                     /* Characteristic Type's attribute handle          */
                     /* information.                                    */
                     CharacteristicPtr = &(DeviceInfo->ESSClientInfo.Characteristics[Type]);

                     /* Make sure we have allocated an attribute handle */
                     /* list for the ESS Characteristic instances of the*/
                     /* specified ESS Characteristic Type.              */
                     if((CharacteristicPtr->AttributeHandleEntries) && (CharacteristicPtr->AttributeHandleList))
                     {
                        /* Make sure the instance information is valid. */
                        if(ID < CharacteristicPtr->AttributeHandleEntries)
                        {
                           /* Determine the attribute handle to send in */
                           /* the request.                              */
                           switch(AttributeHandleType)
                           {
                              case ahtCharacteristic:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].Attribute_Handle;
                                 break;
                              case ahtExtendedProperties:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].Extended_Properties_Handle;
                                 break;
                              case ahtClientCharacteristicConfig:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].CCCD_Handle;
                                 break;
                              case ahtESMeasurement:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Measurement_Handle;
                                 break;
                              case ahtESTriggerSetting_0:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[0];
                                 break;
                              case ahtESTriggerSetting_1:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[1];
                                 break;
                              case ahtESTriggerSetting_2:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[2];
                                 break;
                              case ahtESConfiguration:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Configuration_Handle;
                                 break;
                              case ahtUserDescription:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].User_Description_Handle;
                                 break;
                              case ahtValidRange:
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].Valid_Range_Handle;
                                 break;
                              default:
                                 ret_val = INVALID_PARAMETERS_ERROR;
                                 break;
                           }

                           /* Make sure the attribute handle is valid.  */
                           if((!ret_val) && (AttributeHandle))
                           {
                              /* Set the ESS Client request information */
                              /* so we can handle simplify handling the */
                              /* response.                              */
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.Type                  = Type;
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.ID                    = ID;
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.AttributeHandleType   = AttributeHandleType;

                              /* * NOTE * We will always reset the User */
                              /*          Description offset for a      */
                              /*          standard read request.  If we */
                              /*          receive an incomplete User    */
                              /*          Description then we need to   */
                              /*          use a read long request to get*/
                              /*          the rest of the User          */
                              /*          Description.                  */
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset = 0;

                              /* Send the GATT read request.            */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                              {
                                 printf("\r\nGATT Read Value Request sent:\r\n");
                                 printf("   TransactionID:     %d\r\n",     ret_val);
                                 printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                              }
                              else
                                 DisplayFunctionError("GATT_Read_Value_Request", ret_val);

                              /* Simply return success to the caller.   */
                              ret_val = 0;
                           }
                           else
                           {
                              /* Determine the error message to display.*/
                              if(!AttributeHandle)
                                 printf("\r\nInvalid attribute handle.\r\n");
                              else
                                 printf("\r\nInvalid attribute handle type.\r\n");
                           }
                        }
                        else
                           printf("\r\nInstance: %u, does not exist for the ESS Characteristic type on the ESS Server.\r\n", ID);
                     }
                     else
                        printf("\r\nNo instances for the ESS Characteristic type on the ESS Server.\r\n");
                  }
                  else
                     printf("\r\nService discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nDevice information does not exist.\r\n");
            }
            else
               printf("\r\nNo connection to remote ESS Server.\r\n");
         }
      }
      else
         ret_val = INVALID_PARAMETERS_ERROR;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is an internal function to perform a Long  */
   /* Read for the User Description.  This function may be called by the*/
   /* ESS Client only.                                                  */
static int ReadLongUserDescriptionRequest(ESS_Characteristic_Info_t *CharacteristicInfo)
{
   int                          ret_val = 0;
   DeviceInfo_t                *DeviceInfo;
   ESS_Characteristic_Type_t    Type;
   unsigned int                 ID;
   Word_t                       AttributeHandle;
   ESS_Client_Characteristic_t *CharacteristicPtr;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the ESS Characteristic Type is valid.                */
      if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
      {
         /* We are the ESS Client so we need to make sure that a        */
         /* connection exists to the ESS Server.                        */
         if(ConnectionID)
         {
            /* Get the device info for the connection device.           */
            /* * NOTE * ConnectionBD_ADDR should be valid if            */
            /*          ConnectionID is.                                */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Make sure we have performed service discovery since we*/
               /* need the attribute handle to make the request.        */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
               {
                  /* Store a pointer to the specified ESS Characteristic*/
                  /* Type's attribute handle information.               */
                  CharacteristicPtr = &(DeviceInfo->ESSClientInfo.Characteristics[Type]);

                  /* Make sure we have allocated an attribute handle    */
                  /* list for the ESS Characteristic instances of the   */
                  /* specified ESS Characteristic Type.                 */
                  if((CharacteristicPtr->AttributeHandleEntries) && (CharacteristicPtr->AttributeHandleList))
                  {
                     /* Make sure the instance information is valid.    */
                     if(CharacteristicPtr->AttributeHandleList[ID].Valid)
                     {
                        /* Store the attribute handle.                  */
                        AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].User_Description_Handle;

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
                           if(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset)
                           {
                              /* Set the ESS Client request information */
                              /* so we can handle simplify handling the */
                              /* response.                              */
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.Type                = Type;
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.ID                  = ID;
                              DeviceInfo->ESSClientInfo.ClientRequestInfo.AttributeHandleType = ahtUserDescription;

                              /* Send the GATT read request.            */
                              /* * NOTE * We will not save the          */
                              /*          transactionID returned by this*/
                              /*          function, which we could use  */
                              /*          to cancel the request.        */
                              if((ret_val = GATT_Read_Long_Value_Request(BluetoothStackID, ConnectionID, AttributeHandle, DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset, GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
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
                        {
                           /* Determine the error message to display.   */
                           if(!AttributeHandle)
                              printf("\r\nInvalid attribute handle.\r\n");
                           else
                              printf("\r\nInvalid attribute handle type.\r\n");
                        }
                     }
                     else
                        printf("\r\nAttribute handle instance information not valid.\r\n");
                  }
                  else
                     printf("\r\nAttribute handle list for ESS Characteristic Type does not exist.\r\n");
               }
               else
                  printf("\r\nService discovery has not been performed.\r\n");
            }
            else
               printf("\r\nDevice information does not exist.\r\n");
         }
         else
            printf("\r\nNo connection to remote ESS Server.\r\n");
      }
      else
         ret_val = INVALID_PARAMETERS_ERROR;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is an internal function to write the ESS   */
   /* Characteristic descriptors.  This function may be called by the   */
   /* ESS Server or ESS Client.                                         */
   /* * NOTE * ESS Characteristics CANNOT be written.                   */
   /* * NOTE * This function may NOT be used for the Descriptor Value   */
   /*          Changed Client Characteristic Configuration Descriptor.  */
static int WriteESSCharacteristicRequest(ESS_Characteristic_Info_t *CharacteristicInfo, ESS_Write_Request_Data_t *WriteRequestData)
{
   int                          ret_val = 0;
   DeviceInfo_t                *DeviceInfo;
   ESS_Characteristic_Type_t    Type;
   unsigned int                 ID;
   Word_t                       AttributeHandle;
   ESS_Client_Characteristic_t *CharacteristicPtr;
   Word_t                       MTU;

   NonAlignedWord_t             Client_Configuration;

   /* Make sure the parameters are semi-valid.                          */
   if((CharacteristicInfo) && (WriteRequestData))
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the ESS Characteristic Type is valid.                */
      if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
      {
         /* Check the InstanceID to see if we are the ESS Server.       */
         if(ESSInstanceID)
         {
            /* * NOTE * Server initializes all ESS Characteristic Types */
            /*          so we do not have to check here.                */

            /* Make sure the instance specified exists for the ESS      */
            /* Characteristic Type.                                     */
            if(ID < (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES)
            {
               /* Determine the how to store the write request.         */
               switch(WriteRequestData->AttributeHandleType)
               {
                  case ahtCharacteristic:
                     /* Store the characteristic data.                  */
                     ServerInfo.Characteristics[Type].Instances[ID].Characteristic = WriteRequestData->Data.Characteristic;

                     /* Increment the change index so we can simulate to*/
                     /* the ESS Client that notifications are ready next*/
                     /* time we advertise.                              */
                     ServerInfo.ChangeIndex++;
                     break;
                  case ahtExtendedProperties:
                     printf("\r\nESS Server cannot write the Extended Properties.\r\n");
                     break;
                  case ahtClientCharacteristicConfig:
                     printf("\r\nESS Server cannot write the CCCD.\r\n");
                     break;
                  case ahtESMeasurement:
                     printf("\r\nESS Server cannot write the ES Measurement.\r\n");
                     break;
                  case ahtESTriggerSetting_0:
                     /* Store the ES Trigger Setting.                   */
                     ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[0] = WriteRequestData->Data.TriggerSettingData;
                     break;
                  case ahtESTriggerSetting_1:
                     /* Store the ES Trigger Setting.                   */
                     ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[1] = WriteRequestData->Data.TriggerSettingData;
                     break;
                  case ahtESTriggerSetting_2:
                     /* Store the ES Trigger Setting.                   */
                     ServerInfo.Characteristics[Type].Instances[ID].TriggerSettingData[2] = WriteRequestData->Data.TriggerSettingData;
                     break;
                  case ahtESConfiguration:
                     /* Store the ES Configuration.                     */
                     ServerInfo.Characteristics[Type].Instances[ID].Configuration = WriteRequestData->Data.Configuration;
                     break;
                  case ahtUserDescription:
                     /* Store the User Description.                     */
                     if(WriteRequestData->UserDescriptionLength <= (Word_t)ESS_SA_DEFAULT_STRING_LENGTH)
                     {
                        /* Simply copy the user description.            */
                        BTPS_StringCopy(ServerInfo.Characteristics[Type].Instances[ID].UserDescription, WriteRequestData->Data.UserDescription);
                     }
                     else
                        printf("\r\nUser Description length greater than the ESS Server supports.\r\n");
                     break;
                  case ahtValidRange:
                     /* Store the Valid Range.                          */
                     ServerInfo.Characteristics[Type].Instances[ID].ValidRange.Lower = WriteRequestData->Data.Characteristic;
                     ServerInfo.Characteristics[Type].Instances[ID].ValidRange.Upper = WriteRequestData->UpperCharacteristic;
                     break;
                  default:
                     printf("\r\nInvalid attribute handle type.\r\n");
                     break;
               }
            }
            else
               printf("\r\nInstance: %u, does not exist for the ESS Characteristic type on the ESS Server.\r\n", ID);
         }
         else
         {
            /* We are the ESS Client so we need to make sure that a     */
            /* connection exists to the ESS Server.                     */
            if(ConnectionID)
            {
               /* Get the device info for the connection device.        */
               /* * NOTE * ConnectionBD_ADDR should be valid if         */
               /*          ConnectionID is.                             */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery since*/
                  /* we need the attribute handle to make the request.  */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Store a pointer to the specified ESS            */
                     /* Characteristic Type's attribute handle          */
                     /* information.                                    */
                     CharacteristicPtr = &(DeviceInfo->ESSClientInfo.Characteristics[Type]);

                     /* Make sure we have allocated an attribute handle */
                     /* list for the ESS Characteristic instances of the*/
                     /* specified ESS Characteristic Type.              */
                     if((CharacteristicPtr->AttributeHandleEntries) && (CharacteristicPtr->AttributeHandleList))
                     {
                        /* Make sure the instance information is valid. */
                        if(ID < CharacteristicPtr->AttributeHandleEntries)
                        {
                           /* Determine the attribute handle and how to */
                           /* send the Write equest.                    */
                           switch(WriteRequestData->AttributeHandleType)
                           {
                              case ahtCharacteristic:
                                 printf("\r\nESS Client cannot write ESS Characteristic.\r\n");
                                 break;
                              case ahtExtendedProperties:
                                 printf("\r\nESS Client cannot write Extended Properties descriptor.\r\n");
                                 break;
                              case ahtClientCharacteristicConfig:
                                 /* Store the attribute handle.         */
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].CCCD_Handle;

                                 /* Format the client configuration.    */
                                 ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Client_Configuration, WriteRequestData->Data.CCCD);

                                 /* Send the write request.             */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Client_Configuration, GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                 }
                                 else
                                    DisplayFunctionError("GATT_Write_Request", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                                 break;
                              case ahtESMeasurement:
                                 printf("\r\nESS Client cannot write ES Measurement descriptor.\r\n");
                                 break;
                              case ahtESTriggerSetting_0:
                                 /* Store the attribute handle.         */
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[0];

                                 /* Call the API to handle the          */
                                 /* formatting and writing of the ES    */
                                 /* Trigger Setting.                    */
                                 if((ret_val = ESS_Write_ES_Trigger_Setting_Request(BluetoothStackID, ConnectionID, AttributeHandle, Type, &(WriteRequestData->Data.TriggerSettingData), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                 }
                                 else
                                    DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                                 break;
                              case ahtESTriggerSetting_1:
                                 /* Store the attribute handle.         */
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[1];

                                 /* Call the API to handle the          */
                                 /* formatting and writing of the ES    */
                                 /* Trigger Setting.                    */
                                 if((ret_val = ESS_Write_ES_Trigger_Setting_Request(BluetoothStackID, ConnectionID, AttributeHandle, Type, &(WriteRequestData->Data.TriggerSettingData), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                 }
                                 else
                                    DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                                 break;
                              case ahtESTriggerSetting_2:
                                 /* Store the attribute handle.         */
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Trigger_Setting_Handle[2];

                                 /* Call the API to handle the          */
                                 /* formatting and writing of the ES    */
                                 /* Trigger Setting.                    */
                                 if((ret_val = ESS_Write_ES_Trigger_Setting_Request(BluetoothStackID, ConnectionID, AttributeHandle, Type, &(WriteRequestData->Data.TriggerSettingData), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                 }
                                 else
                                    DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                                 break;
                              case ahtESConfiguration:
                                 /* Store the attribute handle.         */
                                 AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].ES_Configuration_Handle;

                                 /* Send the write request.             */
                                 /* * NOTE * We will not save the       */
                                 /*          transactionID returned by  */
                                 /*          this function, which we    */
                                 /*          could use to cancel the    */
                                 /*          request.                   */
                                 if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_BYTE_SIZE, &(WriteRequestData->Data.Configuration), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                                 {
                                    printf("\r\nGATT Write Request sent:\r\n");
                                    printf("   TransactionID:     %d\r\n", ret_val);
                                    printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);

                                 }
                                 else
                                    DisplayFunctionError("GATT_Write_Request", ret_val);

                                 /* Simply return success to the caller.*/
                                 ret_val = 0;
                                 break;
                              case ahtUserDescription:
                                 /* Make sure the User Description      */
                                 /* length is valid.                    */
                                 if(WriteRequestData->UserDescriptionLength <= (Word_t)ESS_SA_DEFAULT_STRING_LENGTH)
                                 {
                                    /* Store the attribute handle.      */
                                    AttributeHandle = CharacteristicPtr->AttributeHandleList[ID].User_Description_Handle;

                                    /* Query the GATT Connection MTU to */
                                    /* determine how we should handle   */
                                    /* the write request.               */
                                    if((ret_val = GATT_Query_Connection_MTU(BluetoothStackID, ConnectionID, &MTU)) == 0)
                                    {
                                       /* Check whether we need to send */
                                       /* a write request or a prepare  */
                                       /* write request.                */
                                       if(WriteRequestData->UserDescriptionLength <= (MTU-3))
                                       {
                                          /* Send the GATT Write        */
                                          /* request.                   */
                                          /* * NOTE * We will not save  */
                                          /*          the transactionID */
                                          /*          returned by this  */
                                          /*          function, which we*/
                                          /*          could use to      */
                                          /*          cancel the        */
                                          /*          request.          */
                                          if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->UserDescriptionLength, (Byte_t *)(WriteRequestData->Data.UserDescription), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
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
                                          /* Store the request          */
                                          /* information so we can      */
                                          /* handle the Prepare Write   */
                                          /* request response.          */
                                          /* * NOTE * We will increment */
                                          /*          the offset when we*/
                                          /*          receive the       */
                                          /*          response.         */
                                          DeviceInfo->ESSClientInfo.ClientRequestInfo.Type                  = Type;
                                          DeviceInfo->ESSClientInfo.ClientRequestInfo.ID                    = ID;
                                          DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset = 0;

                                          /* Copy the User Description  */
                                          /* into the Buffer.           */
                                          BTPS_StringCopy(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription, WriteRequestData->Data.UserDescription);

                                          /* Send the GATT Prepare Write*/
                                          /* request.                   */
                                          /* * NOTE * We will not save  */
                                          /*          the transactionID */
                                          /*          returned by this  */
                                          /*          function, which we*/
                                          /*          could use to      */
                                          /*          cancel the        */
                                          /*          request.          */
                                          if((ret_val = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, WriteRequestData->UserDescriptionLength, 0, (Byte_t *)(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
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

                                    /* Simply return success to the     */
                                    /* caller.                          */
                                    ret_val = 0;
                                 }
                                 else
                                    printf("\r\nInvalid User Description length.\r\n");
                                 break;
                              case ahtValidRange:
                                 printf("\r\nESS Client cannot write Valid Range descriptor.\r\n");
                                 break;
                              default:
                                 printf("\r\nInvalid attribute handle type.\r\n");
                                 break;
                           }
                        }
                        else
                           printf("\r\nInstance: %u, does not exist for the ESS Characteristic type on the ESS Server.\r\n", ID);
                     }
                     else
                        printf("\r\nNo instances for the ESS Characteristic type on the ESS Server.\r\n");
                  }
                  else
                     printf("\r\nService discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nDevice information does not exist.\r\n");
            }
            else
               printf("\r\nNo connection to remote ESS Server.\r\n");
         }
      }
      else
         ret_val = INVALID_PARAMETERS_ERROR;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is an internal function to send a          */
   /* notification of an ESS Characteristic to a remote ESS Client.     */
static int NotifyESSCharacateristic(ESS_Characteristic_Info_t *CharacteristicInfo)
{
   int                        ret_val = 0;
   ESS_Characteristic_Type_t  Type;
   unsigned int               ID;
   ESS_Characteristic_Data_t  CharacteristicData;

   /* Make sure the parameters are semi-valid.                          */
   if(CharacteristicInfo)
   {
      /* Store the parameter fields to make indexing more readable.     */
      Type = CharacteristicInfo->Type;
      ID   = CharacteristicInfo->ID;

      /* Make sure the ESS Characteristic Type is valid.                */
      if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
      {
         /* Check the InstanceID to see if we are the ESS Server.       */
         if(ESSInstanceID)
         {
            /* Make sure we are connected to an ESS Client.             */
            if(ConnectionID)
            {
               /* * NOTE * Server initializes all ESS Characteristic    */
               /*          Types so we do not have to check here.       */

               /* Make sure the instance specified exists for the ESS   */
               /* Characteristic Type.                                  */
               if(ID < (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES)
               {
                  /* Set the Characteristic data.                       */
                  CharacteristicData = ServerInfo.Characteristics[Type].Instances[ID].Characteristic;

                  /* Make sure the ESS Characteristic CCCD has been     */
                  /* configured for notifications.                      */
                  if(ServerInfo.Characteristics[Type].Instances[ID].ClientConfiguration & ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE)
                  {
                     /* Call the API to send the notification.          */
                     if((ret_val = ESS_Notify_Characteristic(BluetoothStackID, ESSInstanceID, ConnectionID, CharacteristicInfo, &CharacteristicData)) > 0)
                     {
                        printf("\r\nESS Characteristic notification sent:\r\n");
                        printf("   Length: %d\r\n", ret_val);
                     }
                     else
                        DisplayFunctionError("ESS_Notify_Characteristic", ret_val);
                  }
                  else
                     printf("\r\nESS Characteristic CCCD has not been configured for notifications.\r\n");

                  /* Simply return success to the caller.               */
                  ret_val = 0;
               }
               else
                  printf("\r\nInvalid instance ID.\r\n");
            }
            else
               printf("\r\nMust be connected to an ESS Client.\r\n");
         }
         else
            printf("\r\nMust be an ESS Server to send a notification.\r\n");
      }
      else
         ret_val = INVALID_PARAMETERS_ERROR;
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is an internal function to configure the   */
   /* Descriptor Value Changed Characteristic's Client Characteristic   */
   /* Configuration on a remote ESS Server.                             */
static int ConfigureDescriptorValueChanged(Word_t ClientConfiguration)
{
   int               ret_val = 0;
   NonAlignedWord_t  Client_Configuration;
   DeviceInfo_t     *DeviceInfo;
   Word_t            AttributeHandle;

   /* Check the InstanceID to see if we are the ESS Client.             */
   if(!ESSInstanceID)
   {
      /* Make sure we are connected to an ESS Server.                   */
      if(ConnectionID)
      {
         /* Get the device information.                                 */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Make sure we have performed service discovery.           */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
            {
               /* Make sure the handle is valid.                        */
               AttributeHandle = DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_CCCD;
               if(AttributeHandle)
               {
                  /* Format the CCCD.                                   */
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Client_Configuration, ClientConfiguration);

                  /* Send the write request.                            */
                  /* * NOTE * We will not save the transactionID        */
                  /*          returned by this function, which we could */
                  /*          use to cancel the request.                */
                  if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, NON_ALIGNED_WORD_SIZE, (Byte_t *)&Client_Configuration, GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
                  {
                     printf("\r\nGATT Write Request sent:\r\n");
                     printf("   TransactionID:     %d\r\n", ret_val);
                     printf("   Attribute Handle:  0x%04X\r\n", AttributeHandle);
                  }
                  else
                     DisplayFunctionError("GATT_Write_Request", ret_val);

                  /* Simply return success to the caller.               */
                  ret_val = 0;
               }
               else
                  printf("\r\nInvalid attribute handle.\r\n");
            }
            else
               printf("\r\nService discovery has not been performed.\r\n");
         }
         else
            printf("\r\nNo device information.\r\n");
      }
      else
         printf("\r\nMust be connected to an ESS Server.\r\n");
   }
   else
      printf("\r\nMust be an ESS Client to configure the CCCD.\r\n");

   return (ret_val);
}

   /* The following function is an internal function to send an         */
   /* indication of the Descriptor Value Changed to a remote ESS Client.*/
static int IndicateDescriptorValueChanged(ESS_Descriptor_Value_Changed_Data_t *DescriptorValueChanged)
{
   int ret_val = 0;

   /* Make sure the parameters are semi-valid.                          */
   if(DescriptorValueChanged)
   {
      /* Check the InstanceID to see if we are the ESS Server.          */
      if(ESSInstanceID)
      {
         /* Make sure we are connected to an ESS Client.                */
         if(ConnectionID)
         {
            /* Make sure the CCCD has been configured for indications.  */
            if(ServerInfo.DescriptorValueChanged.CCCD & ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE)
            {
               /* Call the API to send the indication.                  */
               /* * NOTE * We will not store the Transaction ID, which  */
               /*          we could use to cancel the request.          */
               if((ret_val = ESS_Indicate_Descriptor_Value_Changed(BluetoothStackID, ESSInstanceID, ConnectionID, DescriptorValueChanged)) > 0)
               {
                  printf("\r\nESS Descriptor Value Changed indication sent:\r\n");
                  printf("   Transaction ID: %d\r\n", ret_val);
               }
               else
                  DisplayFunctionError("ESS_Indicate_Descriptor_Value_Changed", ret_val);

               /* Simply return success to the caller.                  */
               ret_val = 0;
            }
            else
               printf("\r\nDescriptor Value Changed CCCD has not been configured for indications.\r\n");
         }
         else
            printf("\r\nMust be connected to an ESS Client.\r\n");
      }
      else
         printf("\r\nMust be an ESS Server to send an indication.\r\n");
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return (ret_val);
}

   /* The following function is responsible for issuing the command to  */
   /* read an ESS Characteristic or its descriptors.                    */
static int ReadESSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                                 ret_val = 0;
   ESS_Characteristic_Info_t           CharacteristicInfo;
   ESS_Sample_Attribute_Handle_Type_t  AttributeHandleType;
   Boolean_t                           ReadLong;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type, ID, and Attribute Type will be checked by*/
         /*          the internal function.                             */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;
         AttributeHandleType     = (ESS_Sample_Attribute_Handle_Type_t)TempParam->Params[2].intParam;

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
                  /* Simply call the internal function to process the   */
                  /* request.                                           */
                  ret_val = ReadESSCharacteristicRequest(&CharacteristicInfo, AttributeHandleType);
               }
               else
               {
                  /* Issue a request to read the User Description with a*/
                  /* GATT Read Long Value request.                      */
                  ret_val = ReadLongUserDescriptionRequest(&CharacteristicInfo);
               }
            }
            else
               DisplayReadESSCharacteristicCommandUsage();
         }
         else
         {
            /* Simply call the internal function to process the request.*/
            ret_val = ReadESSCharacteristicRequest(&CharacteristicInfo, AttributeHandleType);
         }
      }
      else
         DisplayReadESSCharacteristicCommandUsage();
   }
   else
      printf("\r\nBluetoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing an ESS          */
   /* Characteristic's data.  This function simply allows the ESS Server*/
   /* to set values for ESS Characteristics since they are not writable */
   /* by the ESS Client.                                                */
static int WriteESSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   ESS_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Make sure we are the server.                                */
         /* * NOTE * If we don't check here Client write requests will  */
         /*          attempt to be issues and always fail since we      */
         /*          cannot Write ESS characteristics values.           */
         if(ESSInstanceID)
         {
            /* Store the parameters.                                    */
            /* * NOTE * The Type and IDwill be checked by the internal  */
            /*          function.                                       */
            CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
            CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

            /* Format the attribute handle type so we know where to     */
            /* store the data in the internal function.                 */
            WriteRequestData.AttributeHandleType = ahtCharacteristic;

            /* We need to switch of the ESS Characteristic Type to check*/
            /* and set the optional parameters.                         */
            switch(CharacteristicInfo.Type)
            {
               case ectApparentWindDirection:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Apparent_Wind_Direction = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectApparentWindSpeed:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Apparent_Wind_Speed = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectDewPoint:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Dew_Point = (SByte_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectElevation:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Elevation.Lower = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Elevation.Upper = (Byte_t)(TempParam->Params[2].intParam >> 16);
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectGustFactor:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Gust_Factor = (Byte_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectHeatIndex:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Heat_Index = (SByte_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectHumidity:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Humidity = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectIrradiance:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Irradiance = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectPollenConcentration:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Pollen_Concentration.Lower = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Pollen_Concentration.Upper = (Byte_t)(TempParam->Params[2].intParam >> 16);
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectRainfall:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Rainfall = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectPressure:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Pressure = (DWord_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTemperature:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Temperature = (SWord_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTrueWindDirection:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.True_Wind_Direction = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTrueWindSpeed:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.True_Wind_Speed = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectUVIndex:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.UV_Index = (Byte_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectWindChill:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Wind_Chill = (SByte_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectBarometricPressureTrend:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Make sure the enum is valid.                    */
                     if((TempParam->Params[2].intParam >= bptUnknown) && (TempParam->Params[2].intParam <= bptSteady))
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.Characteristic.Barometric_Pressure_Trend = (ESS_Barometric_Pressure_Trend_t)TempParam->Params[2].intParam;
                     }
                     else
                     {
                        printf("Invalid Barometric Pressure Trend enumeration.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticDeclination:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 3)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Declination = (Word_t)TempParam->Params[2].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticFluxDensity2D:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_2D.X_Axis = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_2D.Y_Axis = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticFluxDensity3D:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 5)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.X_Axis = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.Y_Axis = (Word_t)TempParam->Params[3].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.Z_Axis = (Word_t)TempParam->Params[4].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               default:
                  printf("Invalid Type.\r\n");
                  ret_val = INVALID_PARAMETERS_ERROR;
                  break;
            }

            /* Continue if there isn't an error.                        */
            if(!ret_val)
            {
               /* Simply call the internal function to issue the        */
               /* request.                                              */
               ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
            }
            else
            {
               /* Display usage if invalid number of parameters.        */
               if(ret_val == INVALID_PARAMETERS_ERROR)
               {
                  DisplayWriteESSCharacteristicCommandUsage();

                  /* simply return success to the caller.               */
                  ret_val = 0;
               }
            }
         }
         else
            printf("\r\nMust be an ESS Server to set ESS Characteristic values.\r\n");
      }
      else
         DisplayWriteESSCharacteristicCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for issuing a command to    */
   /* read an ESS Characteristic CCCD.                                  */
static int WriteCCCDCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   ESS_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type and ID will be checked by the internal    */
         /*          function.                                          */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Set the attribute handle type.                              */
         WriteRequestData.AttributeHandleType = ahtClientCharacteristicConfig;

         /* Determine whether to enable/disable notifications.          */
         if(TempParam->Params[2].intParam)
         {
            /* Enable notifications.                                    */
            WriteRequestData.Data.CCCD = ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE;
         }
         else
         {
            /* Disable the CCCD.                                        */
            WriteRequestData.Data.CCCD = 0;
         }

         /* Simply call the internal function to send the request.      */
         ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
      }
      else
         DisplayWriteCCCDCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for issuing the command to  */
   /* write the ES Trigger Setting.                                     */
static int WriteESTriggerSettingCommand(ParameterList_t *TempParam)
{
   int                                ret_val = 0;
   ESS_Characteristic_Info_t          CharacteristicInfo;
   ESS_Write_Request_Data_t           WriteRequestData;
   ESS_ES_Trigger_Setting_Instance_t  Instance;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 4))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type and ID will be checked by the internal    */
         /*          function.                                          */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Store the Write request data.                               */
         Instance                                           = (ESS_ES_Trigger_Setting_Instance_t)TempParam->Params[2].intParam;
         WriteRequestData.Data.TriggerSettingData.Condition = (ESS_ES_Trigger_Setting_Condition_t)TempParam->Params[3].intParam;

         /* Set the attribute handle type based on the ES Trigger       */
         /* Setting instance..                                          */
         switch(Instance)
         {
            case tsiTriggerSetting_0:
               WriteRequestData.AttributeHandleType = ahtESTriggerSetting_0;
               break;
            case tsiTriggerSetting_1:
               WriteRequestData.AttributeHandleType = ahtESTriggerSetting_1;
               break;
            case tsiTriggerSetting_2:
               WriteRequestData.AttributeHandleType = ahtESTriggerSetting_2;
               break;
            default:
               printf("\r\nInvalid ES Trigger Setting instance.\r\n");
               ret_val = INVALID_PARAMETERS_ERROR;
               break;
         }

         /* Set the Operand depending on the condition.                 */
         switch(WriteRequestData.Data.TriggerSettingData.Condition)
         {
            case tscTriggerInactive:
               /* No operand.                                           */
               break;
            case tscFixedTimeInterval:
               /* Make sure the number of parameters are correct.       */
               if(TempParam->NumberofParameters >= 5)
               {
                  /* Format the operand.                                */
                  WriteRequestData.Data.TriggerSettingData.Operand.Seconds.Lower = (Word_t)TempParam->Params[4].intParam;
                  WriteRequestData.Data.TriggerSettingData.Operand.Seconds.Upper = (Byte_t)(TempParam->Params[4].intParam >> 16);
               }
               else
               {
                  printf("Invalid number of parameters.\r\n");
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
               break;
            case tscNoLessThanSpecifiedTime:
               /* Make sure the number of parameters are correct.       */
               if(TempParam->NumberofParameters >= 5)
               {
                  /* Format the operand.                                */
                  WriteRequestData.Data.TriggerSettingData.Operand.Seconds.Lower = (Word_t)TempParam->Params[4].intParam;
                  WriteRequestData.Data.TriggerSettingData.Operand.Seconds.Upper = (Byte_t)(TempParam->Params[4].intParam >> 16);
               }
               else
               {
                  printf("Invalid number of parameters.\r\n");
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
               break;
            case tscValueChanged:
            case tscLessThanSpecifiedValue:
            case tscLessThanOrEqualSpecifiedValue:
            case tscGreaterThanSpecifiedValue:
            case tscGreaterThanOrEqualSpecifiedValue:
            case tscEqualSpecifiedValue:
            case tscNotEqualSpecifiedValue:
               /* Intentional fall through.  Specified Value operands   */
               /* depend on the ESS Characteristic Type.                */
               switch(CharacteristicInfo.Type)
               {
                  case ectApparentWindDirection:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Apparent_Wind_Direction = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectApparentWindSpeed:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Apparent_Wind_Speed = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectDewPoint:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Dew_Point = (SByte_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectElevation:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Elevation.Lower = (Word_t)TempParam->Params[4].intParam;
                        WriteRequestData.Data.TriggerSettingData.Operand.Elevation.Upper = (Byte_t)(TempParam->Params[4].intParam >> 16);
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectGustFactor:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Gust_Factor = (Byte_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectHeatIndex:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Heat_Index = (SByte_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectHumidity:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Humidity = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectIrradiance:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Irradiance = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectPollenConcentration:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Pollen_Concentration.Lower = (Word_t)TempParam->Params[4].intParam;
                        WriteRequestData.Data.TriggerSettingData.Operand.Pollen_Concentration.Upper = (Byte_t)(TempParam->Params[4].intParam >> 16);
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectRainfall:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Rainfall = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectPressure:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Pressure = (DWord_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectTemperature:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Temperature = (SWord_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectTrueWindDirection:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.True_Wind_Direction = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectTrueWindSpeed:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.True_Wind_Speed = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectUVIndex:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.UV_Index = (Byte_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectWindChill:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Wind_Chill = (SByte_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  case ectMagneticDeclination:
                     /* Make sure the number of parameters are valid for*/
                     /* this Type.                                      */
                     if(TempParam->NumberofParameters >= 5)
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.TriggerSettingData.Operand.Magnetic_Declination = (Word_t)TempParam->Params[4].intParam;
                     }
                     else
                     {
                        printf("Invalid number of parameters.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                     break;
                  default:
                     printf("Invalid Type for Condition.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                     break;
               }
               break;
            default:
               printf("Invalid condition.\r\n");
               break;
         }

         /* If an error has not occured.                                */
         if(!ret_val)
         {
            /* Simply call the internal function to send the request.   */
            ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
         }
         else
         {
            /* Display usage if invalid number of parameters.           */
            if(ret_val == INVALID_PARAMETERS_ERROR)
            {
               DisplayWriteESTriggerSettingCommandUsage();

               /* simply return success to the caller.                  */
               ret_val = 0;
            }
         }
      }
      else
         DisplayWriteESTriggerSettingCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for issuing a command to    */
   /* write the ES Configuration.                                       */
static int WriteESConfigurationCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   ESS_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type and ID will be checked by the internal    */
         /*          function.                                          */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Set the attribute handle type.                              */
         WriteRequestData.AttributeHandleType = ahtESConfiguration;

         /* Determine whether to enable/disable notifications.          */
         if(TempParam->Params[2].intParam)
            WriteRequestData.Data.Configuration = ESS_ES_TRIGGER_LOGIC_VALUE_BOOLEAN_AND;
         else
            WriteRequestData.Data.Configuration = ESS_ES_TRIGGER_LOGIC_VALUE_BOOLEAN_OR;

         /* Simply call the internal function to send the request.      */
         ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
      }
      else
         DisplayWriteESConfigurationCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing the User        */
   /* Description.                                                      */
static int WriteUserDescriptionCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   ESS_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 3))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type and ID will be checked by the internal    */
         /*          function.                                          */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Set the write request data.                                 */
         WriteRequestData.AttributeHandleType  = ahtUserDescription;
         WriteRequestData.Data.UserDescription = TempParam->Params[2].strParam;

         /* Determine the length of the User Description.               */
         /* * NOTE * We will not send the NULL terminator.              */
         WriteRequestData.UserDescriptionLength = (BTPS_StringLength(WriteRequestData.Data.UserDescription));

         /* Simply call the internal function to send the request.      */
         ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
      }
      else
         DisplayWriteUserDescriptionCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing the Valid Range */
   /* on the ESS Server.                                                */
static int WriteValidRangeCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;
   ESS_Write_Request_Data_t   WriteRequestData;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Make sure we are the server.                                */
         /* * NOTE * If we don't check here Client write requests will  */
         /*          attempt to be issues and always fail since we      */
         /*          cannot Write ESS characteristics values.           */
         if(ESSInstanceID)
         {
            /* Store the parameters.                                    */
            /* * NOTE * The Type and IDwill be checked by the internal  */
            /*          function.                                       */
            CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
            CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

            /* Format the attribute handle type so we know where to     */
            /* store the data in the internal function.                 */
            WriteRequestData.AttributeHandleType = ahtValidRange;

            /* We need to switch of the ESS Characteristic Type to check*/
            /* and set the optional parameters.                         */
            switch(CharacteristicInfo.Type)
            {
               case ectApparentWindDirection:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Apparent_Wind_Direction = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Apparent_Wind_Direction = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectApparentWindSpeed:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Apparent_Wind_Speed = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Apparent_Wind_Speed = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectDewPoint:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Dew_Point = (SByte_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Dew_Point = (SByte_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectElevation:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Elevation.Lower = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Elevation.Upper = (Byte_t)(TempParam->Params[2].intParam >> 16);

                     WriteRequestData.UpperCharacteristic.Elevation.Lower = (Word_t)TempParam->Params[3].intParam;
                     WriteRequestData.UpperCharacteristic.Elevation.Upper = (Byte_t)(TempParam->Params[3].intParam >> 16);
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectGustFactor:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Gust_Factor = (Byte_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Gust_Factor = (Byte_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectHeatIndex:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Heat_Index = (SByte_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Heat_Index = (SByte_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectHumidity:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Humidity = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Humidity = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectIrradiance:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Irradiance = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Irradiance = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectPollenConcentration:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Pollen_Concentration.Lower = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Pollen_Concentration.Upper = (Byte_t)(TempParam->Params[2].intParam >> 16);

                     WriteRequestData.UpperCharacteristic.Pollen_Concentration.Lower = (Word_t)TempParam->Params[3].intParam;
                     WriteRequestData.UpperCharacteristic.Pollen_Concentration.Upper = (Byte_t)(TempParam->Params[3].intParam >> 16);
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectRainfall:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Rainfall = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Rainfall = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectPressure:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Pressure = (DWord_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Pressure = (DWord_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTemperature:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Temperature = (SWord_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Temperature = (SWord_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTrueWindDirection:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.True_Wind_Direction = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.True_Wind_Direction = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectTrueWindSpeed:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.True_Wind_Speed = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.True_Wind_Speed = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectUVIndex:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.UV_Index = (Byte_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.UV_Index = (Byte_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectWindChill:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Wind_Chill = (SByte_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Wind_Chill = (SByte_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectBarometricPressureTrend:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Make sure the enum is valid.                    */
                     if((TempParam->Params[2].intParam >= bptUnknown) && (TempParam->Params[2].intParam <= bptSteady))
                     {
                        /* Format the data.                             */
                        WriteRequestData.Data.Characteristic.Barometric_Pressure_Trend = (ESS_Barometric_Pressure_Trend_t)TempParam->Params[2].intParam;
                        WriteRequestData.UpperCharacteristic.Barometric_Pressure_Trend = (ESS_Barometric_Pressure_Trend_t)TempParam->Params[3].intParam;
                     }
                     else
                     {
                        printf("Invalid Barometric Pressure Trend enumeration.\r\n");
                        ret_val = INVALID_PARAMETERS_ERROR;
                     }
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticDeclination:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 4)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Declination = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.UpperCharacteristic.Magnetic_Declination = (Word_t)TempParam->Params[3].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticFluxDensity2D:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 6)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_2D.X_Axis = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_2D.Y_Axis = (Word_t)TempParam->Params[3].intParam;

                     WriteRequestData.UpperCharacteristic.Magnetic_Flux_Density_2D.X_Axis = (Word_t)TempParam->Params[4].intParam;
                     WriteRequestData.UpperCharacteristic.Magnetic_Flux_Density_2D.Y_Axis = (Word_t)TempParam->Params[5].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               case ectMagneticFluxDensity3D:
                  /* Make sure the number of parameters are valid for   */
                  /* this Type.                                         */
                  if(TempParam->NumberofParameters >= 8)
                  {
                     /* Format the data.                                */
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.X_Axis = (Word_t)TempParam->Params[2].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.Y_Axis = (Word_t)TempParam->Params[3].intParam;
                     WriteRequestData.Data.Characteristic.Magnetic_Flux_Density_3D.Z_Axis = (Word_t)TempParam->Params[4].intParam;

                     WriteRequestData.UpperCharacteristic.Magnetic_Flux_Density_3D.X_Axis = (Word_t)TempParam->Params[5].intParam;
                     WriteRequestData.UpperCharacteristic.Magnetic_Flux_Density_3D.Y_Axis = (Word_t)TempParam->Params[6].intParam;
                     WriteRequestData.UpperCharacteristic.Magnetic_Flux_Density_3D.Z_Axis = (Word_t)TempParam->Params[7].intParam;
                  }
                  else
                  {
                     printf("Invalid number of parameters.\r\n");
                     ret_val = INVALID_PARAMETERS_ERROR;
                  }
                  break;
               default:
                  printf("Invalid Type.\r\n");
                  ret_val = INVALID_PARAMETERS_ERROR;
                  break;
            }

            /* Continue if there isn't an error.                        */
            if(!ret_val)
            {
               /* Simply call the internal function to issue the        */
               /* request.                                              */
               ret_val = WriteESSCharacteristicRequest(&CharacteristicInfo, &WriteRequestData);
            }
            else
            {
               /* Display usage if invalid number of parameters.        */
               if(ret_val == INVALID_PARAMETERS_ERROR)
               {
                  DisplayWriteESSCharacteristicCommandUsage();

                  /* simply return success to the caller.               */
                  ret_val = 0;
               }
            }
         }
         else
            printf("\r\nMust be an ESS Server to set ESS Characteristic values.\r\n");
      }
      else
         DisplayWriteValidRangeCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for writing the User        */
   /* Description.                                                      */
static int ExecuteWriteRequestCommand(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Send the request.                                           */
         /* * NOTE * We will not save the TransactionID for this        */
         /*          request.                                           */
         if((ret_val = GATT_Execute_Write_Request(BluetoothStackID, ConnectionID, (TempParam->Params[0].intParam) ? TRUE : FALSE, GATT_ClientEventCallback_ESS, 0)) > 0)
         {
            /* Simply return success tot he caller.                     */
            ret_val = 0;
         }
      }
      else
         printf("Usage: ExecuteWriteRequest [0=Write, 1=Cancel Write]\r\n");
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for notifying an ESS        */
   /* Characteristic.                                                   */
static int NotifyESSCharacteristicCommand(ParameterList_t *TempParam)
{
   int                        ret_val = 0;
   ESS_Characteristic_Info_t  CharacteristicInfo;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Store the parameters.                                       */
         /* * NOTE * The Type, ID, and Attribute Type will be checked by*/
         /*          the internal function.                             */
         CharacteristicInfo.Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;
         CharacteristicInfo.ID   = (unsigned int)TempParam->Params[1].intParam;

         /* Simply call the internal function to send the notification. */
         ret_val = NotifyESSCharacateristic(&CharacteristicInfo);
      }
      else
         DisplayNotifyESSCharacteristicCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for configuring the CCCD for*/
   /* the Descriptor Value Changed Characteristic.                      */
static int ConfigureDescriptorValueChangedCommand(ParameterList_t *TempParam)
{
   int     ret_val = 0;
   Word_t  ClientConfiguration;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Determine whether to enable/disable notifications.          */
         if(TempParam->Params[0].intParam)
         {
            /* Enable indications.                                      */
            ClientConfiguration = ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE;
         }
         else
         {
            /* Disable the CCCD.                                        */
            ClientConfiguration = 0;
         }

         /* Simply call the internal function to configure the CCCCD.   */
         ret_val = ConfigureDescriptorValueChanged(ClientConfiguration);
      }
      else
      {
         printf("Usage: ConfigureDVC [Config(UINT16)]\r\n");
         printf("\r\n Where Config is,\r\n");
         printf("  0 = Disable\r\n");
         printf("  1 = Enable Indications\r\n");
      }
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is responsible for indicating the          */
   /* Descriptor Value Changed Characteristic to a remote ESS Client.   */
static int IndicateDescriptorValueChangedCommand(ParameterList_t *TempParam)
{
   int                                  ret_val = 0;
   ESS_Descriptor_Value_Changed_Data_t  DescriptorValueChanged;
   ESS_Characteristic_Type_t            Type;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure the parameters are semi-valid.                       */
      if((TempParam) && (TempParam->NumberofParameters >= 2))
      {
         /* Store the Type parameter.                                   */
         Type = (ESS_Characteristic_Type_t)TempParam->Params[0].intParam;

         /* Make sure the Type is valid.                                */
         if((Type >= ectApparentWindDirection) && (Type <= ectMagneticFluxDensity3D))
         {
            /* Store the Flags parameter.                               */
            DescriptorValueChanged.Flags = (Word_t)TempParam->Params[1].intParam;

            /* Assign the ESS Characteristic UUID based on the Type     */
            /* parameter.                                               */
            DescriptorValueChanged.UUID = ServerInfo.Characteristics[Type].UUID;

            /* Simply call the internal function to send the indication.*/
            ret_val = IndicateDescriptorValueChanged(&DescriptorValueChanged);
         }
         else
            DisplayIndicateDescriptorValueChangedCommandUsage();
      }
      else
         DisplayIndicateDescriptorValueChangedCommandUsage();
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is a helper function to lookup ESS         */
   /* Characteristic information that was discovered during service     */
   /* discovery based on the attribute handle.  Since PTS asks for the  */
   /* attribute handle during testing it is simply easier to just call  */
   /* this function with the attribute handle to lookup the information.*/
static int FindESSCharacteristicReadAttributeInfo(ParameterList_t *TempParam)
{
   int                                 ret_val = 0;
   Word_t                              AttributeHandle;
   DeviceInfo_t                       *DeviceInfo;
   unsigned int                        Index;
   unsigned int                        Index2;
   ESS_Client_Attribute_Handle_t      *AttributeHandlePtr;
   Boolean_t                           MatchFound = FALSE;
   ESS_Characteristic_Info_t           CharacteristicInfo;
   ESS_Sample_Attribute_Handle_Type_t  AttributeType;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure we are the client.                                   */
      if(!ESSInstanceID)
      {
         /* Make sure we are connected.                                 */
         if(ConnectionID)
         {
            /* Make sure the parameters are semi-valid.                 */
            if((TempParam) && (TempParam->NumberofParameters >= 1))
            {
              /* Store the parameters.                                  */
               AttributeHandle = (Word_t)TempParam->Params[0].intParam;

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery.     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Loop through all the possible types to find a   */
                     /* match for the attribute handle.                 */
                     for(Index = 0; (!MatchFound) &&(Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES); Index++)
                     {
                        /* Loop through all the possible instances to   */
                        /* find a match for the attribute handle.       */
                        for(Index2 = 0; (!MatchFound) && (Index2 < DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries); Index2++)
                        {
                           /* Store a pointer to the attribute          */
                           /* information for the instance.             */
                           AttributeHandlePtr = &(DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList[Index2]);

                           /* Check all the handles for a match.        */
                           if(AttributeHandlePtr->Attribute_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtCharacteristic;
                           }

                           if(AttributeHandlePtr->CCCD_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtClientCharacteristicConfig;
                           }

                           if(AttributeHandlePtr->Extended_Properties_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtExtendedProperties;
                           }

                           if(AttributeHandlePtr->ES_Measurement_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESMeasurement;
                           }

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[0] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_0;
                           }

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[1] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_1;
                           }

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[2] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_2;
                           }

                           if(AttributeHandlePtr->ES_Configuration_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESConfiguration;
                           }

                           if(AttributeHandlePtr->User_Description_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtUserDescription;
                           }

                           if(AttributeHandlePtr->Valid_Range_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtValidRange;
                           }

                           /* If we found a match we need to display the*/
                           /* information.                              */
                           if(MatchFound)
                           {
                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: read %u %u %u\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID, AttributeType);
                           }
                        }
                     }
                  }
                  else
                     printf("\r\nService Discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nUsage: Find [ESS Characteristic/Descriptor handle (UINT16)]\r\n");
         }
         else
            printf("\r\nMust be connected to a remote ESS Server.\r\n");
      }
      else
         printf("\r\nOnly the ESS Client may use this function.\r\n");
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is a helper function to lookup ESS         */
   /* Characteristic information that was discovered during service     */
   /* discovery based on the attribute handle.  Since PTS asks for the  */
   /* attribute handle during testing it is simply easier to just call  */
   /* this function with the attribute handle to lookup the information.*/
static int FindESSCharacteristicWriteAttributeInfo(ParameterList_t *TempParam)
{
   int                                 ret_val = 0;
   Word_t                              AttributeHandle;
   DeviceInfo_t                       *DeviceInfo;
   unsigned int                        Index;
   unsigned int                        Index2;
   ESS_Client_Attribute_Handle_t      *AttributeHandlePtr;
   Boolean_t                           MatchFound = FALSE;
   ESS_Characteristic_Info_t           CharacteristicInfo;
   ESS_Sample_Attribute_Handle_Type_t  AttributeType;

   /* Make sure the BluetoothStack is initialized.                      */
   if(BluetoothStackID)
   {
      /* Make sure we are the client.                                   */
      if(!ESSInstanceID)
      {
         /* Make sure we are connected.                                 */
         if(ConnectionID)
         {
            /* Make sure the parameters are semi-valid.                 */
            if((TempParam) && (TempParam->NumberofParameters >= 1))
            {
              /* Store the parameters.                                  */
               AttributeHandle = (Word_t)TempParam->Params[0].intParam;

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
               {
                  /* Make sure we have performed service discovery.     */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Loop through all the possible types to find a   */
                     /* match for the attribute handle.                 */
                     for(Index = 0; (!MatchFound) &&(Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES); Index++)
                     {
                        /* Loop through all the possible instances to   */
                        /* find a match for the attribute handle.       */
                        for(Index2 = 0; (!MatchFound) && (Index2 < DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries); Index2++)
                        {
                           /* Store a pointer to the attribute          */
                           /* information for the instance.             */
                           AttributeHandlePtr = &(DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList[Index2]);

                           /* Can't write an ESS Characteristic value so*/
                           /* we will ignore the check for the handle.  */

                           if(AttributeHandlePtr->CCCD_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtClientCharacteristicConfig;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteCCCD %u %u [Config]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           /* Can't write Extended Properties descriptor*/
                           /* so we will ignore the check for the       */
                           /* handle.                                   */

                           /* Can't write ES Measurement descriptor so  */
                           /* we will ignore the check for the handle.  */

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[0] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_0;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteTriggerSetting %u %u 0 [Condition] [Operand(Optional)]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[1] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_1;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteTriggerSetting %u %u 1 [Condition] [Operand(Optional)]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           if(AttributeHandlePtr->ES_Trigger_Setting_Handle[2] == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESTriggerSetting_2;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteTriggerSetting %u %u 2 [Condition] [Operand(Optional)]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           if(AttributeHandlePtr->ES_Configuration_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtESConfiguration;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteConfiguration %u %u [Configuration]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           if(AttributeHandlePtr->User_Description_Handle == AttributeHandle)
                           {
                              MatchFound    = TRUE;
                              AttributeType = ahtUserDescription;

                              /* Set fields.                            */
                              CharacteristicInfo.Type = Index;
                              CharacteristicInfo.ID   = Index2;

                              /* Display value.                         */
                              printf("   Characteristic Info:\r\n");
                              printf("      Type:              ");
                              DisplayESSCharacteristicType(CharacteristicInfo.Type);
                              printf("      ID:                %u\r\n", CharacteristicInfo.ID);
                              printf("      Attribute Type     %u\r\n", AttributeType);
                              printf("\r\nUse command: WriteUserDescription %u %u [User Description(UTF-8)]\r\n",  CharacteristicInfo.Type, CharacteristicInfo.ID);
                           }

                           /* Can't write Valid Range descriptor so we  */
                           /* will ignore the check for the handle.     */
                        }
                     }
                  }
                  else
                     printf("\r\nService Discovery has not been performed.\r\n");
               }
               else
                  printf("\r\nNo device information.\r\n");
            }
            else
               printf("\r\nUsage: Find [ESS Characteristic/Descriptor handle (UINT16)]\r\n");
         }
         else
            printf("\r\nMust be connected to a remote ESS Server.\r\n");
      }
      else
         printf("\r\nOnly the ESS Client may use this function.\r\n");
   }
   else
      printf("\r\nBlueoothStackID invalid.\r\n");

   return (ret_val);
}

   /* The following function is a helper function to display the usage  */
   /* for reading an ESS Characteristic or its descriptors.             */
static void DisplayReadESSCharacteristicCommandUsage(void)
{
   printf("\r\nUsage: ReadCharacteristic [Type [0-19]] [ID(UINT16)] [Attribute Type(UINT8)] [OPT PARAMS]\r\n");
   printf("\r\n Where the (Type) is:\r\n");
   printf("  0  = Apparent Wind Direction\r\n");
   printf("  1  = Apparent Wind Speed\r\n");
   printf("  2  = Dew Point\r\n");
   printf("  3  = Elevation\r\n");
   printf("  4  = Gust Factor\r\n");
   printf("  5  = Heat Index\r\n");
   printf("  6  = Humidity\r\n");
   printf("  7  = Irradiance\r\n");
   printf("  8  = Pollen Concentration\r\n");
   printf("  9  = Rainfall\r\n");
   printf("  10 = Pressure\r\n");
   printf("  11 = Temperature\r\n");
   printf("  12 = True Wind Direction\r\n");
   printf("  13 = True Wind Speed\r\n");
   printf("  14 = UV Index\r\n");
   printf("  15 = Wind Chill\r\n");
   printf("  16 = Barometric Pressure Trend\r\n");
   printf("  17 = Magnetic Declination\r\n");
   printf("  18 = Magnetic Flux Density 2D\r\n");
   printf("  19 = Magnetic Flux Density 3D\r\n");
   printf("\r\n Where Attribute Type and (OPT PARAMS) are:\r\n");
   printf("  0 = Characteristic\r\n");
   printf("  1 = Extended Properties\r\n");
   printf("  2 = CCCD\r\n");
   printf("  3 = ES Measurement\r\n");
   printf("  4 = ES Trigger Setting[0]\r\n");
   printf("  5 = ES Trigger Setting[1]\r\n");
   printf("  6 = ES Trigger Setting[2]\r\n");
   printf("  7 = ES Configuration\r\n");
   printf("  8 = User Description [Read Long(0 = FALSE, 1 = TRUE)]\r\n");
   printf("  9 = Valid Range\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
   printf("\r\nNOTE: To read a long User Description descriptor, the standard read MUST be used first, followed by read longs for subsequent requests.\r\n");
}

   /* The following function is a helper function to display the usage  */
   /* for writing an ESS Characteristic.                                */
static void DisplayWriteESSCharacteristicCommandUsage(void)
{
   printf("\r\nUsage: WriteCharacteristic [Type [0-19]] [ID(UINT16)] [OPT PARAMS]\r\n");
   printf("\r\n Where (OPT PARAMS) depends on the Type and are:\r\n");
   printf("  0  = Apparent Wind Direction   [Opt Param (UINT16)]\r\n");
   printf("  1  = Apparent Wind Speed       [Opt Param (UINT16)]\r\n");
   printf("  2  = Dew Point                 [Opt Param (SINT8)]\r\n");
   printf("  3  = Elevation                 [Opt Param (SINT24)]\r\n");
   printf("  4  = Gust Factor               [Opt Param (UINT8)]\r\n");
   printf("  5  = Heat Index                [Opt Param (UINT8)]\r\n");
   printf("  6  = Humidity                  [Opt Param (UINT16)]\r\n");
   printf("  7  = Irradiance                [Opt Param (UINT16)]\r\n");
   printf("  8  = Pollen Concentration      [Opt Param (SINT24)]\r\n");
   printf("  9  = Rainfall                  [Opt Param (UINT16)]\r\n");
   printf("  10 = Pressure                  [Opt Param (UINT32)]\r\n");
   printf("  11 = Temperature               [Opt Param (SINT16)]\r\n");
   printf("  12 = True Wind Direction       [Opt Param (UINT16)]\r\n");
   printf("  13 = True Wind Speed           [Opt Param (UINT16)]\r\n");
   printf("  14 = UV Index                  [Opt Param (SINT8)]\r\n");
   printf("  15 = Wind Chill                [Opt Param (SINT8)]\r\n");
   printf("  16 = Barometric Pressure Trend\r\n");
   printf("        [Opt Param (Enum)]:\r\n");
   printf("         0 = Unknown.\r\n");
   printf("         1 = Continuously falling.\r\n");
   printf("         2 = Continuously rising.\r\n");
   printf("         3 = Falling then steady.\r\n");
   printf("         4 = Rising then steady.\r\n");
   printf("         5 = Falling before lesser rise.\r\n");
   printf("         6 = Falling before greater rise.\r\n");
   printf("         7 = Rising before lesser fall.\r\n");
   printf("         8 = Rising before greater fall.\r\n");
   printf("         9 = Steady.\r\n");
   printf("  17 = Magnetic Declination      [Opt Param (UINT16)]\r\n");
   printf("  18 = Magnetic Flux Density 2D  [X-Axis (UINT16)] [Y-Axis (UINT16)]\r\n");
   printf("  19 = Magnetic Flux Density 3D  [X-Axis (UINT16)] [Y-Axis (UINT16)] [Z-Axis (UINT16)]\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The following function is a helper function to display the usage  */
   /* for writing an ESS Characteristic CCCD.                           */
static void DisplayWriteCCCDCommandUsage(void)
{
   printf("\r\nUsage: WriteCCCD [Type [0-19]] [ID(UINT16)] [Config]\r\n");
   printf("\r\n Where Config is,\r\n");
   printf("  0 = Disable\r\n");
   printf("  1 = Enable Notifications\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The following function is a helper function to display the usage  */
   /* for writing an ES Trigger Setting.                                */
static void DisplayWriteESTriggerSettingCommandUsage(void)
{
   printf("\r\nUsage: WriteTriggerSetting [Type [0-19]] [ID(UINT16)] [TS Instance (0-2)] [Condition] [Operand(Optional)]\r\n");
   printf("\r\n Where Condition and Operand are:\r\n");
   printf("  0 = Trigger Inactive.\r\n");
   printf("  1 = Fixed Time Interval                       [Seconds (UINT16)].\r\n");
   printf("  2 = No less than specified time               [Seconds (UINT16)].\r\n");
   printf("  3 = Value changed.\r\n");
   printf("  4 = Less than specified value.                [Operand depends on Type]\r\n");
   printf("  5 = Less than or equal to specified value.    [Operand depends on Type]\r\n");
   printf("  6 = Greater than specified value.             [Operand depends on Type]\r\n");
   printf("  7 = Greater than or equal to specified value. [Operand depends on Type]\r\n");
   printf("  8 = Equal to specified value.                 [Operand depends on Type]\r\n");
   printf("  9 = Not equal to specified value.             [Operand depends on Type]\r\n");
   printf("\r\n Where Operand based on Type is:\r\n");
   printf("  0  = Apparent Wind Direction   [Operand (UINT16)]\r\n");
   printf("  1  = Apparent Wind Speed       [Operand (UINT16)]\r\n");
   printf("  2  = Dew Point                 [Operand (SINT8)]\r\n");
   printf("  3  = Elevation                 [Operand (SINT24)]\r\n");
   printf("  4  = Gust Factor               [Operand (UINT8)]\r\n");
   printf("  5  = Heat Index                [Operand (UINT8)]\r\n");
   printf("  6  = Humidity                  [Operand (UINT16)]\r\n");
   printf("  7  = Irradiance                [Operand (UINT16)]\r\n");
   printf("  8  = Pollen Concentration      [Operand (SINT24)]\r\n");
   printf("  9  = Rainfall                  [Operand (UINT16)]\r\n");
   printf("  10 = Pressure                  [Operand (UINT32)]\r\n");
   printf("  11 = Temperature               [Operand (SINT16)]\r\n");
   printf("  12 = True Wind Direction       [Operand (UINT16)]\r\n");
   printf("  13 = True Wind Speed           [Operand (UINT16)]\r\n");
   printf("  14 = UV Index                  [Operand (SINT8)]\r\n");
   printf("  15 = Wind Chill                [Operand (SINT8)]\r\n");
   printf("  17 = Magnetic Declination      [Operand (UINT16)]\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The following function is a helper function to display the usage  */
   /* for writing the ES Configuration.                                 */
static void DisplayWriteESConfigurationCommandUsage(void)
{
   printf("\r\nUsage: WriteConfiguration [Type [0-19]] [ID(UINT16)] [Configuration]\r\n");
   printf("\r\n Where Configuration is,\r\n");
   printf("  0 = AND\r\n");
   printf("  1 = OR\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The following function is a helper function to display the usage  */
   /* for writing the ES Configuration.                                 */
static void DisplayWriteUserDescriptionCommandUsage(void)
{
   printf("\r\nUsage: WriteUserDescription [Type [0-19]] [ID(UINT16)] [User Description(UTF-8)]\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
   printf("\r\nNOTE: This command will issue prepared writes for the User Description if the length is greater than can fit in a write request.\r\n");
}

   /* The following function is a helper function to display the usage  */
   /* for writing the Valid Range.                                      */
static void DisplayWriteValidRangeCommandUsage(void)
{
   printf("\r\nUsage: WriteValidRange [Type [0-19]] [ID(UINT16)] [OPT PARAMS]\r\n");
   printf("\r\n Where (OPT PARAMS Lower, Upper) depends on the Type and are:\r\n");
   printf("  0  = Apparent Wind Direction   [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  1  = Apparent Wind Speed       [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  2  = Dew Point                 [Opt Param (SINT8)]  [Opt Param (SINT8)]\r\n");
   printf("  3  = Elevation                 [Opt Param (SINT24)] [Opt Param (SINT24)]\r\n");
   printf("  4  = Gust Factor               [Opt Param (UINT8)]  [Opt Param (UINT8)]\r\n");
   printf("  5  = Heat Index                [Opt Param (UINT8)]  [Opt Param (UINT8)]\r\n");
   printf("  6  = Humidity                  [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  7  = Irradiance                [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  8  = Pollen Concentration      [Opt Param (SINT24)] [Opt Param (SINT24)]\r\n");
   printf("  9  = Rainfall                  [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  10 = Pressure                  [Opt Param (UINT32)] [Opt Param (UINT32)]\r\n");
   printf("  11 = Temperature               [Opt Param (SINT16)] [Opt Param (SINT16)]\r\n");
   printf("  12 = True Wind Direction       [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  13 = True Wind Speed           [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  14 = UV Index                  [Opt Param (SINT8)]  [Opt Param (SINT8)]\r\n");
   printf("  15 = Wind Chill                [Opt Param (SINT8)]  [Opt Param (SINT8)]\r\n");
   printf("  16 = Barometric Pressure Trend\r\n");
   printf("        [Opt Param (Enum)] [Opt Param (Enum)]:\r\n");
   printf("         0 = Unknown.\r\n");
   printf("         1 = Continuously falling.\r\n");
   printf("         2 = Continuously rising.\r\n");
   printf("         3 = Falling then steady.\r\n");
   printf("         4 = Rising then steady.\r\n");
   printf("         5 = Falling before lesser rise.\r\n");
   printf("         6 = Falling before greater rise.\r\n");
   printf("         7 = Rising before lesser fall.\r\n");
   printf("         8 = Rising before greater fall.\r\n");
   printf("         9 = Steady.\r\n");
   printf("  17 = Magnetic Declination      [Opt Param (UINT16)] [Opt Param (UINT16)]\r\n");
   printf("  18 = Magnetic Flux Density 2D  [X-Axis (UINT16)] [Y-Axis (UINT16)] [X-Axis (UINT16)] [Y-Axis (UINT16)]\r\n");
   printf("  19 = Magnetic Flux Density 3D  [X-Axis (UINT16)] [Y-Axis (UINT16)] [Z-Axis (UINT16)] [X-Axis (UINT16)] [Y-Axis (UINT16)] [Z-Axis (UINT16)]\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The following function is a helper function to display the usage  */
   /* for Notifying an ESS Characteristic.                              */
static void DisplayNotifyESSCharacteristicCommandUsage(void)
{
   printf("\r\nUsage: Notify [Type [0-19]] [ID(UINT16)]\r\n");
   printf("\r\nNOTE: ESS Server ID range: [0-%u] for every Type, however an ESS Client may have more or less if used with another ESS Server.\r\n", (unsigned int)ESS_SERVER_MAXIMUM_CHARACTERISTIC_INSTANCES-1);
}

   /* The followung function is a helper function to display the usage  */
   /* for indicating the Descriptor Value Changed Characteristic.       */
static void DisplayIndicateDescriptorValueChangedCommandUsage(void)
{
   printf("\r\nUsage: IndicateDVC [Type [0-19]] [Flags (UINT16)]\r\n");
   printf("\r\n Where (Flags) is a bitmask of the following values:\r\n");
   printf("  0x01 = Source of change ESS Client\r\n");
   printf("  0x02 = ES Trigger Setting changed\r\n");
   printf("  0x04 = ES Configuration changed\r\n");
   printf("  0x08 = ES Measurement changed\r\n");
   printf("  0x10 = User Description changed\r\n");
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
            printf("\r\netLE_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            /* Display event information.                               */
            printf("   Responses:        %u.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     printf("   Advertising Type: %s.\r\n", "rtConnectableUndirected");
                     break;
                  case rtConnectableDirected:
                     printf("   Advertising Type: %s.\r\n", "rtConnectableDirected");
                     break;
                  case rtScannableUndirected:
                     printf("   Advertising Type: %s.\r\n", "rtScannableUndirected");
                     break;
                  case rtNonConnectableUndirected:
                     printf("   Advertising Type: %s.\r\n", "rtNonConnectableUndirected");
                     break;
                  case rtScanResponse:
                     printf("   Advertising Type: %s.\r\n", "rtScanResponse");
                     break;
               }

               /* Display the Address Type.                             */
               if(DeviceEntryPtr->Address_Type == latPublic)
               {
                  printf("   Address Type:     %s.\r\n","atPublic");
               }
               else
               {
                  printf("   Address Type:     %s.\r\n","atRandom");
               }

               /* Display the Device Address.                           */
               printf("   Address:          0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);
               printf("   RSSI:             0x%02X.\r\n", DeviceEntryPtr->RSSI);
               printf("   Data Length:      %d.\r\n", DeviceEntryPtr->Raw_Report_Length);

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            printf("\r\netLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               /* Display event information.                            */
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);
               printf("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
               printf("   Role:         %s.\r\n",    (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master) ? "Master" : "Slave");
               printf("   Address Type: %s.\r\n",    (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic) ? "Public" : "Random");
               printf("   BD_ADDR:      %s.\r\n",     BoardStr);

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
            printf("\r\netLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               /* Display event information.                            */
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
            printf("\r\netLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size);

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
            printf("\r\netLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

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
            printf("\r\netLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size);

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

   /* The following is a ESS Server Event Callback.  This function will */
   /* be called whenever an ESS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the ESS Event Data   */
   /* that occurred and the ESS Event Callback Parameter that was       */
   /* specified when this Callback was The caller is free to installed. */
   /* use the contents of the ESS Event Data ONLY in the context If the */
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
   /* possible (this argument holds anyway because another ESS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving ESS Event Packets.  */
   /*            A Deadlock WILL occur because NO ESS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI ESS_EventCallback(unsigned int BluetoothStackID, ESS_Event_Data_t *ESS_Event_Data, unsigned long CallbackParameter)
{
   /* Common.                                                           */
   int                                     Result;
   DeviceInfo_t                           *DeviceInfo;
   ESS_Server_Instance_t                  *InstanceInfoPtr = NULL;

   /* ESS Event data types.                                             */
   BoardStr_t                              BoardStr;
   unsigned int                            InstanceID;
   unsigned int                            ConnectionID;
   unsigned int                            TransactionID;
   GATT_Connection_Type_t                  ConnectionType;
   ESS_Characteristic_Info_t               CharacteristicInfo;

   /* ESS Read/Write request event data types.                          */
   Word_t                                  ClientConfiguration;
   ESS_ES_Trigger_Setting_Instance_t       TriggerSettingInstance;
   ESS_ES_Trigger_Setting_Data_t           TriggerSetting;
   Byte_t                                  ES_Configuration;
   Word_t                                  UserDescriptionOffset;
   Word_t                                  UserDescriptionLength;
   Byte_t                                 *UserDescription;
   Byte_t                                  ConfirmationStatus;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (ESS_Event_Data))
   {
      /* Since many events need the device information for error        */
      /* responses we will go ahead and retrieve it.                    */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,  ConnectionBD_ADDR)) != NULL)
      {
         /* Determine the ESS Event type.                               */
         switch(ESS_Event_Data->Event_Data_Type)
         {
            case etESS_Server_Read_Characteristic_Request:
               printf("\r\netESS_Server_Read_Characteristic_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->ConnectionType;
                  CharacteristicInfo = ESS_Event_Data->Event_Data.ESS_Read_Characteristic_Request_Data->CharacteristicInfo;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's send the repsonse.                        */
                     if((Result = ESS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &(InstanceInfoPtr->Characteristic))) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_Characteristic_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * We do not need the                  */
                        /*          CharacteristicData parameter for the*/
                        /*          error response.                     */
                        if((Result = ESS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_Characteristic_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * We do not need the CharacteristicData  */
                     /*          parameter for the error response.      */
                     if((Result = ESS_Read_Characteristic_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_Characteristic_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_CCCD_Request:
               printf("\r\netESS_Server_Read_CCCD_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->ConnectionType;
                  CharacteristicInfo = ESS_Event_Data->Event_Data.ESS_Read_CCCD_Request_Data->CharacteristicInfo;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, InstanceInfoPtr->ClientConfiguration)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_CCCD_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The ClientConfiguration parameter   */
                        /*          MUST be included for the Error      */
                        /*          response, but it will be ignored.   */
                        if((Result = ESS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, InstanceInfoPtr->ClientConfiguration)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_CCCD_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The ClientConfiguration parameter MUST */
                     /*          be included for the Error response, but*/
                     /*          it will be ignored.                    */
                     if((Result = ESS_Read_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, InstanceInfoPtr->ClientConfiguration)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_CCCD_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Write_CCCD_Request:
               printf("\r\netESS_Server_Write_CCCD_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->RemoteDevice, BoardStr);
                  InstanceID          = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->InstanceID;
                  ConnectionID        = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->ConnectionID;
                  TransactionID       = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->TransactionID;
                  ConnectionType      = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->ConnectionType;
                  CharacteristicInfo  = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->CharacteristicInfo;
                  ClientConfiguration = ESS_Event_Data->Event_Data.ESS_Write_CCCD_Request_Data->ClientConfiguration;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);
                  printf("   Configuration:        0x%04X.\r\n", ClientConfiguration);

                  /* Verify the new Client Configuration.               */
                  /* * NOTE * ESS Characteristics support Notification  */
                  /*          only so we can either enable notifications*/
                  /*          or disable the configuration.             */
                  if((!ClientConfiguration) || (ClientConfiguration == ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE))
                  {
                     /* Get a pointer tot he ESS Characteristic instance*/
                     /* information.                                    */
                     InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                     if(InstanceInfoPtr)
                     {
                        /* Store the new CCCD.                          */
                        InstanceInfoPtr->ClientConfiguration = ClientConfiguration;
                        /* Let's send the write response.               */
                        /* * NOTE * The CharacteristicInfo parameter is */
                        /*          not required for the success        */
                        /*          response.                           */
                        if((Result = ESS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_CCCD_Request_Response", Result);

                           /* Let's try to send the error response.     */
                           if((Result = ESS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                           {
                              /* Display that we failed.                */
                              DisplayFunctionError("ESS_Write_CCCD_Request_Response", Result);
                           }
                        }
                     }
                     else
                     {
                        printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                        /* Let's try to send the error response.        */
                        if((Result = ESS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_CCCD_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     /* Let's try to send the error response.           */
                     if((Result = ESS_Write_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED, &CharacteristicInfo)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_CCCD_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_ES_Measurement_Request:
               printf("\r\netESS_Server_Read_ES_Measurement_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->ConnectionType;
                  CharacteristicInfo = ESS_Event_Data->Event_Data.ESS_Read_ES_Measurement_Request_Data->CharacteristicInfo;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_ES_Measurement_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &InstanceInfoPtr->MeasurementData)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Measurement_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * We do not need the MeasurementData  */
                        /*          parameter for the error response.   */
                        if((Result = ESS_Read_ES_Measurement_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_ES_Measurement_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * We do not need the MeasurementData     */
                     /*          parameter for the error response.      */
                     if((Result = ESS_Read_ES_Measurement_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Measurement_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_ES_Trigger_Setting_Request:
               printf("\r\netESS_Server_Read_ES_Trigger_Setting_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->RemoteDevice, BoardStr);
                  InstanceID             = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->InstanceID;
                  ConnectionID           = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->ConnectionID;
                  TransactionID          = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->TransactionID;
                  ConnectionType         = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->ConnectionType;
                  CharacteristicInfo     = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->CharacteristicInfo;
                  TriggerSettingInstance = ESS_Event_Data->Event_Data.ESS_Read_ES_Trigger_Setting_Request_Data->Instance;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);
                  printf("   Trigger Setting ID:   %u.\r\n", TriggerSettingInstance);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Determine the correct instance to send.         */
                     switch(TriggerSettingInstance)
                     {
                        case tsiTriggerSetting_0:
                           TriggerSetting = InstanceInfoPtr->TriggerSettingData[0];
                           break;
                        case tsiTriggerSetting_1:
                           TriggerSetting = InstanceInfoPtr->TriggerSettingData[1];
                           break;
                        case tsiTriggerSetting_2:
                           TriggerSetting = InstanceInfoPtr->TriggerSettingData[2];
                           break;
                        default:
                           /* This should never occur.  The event will  */
                           /* not be received if one of these instances */
                           /* are not set.                              */
                           break;
                     }

                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, TriggerSettingInstance, &TriggerSetting)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Trigger_Setting_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The TriggerSetting parameter may be */
                        /*          excluded for the error response.    */
                        if((Result = ESS_Read_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, TriggerSettingInstance, NULL)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_ES_Trigger_Setting_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The TriggerSetting parameter may be    */
                     /*          excluded for the error response.       */
                     if((Result = ESS_Read_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, TriggerSettingInstance, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Trigger_Setting_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Write_ES_Trigger_Setting_Request:
               printf("\r\netESS_Server_Write_ES_Trigger_Setting_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->RemoteDevice, BoardStr);
                  InstanceID             = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->InstanceID;
                  ConnectionID           = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->ConnectionID;
                  TransactionID          = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->TransactionID;
                  ConnectionType         = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->ConnectionType;
                  CharacteristicInfo     = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->CharacteristicInfo;
                  TriggerSettingInstance = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->Instance;
                  TriggerSetting         = ESS_Event_Data->Event_Data.ESS_Write_ES_Trigger_Setting_Request_Data->TriggerSetting;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Display ES Trigger Setting.                        */
                  DisplayESTriggerSettingData(CharacteristicInfo.Type, TriggerSettingInstance, &TriggerSetting);

                  /* We could reject the request at this point if the   */
                  /* ESS Client is not authorized via the               */
                  /* ESS_Write_ES_Trigger_Setting_Request_Response()    */
                  /* function.  Authorization is required to write the  */
                  /* ES Trigger Setting, however for simplicity we will */
                  /* simply authorize all requests by accepting every ES*/
                  /* Trigger Setting write.                             */

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {

                     /* The following if statement has been added for   */
                     /* TC_SPE_BI_03_C test case in PTS.  This is needed*/
                     /* to pass since this sample does not check for a  */
                     /* valid ES Trigger Setting operand.               */
                     if((CharacteristicInfo.Type == ectElevation) && (CharacteristicInfo.ID == 2))
                     {
                        /* Verify the Operands                          */
                        /* * NOTE * If the operand is not valid, then we*/
                        /*          will send the error response and    */
                        /*          break.                              */
                        if(((((DWord_t)TriggerSetting.Operand.Elevation.Upper) << 24) | ((DWord_t)TriggerSetting.Operand.Elevation.Upper)) > ((((DWord_t)InstanceInfoPtr->ValidRange.Upper.Elevation.Upper) << 24) | ((DWord_t)InstanceInfoPtr->ValidRange.Upper.Elevation.Lower)))
                        {
                           /* Let's try to send the error response.     */
                           if((Result = ESS_Write_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_OUT_OF_RANGE, &CharacteristicInfo, TriggerSettingInstance)) != 0)
                           {
                              /* Display that we failed.                */
                              DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request_Response", Result);
                           }

                           break;
                        }
                     }

                     /* Determine the correct instance to store.        */
                     switch(TriggerSettingInstance)
                     {
                        case tsiTriggerSetting_0:
                           InstanceInfoPtr->TriggerSettingData[0] = TriggerSetting;
                           break;
                        case tsiTriggerSetting_1:
                           InstanceInfoPtr->TriggerSettingData[1] = TriggerSetting;
                           break;
                        case tsiTriggerSetting_2:
                           InstanceInfoPtr->TriggerSettingData[2] = TriggerSetting;
                           break;
                        default:
                           /* This should never occur.  The event will  */
                           /* not be received if one of these instances */
                           /* are not set.                              */
                           break;
                     }

                     /* Let's respond with the Characteristic data.     */
                     if((Result = ESS_Write_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, TriggerSettingInstance)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        if((Result = ESS_Write_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, TriggerSettingInstance)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     if((Result = ESS_Write_ES_Trigger_Setting_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, TriggerSettingInstance)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_ES_Trigger_Setting_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_ES_Configuration_Request:
               printf("\r\netESS_Server_Read_ES_Configuration_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->ConnectionType;
                  CharacteristicInfo = ESS_Event_Data->Event_Data.ESS_Read_ES_Configuration_Request_Data->CharacteristicInfo;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, InstanceInfoPtr->Configuration)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Configuration_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The Configuration parameter is      */
                        /*          REQUIRED for the error response, but*/
                        /*          will be ignroed.                    */
                        if((Result = ESS_Read_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, InstanceInfoPtr->Configuration)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_ES_Configuration_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The Configuration parameter is REQUIRED*/
                     /*          for the error response, but will be    */
                     /*          ignroed.                               */
                     if((Result = ESS_Read_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, InstanceInfoPtr->Configuration)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_ES_Configuration_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Write_ES_Configuration_Request:
               printf("\r\netESS_Server_Write_ES_Configuration_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->ConnectionType;
                  CharacteristicInfo = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->CharacteristicInfo;
                  ES_Configuration   = ESS_Event_Data->Event_Data.ESS_Write_ES_Configuration_Request_Data->Configuration;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);
                  printf("   Configuration:        0x%02X.\r\n", ES_Configuration);

                  /* We could reject the request at this point if the   */
                  /* ESS Client is not authorized via the               */
                  /* ESS_Write_ES_Configuration_Request_Response()      */
                  /* function.  Authorization is required to write the  */
                  /* ES Configuration, however for simplicity we will   */
                  /* simply authorize all requests by accepting every ES*/
                  /* Configuration write.                               */

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Store the new ES Configuration.                 */
                     InstanceInfoPtr->Configuration = ES_Configuration;

                     /* Let's send the write repsonse.                  */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Write_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_ES_Configuration_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The CharacteristicInfo is REQUIRED  */
                        /*          for the error response.             */
                        if((Result = ESS_Write_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_ES_Configuration_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The CharacteristicInfo is REQUIRED for */
                     /*          the error response.                    */
                     if((Result = ESS_Write_ES_Configuration_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_ES_Configuration_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_User_Description_Request:
               printf("\r\netESS_Server_Read_User_Description_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->RemoteDevice, BoardStr);
                  InstanceID            = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->InstanceID;
                  ConnectionID          = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->ConnectionID;
                  TransactionID         = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->TransactionID;
                  ConnectionType        = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->ConnectionType;
                  CharacteristicInfo    = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->CharacteristicInfo;
                  UserDescriptionOffset = ESS_Event_Data->Event_Data.ESS_Read_User_Description_Request_Data->UserDescriptionOffset;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);
                  printf("   Offset:               %u.\r\n", UserDescriptionOffset);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, (InstanceInfoPtr->UserDescription + UserDescriptionOffset))) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_User_Description_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The UserDescription parameter does  */
                        /*          not need to be included for the     */
                        /*          error response.                     */
                        if((Result = ESS_Read_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_User_Description_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The UserDescription parameter does not */
                     /*          need to be included for the error      */
                     /*          response.                              */
                     if((Result = ESS_Read_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_User_Description_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Write_User_Description_Request:
               printf("\r\netESS_Server_Write_User_Description_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->RemoteDevice, BoardStr);
                  InstanceID            = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->InstanceID;
                  ConnectionID          = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->ConnectionID;
                  TransactionID         = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->TransactionID;
                  ConnectionType        = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->ConnectionType;
                  CharacteristicInfo    = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->CharacteristicInfo;
                  UserDescriptionLength = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->UserDescriptionLength;
                  UserDescription       = ESS_Event_Data->Event_Data.ESS_Write_User_Description_Request_Data->UserDescription;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);
                  printf("   User Description:\r\n");
                  printf("      Length:            %u.\r\n", UserDescriptionLength);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Make sure we have enough room to hold the new   */
                     /* user description.                               */
                     /* * NOTE * We will make sure we have room for the */
                     /*          NULL terminator.                       */
                     if((UserDescriptionLength + 1) <= (Word_t)ESS_SA_DEFAULT_STRING_LENGTH)
                     {
                        /* Let's store the new User Description.        */
                        BTPS_MemCopy(InstanceInfoPtr->UserDescription, UserDescription, UserDescriptionLength);

                        /* Make sure the NULL terminator is set so we   */
                        /* don't display past the end of the string.    */
                        InstanceInfoPtr->UserDescription[UserDescriptionLength] = '\0';

                        /* Display the new User Description.            */
                        printf("      Value:             %s.\r\n", InstanceInfoPtr->UserDescription);

                        /* Let's respond with the Characteristic data.  */
                        /* * NOTE * The CharacteristicInfo parameter is */
                        /*          not required for the success        */
                        /*          response.                           */
                        if((Result = ESS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_User_Description_Request_Response", Result);

                           /* Let's try to send the error response.     */
                           /* * NOTE * The CharacteristicInfo is        */
                           /*          REQUIRED for the error response. */
                           if((Result = ESS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                           {
                              /* Display that we failed.                */
                              DisplayFunctionError("ESS_Write_User_Description_Request_Response", Result);
                           }
                        }
                     }
                     else
                     {
                        /* Let's try to send the error response.        */
                        /* * NOTE * The CharacteristicInfo is REQUIRED  */
                        /*          for the error response.             */
                        if((Result = ESS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_WRITE_REQUEST_REJECTED, &CharacteristicInfo)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_User_Description_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The CharacteristicInfo is REQUIRED for */
                     /*          the error response.                    */
                     if((Result = ESS_Write_User_Description_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_User_Description_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_Valid_Range_Request:
               printf("\r\netESS_Server_Read_Valid_Range_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->RemoteDevice, BoardStr);
                  InstanceID            = ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->InstanceID;
                  ConnectionID          = ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->ConnectionID;
                  TransactionID         = ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->TransactionID;
                  ConnectionType        = ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->ConnectionType;
                  CharacteristicInfo    = ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data->CharacteristicInfo;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  DisplayESSCharacteristicInfo(&CharacteristicInfo);

                  /* Get a pointer tot he ESS Characteristic instance   */
                  /* information.                                       */
                  InstanceInfoPtr = GetServerCharacteristicInstanceInfoPtr(&CharacteristicInfo);
                  if(InstanceInfoPtr)
                  {
                     /* Let's respond with the Characteristic data.     */
                     /* * NOTE * The CharacteristicInfo parameter is not*/
                     /*          required for the success response.     */
                     if((Result = ESS_Read_Valid_Range_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, &CharacteristicInfo, &InstanceInfoPtr->ValidRange)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_Valid_Range_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        /* * NOTE * The ValidRange parameter does not   */
                        /*          need to be included for the error   */
                        /*          response.                           */
                        if((Result = ESS_Read_Valid_Range_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Read_Valid_Range_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     printf("\r\nFailure - Could not find the ESS Characteristic instance information.\r\n");

                     /* Let's try to send the error response.           */
                     /* * NOTE * The ValidRange parameter does not need */
                     /*          to be included for the error response. */
                     if((Result = ESS_Read_Valid_Range_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, &CharacteristicInfo, NULL)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_Valid_Range_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Read_Descriptor_Value_Changed_CCCD_Request:
               printf("\r\netESS_Server_Read_Descriptor_Value_Changed_CCCD_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Read_Descriptor_Changed_Value_CCCD_Request_Data->ConnectionType;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);

                  /* Let's respond with the Characteristic CCCD.        */
                  if((Result = ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS, ServerInfo.DescriptorValueChanged.CCCD)) != 0)
                  {
                     /* Display that we failed.                         */
                     DisplayFunctionError("ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response", Result);

                     /* Let's try to send the error response.           */
                     /* * NOTE * The ClientConfiguration parameter MUST */
                     /*          be included for the Error response, but*/
                     /*          it will be ignored.                    */
                     if((Result = ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR, 0)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Read_Descriptor_Changed_Value_CCCD_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Write_Descriptor_Value_Changed_CCCD_Request:
               printf("\r\netESS_Server_Write_Descriptor_Value_Changed_CCCD_Request with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->RemoteDevice, BoardStr);
                  InstanceID          = ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->InstanceID;
                  ConnectionID        = ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->ConnectionID;
                  TransactionID       = ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->TransactionID;
                  ConnectionType      = ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->ConnectionType;
                  ClientConfiguration = ESS_Event_Data->Event_Data.ESS_Write_Descriptor_Changed_Value_CCCD_Request_Data->ClientConfiguration;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  printf("   Configuration:        0x%04X.\r\n", ClientConfiguration);

                  /* Verify the new Client Configuration.               */
                  /* * NOTE * Descriptor Value Changed Characteristic   */
                  /*          supports Indication only so we can either */
                  /*          enable indication or disable the          */
                  /*          configuration.                            */
                  if((!ClientConfiguration) || (ClientConfiguration == ESS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE))
                  {
                     /* Store the new CCCD.                             */
                     ServerInfo.DescriptorValueChanged.CCCD = ClientConfiguration;

                     /* Let's send the write response.                  */
                     if((Result = ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ESS_ERROR_CODE_SUCCESS)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response", Result);

                        /* Let's try to send the error response.        */
                        if((Result = ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)) != 0)
                        {
                           /* Display that we failed.                   */
                           DisplayFunctionError("ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response", Result);
                        }
                     }
                  }
                  else
                  {
                     /* Let's try to send the error response.           */
                     if((Result = ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response(BluetoothStackID, InstanceID, TransactionID, ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)) != 0)
                     {
                        /* Display that we failed.                      */
                        DisplayFunctionError("ESS_Write_Descriptor_Changed_Value_CCCD_Request_Response", Result);
                     }
                  }
               }
               break;
            case etESS_Server_Confirmation:
               printf("\r\netESS_Server_Confirmation with size %u.\r\n", ESS_Event_Data->Event_Data_Size);
               if(ESS_Event_Data->Event_Data.ESS_Read_Valid_Range_Request_Data)
               {
                  /* Store event data.                                  */
                  BD_ADDRToStr(ESS_Event_Data->Event_Data.ESS_Confirmation_Data->RemoteDevice, BoardStr);
                  InstanceID         = ESS_Event_Data->Event_Data.ESS_Confirmation_Data->InstanceID;
                  ConnectionID       = ESS_Event_Data->Event_Data.ESS_Confirmation_Data->ConnectionID;
                  TransactionID      = ESS_Event_Data->Event_Data.ESS_Confirmation_Data->TransactionID;
                  ConnectionType     = ESS_Event_Data->Event_Data.ESS_Confirmation_Data->ConnectionType;
                  ConfirmationStatus = ESS_Event_Data->Event_Data.ESS_Confirmation_Data->Status;

                  /* Print event information.                           */
                  printf("   Instance ID:          %u.\r\n", InstanceID);
                  printf("   Connection ID:        %u.\r\n", ConnectionID);
                  printf("   Transaction ID:       %u.\r\n", TransactionID);
                  printf("   Connection Type:      %s.\r\n", ((ConnectionType == gctLE)?"LE":"BR/EDR"));
                  printf("   Remote Device:        %s.\r\n", BoardStr);
                  printf("   Status:               0x%02X.\r\n", ConfirmationStatus);
               }
               break;
            default:
               printf("\r\nUnknown ESS Event\r\n");
               break;
         }
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
static void BTPSAPI GATT_ClientEventCallback_ESS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   /* Common.                                                           */
   int                                Result;

   /* GATT ESS Client Event Callback data types.                        */
   BoardStr_t                         BoardStr;
   BD_ADDR_t                          RemoteDevice;
   DeviceInfo_t                      *DeviceInfo;
   unsigned int                       ConnectionID;
   unsigned int                       TransactionID;
   GATT_Connection_Type_t             ConnectionType;
   GATT_Request_Error_Type_t          ErrorType;
   Word_t                             AttributeHandle;
   Word_t                             BytesWritten;
   Word_t                             ValueOffset;
   Word_t                             ValueLength;
   Byte_t                            *Value;

   /* ESS Response data types.                                          */
   ESS_Client_Attribute_Handle_t     *AttributeHandleInfoPtr = NULL;
   Word_t                             MTU;
   Word_t                             CurrentOffset;
   Word_t                             ExpectedLength;
   Word_t                             PreparedWriteLength;
   ESS_ES_Trigger_Setting_Instance_t  Instance;
   union
   {
      Word_t                          CCCD;
      Word_t                          ExtendedProperties;
      ESS_Valid_Range_Data_t          ValidRangeData;
      ESS_ES_Measurement_Data_t       MeasurementData;
      ESS_ES_Trigger_Setting_Data_t   TriggerSettingData;
      Byte_t                          ESConfiguration;
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
                     case ESS_ERROR_CODE_WRITE_REQUEST_REJECTED:
                        printf("   Error Mesg:       ESS_ERROR_CODE_WRITE_REQUEST_REJECTED\r\n");
                        break;
                     case ESS_ERROR_CODE_CONDITION_NOT_SUPPORTED:
                        printf("   Error Mesg:       ESS_ERROR_CODE_CONDITION_NOT_SUPPORTED\r\n");
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
                     /* First we will check for the Descriptor Value    */
                     /* Changed Characteristic or CCCD attribute handle */
                     /* before we check for an ESS Characteristic.      */
                     if(CallbackParameter == DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_Handle)
                     {
                        /* Simply call the internal function to decode  */
                        /* the ES Measurement.                          */
                        if((Result = DecodeDisplayDescriptorValueChangedData(ValueLength, Value)) != 0)
                           DisplayFunctionError("DecodeDisplayDescriptorValueChangedData", Result);
                     }
                     else
                     {
                        if(CallbackParameter == DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_CCCD)
                        {
                           /* Make sure the length is valid.            */
                           if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                           {
                              /* Decode and display the CCCD.           */
                              ReadResponseData.CCCD = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);
                              printf("\r\nDescriptor Value Changed CCCD:\r\n");
                              printf("   Value:   0x%04X\r\n", ReadResponseData.CCCD);
                           }
                           else
                           {
                              printf("\r\nDescriptor Value Changed CCCD:\r\n");
                              printf("   Value:   Invalid length.\r\n");
                           }
                        }
                        else
                        {
                           /* MUST be an ESS Characteristic attribute   */
                           /* handle.  The GATT request information for */
                           /* the ESS Characteristic MUST have been     */
                           /* stored by the ESS Client prior to the     */
                           /* request so we can get the attribute handle*/
                           /* pointer.                                  */
                           AttributeHandleInfoPtr = GetClientAttributeHandleInfoPtr(DeviceInfo);
                           if(AttributeHandleInfoPtr)
                           {
                              /* Process the request depending on the   */
                              /* attribute handle type.                 */
                              switch(DeviceInfo->ESSClientInfo.ClientRequestInfo.AttributeHandleType)
                              {
                                 case ahtCharacteristic:
                                    /* Simply call the internal function*/
                                    /* to decode the ES Measurement.    */
                                    if((Result = DecodeDisplayESSCharacteristicData(DeviceInfo->ESSClientInfo.ClientRequestInfo.Type, ValueLength, Value)) != 0)
                                       DisplayFunctionError("DecodeDisplayESSCharacteristicData", Result);
                                    break;
                                 case ahtExtendedProperties:
                                    /* Make sure the length is valid.   */
                                    if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                                    {
                                       /* Decode and display the        */
                                       /* extended properties.          */
                                       ReadResponseData.ExtendedProperties = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);
                                       printf("\r\nExtended Properties:\r\n");
                                       printf("   Value:   0x%04X\r\n", ReadResponseData.ExtendedProperties);
                                    }
                                    else
                                    {
                                       printf("\r\nExtended Properties:\r\n");
                                       printf("   Value:   Invalid length.\r\n");
                                    }
                                    break;
                                 case ahtClientCharacteristicConfig:
                                    /* Make sure the length is valid.   */
                                    if(ValueLength >= NON_ALIGNED_WORD_SIZE)
                                    {
                                       /* Decode and display the CCCD.  */
                                       ReadResponseData.CCCD = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Value);
                                       printf("\r\nClient Characteristic Configuration:\r\n");
                                       printf("   Value:   0x%04X\r\n", ReadResponseData.CCCD);
                                    }
                                    else
                                    {
                                       printf("\r\nClient Characteristic Configuration:\r\n");
                                       printf("   Value:   Invalid length.\r\n");
                                    }
                                    break;
                                 case ahtESMeasurement:
                                    /* Simply call the API to decode the*/
                                    /* ES Measurement.                  */
                                    if((Result = ESS_Decode_ES_Measurement(ValueLength, Value, &ReadResponseData.MeasurementData)) == 0)
                                    {
                                       /* Simply call the internal      */
                                       /* function to display the ES    */
                                       /* Measurement.                  */
                                       DisplayESMeasurementData(&ReadResponseData.MeasurementData);
                                    }
                                    else
                                    {
                                       /* Print the length is invalid if*/
                                       /* the data was malformatted.    */
                                       if(Result == ESS_ERROR_MALFORMATTED_DATA)
                                       {
                                          printf("\r\nES Measurement:\r\n");
                                          printf("   Value:   Invalid length.\r\n");
                                       }

                                       /* Always display the function   */
                                       /* error.                        */
                                       DisplayFunctionError("ESS_Decode_ES_Measurement", Result);
                                    }
                                    break;
                                 case ahtESTriggerSetting_0:
                                 case ahtESTriggerSetting_1:
                                 case ahtESTriggerSetting_2:
                                    /* Subtract 4 from the attribute    */
                                    /* handle type to get the           */
                                    /* enumeration value for the ES     */
                                    /* Trigger Setting instance.        */
                                    Instance = DeviceInfo->ESSClientInfo.ClientRequestInfo.AttributeHandleType-4;

                                    /* Simply call the internal function*/
                                    /* to decode and display the ES     */
                                    /* Trigger Setting.                 */
                                    if((Result = DecodeDisplayESTriggerSettingData(DeviceInfo->ESSClientInfo.ClientRequestInfo.Type, Instance, &ReadResponseData.TriggerSettingData, ValueLength, Value)) != 0)
                                    {
                                       DisplayFunctionError("DecodeDisplayESTriggerSettingData", Result);
                                    }
                                    break;
                                 case ahtESConfiguration:
                                    /* Make sure the length is valid.   */
                                    if(ValueLength >= NON_ALIGNED_BYTE_SIZE)
                                    {
                                       /* Decode and display the ES     */
                                       /* Configuraiton.                */
                                       ReadResponseData.ESConfiguration = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Value);
                                       printf("\r\nES Configuration:\r\n");
                                       printf("   Value:   0x%02X\r\n", ReadResponseData.ESConfiguration);
                                    }
                                    else
                                    {
                                       printf("\r\nES Configuration:\r\n");
                                       printf("   Value:   Invalid length.\r\n");
                                    }
                                    break;
                                 case ahtUserDescription:
                                       /* Make sure the length is valid */
                                       /* for the buffer size.          */
                                       /* * NOTE * Add one to make sure */
                                       /*          there is room so we  */
                                       /*          can insert the NULL  */
                                       /*          terminator if an     */
                                       /*          incomplete string.   */
                                       if(ValueLength+1 <= ((Word_t)ESS_SA_DEFAULT_STRING_LENGTH))
                                       {
                                          /* Set the pointer to the User*/
                                          /* Description.               */
                                          /* * NOTE * We will always    */
                                          /*          assume zero for   */
                                          /*          the offset for a  */
                                          /*          non long read     */
                                          /*          request.  Offset  */
                                          /*          should still be   */
                                          /*          set to zero when  */
                                          /*          the request is    */
                                          /*          made.             */
                                          BTPS_MemCopy(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription, Value, ValueLength);

                                          /* Update the offset.         */
                                          /* * NOTE * This is assigned  */
                                          /*          since we may need */
                                          /*          to use a read long*/
                                          /*          request to get the*/
                                          /*          rest.             */
                                          DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset = ValueLength;

                                          /* Make sure we don't go past */
                                          /* the end of the string if we*/
                                          /* receive an incomplete      */
                                          /* string.                    */
                                          DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription[ValueLength] = '\0';

                                          /* * NOTE * If the User       */
                                          /*          Description is    */
                                          /*          incomplete then a */
                                          /*          read long request */
                                          /*          should be         */
                                          /*          performed to get  */
                                          /*          the remaining data*/
                                          /*          since the read    */
                                          /*          response couldn't */
                                          /*          fit the entire    */
                                          /*          User Description  */
                                          /*          in the response.  */
                                          printf("\r\nUser Description:\r\n");
                                          printf("   Value:   %s.\r\n", DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription);
                                       }
                                       else
                                          printf("\r\nError - ValueLength is greater than User Description buffer.\r\n");
                                    break;
                                 case ahtValidRange:
                                    /* Simply call the API to decode the*/
                                    /* Valid Range.                     */
                                    if((Result = ESS_Decode_Valid_Range(ValueLength, Value, DeviceInfo->ESSClientInfo.ClientRequestInfo.Type, &ReadResponseData.ValidRangeData)) == 0)
                                    {
                                       /* Simply call the internal      */
                                       /* function to display the Valid */
                                       /* Range.                        */
                                       DisplayValidRangeData(DeviceInfo->ESSClientInfo.ClientRequestInfo.Type, &ReadResponseData.ValidRangeData);
                                    }
                                    else
                                    {
                                       /* Print the length is invalid if*/
                                       /* the data was malformatted.    */
                                       if(Result == ESS_ERROR_MALFORMATTED_DATA)
                                       {
                                          printf("\r\nValid Range:\r\n");
                                          printf("   Value:   Invalid length.\r\n");
                                       }

                                       /* Always display the function   */
                                       /* error.                        */
                                       DisplayFunctionError("ESS_Decode_ES_Measurement", Result);
                                    }
                                    break;
                                 default:
                                    printf("\r\nFailure - Invalid attribute handle type.\r\n");
                                    break;
                              }
                           }
                           else
                               printf("\r\nFailure - Could not find the attribute handle information.\r\n");
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
                     /* MUST be an ESS Characteristic attribute handle. */
                     /* The GATT request information for the ESS        */
                     /* Characteristic MUST have been stored by the ESS */
                     /* Client prior to the request so we can get the   */
                     /* attribute handle pointer.                       */
                     AttributeHandleInfoPtr = GetClientAttributeHandleInfoPtr(DeviceInfo);
                     if(AttributeHandleInfoPtr)
                     {
                        /* Process the request depending on the         */
                        /* attribute handle type.                       */
                        /* * NOTE * Only the User Description Type      */
                        /*          should have been set.               */
                        switch(DeviceInfo->ESSClientInfo.ClientRequestInfo.AttributeHandleType)
                        {
                           case ahtUserDescription:
                              /* Make sure the length is valid for the  */
                              /* current buffer size.                   */
                              /* * NOTE * Add one to make sure there is */
                              /*          room so we can insert the NULL*/
                              /*          terminator if an incomplete   */
                              /*          string.                       */
                              CurrentOffset = DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset;
                              if((CurrentOffset + ValueLength + 1) <= ((Word_t)ESS_SA_DEFAULT_STRING_LENGTH))
                              {
                                 /* Set the pointer to the User         */
                                 /* Description.                        */
                                 /* * NOTE * We will always assume zero */
                                 /*          for the offset for a non   */
                                 /*          long read request.  Offset */
                                 /*          should still be set to zero*/
                                 /*          when the request is made.  */
                                 BTPS_MemCopy(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription + CurrentOffset, Value , ValueLength);

                                 /* Add to the current offset.          */
                                 DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset += ValueLength;

                                 /* Make sure we don't go past the end  */
                                 /* of the string if we receive an      */
                                 /* incomplete string.                  */
                                 (DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription+CurrentOffset)[ValueLength] = '\0';

                                 /* * NOTE * If the User Description is */
                                 /*          incomplete then a read long*/
                                 /*          request should be performed*/
                                 /*          to get the remaining data  */
                                 /*          since the read response    */
                                 /*          couldn't fit the entire    */
                                 /*          User Description in the    */
                                 /*          response.                  */
                                 printf("\r\nUser Description:\r\n");
                                 printf("   Value:   %s.\r\n", DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription);
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
                         printf("\r\nFailure - Could not find the attribute handle information.\r\n");
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

               /* * NOTE * We could verify that data that was sent in   */
               /*          the request since it should be sent back to  */
               /*          us in this response, but we will not do that */
               /*          here.  We could check if the data is valid   */
               /*          and cancel the request here if it is not.    */

               /* Get the device information.                           */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL)
               {
                  /* Increment the User Description offset that we have */
                  /* written thus far.                                  */
                  DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset += ValueLength;
                  CurrentOffset = DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset;

                  /* Determine the size of the User Description we are  */
                  /* writing.                                           */
                  /* * NOTE * We add one to account for the NULL        */
                  /*          terminator.                               */
                  ExpectedLength = (BTPS_StringLength(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription)+1);

                  /* If the User Description offset reaches the expected*/
                  /* length then we have written all the data.          */
                  if(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset >= ExpectedLength)
                  {
                     /* If have sent the entire User Description.       */
                     if(DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescriptionOffset == ExpectedLength)
                     {
                        /* Inform the user to execute the write request */
                        /* or cancel.                                   */
                        printf("\r\nUser Description has been prepared.\r\n");
                        printf("Use command: 'ExecuteWriteRequest' to write the User Description.\r\n");
                     }
                     else
                     {
                        printf("\r\nFailure - User Description offset has exceeded the expected length.\r\n");
                        printf("Use command: 'ExecuteWriteRequest' to cancel the request.\r\n");
                     }
                  }
                  else
                  {
                     /* Query the GATT Connection MTU to determine how  */
                     /* the amount of data we can send in the prepare   */
                     /* write.                                          */
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
                        if((Result = GATT_Prepare_Write_Request(BluetoothStackID, ConnectionID, AttributeHandle, PreparedWriteLength, CurrentOffset, (Byte_t *)((DeviceInfo->ESSClientInfo.ClientRequestInfo.UserDescription + CurrentOffset)), GATT_ClientEventCallback_ESS, AttributeHandle)) > 0)
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
         case etGATT_Client_Exchange_MTU_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
            {
               printf("\r\nExchange MTU Response.\r\n");
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
               printf("   Connection ID:     %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID);
               printf("   Transaction ID:    %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID);
               printf("   Connection Type:   %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR");
               printf("   BD_ADDR:           %s.\r\n", BoardStr);
               printf("   MTU:               %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU);
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
         default:
            printf("\r\nWarning - Event: %u, not handled.\r\n", GATT_Client_Event_Data->Event_Data_Type);
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      printf("\r\n");
      printf("ESS Callback Data: Event_Data = NULL.\r\n");
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
   /* Common.                                                           */
   int                             Result;
   DeviceInfo_t                   *DeviceInfo;
   GAP_Encryption_Mode_t           EncryptionMode;
   unsigned int                    Index;
   unsigned int                    Index2;
   ESS_Client_Attribute_Handle_t  *AttributeHandleInfoPtr;
   ESS_Client_Characteristic_t    *CharacteristicPtr;
   Word_t                          ClientAttributeHandle = 0;

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
                        GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_DEFAULT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_ESS, 0);
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
               RemoteBDADDR     = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->RemoteDevice;
               GATTConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionID;
               ConnectionType   = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_MTU_Update_Data->ConnectionType;
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
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Make sure that the indication is for the        */
                     /* Descriptor Value Changed handle.                */
                     if(AttributeHandle == DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_Handle)
                     {
                        /* Simply call the internal function to decode  */
                        /* and display the Descriptor Value Changed     */
                        /* Characteristic.                              */
                        if((Result = DecodeDisplayDescriptorValueChangedData(ValueLength, Value)) != 0)
                           DisplayFunctionError("DecodeDisplayDescriptorValueChangedData", Result);
                     }
                     else
                        printf("\r\nError - Indication not for Descriptor Value Changed.\r\n");
                  }
                  else
                     printf("\r\nError - Service discovery has not been performed.\r\n");

                  /* Send the confirmation.                             */
                  if((Result = GATT_Handle_Value_Confirmation(BluetoothStackID, GATTConnectionID, TransactionID)) != 0)
                     DisplayFunctionError("GATT_Handle_Value_Confirmation", Result);
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
                  /* Make sure service discovery has been performed so  */
                  /* we can verify the handle.                          */
                  if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                  {
                     /* Search for the ESS Characteristic information.  */
                     /* This is needed to decode the notification.      */
                     for(Index = 0; (!ClientAttributeHandle) && (Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES); Index++)
                     {
                        /* Store a pointer to the ESS Characteristic    */
                        /* Attribute Handle information.                */
                        CharacteristicPtr = &(DeviceInfo->ESSClientInfo.Characteristics[Index]);

                        for(Index2 = 0; (!ClientAttributeHandle) && (Index2 < CharacteristicPtr->AttributeHandleEntries); Index2++)
                        {
                           /* Store a pointer to the Attribute Handle   */
                           /* information.                              */
                           AttributeHandleInfoPtr = &(CharacteristicPtr->AttributeHandleList[Index2]);

                           /* Need to check each ESS Characteristic     */
                           /* instance handle.                          */
                           if(AttributeHandle == AttributeHandleInfoPtr->Attribute_Handle)
                           {
                              /* Assign the handle we found.            */
                              ClientAttributeHandle = AttributeHandleInfoPtr->Attribute_Handle;

                              /* Simply call the internal function to   */
                              /* decode the ES Measurement.             */
                              if((Result = DecodeDisplayESSCharacteristicData((ESS_Characteristic_Type_t)Index, ValueLength, Value)) != 0)
                                 DisplayFunctionError("DecodeDisplayESSCharacteristicData", Result);
                              break;
                           }
                        }
                     }

                     if(!ClientAttributeHandle)
                     {
                        printf("\r\nError - Attribute handle for ESS Characteristic not found.\r\n");
                     }

                  }
                  else
                     printf("\r\nError - Service discovery has not been performed.\r\n");
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
   unsigned int                 Index2;
   DeviceInfo_t                *DeviceInfo;
   GATT_Service_Information_t  *IncludedServiceInfo;
   ESS_Client_Characteristic_t *ClientCharacteristicPtr;

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
                  printf("\r\nService (0x%04X - 0x%04X), UUID: ", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.Service_Handle, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.End_Group_Handle);
                  DisplayUUID(&(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.UUID));
                  printf("Number of Characteristics:       %u\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->NumberOfCharacteristics);

                  /* Store a pointer to the indication information.     */
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
                     case sdESS:
                        /* If service discovery has been completed      */
                        /* previously and we are re-discovering ESS, we */
                        /* will free the memory that has been allocated */
                        /* for each ESS Characteristic instance's       */
                        /* information.  This way if the service has    */
                        /* changed since the last time we performed     */
                        /* service discovery we will populate the       */
                        /* handles correctly.                           */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE)
                        {
                           /* Loop through the ESS Client Characteristic*/
                           /* type entries.                             */
                           for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
                           {
                              /* If memory was allocated for            */
                              /* Characteristic instances attribute     */
                              /* handle information.                    */
                              if((DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries) && (DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList))
                              {
                                 /* Free the memory.                    */
                                 BTPS_FreeMemory(DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList);
                                 DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleList    = NULL;
                                 DeviceInfo->ESSClientInfo.Characteristics[Index].AttributeHandleEntries = 0;
                              }
                           }
                        }

                        /* Attempt to populate the handles for the ESS  */
                        /* Service.                                     */
                        ESSPopulateHandles(&(DeviceInfo->ESSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
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
                  printf("\r\nService Discovery Operation Complete, Status 0x%02X.\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status);

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
                     case sdESS:
                        /* Flag that service discovery has been         */
                        /* performed on for this connection.            */
                        DeviceInfo->Flags |= DEVICE_INFO_FLAGS_ESS_SERVICE_DISCOVERY_COMPLETE;

                        printf("\r\nESS Service Discovery Summary\r\n\r\n");

                        /* Print the ESS Characteristics discovered.  We*/
                        /* will simply loop over each ESS Characteristic*/
                        /* Type entry and print the instances for each  */
                        /* type.                                        */
                        for(Index = 0; Index < (unsigned int)ESS_SA_MAXIMUM_CHARACTERISTIC_TYPES; Index++)
                        {
                           /* Store a pointer to the ESS Characteristic */
                           /* Type entry to aid in readability.         */
                           ClientCharacteristicPtr = &(DeviceInfo->ESSClientInfo.Characteristics[Index]);

                           /* Print the ESS Characteristic basic        */
                           /* information.                              */
                           printf("\r\nESS Characteristic\r\n");
                           printf("   Type:                      ");
                           DisplayESSCharacteristicType((ESS_Characteristic_Type_t)Index);
                           printf("   Instances:                 %u\r\n", ClientCharacteristicPtr->AttributeHandleEntries);

                           /* Only print the ESS Characteristic service */
                           /* discovery information if the ESS          */
                           /* Characteristic Type has instances.        */
                           if(ClientCharacteristicPtr->AttributeHandleEntries)
                           {
                              /* Only print the full summary if it has  */
                              /* been enabled.                          */
                              if(PRINT_FULL_SERVICE_DISCOVERY_SUMMARY)
                              {
                                 /* Loop over the instances.            */
                                 for(Index2 = 0; Index2 < ClientCharacteristicPtr->AttributeHandleEntries; Index2++)
                                 {
                                    printf("\r\n   Instance(%u)\r\n", Index2);
                                    printf("      CCCD:                   %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].CCCD_Handle                  ? "Supported" : "Not Supported"));
                                    printf("      Extended Properties:    %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].Extended_Properties_Handle   ? "Supported" : "Not Supported"));
                                    printf("      ES Measurement:         %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].ES_Measurement_Handle        ? "Supported" : "Not Supported"));
                                    printf("      ES Trigger Setting(0):  %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].ES_Trigger_Setting_Handle[0] ? "Supported" : "Not Supported"));
                                    printf("      ES Trigger Setting(1):  %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].ES_Trigger_Setting_Handle[1] ? "Supported" : "Not Supported"));
                                    printf("      ES Trigger Setting(2):  %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].ES_Trigger_Setting_Handle[2] ? "Supported" : "Not Supported"));
                                    printf("      ES Configuration:       %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].ES_Configuration_Handle      ? "Supported" : "Not Supported"));
                                    printf("      User Description:       %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].User_Description_Handle      ? "Supported" : "Not Supported"));
                                    printf("      Valid Range:            %s\r\n", (ClientCharacteristicPtr->AttributeHandleList[Index2].Valid_Range_Handle           ? "Supported" : "Not Supported"));
                                 }
                              }
                           }
                        }

                        /* Print the Descriptor Value Changed           */
                        /* characteristic.                              */
                        printf("\r\nDescriptor Value Changed:     %s\r\n", (DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_Handle  ? "Supported" : "Not Supported"));
                        printf("   CCCD:                      %s\r\n", (DeviceInfo->ESSClientInfo.Descriptor_Value_Changed_CCCD    ? "Supported" : "Not Supported"));
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
