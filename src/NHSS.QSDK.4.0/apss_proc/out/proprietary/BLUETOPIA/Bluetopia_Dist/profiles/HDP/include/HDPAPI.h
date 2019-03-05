/*****< HDPAPI.h >*************************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPAPI - Stonestreet One Bluetooth Health Device Profile (HDP) API        */
/*           Type Definitions, Constants, and Prototypes.                     */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/09/09  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __HDPAPIH__
#define __HDPAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */

#include "MCAPType.h"       /* Bluetooth MCAP Prototypes.                     */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTHDP_ERROR_INVALID_PARAMETER                           (-1000)
#define BTHDP_ERROR_NOT_INITIALIZED                             (-1001)
#define BTHDP_ERROR_INVALID_BLUETOOTH_STACK_ID                  (-1002)
#define BTHDP_ERROR_UNSPECIFIED_ERROR                           (-1003)
#define BTHDP_ERROR_CONTEXT_ALREADY_EXISTS                      (-1004)
#define BTHDP_ERROR_PSM_IN_USE                                  (-1005)
#define BTHDP_ERROR_INSUFFICIENT_RESOURCES                      (-1006)
#define BTHDP_ERROR_INVALID_INSTANCE_ID                         (-1007)
#define BTHDP_ERROR_INVALID_MCL_ID                              (-1008)
#define BTHDP_ERROR_INVALID_MDEP_ID                             (-1009)
#define BTHDP_ERROR_INVALID_DATA_LINK_ID                        (-1010)
#define BTHDP_ERROR_INVALID_CHANNEL_MODE                        (-1011)
#define BTHDP_ERROR_INVALID_CONFIGURATION_PARAMETER             (-1012)
#define BTHDP_ERROR_REQUEST_ALREADY_OUTSTANDING                 (-1013)
#define BTHDP_ERROR_ACTION_NOT_ALLOWED                          (-1014)
#define BTHDP_ERROR_INSUFFICIENT_PACKET_LENGTH                  (-1015)
#define BTHDP_ERROR_CHANNEL_NOT_IN_OPEN_STATE                   (-1016)
#define BTHDP_ERROR_CHANNEL_NOT_CONNECTED                       (-1017)
#define BTHDP_ERROR_INSTANCE_CONNECTION_ALREADY_EXISTS          (-1018)
#define BTHDP_ERROR_MDEP_ALREADY_REGISTERED                     (-1019)
#define BTHDP_ERROR_NO_MDEP_REGISTERED                          (-1020)
#define BTHDP_ERROR_MDEP_NOT_FOUND                              (-1021)
#define BTHDP_ERROR_SYNC_ROLE_NOT_SUPPORTED                     (-1022)

   /* The following maps MCAP Response codes to those used for Control  */
   /* Channel response values.                                          */
#define HDP_RESPONSE_CODE_SUCCESS                  (MCAP_RESPONSE_CODE_SUCCESS)
#define HDP_RESPONSE_CODE_INVALID_OPCODE           (MCAP_RESPONSE_CODE_INVALID_OPCODE)
#define HDP_RESPONSE_CODE_INVALID_PARAMETER_VALUE  (MCAP_RESPONSE_CODE_INVALID_PARAMETER_VALUE)
#define HDP_RESPONSE_CODE_INVALID_DATA_ENDPOINT    (MCAP_RESPONSE_CODE_INVALID_DATA_ENDPOINT)
#define HDP_RESPONSE_CODE_DATA_ENDPOINT_BUSY       (MCAP_RESPONSE_CODE_DATA_ENDPOINT_BUSY)
#define HDP_RESPONSE_CODE_INVALID_DATA_LINK_ID     (MCAP_RESPONSE_CODE_INVALID_DATA_LINK_ID)
#define HDP_RESPONSE_CODE_DATA_LINK_BUSY           (MCAP_RESPONSE_CODE_DATA_LINK_BUSY)
#define HDP_RESPONSE_CODE_INVALID_OPERATION        (MCAP_RESPONSE_CODE_INVALID_OPERATION)
#define HDP_RESPONSE_CODE_RESOURCE_UNAVAILABLE     (MCAP_RESPONSE_CODE_RESOURCE_UNAVAILABLE)
#define HDP_RESPONSE_CODE_UNSPECIFIED_ERROR        (MCAP_RESPONSE_CODE_UNSPECIFIED_ERROR)
#define HDP_RESPONSE_CODE_REQUEST_NOT_SUPPORTED    (MCAP_RESPONSE_CODE_REQUEST_NOT_SUPPORTED)
#define HDP_RESPONSE_CODE_CONFIGURATION_REJECTED   (MCAP_RESPONSE_CODE_CONFIGURATION_REJECTED)

   /* The following define the bit masks for the MCAP Supported         */
   /* Procedures Byte.                                                  */
#define MCAP_SUPPORTED_PROCEDURE_RECONNECT_INITIATION                   (1 << 1)
#define MCAP_SUPPORTED_PROCEDURE_RECONNECT_ACCEPTANCE                   (1 << 2)
#define MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL                    (1 << 3)
#define MCAP_SUPPORTED_PROCEDURE_CLOCK_SYNC_PROTOCOL_MASTER_ROLE        (1 << 4)

   /* SDP Multi-Channel Adaptation Protocol UUID's.                     */

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Control Channel       */
   /* Bluetooth Universally Unique Identifier                           */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_16) to the*/
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_16 Constant*/
   /* value.                                                            */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_16(_x)  ASSIGN_SDP_UUID_16((_x), 0x00, 0x1E)

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Control Channel       */
   /* Bluetooth Universally Unique Identifier                           */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_32) to the*/
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_32 Constant*/
   /* value.                                                            */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_32(_x)   ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x1E)

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Control Channel       */
   /* Bluetooth Universally Unique Identifier                           */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_128) to   */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_128        */
   /* Constant value.                                                   */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_CONTROL_CHANNEL_UUID_128(_x)  ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Data Channel Bluetooth*/
   /* Universally Unique Identifier                                     */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_16) to the   */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_16 Constant   */
   /* value.                                                            */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_16(_x)      ASSIGN_SDP_UUID_16((_x), 0x00, 0x1F)

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Data Channel Bluetooth*/
   /* Universally Unique Identifier                                     */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_32) to the   */
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_32 Constant   */
   /* value.                                                            */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATAL_CHANNEL_UUID_32(_x)   ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x1F)

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Multi-Channel Adaptation Transport Protocol Data Channel Bluetooth*/
   /* Universally Unique Identifier                                     */
   /* (MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_128) to the  */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_128 Constant  */
   /* value.                                                            */
#define SDP_ASSIGN_MULTI_CHANNEL_ADAPTATION_PROTOCOL_DATA_CHANNEL_UUID_128(_x)   ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Service Class Bluetooth Universally Unique Identifier to   */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* Constant value.                                                   */
#define SDP_ASSIGN_HEALTH_DEVICE_SERVICE_CLASS_UUID_16(_x)                       ASSIGN_SDP_UUID_16((_x), 0x14, 0x00)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Service Class Bluetooth Universally Unique Identifier to   */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* Constant value.                                                   */
#define SDP_ASSIGN_HEALTH_DEVICE_SERVICE_CLASS_UUID_32(_x)                       ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x14, 0x00)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Service Class Bluetooth Universally Unique Identifier to   */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* Constant value.                                                   */
#define SDP_ASSIGN_HEALTH_DEVICE_SERVICE_CLASS_UUID_128(_x)                      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Sink (SNK) Service Class Bluetooth Universally Unique      */
   /* Identifier to the specified UUID_16_t variable.  This MACRO       */
   /* accepts one parameter which is the UUID_16_t variable that is to  */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SINK_SERVICE_CLASS_UUID_16(_x)                  ASSIGN_SDP_UUID_16((_x), 0x14, 0x02)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Sink (SNK) Service Class Bluetooth Universally Unique      */
   /* Identifier to the specified UUID_32_t variable.  This MACRO       */
   /* accepts one parameter which is the UUID_32_t variable that is to  */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SINK_SERVICE_CLASS_UUID_32(_x)                  ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x14, 0x02)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Sink (SNK) Service Class Bluetooth Universally Unique      */
   /* Identifier to the specified UUID_128_t variable.  This MACRO      */
   /* accepts one parameter which is the UUID_128_t variable that is to */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SINK_SERVICE_CLASS_UUID_128(_x)                 ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x14, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Source (SRC) Service Class Bluetooth Universally Unique    */
   /* Identifier to the specified UUID_16_t variable.  This MACRO       */
   /* accepts one parameter which is the UUID_16_t variable that is to  */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SOURCE_SERVICE_CLASS_UUID_16(_x)                ASSIGN_SDP_UUID_16((_x), 0x14, 0x01)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Source (SRC) Service Class Bluetooth Universally Unique    */
   /* Identifier to the specified UUID_32_t variable.  This MACRO       */
   /* accepts one parameter which is the UUID_32_t variable that is to  */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SOURCE_SERVICE_CLASS_UUID_32(_x)                ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x14, 0x01)

   /* The following MACRO is a utility MACRO that assigns the Health    */
   /* Device Source (SRC) Service Class Bluetooth Universally Unique    */
   /* Identifier to the specified UUID_128_t variable.  This MACRO      */
   /* accepts one parameter which is the UUID_128_t variable that is to */
   /* receive the Constant value.                                       */
#define SDP_ASSIGN_HEALTH_DEVICE_SOURCE_SERVICE_CLASS_UUID_128(_x)               ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x14, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following define the values used for the SDP Record.          */
#define SDP_ATTRIBUTE_VALUE_DATA_EXCHANGE_SPECIFICATION                 0x01

   /* The following constants represent the Connection Status Values    */
   /* that are possible in the Connection Confirmation Events - Control */
   /* and Data Link.                                                    */
#define HDP_CONNECTION_STATUS_SUCCESS                                   (0x00)
#define HDP_CONNECTION_STATUS_CONNECTION_TIMEOUT                        (0x01)
#define HDP_CONNECTION_STATUS_CONNECTION_REFUSED                        (0x02)
#define HDP_CONNECTION_STATUS_CONNECTION_TERMINATED                     (0x03)
#define HDP_CONNECTION_STATUS_CONFIGURATION_ERROR                       (0x04)
#define HDP_CONNECTION_STATUS_INSTANCE_NOT_REGISTERED                   (0x05)
#define HDP_CONNECTION_STATUS_UNKNOWN_ERROR                             (0x06)

   /* * NOTE * These exist for legacy purposes only, please use the     */
   /*          constants above.                                         */
#define HDP_OPEN_STATUS_SUCCESS                                        (HDP_CONNECTION_STATUS_SUCCESS)
#define HDP_OPEN_STATUS_CONNECTION_TIMEOUT                             (HDP_CONNECTION_STATUS_CONNECTION_TIMEOUT)
#define HDP_OPEN_STATUS_CONNECTION_REFUSED                             (HDP_CONNECTION_STATUS_CONNECTION_REFUSED)
#define HDP_OPEN_STATUS_CONNECTION_TERMINATED                          (HDP_CONNECTION_STATUS_CONNECTION_TERMINATED)
#define HDP_OPEN_STATUS_CONFIGURATION_ERROR                            (HDP_CONNECTION_STATUS_CONFIGURATION_ERROR)
#define HDP_OPEN_STATUS_INSTANCE_NOT_REGISTERED                        (HDP_CONNECTION_STATUS_INSTANCE_NOT_REGISTERED)
#define HDP_OPEN_STATUS_UNKNOWN_ERROR                                  (HDP_CONNECTION_STATUS_UNKNOWN_ERROR)

   /* The following defines a value that represent all Data Link IDs.   */
#define DATA_LINK_ALL_ID                                               (-1)

   /* The following define values used with the Sync Set Function.      */
#define CLOCK_MAX_VALUE                                                (0x0FFFFFFF)
#define CLOCK_SYNC_NOW                                                 (0xFFFFFFFF)
#define NO_TIMESTAMP_SET                                               (0xFFFFFFFFFFFFFFFF)

   /* The following enumerated type represents the supported Connection */
   /* Modes supported by the HDP Instance.                              */
typedef enum
{
   hcmAutomaticAccept,
   hcmManualAccept
} HDP_Connection_Mode_t;

   /* The following enumerated type represents the Device Role that a   */
   /* device may support on data channels.                              */
typedef enum
{
   drSource,
   drSink
} HDP_Device_Role_t;

   /* The following constant defines the Protocol Version Number used   */
   /* within the SDP Record for Multi-Channel Adaptation Transport      */
   /* Protocol (supported by this implementation).                      */
#define MCAP_PROTOCOL_VERSION                                           (0x0100)

   /* The following constant defines the Protocol Version Number used   */
   /* within the SDP Record for Health Device Profile (supported by this*/
   /* implementation).                                                  */
#define HDP_PROFILE_VERSION                                             (0x0100)

   /* The Health Device Profile sends data using Enhanced Retransmission*/
   /* Mode or Streaming Mode.  To avoid delays in other connections when*/
   /* one device has a problem communicating, a Flush Timeout is        */
   /* specified to prevent other connection from starving.              */
#define HDP_DEFAULT_FLUSH_TIMEOUT                                       (65535)

#define HDP_DEFAULT_NUM_TX_ATTEMPTS                                     (3)

   /* The following enumerate the channel modes that are supported by   */
   /* this profile.                                                     */
typedef enum
{
   cmNoPreference,
   cmReliable,
   cmStreaming
} HDP_Channel_Mode_t;

   /* The following enumerate the FCS modes that are supported by this  */
   /* profile.                                                          */
typedef enum
{
   fcsNoPreference,
   fcsDisabled,
   fcsEnabled
} HDP_FCS_Mode_t;

   /* The following structure is used to define the L2CAP parameters    */
   /* that should be negotiated for a channel.  The FCSMode specifies if*/
   /* a frame check sequence should be appended to each message.  The   */
   /* MaxTxPacketSize defines the maximum number of bytes that will ever*/
   /* be sent to the peer device.  The TxSegmentSize defines the number */
   /* of byte that will be used to package a segment of the data packet.*/
   /* The NumberOfTxSegmentBuffers defines the number of data segments  */
   /* that can be outstanding before the remote device must acknowledge */
   /* the packet.                                                       */
typedef struct _tagHDP_Channel_Config_Info_t
{
   HDP_FCS_Mode_t FCSMode;
   Word_t         MaxTxPacketSize;
   Word_t         TxSegmentSize;
   Byte_t         NumberOfTxSegmentBuffers;
} HDP_Channel_Config_Info_t;

   /* The following structure is used with the HDP_Register_SDP_Record()*/
   /* function.  This structure (when specified) contains additional SDP*/
   /* Service Information that will be added to the SDP HDP Service     */
   /* Record Entry.  The first member of this structure specifies the   */
   /* Number of Service Class UUID's that are present in the            */
   /* SDPUUIDEntries Array.  This member must be at least one, and the  */
   /* SDPUUIDEntries member must point to an array of SDP UUID Entries  */
   /* that contains (at least) as many entries specified by the         */
   /* NumberServiceClassUUID member.  The ProtocolList member is an SDP */
   /* Data Element Sequence that contains a list of Protocol Information*/
   /* that will be added to the generic SDP Service Record.  The        */
   /* ProtocolList SDP Data Element must be a Data Element Sequence     */
   /* containing the Protocol List information to add (in addition to   */
   /* the HDP Profile Information).  This element is optional and can be*/
   /* NULL (signifying no additional Protocol Information).  The        */
   /* ProfileList SDP Data Element must be a Data Element Sequence      */
   /* containing the Profile List information to add.  This element is  */
   /* optional and can be NULL (signifying that no Profile Information  */
   /* is to be added).                                                  */
typedef struct _tagHDP_SDP_Service_Record_t
{
   unsigned int        NumberServiceClassUUID;
   SDP_UUID_Entry_t   *SDPUUIDEntries;
   SDP_Data_Element_t *ProtocolList;
   SDP_Data_Element_t *ProfileList;
} HDP_SDP_Service_Record_t;

#define HDP_SDP_SERVICE_RECORD_SIZE                                     (sizeof(HDP_SDP_Service_Record_t))

   /* The following structure is used to define the information that    */
   /* describes an HDP Endpoint.  This information will be used to      */
   /* populate the SDP record for the HDP Instance.  The MDEP_ID is used*/
   /* to identify an endpoint for the HDP Instance.  The Data Type      */
   /* defines the Device Data Synchronization Code used and is assigned */
   /* by the Bluetooth SIG.  The Role will indicate if the endpoint is a*/
   /* source or sync.  The Description is an optional NULL terminated   */
   /* character string and is a user friendly description of the        */
   /* endpoint.  If the description field is not used, the description  */
   /* pointer is set to NULL.                                           */
typedef struct _tagHDP_MDEP_Info_t
{
   Byte_t             MDEP_ID;
   Word_t             MDEP_DataType;
   HDP_Device_Role_t  MDEP_Role;
   char              *MDEP_Description;
} HDP_MDEP_Info_t;

#define HDP_MDEP_INFO_DATA_SIZE                                         (sizeof(HDP_MDEP_Info_t))

   /* The following structure is used to register a number of endpoints */
   /* with a local HDP Instance.  The first parameter identifies the    */
   /* number of endpoints that are to be registered.  The second        */
   /* parameter is a pointer to an array of MDEP Record structures that */
   /* contains NumberOfEndpoints elements.                              */
typedef struct _tagHDP_MDEP_List_t
{
   int              NumberOfEndpoints;
   HDP_MDEP_Info_t *MDEP_Record_Entry;
} HDP_MDEP_List_t;

#define HDP_MDEP_LIST_DATA_SIZE                                         (sizeof(HDP_MDEP_List_t))

   /* Health Device Profile Event API Types.                            */
typedef enum
{
   etHDP_Connect_Request_Indication,
   etHDP_Control_Connect_Indication,
   etHDP_Control_Connect_Confirmation,
   etHDP_Control_Disconnect_Indication,
   etHDP_Control_Create_Data_Link_Indication,
   etHDP_Control_Create_Data_Link_Confirmation,
   etHDP_Control_Abort_Data_Link_Indication,
   etHDP_Control_Abort_Data_Link_Confirmation,
   etHDP_Control_Delete_Data_Link_Indication,
   etHDP_Control_Delete_Data_Link_Confirmation,
   etHDP_Data_Link_Connect_Indication,
   etHDP_Data_Link_Connect_Confirmation,
   etHDP_Data_Link_Disconnect_Indication,
   etHDP_Data_Link_Data_Indication,
   etHDP_Sync_Capabilities_Indication,
   etHDP_Sync_Capabilities_Confirmation,
   etHDP_Sync_Set_Indication,
   etHDP_Sync_Set_Confirmation,
   etHDP_Sync_Info_Indication
} HDP_Event_Type_t;

   /* The following event is dispatched when a remote service is        */
   /* requesting a connection to the local service.  The HDPInstanceID  */
   /* references the local HDP Instance that received the request.  The */
   /* MCLID if the assigned ID that references the Control Channel      */
   /* connection between the two HDP instances.  The BD_ADDR specifies  */
   /* the Bluetooth Address of the Remote Device that is connecting.    */
   /* ** NOTE ** This event is only dispatched to Instances that are in */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            HDP_Connect_Request_Response() function in order to    */
   /*            accept or reject the outstanding Connect Request.      */
typedef struct _tagHDP_Connect_Request_Indication_Data_t
{
   unsigned int HDPInstanceID;
   unsigned int MCLID;
   BD_ADDR_t    BD_ADDR;
} HDP_Connect_Request_Indication_Data_t;

#define HDP_CONNECT_REQUEST_INDICATION_DATA_SIZE                        (sizeof(HDP_Connect_Request_Indication_Data_t))

   /* The following event is dispatched when a remote HDP instance      */
   /* connects to the local HDP instance.  The BD_ADDR specifies the    */
   /* Bluetooth Address of the Remote HDP Instance that is connected.   */
   /* The MCLID uniquely identifies the Control channel connection to   */
   /* the remote device.                                                */
typedef struct _tagHDP_Control_Connect_Indication_Data_t
{
   unsigned int HDPInstanceID;
   unsigned int MCLID;
   BD_ADDR_t    BD_ADDR;
} HDP_Control_Connect_Indication_Data_t;

#define HDP_CONTROL_CONNECT_INDICATION_DATA_SIZE                        (sizeof(HDP_Control_Connect_Indication_Data_t))

   /* The following event is dispatched to the application when an      */
   /* attempt to connect to a remote HDP Instance is complete.  The     */
   /* MCLID identifies the Remote Instance connection that is associated*/
   /* with the confirmation.  The Status parameter indicates successful */
   /* or failure of the connection.                                     */
typedef struct _tagHDP_Control_Connect_Confirmation_Data_t
{
   unsigned int MCLID;
   int          Status;
} HDP_Control_Connect_Confirmation_Data_t;

#define HDP_CONTROL_CONNECT_CONFIRMATION_DATA_SIZE                      (sizeof(HDP_Control_Connect_Confirmation_Data_t))

   /* The following event is dispatched to the application when the     */
   /* remote HDP Instance disconnects from the Local HDP Instance.      */
typedef struct _tagHDP_Control_Disconnect_Indication_Data_t
{
   unsigned int MCLID;
} HDP_Control_Disconnect_Indication_Data_t;

#define HDP_CONTROL_DISCONNECT_INDICATION_DATA_SIZE                     (sizeof(HDP_Control_Disconnect_Indication_Data_t))

   /* The following event is dispatched when a Create Data Link request */
   /* is received from the remote HDP Instance.  The MCLID identifies   */
   /* the HDP Instance connection on which the request was received.    */
   /* The DataLinkID identifies the Data channel that is being          */
   /* requested.  The MDEPID identifies the endpoint that will be       */
   /* supporting the data link.  The Configuration byte is used to      */
   /* transport configuration information for the data link and its     */
   /* meaning is defined by the profile that uses the HDP services.     */
typedef struct _tagHDP_Control_Create_Data_Link_Indication_t
{
   unsigned int       MCLID;
   unsigned int       DataLinkID;
   Byte_t             MDEPID;
   HDP_Channel_Mode_t ChannelMode;
} HDP_Control_Create_Data_Link_Indication_t;

#define HDP_CONTROL_CREATE_DATA_LINK_INDICATION_DATA_SIZE               (sizeof(HDP_Control_Create_Data_Link_Indication_t))

   /* The following event is dispatched when a response is received for */
   /* a previous Create Data Link request is received.  The MCLID       */
   /* identifies the HDP Instance connection on which the response was  */
   /* received.  The DataLinkID identifies the Data channel being       */
   /* established.  The ResponseCode contains the result of the request.*/
   /* The Configuration byte contains profile specific configuration    */
   /* information.                                                      */
typedef struct _tagHDP_Control_Create_Data_Link_Confirmation_t
{
   unsigned int       MCLID;
   unsigned int       DataLinkID;
   Byte_t             ResponseCode;
   HDP_Channel_Mode_t ChannelMode;
} HDP_Control_Create_Data_Link_Confirmation_t;

#define HDP_CONTROL_CREATE_DATA_LINK_CONFIRMATION_DATA_SIZE             (sizeof(HDP_Control_Create_Data_Link_Confirmation_t))

   /* The following event is dispatched when a request to abort a data  */
   /* link create or reconnect operation is received from a remote HDP  */
   /* Instance.  The MCLID identifies the HDP Instance connection which */
   /* the request was received.  The DataLinkID identifies the Data     */
   /* channel that is associated with the Abort Request.                */
typedef struct _tagHDP_Control_Abort_Data_Link_Indication_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
} HDP_Control_Abort_Data_Link_Indication_t;

#define HDP_CONTROL_ABORT_DATA_LINK_INDICATION_DATA_SIZE                (sizeof(HDP_Control_Abort_Data_Link_Indication_t))

   /* The following event is dispatched when a response to an Abort     */
   /* command is received from the remote HDP Instance.  The MCLID      */
   /* identifies the HDP Instance connection on which the response was  */
   /* received.  The DataLinkID identifies Data channel associated with */
   /* the abort request.  The ResponseCode contains the result of the   */
   /* request.                                                          */
typedef struct _tagHDP_Control_Abort_Data_Link_Confirmation_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
   Byte_t       ResponseCode;
} HDP_Control_Abort_Data_Link_Confirmation_t;

#define HDP_CONTROL_ABORT_DATA_LINK_CONFIRMATION_DATA_SIZE              (sizeof(HDP_Control_Abort_Data_Link_Confirmation_t))

   /* The following event is dispatched when a request to delete a data */
   /* link is received from a remote HDP Instance.  The MCLID identifies*/
   /* the HDP Instance connection on which the request was received.    */
   /* The DataLinkID identifies the Data channel to be deleted.         */
typedef struct _tagHDP_Control_Delete_Data_Link_Indication_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
} HDP_Control_Delete_Data_Link_Indication_t;

#define HDP_CONTROL_DELETE_DATA_LINK_INDICATION_DATA_SIZE               (sizeof(HDP_Control_Delete_Data_Link_Indication_t))

   /* The following event is dispatched when a response to a delete     */
   /* command is received from the remote HDP Instance.  The MCLID      */
   /* identifies the HDP Instance connection on which the response was  */
   /* received.  The DataLinkID identifies Data channel being deleted.  */
   /* The ResponseCode contains the result of the request.              */
typedef struct _tagHDP_Control_Delete_Data_Link_Confirmation_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
   Byte_t       ResponseCode;
} HDP_Control_Delete_Data_Link_Confirmation_t;

#define HDP_CONTROL_DELETE_DATA_LINK_CONFIRMATION_DATA_SIZE             (sizeof(HDP_Control_Delete_Data_Link_Confirmation_t))

   /* The following event is dispatched when a Data Link is successfully*/
   /* established.  The MCLID identifies the HDP Instance connection on */
   /* which the data channel resides.  The MLDID identifies the Data    */
   /* channel that has been established.                                */
typedef struct _tagHDP_Data_Link_Connect_Indication_Data_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
} HDP_Data_Link_Connect_Indication_Data_t;

#define HDP_DATA_LINK_CONNECT_INDICATION_DATA_SIZE                      (sizeof(HDP_Data_Link_Connect_Indication_Data_t))

   /* The following event is dispatched when a Create Data Link         */
   /* operation is complete.  The MCLID identifies that HDP Instance    */
   /* connection on which the data channel creation was attempted.  The */
   /* DataLinkID identifies the data channel that was requested to be   */
   /* established.  The Status parameter indicates successful or failure*/
   /* of the connection.                                                */
typedef struct _tagHDP_Data_Link_Connect_Confirmation_Data_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
   int          Status;
} HDP_Data_Link_Connect_Confirmation_Data_t;

#define HDP_DATA_LINK_CONNECT_CONFIRMATION_DATA_SIZE                    (sizeof(HDP_Data_Link_Connect_Confirmation_Data_t))

   /* The following event is dispatched when an established data link is*/
   /* disconnected from the remote HDP Instance.  The MCLID identifies  */
   /* the HDP Instance connection on which the data channel resided.    */
   /* the DataLinkID identifies the data channel that was disconnected. */
typedef struct _tagHDP_Data_Link_Disconnect_Indication_Data_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
} HDP_Data_Link_Disconnect_Indication_Data_t;

#define HDP_DATA_LINK_DISCONNECT_INDICATION_DATA_SIZE                   (sizeof(HDP_Data_Link_Disconnect_Indication_Data_t))

   /* The following event is dispatched when data is received from the  */
   /* remote HDP Instance on an open data link.  The MCLID identifies   */
   /* the HDP Instance connection on which the data channel resided.    */
   /* the DataLinkID identifies the data channel on which the data was  */
   /* received.  DataLength identifies the number of octets that is     */
   /* contained in the buffer that is pointed to by DataPtr.            */
typedef struct _tagHDP_Data_Link_Data_Indication_Data_t
{
   unsigned int MCLID;
   unsigned int DataLinkID;
   Word_t       DataLength;
   Byte_t      *DataPtr;
} HDP_Data_Link_Data_Indication_Data_t;

#define HDP_DATA_LINK_DATA_INDICATION_DATA_SIZE                         (sizeof(HDP_Data_Link_Data_Indication_Data_t))

   /* The following event is dispatched when a sync capabilities request*/
   /* is received from the remote HDP Instance.  The MCLID identifies   */
   /* the HDP Instance connection on which the request was received.    */
   /* The RequiredAccuracy value indicates the accuracy of the local    */
   /* timestamp required by the sender of the request.                  */
typedef struct _tagHDP_Sync_Capabilities_Indication_t
{
   unsigned int MCLID;
   Word_t       RequiredAcuracy;
} HDP_Sync_Capabilities_Indication_t;

#define HDP_SYNC_CAPABILITIES_INDICATION_DATA_SIZE                      (sizeof(HDP_Sync_Capabilities_Indication_t))

   /* The following event is dispatched when a response to a sync       */
   /* capabilities request received from a remote HDP Instance.  The    */
   /* MCLID identifies the HDP Instance connection on which the response*/
   /* was received.  The AccessResolution identifies the accuracy of the*/
   /* remote Bluetooth Clock.  The SyncLeadTime identifies the amount of*/
   /* time the remote device requires to access the local clock         */
   /* information.  The NativeResolution identifies the resolution, in  */
   /* microseconds, of the remote timestamp.  The NativeAccuracy        */
   /* identifies the accuracy of the remote timestamp.The ResponseCode  */
   /* contains the result of the request.                               */
typedef struct _tagHDP_Sync_Capabilities_Confirmation_t
{
   unsigned int MCLID;
   Byte_t       AccessResolution;
   Word_t       SyncLeadTime;
   Word_t       NativeResolution;
   Word_t       NativeAccuracy;
   Byte_t       ResponseCode;
} HDP_Sync_Capabilities_Confirmation_t;

#define HDP_SYNC_CAPABILITIES_CONFIRMATION_DATA_SIZE                    (sizeof(HDP_Sync_Capabilities_Confirmation_t))

   /* The following event is dispatched when a sync set request is      */
   /* received from the remote HDP Instance.  The MCLID identifies the  */
   /* HDP Instance connection on which the request was received.  The   */
   /* UpdateInformationRequest value indicates the desire to receive    */
   /* Sync Info indication.  The ClockSyncTime identifies that Bluetooth*/
   /* Clock value at which synchronization should occur.  The           */
   /* TimestampSyncTime indicates the value to be set for the timestamp */
   /* at the time of synchronization.                                   */
typedef struct _tagHDP_Sync_Set_Indication_t
{
   unsigned int MCLID;
   Boolean_t    UpdateInformationRequest;
   DWord_t      ClockSyncTime;
   QWord_t      TimestampSyncTime;
} HDP_Sync_Set_Indication_t;

#define HDP_SYNC_SET_INDICATION_DATA_SIZE                               (sizeof(HDP_Sync_Set_Indication_t))

   /* The following event is dispatched when a response to a sync set   */
   /* request received from a remote HDP Instance.  The MCLID identifies*/
   /* the HDP Instance connection on which the response was received.   */
   /* The ClockSyncTime identifies the Bluetooth clock at the time the  */
   /* response was generated.  The TimestampSyncTime identifies the     */
   /* timestamp value at the time the response was generated.  The      */
   /* TimestampSampleAccuracy identifies the maximum error that may     */
   /* exist in the timestamp value.                                     */
typedef struct _tagHDP_Sync_Set_Confirmation_t
{
   unsigned int MCLID;
   DWord_t      ClockSyncTime;
   QWord_t      TimestampSyncTime;
   Word_t       TimestampSampleAccuracy;
   Byte_t       ResponseCode;
} HDP_Sync_Set_Confirmation_t;

#define HDP_SYNC_SET_CONFIRMATION_DATA_SIZE                             (sizeof(HDP_Sync_Set_Confirmation_t))

   /* The following event is dispatched when a sync info packet is      */
   /* received from a remote HDP Instance.  The MCLID identifies the HDP*/
   /* Instance connection on which the response was received.  The      */
   /* ClockSyncTime identifies the Bluetooth clock at the time the      */
   /* response was generated.  The TimestampSyncTime identifies the     */
   /* timestamp value at the time the response was generated.  The      */
   /* TimestampSampleAccuracy identifies the maximum error that may     */
   /* exist in the timestamp value.                                     */
typedef struct _tagHDP_Sync_Info_Indication_t
{
   unsigned int MCLID;
   DWord_t      ClockSyncTime;
   QWord_t      TimestampSyncTime;
   Word_t       TimestampSampleAccuracy;
} HDP_Sync_Info_Indication_t;

#define HDP_SYNC_INFO_INDICATION_DATA_SIZE                              (sizeof(HDP_Sync_Info_Indication_t))

   /* The following structure represents the container structure for    */
   /* Holding all HDP Event Data Data.                                  */
typedef struct _tagHDP_Event_Data_t
{
   HDP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      HDP_Connect_Request_Indication_Data_t          *HDP_Connect_Request_Indication_Data;
      HDP_Control_Connect_Indication_Data_t          *HDP_Control_Connect_Indication_Data;
      HDP_Control_Connect_Confirmation_Data_t        *HDP_Control_Connect_Confirmation_Data;
      HDP_Control_Disconnect_Indication_Data_t       *HDP_Control_Disconnect_Indication_Data;
      HDP_Control_Create_Data_Link_Indication_t      *HDP_Control_Create_Data_Link_Indication_Data;
      HDP_Control_Create_Data_Link_Confirmation_t    *HDP_Control_Create_Data_Link_Confirmation_Data;
      HDP_Control_Abort_Data_Link_Indication_t       *HDP_Control_Abort_Data_Link_Indication_Data;
      HDP_Control_Abort_Data_Link_Confirmation_t     *HDP_Control_Abort_Data_Link_Confirmation_Data;
      HDP_Control_Delete_Data_Link_Indication_t      *HDP_Control_Delete_Data_Link_Indication_Data;
      HDP_Control_Delete_Data_Link_Confirmation_t    *HDP_Control_Delete_Data_Link_Confirmation_Data;
      HDP_Data_Link_Connect_Indication_Data_t        *HDP_Data_Link_Connect_Indication_Data;
      HDP_Data_Link_Connect_Confirmation_Data_t      *HDP_Data_Link_Connect_Confirmation_Data;
      HDP_Data_Link_Disconnect_Indication_Data_t     *HDP_Data_Link_Disconnect_Indication_Data;
      HDP_Data_Link_Data_Indication_Data_t           *HDP_Data_Link_Data_Indication_Data;
      HDP_Sync_Capabilities_Indication_t             *HDP_Sync_Capabilities_Indication_Data;
      HDP_Sync_Capabilities_Confirmation_t           *HDP_Sync_Capabilities_Confirmation_Data;
      HDP_Sync_Set_Indication_t                      *HDP_Sync_Set_Indication_Data;
      HDP_Sync_Set_Confirmation_t                    *HDP_Sync_Set_Confirmation_Data;
      HDP_Sync_Info_Indication_t                     *HDP_Sync_Info_Indication_Data;
   } Event_Data;
} HDP_Event_Data_t;

#define HDP_EVENT_DATA_SIZE                              (sizeof(HDP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Health Device Profile Event Receive Data Callback.  This        */
   /* function will be called whenever an HDP Event occurs that is      */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the HDP Event Data   */
   /* that occurred and the HDP Event Callback Parameter that was       */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the HDP Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be reentrant).  It needs to be */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Health Device*/
   /* Profile Event will not be processed while this function call is   */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving HDP Events.  A      */
   /*            Deadlock WILL occur because NO HDP Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *HDP_Event_Callback_t)(unsigned int BluetoothStackID, HDP_Event_Data_t *HDP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for initializing the HDP.   */
   /* This function MUST be called before any other HDP function.  The  */
   /* BluetoothStackID parameter to this function is the Bluetooth Stack*/
   /* ID of the Bluetooth Stack to be used by this HDP instance.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function must be called once for a given Bluetooth*/
   /*            Stack ID.                                              */
BTPSAPI_DECLARATION int BTPSAPI HDP_Initialize(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Initialize_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for cleaning up a previously*/
   /* initialized HDP instance.  After calling this function,           */
   /* HDP_Initialize() must be called for the HDP using the specified   */
   /* Bluetooth Stack again before any other HDP function can be called.*/
   /* This function accepts the Bluetooth Stack ID of the Bluetooth     */
   /* Stack used by the HDP Instance(s) being cleaned up.  This function*/
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI HDP_Cleanup(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Cleanup_t)(unsigned int BluetoothStackID);
#endif

   /* This function will register a Local HDP Instance so that remote   */
   /* HDP Profiles/Applications can connect to us.  The function takes  */
   /* as its first parameter the BluetoothStackID of the Bluetooth Stack*/
   /* that will be associated with the HDP Profile being registered.    */
   /* The ControlPSM and DataPSM values identify the L2CAP Control and  */
   /* Data PSM ports respectively that will be used by a remote HDP     */
   /* Instance to access the local services.  The SupportedProcedures   */
   /* parameter defines the features that are supported by this         */
   /* instance.  EventCallback is the callback function to be invoked by*/
   /* HDP with the CallbackParameter whenever there are any events of   */
   /* interest for this profile.  This function returns a positive      */
   /* HDPInstanceID if successful, or a negative return value if there  */
   /* was an error.                                                     */
   /* * NOTE * The Instance is set for Automatic Accept for incoming    */
   /*          connection by default.                                   */
BTPSAPI_DECLARATION int BTPSAPI HDP_Register_Instance(unsigned int BluetoothStackID, Word_t ControlPSM, Word_t DataPSM, Byte_t SupportedProcedures, HDP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Register_Instance_t)(unsigned int BluetoothStackID, Word_t ControlPSM, Word_t DataPSM, Byte_t SupportedProcedures, HDP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* This function will un-register a Local Profile so that remote     */
   /* Profiles/Applications can no longer connect to us.  The function  */
   /* takes as its first parameter BluetoothStackID of the Bluetooth    */
   /* Stack that is associated with the HDP Instance that is being      */
   /* unregisterred.  HDPInstanceID is the Identifier of the Instance   */
   /* that is to be unregistered.  This function returns zero if        */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI HDP_UnRegister_Instance(unsigned int BluetoothStackID, unsigned int HDPInstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_UnRegister_Instance_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID);
#endif

   /* The following function is provided to allow a means to add an SDP */
   /* Service Record to the SDP Database.  The function takes as its    */
   /* first parameter the Bluetooth Stack ID of the Local Bluetooth     */
   /* Profile Stack.  The second parameter identifies a registered      */
   /* Instance ID that is associated with the SDP record that is being  */
   /* added.  The next parameter (if specified) specifies any additional*/
   /* SDP Information to add to the record.  The fourth parameter is a  */
   /* pointer to a structure that contains information about the        */
   /* Endpoints that are to be exposed.  The fifth parameter specifies  */
   /* the Service Name to associate with the SDP Record and the sixth   */
   /* parameter specifies the provider name to associate with the SDP   */
   /* record.  The final parameter is a pointer to a DWord_t which      */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* (see BTERRORS.H) and the SDPServiceRecordHandle value will be     */
   /* undefined.                                                        */
   /* ** NOTE ** The Service Record Handle that is returned from this   */
   /*            function will remain in the SDP Record Database until  */
   /*            it is deleted by calling the                           */
   /*            SDP_Delete_Service_Record() function.                  */
   /* ** NOTE ** A MACRO is provided to Delete the Service Record from  */
   /*            the SDP Data Base.  This MACRO maps the                */
   /*            HDP_Unregister_SDP_Record() to                         */
   /*            SDP_Delete_Service_Record().                           */
   /* ** NOTE ** Any Profile Information that is specified will be added*/
   /*            in the Profile Attribute AFTER the default Profile List*/
   /*            (L2CAP and HDP).                                       */
   /* ** NOTE ** The Service Name is always added at Attribute ID       */
   /*            0x0100.  A Language Base Attribute ID List is created  */
   /*            that specifies that 0x0100 is UTF-8 Encoded, English   */
   /*            Language.                                              */
BTPSAPI_DECLARATION int BTPSAPI HDP_Register_SDP_Record(unsigned int BluetoothStackID, unsigned int HDPInstanceID, char *ServiceName, char *ProviderName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Register_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, char *ServiceName, char *ProviderName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the HDP*/
   /* SDP Service Record (specified by the next parameter) from SDP     */
   /* Database.  This MACRO simply maps to the                          */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Profile Stack that the Service*/
   /* Record exists on and the SDP Service Record Handle.  The SDP      */
   /* Service Record Handle was returned via a successful call to the   */
   /* HDP_Register_SDP_Record() function.  See the                      */
   /* HDP_Register_SDP_Record() function for more information.  This    */
   /* MACRO returns the result of the SDP_Delete_Service_Record()       */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define HDP_UnRegister_SDP_Record(__BluetoothStackID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is used to register an endpoint on a       */
   /* specified HDP Instance.  The function takes as its first parameter*/
   /* the BluetoothStackID on which the Instance exists.  The second    */
   /* parameter is the HDPInstanceID of a Instance that has previously  */
   /* been registered.  The HDPMDEPInfoPtr is a pointer to the          */
   /* information about the endpoint that is being registered.  The     */
   /* EventCallback is the callback function to be invoked by HDP with  */
   /* the CallbackParameter whenever there are any events of interest   */
   /* for this endpoint.  The function returns a negative value on      */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI HDP_Register_Endpoint(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_MDEP_Info_t *HDPMDEPInfoPtr, HDP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Register_Endpoint_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_MDEP_Info_t *HDPMDEPInfoPtr, HDP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is used to un-register an endpoint on a    */
   /* specified HDP Instance.  The function takes as its first parameter*/
   /* the BluetoothStackID on which the Instance exists.  The second    */
   /* parameter is the HDPInstanceID of a Instance that has previously  */
   /* been registered.  The HDPMDEPInfoPtr is a pointer to the          */
   /* information describing the endpoint that is being un-registered.  */
   /* The function returns a negative value on error.                   */
BTPSAPI_DECLARATION int BTPSAPI HDP_Un_Register_Endpoint(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_MDEP_Info_t *HDPMDEPInfoPtr);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Un_Register_Endpoint_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_MDEP_Info_t *HDPMDEPInfoPtr);
#endif

   /* The following function is responsible for setting the HDP Instance*/
   /* Connection Mode.  This function takes as its first parameter the  */
   /* Bluetooth Stack ID of the Bluetooth Stack on which the Instance   */
   /* exists.  The second parameter is the HDPInstanceID of a Instance  */
   /* that has previously been registered.  The third parameter is the  */
   /* Instance Connection Mode to set for the Instance to use.          */
   /* ** NOTE ** The Default Instance Connection Mode is                */
   /*            asmAutomaticAccept.                                    */
BTPSAPI_DECLARATION int BTPSAPI HDP_Set_Connection_Mode(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_Connection_Mode_t ConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Set_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, HDP_Connection_Mode_t ConnectionMode);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to establish a Control Channel connection to a */
   /* local HDP Instance.  The function takes as its first parameter the*/
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* request.  The second parameter is the HDPInstanceID of the        */
   /* Instance associated with the connect request.  The third parameter*/
   /* is the MCLID that references the Control Channel that is being    */
   /* established.  The final parameter to this function specifies      */
   /* whether to accept the pending connection request (or to reject the*/
   /* request).  This function returns zero if successful, or a negative*/
   /* return error code if an error occurred.                           */
   /* ** NOTE ** The connection to the Instance is not established until*/
   /*            a etHDP_Connect_Indication event has occurred.         */
BTPSAPI_DECLARATION int BTPSAPI HDP_Connect_Request_Response(unsigned int BluetoothStackID, unsigned int HDPInstanceID, unsigned int MCLID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Connect_Request_Response_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, unsigned int MCLID, Boolean_t AcceptConnection);
#endif

   /* This function is responsible for initiating the establishment of a*/
   /* Control Channel to a remote HDP Instance.  The function takes as  */
   /* its first parameter the BluetoothStackID associated with the HDP  */
   /* Instance this profile is registered with.  The second parameter   */
   /* identifies the local instance that will associated with the remote*/
   /* instance.  The third parameter is the BD_ADDR of the remote device*/
   /* that hosts the HDP Instance.  The ControlPSM and DataPSM          */
   /* identifies the Control and Data PSM values where the remote       */
   /* Instance is registered.  The EventCallback is the callback        */
   /* function to be invoked by HDP with the CallbackParameter whenever */
   /* there are any events of interest that occur on this connection.   */
   /* This function returns a MCLID if successful, or a negative return */
   /* value if there was an error.  The MCLID can be used to reference  */
   /* this connection and must be supplied in functions that operate on */
   /* this connection.                                                  */
BTPSAPI_DECLARATION int BTPSAPI HDP_Connect_Remote_Instance(unsigned int BluetoothStackID, unsigned int HDPInstanceID, BD_ADDR_t RemoteBD_ADDR, Word_t ControlPSM, Word_t DataPSM);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Connect_Remote_Instance_t)(unsigned int BluetoothStackID, unsigned int HDPInstanceID, BD_ADDR_t RemoteBD_ADDR, Word_t ControlPSM, Word_t DataPSM);
#endif

   /* This function is responsible for disconnecting the Control Channel*/
   /* of and HDP Instance.  This function will also, by side effect,    */
   /* disconnect all connected data channels that are associated with   */
   /* it.  The function takes as its first parameter the                */
   /* BluetoothStackID associated with the HDP Instance that is being   */
   /* disconnected.  The MCLID identifies the connection that is to be  */
   /* disconnected.  This function returns a zero on success, or a      */
   /* negative return value if there was an error.                      */
   /* * NOTE * This function ONLY terminates a connection to a remote   */
   /*          HDP instance.  The local HDP Instance that is associated */
   /*          with this MCLID will remain registered.                  */
BTPSAPI_DECLARATION int BTPSAPI HDP_Close_Connection(unsigned int BluetoothStackID, unsigned int MCLID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int MCLID);
#endif

   /* This function is responsible for initiating a data connection to a*/
   /* remote HDP endpoint.  The function takes as its first parameter   */
   /* the BluetoothStackID associated with the HDP Instance this profile*/
   /* is registered with.  The second parameter identifies the Control  */
   /* Channel connection to the remote device that hosts the endpoint.  */
   /* The third parameter identifies the endpoint that is being targeted*/
   /* in this request.  The fourth parameter identifies the role that   */
   /* will be assumed by the local entity.  The fifth parameter         */
   /* specifies the configuration of the data channel that is to be     */
   /* established.  The last parameter is a pointer to configuration    */
   /* parameters that are to be used when negotiating the channel       */
   /* parameters.  This function returns a positive value if successful,*/
   /* or a negative return value if there was an error.  A successful   */
   /* return value will be the DataLinkID that will be used for future  */
   /* calls to reference this data channel.                             */
   /* * NOTE * The first data channel created on an MCL must be a       */
   /*          reliable data channel.                                   */
   /* * NOTE * For channels initiated by a Sink device, the ChannelMode */
   /*          parameter MUST indicate 'No Preference'.                 */
   /* * NOTE * For channels initiated by a Source device, the           */
   /*          ChannelMode parameter MUST NOT indicate 'No Preference'. */
BTPSAPI_DECLARATION int BTPSAPI HDP_Create_Data_Channel_Request(unsigned int BluetoothStackID, unsigned int MCLID, Byte_t MDEP_ID, HDP_Device_Role_t Role, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Create_Data_Channel_Request_t)(unsigned int BluetoothStackID, unsigned int MCLID, Byte_t MDEP_ID, HDP_Device_Role_t Role, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);
#endif

   /* This function is responsible for initiating a response for a Data */
   /* Channel connection request received from a remote HDP instance.   */
   /* The function takes as its first parameter the BluetoothStackID    */
   /* associated with the HDP Instance this profile is registered with. */
   /* The second parameter is a reference to the Data Channel that was  */
   /* requested.  The third parameter indicates acceptance/rejection of */
   /* the request.  The forth parameter indicates the configuration of  */
   /* channel that is to be established.  This function returns zero if */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI HDP_Create_Data_Channel_Response(unsigned int BluetoothStackID, unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Create_Data_Channel_Response_t)(unsigned int BluetoothStackID, unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode, HDP_Channel_Config_Info_t *ConfigInfoPtr);
#endif

   /* This function is responsible for initiating an Abort to a remote  */
   /* HDP endpoint.  It will try to terminate a process of establishing */
   /* an L2CAP channel for HDP Data.  The function takes as its first   */
   /* parameter the BluetoothStackID associated with the HDP Instance   */
   /* this profile is registered with.  The MCLID identifies the control*/
   /* channel on which the abort process of being performed.  This      */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDP_Abort_Data_Channel_Request(unsigned int BluetoothStackID, unsigned int MCLID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Abort_Data_Channel_Request_t)(unsigned int BluetoothStackID, unsigned int MCLID);
#endif

   /* This function is responsible for the deletion of Data Channel     */
   /* information.  The Delete function performs the disconnect, if     */
   /* currently connected, and removes all reference information about  */
   /* the data channel.  The function takes as its first parameter the  */
   /* BluetoothStackID associated with the HDP Instance this profile is */
   /* registered with.  The second parameter references the Control     */
   /* Channel on which the data channel resides.  The DataLinkID that   */
   /* identifies the data channel that is to be deleted.  If the        */
   /* DataLinkID is specified as DATA_LINK_ALL_ID data links, all data  */
   /* links associated with the Control Channel will be deleted.  The   */
   /* function will return zero on success and a negative value on      */
   /* failure.                                                          */
BTPSAPI_DECLARATION int BTPSAPI HDP_Delete_Data_Channel(unsigned int BluetoothStackID, unsigned int MCLID, unsigned int DataLinkID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Delete_Data_Channel_t)(unsigned int BluetoothStackID, unsigned int MCLID, unsigned int DataLinkID);
#endif

   /* This function is responsible for sending data over a specified    */
   /* Data Channel.  The function takes as its first parameter the      */
   /* BluetoothStackID associated with the HDP Instance this profile is */
   /* registered with.  The second parameter identifies the data channel*/
   /* over which the data is to be sent.  The third parameter identifies*/
   /* the number of octets that are to be sent.  The fourth parameter is*/
   /* a pointer to the data that is to be sent.  The function will      */
   /* either send all of the data or none of the data.  The function    */
   /* will return zero on success and a negative value on failure.      */
BTPSAPI_DECLARATION int BTPSAPI HDP_Write_Data(unsigned int BluetoothStackID, unsigned int DataLinkID, unsigned int DataLength, unsigned char *DataPtr);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Write_Data_t)(unsigned int BluetoothStackID, unsigned int DataLinkID, unsigned int DataLength, unsigned char *DataPtr);
#endif

   /* This function is responsible for the retrieving the Bluetooth     */
   /* Clock Value of the specified HDP Instance.  The function takes as */
   /* its first parameter the BluetoothStackID associated with the HDP  */
   /* Instance this profile is registered with.  The second parameter   */
   /* identifies the Control Channel connection to a remote HDP instance*/
   /* on which the clock is to be read.  The ClockValue is a pointer to */
   /* memory where that value read from the Bluetooth device will be    */
   /* retuned.  The Accuracy parameter identifies the accuracy of the   */
   /* value that was retrieved.  The function will return zero on       */
   /* success and a negative value on failure.                          */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Get_Bluetooth_Clock_Value(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t *ClockValue, Word_t *Accuracy);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Get_Bluetooth_Clock_Value_t)(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t *ClockValue, Word_t *Accuracy);
#endif

   /* This function is responsible for the sending of a Sync            */
   /* Capabilities Request to the remote HDP Instance.  The function    */
   /* takes as its first parameter the BluetoothStackID associated with */
   /* the HDP Instance this profile is registered with.  The second     */
   /* parameter identifies the Control Channel connection to a remote   */
   /* HDP Instance where the Sync Slave resides.  The RequiredAccuracy  */
   /* parameter identifies the minimum accuracy that is required by the */
   /* Sync Master.  The function will return zero on success and a      */
   /* negative value on failure.                                        */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Capabilities_Request(unsigned int BluetoothStackID, unsigned int MCLID, Word_t RequiredAccuracy);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Capabilities_Request_t)(unsigned int BluetoothStackID, unsigned int MCLID, Word_t RequiredAccuracy);
#endif

   /* This function is responsible for initiating a response for a Sync */
   /* Capabilities request received from a remote HDP instance.  The    */
   /* function takes as its first parameter the BluetoothStackID        */
   /* associated with the HDP Instance this profile is registered with. */
   /* The second parameter identifies the Control Channel connection to */
   /* a remote HDP Instance where the Sync Master resides.  The third   */
   /* parameter is the resolution at which the clock can be accessed and*/
   /* is provided in baseband half slots.  The fourth parameter defines */
   /* the minimum time, in milliseconds, required to process a sync     */
   /* request.  The fifth parameter defines the resolution, in          */
   /* microseconds, of the local timestamp.  The sixth parameter        */
   /* identifies the accuracy, in parts-per-million, of the local       */
   /* timestamp.  The last parameter indicates acceptance/rejection of  */
   /* the request.  This function returns a zero on success or a        */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Capabilities_Response(unsigned int BluetoothStackID, unsigned int MCLID, Byte_t AccessResolution, Word_t SyncLeadTime, Word_t NativeResolution, Word_t NativeAccuracy, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Capabilities_Response_t)(unsigned int BluetoothStackID, unsigned int MCLID, Byte_t AccessResolution, Word_t SyncLeadTime, Word_t NativeResolution, Word_t NativeAccuracy, Byte_t ResponseCode);
#endif

   /* This function is responsible for the sending of a Sync Set Request*/
   /* to the remote HDP Instance.  The function takes as its first      */
   /* parameter the BluetoothStackID associated with the HDP Instance   */
   /* this profile is registered with.  The second parameter identifies */
   /* the Control Channel connection to a remote HDP Instance where the */
   /* Sync Slave resides.  The third parameter is a boolean flag that   */
   /* indicates the desire to have Sync Info Indications sent.  The     */
   /* fourth parameter identifies the Bluetooth Clock time half slot at */
   /* which synchronization is requested.  The last parameter indicates */
   /* the Timestamp Clock value to be set at the requested Bluetooth    */
   /* Clock Time.  The function will return zero on success and a       */
   /* negative value on failure.                                        */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Set_Request(unsigned int BluetoothStackID, unsigned int MCLID, Boolean_t UpdateInformationRequest, DWord_t ClockSyncTime, QWord_t TimestampSyncTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Set_Request_t)(unsigned int BluetoothStackID, unsigned int MCLID, Boolean_t UpdateInformationRequest, DWord_t ClockSyncTime, QWord_t TimestampSyncTime);
#endif

   /* This function is responsible for initiating a response for a Sync */
   /* Set request received from a remote HDP instance.  The function    */
   /* takes as its first parameter the BluetoothStackID associated with */
   /* the HDP Instance this profile is registered with.  The second     */
   /* parameter identifies the Control Channel connection to a remote   */
   /* HDP Instance where the Sync Master resides.  The third parameter  */
   /* identifies the Timestamp Clock value at the time of the response. */
   /* The fourth parameter identifies the maximum error, in             */
   /* parts-per-million, of the clock sample.  The last parameter       */
   /* indicates acceptance/rejection of the request.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Set_Response(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t ClockSyncTime, QWord_t TimestampSyncTime, Word_t TimestampSampleAccuracy, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Set_Response_t)(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t ClockSyncTime, QWord_t TimestampSyncTime, Word_t TimestampSampleAccuracy, Byte_t ResponseCode);
#endif

   /* This function is responsible for the sending of a Sync Info       */
   /* Indication to the remote HDP Instance.  The function takes as its */
   /* first parameter the BluetoothStackID associated with the HDP      */
   /* Instance this profile is registered with.  The second parameter   */
   /* identifies the Control Channel connection on which the indication */
   /* is to be sent.  The third parameter is the value of the Bluetooth */
   /* clock at the time the response was sent.  The fourth parameter    */
   /* identifies the Timestamp Clock value at the time of the response. */
   /* The last parameter identifies the maximum error, in               */
   /* parts-per-million, of the clock sample.  The function returns zero*/
   /* on success and will return a negative value on failure.           */
BTPSAPI_DECLARATION int BTPSAPI HDP_Sync_Info_Indication(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t ClockSyncTime, QWord_t TimestampSyncTime, Word_t TimestampSampleAccuracy);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDP_Sync_Info_Indication_t)(unsigned int BluetoothStackID, unsigned int MCLID, DWord_t ClockSyncTime, QWord_t TimestampSyncTime, Word_t TimestampSampleAccuracy);
#endif

#endif

