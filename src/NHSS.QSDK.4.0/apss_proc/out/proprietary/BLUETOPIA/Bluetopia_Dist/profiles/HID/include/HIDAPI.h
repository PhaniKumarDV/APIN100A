/*****< hidapi.h >*************************************************************/
/*      Copyright 2002 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDAPI - Stonestreet One Bluetooth Stack Human Interface Device (HID)     */
/*           Profile API Type Definitions, Constants, and Prototypes.         */
/*                                                                            */
/*  Author:  Michael Gramelspacher                                            */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname       Description of Modification                  */
/*   --------  -----------       ---------------------------------------------*/
/*   03/15/02  M. Gramelspacher  Initial creation.                            */
/*   04/15/02  R. Sledge         Continued development.                       */
/******************************************************************************/
#ifndef __HIDAPIH__
#define __HIDAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTHID_ERROR_INVALID_PARAMETER                           (-1000)
#define BTHID_ERROR_NOT_INITIALIZED                             (-1001)
#define BTHID_ERROR_INVALID_BLUETOOTH_STACK_ID                  (-1002)
#define BTHID_ERROR_INSUFFICIENT_RESOURCES                      (-1004)
#define BTHID_ERROR_INVALID_OPERATION                           (-1005)
#define BTHID_ERROR_REQUEST_OUTSTANDING                         (-1006)

   /* HID Protocol UUID's.                                              */

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_16) to */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* HID_PROTOCOL_UUID_16 Constant value.                              */
#define SDP_ASSIGN_HID_PROTOCOL_UUID_16(_x)             ASSIGN_SDP_UUID_16((_x), 0x00, 0x11)

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_32) to */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* HID_PROTOCOL_UUID_32 Constant value.                              */
#define SDP_ASSIGN_HID_PROTOCOL_UUID_32(_x)             ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x11)

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Protocol Bluetooth Universally Unique Identifier (SDP_UUID_128) to*/
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* HID_PROTOCOL_UUID_128 Constant value.                             */
#define SDP_ASSIGN_HID_PROTOCOL_UUID_128(_x)            ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* HID Profile UUID's.                                               */

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Profile Bluetooth Universally Unique Identifier (SDP_UUID_16) to  */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* HID_PROTOCOL_UUID_16 Constant value.                              */
#define SDP_ASSIGN_HID_PROFILE_UUID_16(_x)              ASSIGN_SDP_UUID_16((_x), 0x11, 0x24)

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Profile Bluetooth Universally Unique Identifier (SDP_UUID_32) to  */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* HID_PROTOCOL_UUID_32 Constant value.                              */
#define SDP_ASSIGN_HID_PROFILE_UUID_32(_x)              ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x24)

   /* The following MACRO is a utility MACRO that assigns the HID       */
   /* Profile Bluetooth Universally Unique Identifier (SDP_UUID_128) to */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* HID_PROTOCOL_UUID_128 Constant value.                             */
#define SDP_ASSIGN_HID_PROFILE_UUID_128(_x)             ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x24, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following constants define the previous Profile Versions of   */
   /* the HID Profile that are supported by this implementation.  These */
   /* values can be used in the SDP Records to specify a specific HID   */
   /* Profile Version.                                                  */
#define HID_PROFILE_VERSION_1_0                                   (0x0100)
#define HID_PROFILE_VERSION_1_1                                   (0x0101)

   /* The following constant defines the Profile Version Number that is */
   /* supported by this implementation (this is the value that is used  */
   /* in the SDP Records for the actual Profile Version).               */
#define HID_PROFILE_VERSION                                       (HID_PROFILE_VERSION_1_1)

   /* The following constants represent the Device Open Status Values   */
   /* that are possible in the Open Confirmation Event.                 */
#define HID_OPEN_PORT_STATUS_SUCCESS                                  0x00
#define HID_OPEN_PORT_STATUS_CONNECTION_TIMEOUT                       0x01
#define HID_OPEN_PORT_STATUS_CONNECTION_REFUSED                       0x02
#define HID_OPEN_PORT_STATUS_UNKNOWN_ERROR                            0x03

   /* The following enumerated type is used with the                    */
   /* HID_Register_Device_SDP_Record_Version() function to specify a    */
   /* specific HID Version to be supported.  This allows a device to    */
   /* only support a certain version and not some of the more advanced  */
   /* features introduced in later profile versions.                    */
   /* * NOTE * The original SDP Registration function,                  */
   /*          HID_Register_Device_SDP_Record(), merely registers an SDP*/
   /*          record claiming support for HID version 1.0.             */
typedef enum
{
   hpvVersion1_0,
   hpvVersion1_1
} HID_Version_t;

   /* The following structure contains all of the information on the    */
   /* desired Sniff Subrating parameters for a HID Device.              */
typedef struct _tagHID_Sniff_Subrating_Parameters_t
{
   Word_t MinimumTimeout;
   Word_t MaximumLatency;
} HID_Sniff_Subrating_Parameters_t;

   /* The following constants are used with the                         */
   /* HID_Sniff_Subrating_Parameters_t structure to specify the bounds  */
   /* of the Sniff subrating parameters (minimum timeout and maximum    */
   /* latency).                                                         */
   /* * NOTE * The units of these parameters are in baseband slots      */
   /*          (0.625ms per slot).                                      */
   /* * NOTE * There is a special constant,                             */
   /*          HID_SNIFF_SUBRATING_NOT_SPECIFED_VALUE, which means that */
   /*          a value is not specified.                                */
#define HID_SNIFF_SUBRATING_MINIMUM_VALUE                      (0x0000)
#define HID_SNIFF_SUBRATING_MAXIMUM_VALUE                      (0xFFFE)

#define HID_SNIFF_SUBRATING_NOT_SPECIFED_VALUE                 (0xFFFF)

   /* The following structure contains additional protocol data that can*/
   /* optionally be passed to HID_Register_Device_SDP_Record_Version()  */
   /* to register additional optional attributes into the HID Device SDP*/
   /* Record.                                                           */
   /* * NOTE * The unit of the LinkSupervisionTimeout member, if        */
   /*          specified, are in baseband slots (0.625ms per slot).     */
   /* * NOTE * If the                                                   */
   /*        HID_SDP_RECORD_INFORMATION_FLAGS_SNIFF_SUBRATING_VALID     */
   /*          bit is set in the Flags member then the                  */
   /*          SniffSubratingParameters member must contain valid sniff */
   /*          subrating parameters.                                    */
   /* * NOTE * If the                                                   */
   /*        HID_SDP_RECORD_INFORMATION_FLAGS_SUPERVISION_TIMEOUT_VALID */
   /*          bit is set in the Flags member then the                  */
   /*          LinkSupervisionTimeout member must contain a valid Link  */
   /*          Supervision timeout valid (in baseband slots).           */
typedef struct _tagHID_SDP_Record_Information_t
{
   unsigned long                    Flags;
   HID_Sniff_Subrating_Parameters_t SniffSubratingParameters;
   Word_t                           LinkSupervisionTimeout;
} HID_SDP_Record_Information_t;

   /* The following bit definitions are used with the Flags member of   */
   /* the HID_SDP_Record_Information_t structure to denote which        */
   /* optional fields in the structure are valid.                       */
#define HID_SDP_RECORD_INFORMATION_FLAGS_SNIFF_SUBRATING_VALID       0x00000001
#define HID_SDP_RECORD_INFORMATION_FLAGS_SUPERVISION_TIMEOUT_VALID   0x00000002

   /* The following enumerated type represents the supported Server     */
   /* Connection Modes supported by a HID Server.                       */
typedef enum
{
   hsmAutomaticAccept,
   hsmAutomaticReject,
   hsmManualAccept
} HID_Server_Connection_Mode_t;

   /* The following definitions define the bits field constants used to */
   /* represent the Attribute BOOLEAN Value of the SDP Attribute with   */
   /* similar name.  These Bit Definitions are used with the            */
   /* HID_Register_Device_SDP_Record() function DeviceFlags variable to */
   /* specify the Attributes that the HID Device has Enabled.           */
#define HID_VIRTUAL_CABLE_BIT                                 (0x00000001L)
#define HID_RECONNECT_INITIATE_BIT                            (0x00000002L)
#define HID_SDP_DISABLE_BIT                                   (0x00000004L)
#define HID_BATTERY_POWER_BIT                                 (0x00000008L)
#define HID_REMOTE_WAKE_BIT                                   (0x00000010L)
#define HID_NORMALLY_CONNECTABLE_BIT                          (0x00000020L)
#define HID_BOOT_DEVICE_BIT                                   (0x00000040L)

   /* The following constant represents the value to use for an Invalid */
   /* Report ID when a Report ID is not used.                           */
#define HID_INVALID_REPORT_ID                                           (0)

   /* HID Event API Types.                                              */
typedef enum
{
   etHID_Open_Indication,
   etHID_Open_Confirmation,
   etHID_Close_Indication,
   etHID_Control_Indication,
   etHID_Get_Report_Indication,
   etHID_Get_Report_Confirmation,
   etHID_Set_Report_Indication,
   etHID_Set_Report_Confirmation,
   etHID_Get_Protocol_Indication,
   etHID_Get_Protocol_Confirmation,
   etHID_Set_Protocol_Indication,
   etHID_Set_Protocol_Confirmation,
   etHID_Get_Idle_Indication,
   etHID_Get_Idle_Confirmation,
   etHID_Set_Idle_Indication,
   etHID_Set_Idle_Confirmation,
   etHID_Data_Indication,
   etHID_Open_Request_Indication,
   etHID_Data_Buffer_Empty_Indication
} HID_Event_Type_t;

   /* The following enumerated type represents the available Result     */
   /* Types to be used with the HID Transaction Response Functions.     */
typedef enum
{
   rtSuccessful,
   rtNotReady,
   rtErrInvalidReportID,
   rtErrUnsupportedRequest,
   rtErrInvalidParameter,
   rtErrUnknown,
   rtErrFatal,
   rtData
} HID_Result_Type_t;

   /* The following enumerated type represents the available Types for  */
   /* HID_Control Control Operations.                                   */
typedef enum
{
   hcNop,
   hcHardReset,
   hcSoftReset,
   hcSuspend,
   hcExitSuspend,
   hcVirtualCableUnplug
} HID_Control_Operation_Type_t;

   /* The following enumerated type represents the available Types for  */
   /* Get_Report Sizes.                                                 */
typedef enum
{
   grSizeOfReport,
   grUseBufferSize
} HID_Get_Report_Size_Type_t;

   /* The following enumerated type represents the available Types for  */
   /* GET_REPORT and SET_REPORT Transaction Types as well as the Report */
   /* Type to be associated with Data Transactions.  Note that rtOther  */
   /* is not a valid Report type for the GET_REPORT and SET_REPORT      */
   /* Transaction.                                                      */
typedef enum
{
   rtOther,
   rtInput,
   rtOutput,
   rtFeature
} HID_Report_Type_Type_t;

   /* The following enumerated type represents the available Types for  */
   /* Set_Protocol_Request and Get_Protocol_Response Protocols.         */
typedef enum
{
   ptBoot,
   ptReport
} HID_Protocol_Type_t;

   /* The following structure represents the HID QoS Flow Specification */
   /* parameters used to configure the L2CA Interrupt Channels.         */
typedef struct _tagHID_Flow_Specification_t
{
   Byte_t  ServiceType;
   DWord_t TokenRate;
   DWord_t TokenBucketSize;
   DWord_t PeakBandwidth;
   DWord_t Latency;
   DWord_t DelayVariation;
} HID_Flow_Specification_t;

#define HID_FLOW_SPECIFICATION_SIZE                     (sizeof(HID_Flow_Specification_t))

   /* The following structure represents the HID QoS Flow configuration */
   /* parameters.                                                       */
typedef struct _tagHID_Flow_Config_t
{
   HID_Flow_Specification_t MaxFlow;
   HID_Flow_Specification_t MinFlow;
} HID_Flow_Config_t;

#define HID_FLOW_CONFIG_SIZE                            (sizeof(HID_Flow_Config_t))

   /* The following structure represents the HID Connection             */
   /* configuration that is passed to the open and register functions.  */
   /* The Flow parameters are used for the Interrupt channel.  The      */
   /* Control Channel does negotiate these parameters as it is a Best   */
   /* Effort Channel.  For the OutFlow parameter, the MaxFlow represents*/
   /* the maximum desired bandwidth out of the HID device or server.    */
   /* The MinFlow represents the minimum bandwidth that can be          */
   /* negotiated for the HID to operate properly.  For the InFlow       */
   /* parameter, the MaxFlow represents the maximum bandwidth that a HID*/
   /* can support for a given connection.  The MinFlow parameter of the */
   /* InFlow is ignored.  The InMTU describes the largest MTU that will */
   /* be allowed to be negotiated for incoming data.  The MTU values    */
   /* will be used for both the Control channel and the Interrupt       */
   /* channel.                                                          */
typedef struct _tagHID_Configuration_t
{
   HID_Flow_Config_t OutFlow;
   HID_Flow_Config_t InFlow;
   Word_t            InMTU;
} HID_Configuration_t;

#define HID_CONFIGURATION_SIZE                          (sizeof(HID_Configuration_t))

   /* The following structure represents the HID Descriptor information */
   /* that is passed to the HID_Register_Device_SDP_Record() function.  */
   /* This structure is used to describe the information associated with*/
   /* the HID Descriptor List Attribute.  The Descriptor Type member    */
   /* specifies the Class Descriptor Type as defined in the Bluetooth   */
   /* Human Interface Device (HID) Profile.  The Descriptor Length      */
   /* member specifies the Length of the Descriptor member which        */
   /* follows.  The Descriptor member specifies the actual Class        */
   /* Descriptor Data in which to use.                                  */
typedef struct _tagHID_Descriptor_t
{
   Byte_t  DescriptorType;
   Word_t  DescriptorLength;
   Byte_t *Descriptor;
} HID_Descriptor_t;

#define HID_DESCRIPTOR_SIZE                             (sizeof(HID_Descriptor_t))

   /* The following event is dispatched when a remote service is        */
   /* requesting a connection to the local HID service.  The BD_ADDR    */
   /* specifies the Bluetooth Address of the Remote Device that is      */
   /* connecting.                                                       */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            HID_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagHID_Open_Request_Indication_Data_t
{
   unsigned int HIDID;
   BD_ADDR_t    BD_ADDR;
} HID_Open_Request_Indication_Data_t;

#define HID_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(HID_Open_Request_Indication_Data_t))

   /* The following Event is dispatched when a Client Connects to a     */
   /* Registered Server.  The HIDID member specifies the Identifier of  */
   /* the connection associated with the Registered Server.  The        */
   /* BD_ADDR member specifies the address of the Client which is       */
   /* connecting to the locally Registered Server.                      */
typedef struct _tagHID_Open_Indication_Data_t
{
   unsigned int HIDID;
   BD_ADDR_t    BD_ADDR;
} HID_Open_Indication_Data_t;

#define HID_OPEN_INDICATION_DATA_SIZE                   (sizeof(HID_Open_Indication_Data_t))

   /* The following Event is dispatched to the Local Client to indicate */
   /* success or failure of a previously submitted Connection Request.  */
   /* The HIDID member specifies the Identifier of the Local Client that*/
   /* has requested the Connection.  The OpenStatus member specifies    */
   /* the status of the Connection attempt.  Possible values for the    */
   /* Open Status can be found above.                                   */
typedef struct _tagHID_Open_Confirmation_Data_t
{
   unsigned int HIDID;
   unsigned int OpenStatus;
} HID_Open_Confirmation_Data_t;

#define HID_OPEN_CONFIRMATION_DATA_SIZE                 (sizeof(HID_Open_Confirmation_Data_t))

   /* The following event is dispatched when the remote device          */
   /* disconnects from the local device.  The HIDID member specifies    */
   /* the Identifier for the Local Device being disconnected.  This     */
   /* Event is NOT Dispatched in response to the Local Device requesting*/
   /* the disconnection.  This Event is dispatched only when the remote */
   /* device terminates the Connection (and/or Bluetooth Link).         */
typedef struct _tagHID_Close_Indication_Data_t
{
   unsigned int HIDID;
} HID_Close_Indication_Data_t;

#define HID_CLOSE_INDICATION_DATA_SIZE                  (sizeof(HID_Close_Indication_Data_t))

   /* The following event is dispatched when the local entity receives a*/
   /* HID_CONTROL Transaction.  This transaction type is used to request*/
   /* a major state change.  This event may be received by either HID   */
   /* Hosts or Devices.  The HIDID member specifies the Identifier of   */
   /* the local entity receiving this indication.  The Control Operation*/
   /* member specifies the Control Operation which is being requested.  */
   /* Note that all Control Operation types are valid for reception by  */
   /* HID Devices but HID Hosts are only capable of receiving the       */
   /* Virtual Cable Unplugged Control Operation.  An Indication of this */
   /* transaction type requires no response.                            */
typedef struct _tagHID_Control_Indication_Data_t
{
   unsigned int                 HIDID;
   HID_Control_Operation_Type_t ControlOperation;
} HID_Control_Indication_Data_t;

#define HID_CONTROL_INDICATION_DATA_SIZE                (sizeof(HID_Control_Indication_Data_t))

   /* The following event is dispatched when the local HID device       */
   /* receives a GET_REPORT Transaction.  This transaction type is used */
   /* to indicate that the HID Host wants to retrieve a report from the */
   /* HID Device.  The HIDID member specifies the Identifier of the     */
   /* local HID Device receiving this indication.  The Size member      */
   /* specifies the type of allocation that the Host has used for the   */
   /* returned report.  If the Size member is grUseBufferSize the Buffer*/
   /* Size member will indicate the Buffer Size allocated by the HID    */
   /* Host, otherwise the Buffer Size member will be invalid.  The      */
   /* ReportType member specifies the type of Report the HID Host is    */
   /* requesting.  The ReportID member specifies the Report ID of the   */
   /* report which is being requested, if this value is                 */
   /* HID_INVALID_REPORT_ID it is not used and invalid.  The BufferSize */
   /* member, valid only if the Size member is grUseBufferSize,         */
   /* indicates the Buffer Size that has been allocated by the HID Host */
   /* for the Report in response to this request.  An Indication of this*/
   /* type requires a response using the HID_Get_Report_Response()      */
   /* function.                                                         */
typedef struct _tagHID_Get_Report_Indication_Data_t
{
   unsigned int               HIDID;
   HID_Get_Report_Size_Type_t Size;
   HID_Report_Type_Type_t     ReportType;
   Byte_t                     ReportID;
   Word_t                     BufferSize;
} HID_Get_Report_Indication_Data_t;

#define HID_GET_REPORT_INDICATION_DATA_SIZE             (sizeof(HID_Get_Report_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding GET_REPORT Transaction.  This        */
   /* response may either take the form of a HANDSHAKE or a DATA        */
   /* transaction.  The HIDID member specifies the Identifier of the    */
   /* local HID Host receiving this confirmation.  The Status member    */
   /* specifies the Result Type of the Response.  If the Status is      */
   /* rtData the members which follow it will be valid.  The            */
   /* rtSuccessful value is an invalid Result Type and the other        */
   /* possible Status types rtErr and rtNotReady indicate an error      */
   /* occurred.  The Report Type member specifies the type of the Report*/
   /* the HID Device is returning.  The ReportLength member specifies   */
   /* the Length of the Report Data Payload Member.  The Report Data    */
   /* Payload is the actual report data.  The ReportType, ReportLength, */
   /* and ReportDataPayload members will only be valid if the Status is */
   /* of type rtData.                                                   */
typedef struct _tagHID_Get_Report_Confirmation_Data_t
{
   unsigned int            HIDID;
   HID_Result_Type_t       Status;
   HID_Report_Type_Type_t  ReportType;
   Word_t                  ReportLength;
   Byte_t                 *ReportDataPayload;
} HID_Get_Report_Confirmation_Data_t;

#define HID_GET_REPORT_CONFIRMATION_DATA_SIZE           (sizeof(HID_Get_Report_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Device       */
   /* receives a SET_REPORT Transaction.  This transaction type is used */
   /* by the HID Host to send a Report to the HID Device.  The HIDID    */
   /* member specifies the Identifier of the local HID Device receiving */
   /* this indication.  The ReportType member specifies the type of the */
   /* Report the HID Host is sending to the HID Device.  The            */
   /* ReportLength member specifies the Length of the Report Data       */
   /* Payload member.  The ReportDataPayload is the actual report data. */
   /* An Indication of this type requires a response using the          */
   /* HID_Set_Report_Response() function.                               */
typedef struct _tagHID_Set_Report_Indication_Data_t
{
   unsigned int            HIDID;
   HID_Report_Type_Type_t  ReportType;
   Word_t                  ReportLength;
   Byte_t                 *ReportDataPayload;
} HID_Set_Report_Indication_Data_t;

#define HID_SET_REPORT_INDICATION_DATA_SIZE             (sizeof(HID_Set_Report_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_REPORT Transaction.  The HIDID   */
   /* member specifies the Identifier of the local HID Host receiving   */
   /* this confirmation.  The Status member specifies the Result Type of*/
   /* the Response.  If the Status is rtSuccessful the SET_REPORT       */
   /* Transaction was successful.  The Status Result rtData is an       */
   /* invalid Result Type and the other possible Status types rtErr and */
   /* rtNotReady indicate that an error occurred.                       */
typedef struct _tagHID_Set_Report_Confirmation_Data_t
{
   unsigned int      HIDID;
   HID_Result_Type_t Status;
} HID_Set_Report_Confirmation_Data_t;

#define HID_SET_REPORT_CONFIRMATION_DATA_SIZE           (sizeof(HID_Set_Report_Confirmation_Data_t))

   /* The following event is dispatched when the local HID device       */
   /* receives a GET_PROTOCOL Transaction.  This transaction type is    */
   /* used to indicate that the HID Host wants to retrieve the current  */
   /* protocol type from the HID Device.  The HIDID member specifies the*/
   /* Identifier of the local HID Device receiving this indication.  An */
   /* Indication of this type requires a response using the             */
   /* HID_Get_Protocol_Response() function.                             */
typedef struct _tagHID_Get_Protocol_Indication_Data_t
{
   unsigned int HIDID;
} HID_Get_Protocol_Indication_Data_t;

#define HID_GET_PROTOCOL_INDICATION_DATA_SIZE           (sizeof(HID_Get_Protocol_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding GET_PROTOCOL Transaction.  This      */
   /* response may either take the form of a HANDSHAKE or a DATA        */
   /* transaction.  The HIDID member specifies the Identifier of the    */
   /* local HID Host receiving this confirmation.  The Status member    */
   /* specifies the Result Type of the Response.  If the Status is      */
   /* rtData, the members which follow it will be valid.  The Result    */
   /* rtSuccessful is an invalid Result Type and the other possible     */
   /* Status types rtErr and rtNotReady indicate an error occurred.  The*/
   /* Protocol member specifies the current protocol.  The Protocol     */
   /* member will only be valid if the status is of type rtData.        */
typedef struct _tagHID_Get_Protocol_Confirmation_Data_t
{
   unsigned int        HIDID;
   HID_Result_Type_t   Status;
   HID_Protocol_Type_t Protocol;
} HID_Get_Protocol_Confirmation_Data_t;

#define HID_GET_PROTOCOL_CONFIRMATION_DATA_SIZE         (sizeof(HID_Get_Protocol_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Device       */
   /* receives a SET_PROTOCOL Transaction.  This transaction type is    */
   /* used to indicate that the HID Host wants to change the current    */
   /* protocol type on the HID Device.  The HIDID member specifies the  */
   /* Identifier of the local HID Device receiving this indication.  The*/
   /* Protocol member specifies the protocol type which the HID Host is */
   /* requesting the HID Device to change to.  An Indication of this    */
   /* type requires a response using the HID_Set_Protocol_Response()    */
   /* function.                                                         */
typedef struct _tagHID_Set_Protocol_Indication_Data_t
{
   unsigned int        HIDID;
   HID_Protocol_Type_t Protocol;
} HID_Set_Protocol_Indication_Data_t;

#define HID_SET_PROTOCOL_INDICATION_DATA_SIZE           (sizeof(HID_Set_Protocol_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_PROTOCOL Transaction.  The HIDID */
   /* member specifies the Identifier of the local HID Host receiving   */
   /* this confirmation.  If the Status is rtSuccessful, the            */
   /* SET_PROTOCOL Transaction was successful.  The Status rtData is an */
   /* invalid Result Type and the other possible Status types rtErr and */
   /* rtNotReady indicate that an error occurred.                       */
typedef struct _tagHID_Set_Protocol_Confirmation_Data_t
{
   unsigned int      HIDID;
   HID_Result_Type_t Status;
} HID_Set_Protocol_Confirmation_Data_t;

#define HID_SET_PROTOCOL_CONFIRMATION_DATA_SIZE         (sizeof(HID_Set_Protocol_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Device       */
   /* receives a GET_IDLE Transaction.  This transaction type is used to*/
   /* indicate that the HID Host wants to retrieve the current Idle Rate*/
   /* from the HID Device.  The HIDID member specifies the Identifier of*/
   /* the local HID Device receiving this indication.  An Indication of */
   /* this type requires a response using the HID_Get_Idle_Response()   */
   /* function.                                                         */
typedef struct _tagHID_Get_Idle_Indication_Data_t
{
   unsigned int HIDID;
} HID_Get_Idle_Indication_Data_t;

#define HID_GET_IDLE_INDICATION_DATA_SIZE               (sizeof(HID_Get_Idle_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding GET_IDLE Transaction.  This response */
   /* may either take the form of a HANDSHAKE or a DATA transaction.    */
   /* The HIDID member specifies the Identifier of the local HID Host   */
   /* receiving this confirmation.  The Status member specifies the     */
   /* Result Type of the Response.  If the Status is rtData the members */
   /* which follow it will be valid.  The Status value rtSuccessful is  */
   /* an invalid Result Type and the other possible Status types rtErr  */
   /* and rtNotReady indicate an error occurred.  The IdleRate member   */
   /* specifies the current idle rate.  The IdleRate member will only be*/
   /* valid if the status is of type rtData.                            */
typedef struct _tag_HID_Get_Idle_Confirmation_Data_t
{
   unsigned int      HIDID;
   HID_Result_Type_t Status;
   Byte_t            IdleRate;
} HID_Get_Idle_Confirmation_Data_t;

#define HID_GET_IDLE_CONFIRMATION_DATA_SIZE             (sizeof(HID_Get_Idle_Confirmation_Data_t))

   /* The following event is dispatched when the local HID Device       */
   /* receives a SET_IDLE Transaction.  This transaction type is used to*/
   /* indicate that the HID Host wants to change the current Idle Rate  */
   /* on the HID Device.  The HIDID member specifies the Identifier of  */
   /* the local HID Device receiving this indication.  The IdleRate     */
   /* member specifies the Idle Rate which the HID Host is requesting   */
   /* the HID Device to change to.  An Indication of this type requires */
   /* a response using the HID_Set_Idle_Response() function.            */
typedef struct _tagHID_Set_Idle_Indication_Data_t
{
   unsigned int HIDID;
   Byte_t       IdleRate;
} HID_Set_Idle_Indication_Data_t;

#define HID_SET_IDLE_INDICATION_DATA_SIZE               (sizeof(HID_Set_Idle_Indication_Data_t))

   /* The following event is dispatched when the local HID Host receives*/
   /* a response to an outstanding SET_IDLE Transaction.  The HIDID     */
   /* member specifies the Identifier of the local HID Host receiving   */
   /* this confirmation.  If the Status is rtSuccessful then the        */
   /* SET_IDLE Transaction was successful.  The Status value rtData is  */
   /* an invalid Result Type and the other possible Status types rtErr  */
   /* and rtNotReady indicate that an error occurred.                   */
typedef struct _tag_HID_Set_Idle_Confirmation_Data_t
{
   unsigned int      HIDID;
   HID_Result_Type_t Status;
} HID_Set_Idle_Confirmation_Data_t;

#define HID_SET_IDLE_CONFIRMATION_DATA_SIZE             (sizeof(HID_Set_Idle_Confirmation_Data_t))

   /* The following event is dispatched when the local entity receives a*/
   /* DATA Transaction on the Interrupt Channel.  This event may be     */
   /* received by either HID Hosts or HID Devices depending on the      */
   /* report type.  The HIDID member specifies the Identifier of the    */
   /* local entity receiving this indication.  The ReportType member    */
   /* specifies the type of Report the entity is receiving.  HID Hosts  */
   /* will only receive reports of type rtInput while HID Devices will  */
   /* only receive reports of type rtOutput.  All other report types are*/
   /* invalid for this event type.  The ReportLength member specifies   */
   /* the Length of the ReportDataPayload Member.  The ReportDataPayload*/
   /* is the actual report data.                                        */
typedef struct _tagHID_Data_Indication_Data_t
{
   unsigned int            HIDID;
   HID_Report_Type_Type_t  ReportType;
   Word_t                  ReportLength;
   Byte_t                 *ReportDataPayload;
} HID_Data_Indication_Data_t;

#define HID_DATA_INDICATION_DATA_SIZE                   (sizeof(HID_Data_Indication_Data_t))

   /* This following event is dispatched by the local entity to the     */
   /* local application when a HID Data Channel no longer has any data  */
   /* queued to be sent (on the Data Channel).  The HIDID member        */
   /* represents the Local HID Profile Endpoint that now has an empty   */
   /* Data Channel.                                                     */
   /* * NOTE * This event is only dispatched when the Data Write        */
   /*          function returned the error code                         */
   /*                                                                   */
   /*             BTPS_ERROR_INSUFFICIENT_RESOURCES                     */
   /*                                                                   */
   /*          If the return value of the Data Write function is not    */
   /*          the above mentioned error then this event will not be    */
   /*          dispatched.                                              */
typedef struct _tagHID_Data_Buffer_Empty_Indication_Data_t
{
   unsigned int HIDID;
} HID_Data_Buffer_Empty_Indication_Data_t;

#define HID_DATA_BUFFER_EMPTY_INDICATION_DATA_SIZE       (sizeof(HID_Data_Buffer_Empty_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all HID Event Data Data.                                  */
typedef struct _tagHID_Event_Data_t
{
   HID_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      HID_Open_Request_Indication_Data_t      *HID_Open_Request_Indication_Data;
      HID_Open_Indication_Data_t              *HID_Open_Indication_Data;
      HID_Open_Confirmation_Data_t            *HID_Open_Confirmation_Data;
      HID_Close_Indication_Data_t             *HID_Close_Indication_Data;
      HID_Control_Indication_Data_t           *HID_Control_Indication_Data;
      HID_Get_Report_Indication_Data_t        *HID_Get_Report_Indication_Data;
      HID_Get_Report_Confirmation_Data_t      *HID_Get_Report_Confirmation_Data;
      HID_Set_Report_Indication_Data_t        *HID_Set_Report_Indication_Data;
      HID_Set_Report_Confirmation_Data_t      *HID_Set_Report_Confirmation_Data;
      HID_Get_Protocol_Indication_Data_t      *HID_Get_Protocol_Indication_Data;
      HID_Get_Protocol_Confirmation_Data_t    *HID_Get_Protocol_Confirmation_Data;
      HID_Set_Protocol_Indication_Data_t      *HID_Set_Protocol_Indication_Data;
      HID_Set_Protocol_Confirmation_Data_t    *HID_Set_Protocol_Confirmation_Data;
      HID_Get_Idle_Indication_Data_t          *HID_Get_Idle_Indication_Data;
      HID_Get_Idle_Confirmation_Data_t        *HID_Get_Idle_Confirmation_Data;
      HID_Set_Idle_Indication_Data_t          *HID_Set_Idle_Indication_Data;
      HID_Set_Idle_Confirmation_Data_t        *HID_Set_Idle_Confirmation_Data;
      HID_Data_Indication_Data_t              *HID_Data_Indication_Data;
      HID_Data_Buffer_Empty_Indication_Data_t *HID_Data_Buffer_Empty_Indication_Data;
   } Event_Data;
} HID_Event_Data_t;

#define HID_EVENT_DATA_SIZE                             (sizeof(HID_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Human Interface Device Profile Event Callback.  This function   */
   /* will be called whenever a HID Event occurs that is associated with*/
   /* the specified Bluetooth Stack ID.  This function passes to the    */
   /* caller the Bluetooth Stack ID, the HID Event Data that occurred   */
   /* and the HID Event Callback Parameter that was specified when this */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the HID Event Data ONLY in the context of this callback.  If the  */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another HID Profile Event will*/
   /* not be processed while this function call is outstanding).        */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving HID Event Packets.  */
   /*            A Deadlock WILL occur because NO HID Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *HID_Event_Callback_t)(unsigned int BluetoothStackID, HID_Event_Data_t *HID_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Registering a Host      */
   /* Server.  Note that only one Server can be Registered for each     */
   /* Bluetooth Stack.  This function accepts the Bluetooth Stack ID of */
   /* the Bluetooth Stack which this Server is to be associated with.   */
   /* The second parameter to this function is the HID Configuration    */
   /* Specification to be used in the negotiation of the L2CAP channels */
   /* associated with this Host Server.  The final two parameters       */
   /* specify the HID Event Callback function  and Callback parameter,  */
   /* respectively, of the HID Event Callback that is to process any    */
   /* further events associated with this Host Server.  The HIDID to be */
   /* used in function calls associated with connections made on this   */
   /* server will be returned in Open Indication Events via the Callback*/
   /* associated with this Server.  This function returns zero if       */
   /* successful, or a negative return error code if an error occurred  */
   /* (see BTERRORS.H).                                                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Register_Host_Server(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Register_Host_Server_t)(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Registering a Device    */
   /* Server.  Note that only one Server can be Registered for each     */
   /* Bluetooth Stack ID.  This function accepts the Bluetooth Stack ID */
   /* of the Bluetooth Stack which this Server is to be associated with.*/
   /* The second parameter to this function is the HID Configuration    */
   /* Specification to be used in the negotiation of the L2CAP channels */
   /* associated with this Device Server.  The final two parameters     */
   /* specify the HID Event Callback function and Callback Parameter,   */
   /* respectively, of the HID Event Callback that is to process any    */
   /* further events associated with this Device Server.  The HIDID to  */
   /* be used in function calls associated with connections made on this*/
   /* server will be returned in Open Indication Events via the callback*/
   /* associated with this Server.  This function returns zero if       */
   /* successful, or a negative return error code if an error occurred  */
   /* (see BTERRORS.H).                                                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Register_Device_Server(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Register_Device_Server_t)(unsigned int BluetoothStackID, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local HID Server.  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack associated with the HID Server that is responding */
   /* to the request.  The second parameter to this function is the HID */
   /* ID of the HID Server for which an open request was received.  The */
   /* final parameter to this function specifies whether to accept the  */
   /* pending connection request (or to reject the request).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etHID_Open_Indication event has occurred.              */
   /* ** NOTE ** This event will only be dispatched if the server mode  */
   /*            was explicitly set to hsmManualAccept via the          */
   /*            HID_Set_Server_Connection_Mode() function.             */
BTPSAPI_DECLARATION int BTPSAPI HID_Open_Request_Response(unsigned int BluetoothStackID, unsigned int HIDID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for Un-Registering a HID    */
   /* Server (which was Registered by a successful call to either the   */
   /* HID_Register_Host_Server() or HID_Register_Device_Server()        */
   /* function).  This function accepts as input the Bluetooth Stack ID */
   /* of the Bluetooth Protocol Stack that the Server was registered    */
   /* for.  This function returns zero if successful, or a negative     */
   /* return error code if an error occurred (see BTERRORS.H).  Note    */
   /* that this function does NOT delete any SDP Service Record Handles.*/
BTPSAPI_DECLARATION int BTPSAPI HID_Un_Register_Server(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Un_Register_Server_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is provided to allow a means to add a Human*/
   /* Interface Device (HID) Record to the SDP Database.  This function */
   /* takes as its first parameter the Bluetooth Stack ID of the Local  */
   /* Bluetooth Protocol Stack.  The second parameter to this function  */
   /* is the Device Flags which are used to specify the value for the   */
   /* BOOLEAN Type Attributes associated with this SDP Record.  The Bit */
   /* Definitions that are used can be found near the top of this file. */
   /* The Device Release Number, Parser Version, and Device Subclass    */
   /* parameters are the values used with the Attributes of the same    */
   /* name for this SDP Record.  The next parameter to this function,   */
   /* NumberDescriptors, defines the number of HID Descriptor structures*/
   /* found in the array specified by the next parameter, the           */
   /* DescriptorList.  This parameter specifies the List of Descriptors */
   /* to be associated with this SDP Record.  The ServiceName parameter */
   /* specifies the Service Name to associate with this SDP Record.  The*/
   /* final parameter is a pointer to a DWord_t which receives the SDP  */
   /* Service Record Handle if this function successfully creates an SDP*/
   /* Service Record.  If this function returns zero, then the          */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          HID_Un_Register_Device_SDP_Record() to the               */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
   /* * NOTE * A HID LANGID Base List Attribute ID is created that      */
   /*          specifies 0x0100, English (United States).               */
   /* * NOTE * A HID Country Code Attribute ID is created that specifies*/
   /*          that the hardware is not localized (a value of zero).    */
BTPSAPI_DECLARATION int BTPSAPI HID_Register_Device_SDP_Record(unsigned int BluetoothStackID, unsigned long DeviceFlags, Word_t DeviceReleaseNumber, Word_t ParserVersion, Byte_t DeviceSubclass, unsigned int NumberDescriptors, HID_Descriptor_t DescriptorList[], char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Register_Device_SDP_Record_t)(unsigned int BluetoothStackID, unsigned long DeviceFlags, Word_t DeviceReleaseNumber, Word_t ParserVersion, Word_t DeviceSubclass, unsigned int NumberDescriptors, HID_Descriptor_t DescriptorList[], char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following function is provided to allow a means to add a Human*/
   /* Interface Device (HID) Record to the SDP Database (specifying a   */
   /* specific HID version).  This function takes as its first parameter*/
   /* the Bluetooth Stack ID of the Local Bluetooth Protocol Stack.  The*/
   /* second parameter to this function is the Device Flags which are   */
   /* used to specify the value for the BOOLEAN Type Attributes         */
   /* associated with this SDP Record.  The Bit Definitions that are    */
   /* used can be found near the top of this file.  The Device Release  */
   /* Number, Parser Version, and Device Subclass parameters are the    */
   /* values used with the Attributes of the same name for this SDP     */
   /* Record.  The next parameter to this function, NumberDescriptors,  */
   /* defines the number of HID Descriptor structures found in the array*/
   /* specified by the next parameter, the DescriptorList.  This        */
   /* parameter specifies the List of Descriptors to be associated with */
   /* this SDP Record.  The ServiceName parameter specifies the Service */
   /* Name to associate with this SDP Record.  The next parameter       */
   /* specifies the actual HID version that is to be published.  The    */
   /* next parameter is an optional pointer to a structure containing   */
   /* additional information to add to the HID record.  The final       */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          HID_Un_Register_Device_SDP_Record() to the               */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
   /* * NOTE * A HID LANGID Base List Attribute ID is created that      */
   /*          specifies 0x0100, English (United States).               */
   /* * NOTE * A HID Country Code Attribute ID is created that specifies*/
   /*          that the hardware is not localized (a value of zero).    */
   /* * NOTE * The RecordInformation parameter is an optional parameter */
   /*          and can be used to add optional attributes to the SDP    */
   /*          record.                                                  */
BTPSAPI_DECLARATION int BTPSAPI HID_Register_Device_SDP_Record_Version(unsigned int BluetoothStackID, unsigned long DeviceFlags, Word_t DeviceReleaseNumber, Word_t ParserVersion, Byte_t DeviceSubclass, unsigned int NumberDescriptors, HID_Descriptor_t DescriptorList[], char *ServiceName, HID_Version_t HIDVersion, HID_SDP_Record_Information_t *RecordInformation, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Register_Device_SDP_Record_Version_t)(unsigned int BluetoothStackID, unsigned long DeviceFlags, Word_t DeviceReleaseNumber, Word_t ParserVersion, Word_t DeviceSubclass, unsigned int NumberDescriptors, HID_Descriptor_t DescriptorList[], char *ServiceName, HID_Version_t HIDVersion, HID_SDP_Record_Information_t *RecordInformation, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the HID*/
   /* SDP Service Record (specified by the third parameter) from the SDP*/
   /* Database.  This MACRO simply maps to the                          */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on, the HID ID (returned from a successful  */
   /* call to the HID_Register_Device_Server() function), and the SDP   */
   /* Service Record Handle.  The SDP Service Record Handle was returned*/
   /* via a successful call to the HID_Register_Device_SDP_Record()     */
   /* function.  See the HID_Register_Device_SDP_Record() function for  */
   /* more information.  This MACRO returns the result of the           */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define HID_Un_Register_Device_SDP_Record(__BluetoothStackID, __HIDID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a connection to */
   /* a Remote HID Device on the Specified Bluetooth Device.  This      */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack which is to open the HID Connection.  The     */
   /* second parameter specifies the Board Address (NON NULL) of the    */
   /* Remote Bluetooth Device to connect with.  The third parameter to  */
   /* this function is the HID Configuration Specification to be used in*/
   /* the negotiation of the L2CAP Channels associated with this Device */
   /* Client.  The final two parameters specify the HID Event Callback  */
   /* function and Callback Parameter, respectively, of the HID Event   */
   /* Callback that is to process any further events associated with    */
   /* this Device Client.  This function returns a non-zero, positive,  */
   /* value if successful, or a negative return error code if this      */
   /* function is unsuccessful.  If this function is successful, the    */
   /* return value will represent the HID ID that can be passed to all  */
   /* other functions that require it.  Once a Connection is opened to a*/
   /* Remote Device it can only be closed via a call to the             */
   /* HID_Close_Connection() function (passing in the return value from */
   /* a successful call to this function as the HID ID input parameter).*/
BTPSAPI_DECLARATION int BTPSAPI HID_Connect_Remote_Device(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Connect_Remote_Device_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a connection to */
   /* a Remote HID Host on the Specified Bluetooth Device.  This        */
   /* function accepts as its first parameter the Bluetooth Stack ID of */
   /* the Bluetooth Stack which is to open the HID Connection.  The     */
   /* second parameter specifies the Board Address (NON NULL) of the    */
   /* Remote Bluetooth Device to connect with.  The third parameter to  */
   /* this function is the HID Configuration Specification to be used in*/
   /* the negotiation of the L2CAP Channels associated with this Host   */
   /* Client.  The final two parameters specify the HID Event Callback  */
   /* function and Callback Parameter, respectively, of the HID Event   */
   /* Callback that is to process any further events associated with    */
   /* this Host Client.  This function returns a non-zero, positive,    */
   /* value if successful, or a negative return error code if this      */
   /* function is unsuccessful.  If this function is successful, the    */
   /* return value will represent the HID ID that can be passed to all  */
   /* other functions that require it.  Once a Connection is opened to a*/
   /* Remote Host it can only be closed via a call to the               */
   /* HID_Close_Connection() function (passing in the return value from */
   /* a successful call to this function as the HID ID input parameter).*/
BTPSAPI_DECLARATION int BTPSAPI HID_Connect_Remote_Host(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Connect_Remote_Host_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, HID_Configuration_t *HIDConfiguration, HID_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for closing a HID connection*/
   /* established through a connection made to a Registered Server or a */
   /* connection that was made by calling either the                    */
   /* HID_Open_Remote_Device() or HID_Open_Remote_Host() functions.     */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HID ID specified by the Second  */
   /* Parameter is valid for.  This function returns zero if successful,*/
   /* or a negative return error code if an error occurred.  Note that  */
   /* if this function is called with the HID ID of a Local Server, the */
   /* Server will remain registered but the connection associated with  */
   /* the specified HID ID will be closed.                              */
BTPSAPI_DECLARATION int BTPSAPI HID_Close_Connection(unsigned int BluetoothStackID, unsigned int HIDID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int HIDID);
#endif

   /* The following function is responsible for Sending a HID_CONTROL   */
   /* transaction to the remote side.  This function accepts as input   */
   /* the Bluetooth Stack ID of the Bluetooth Stack which is to send the*/
   /* request and the HID ID for which the Connection has been          */
   /* established.  The third parameter is the Control Operation that   */
   /* will be sent.  This function returns zero if successful, or a     */
   /* negative return error code if there as an error.                  */
   /* * NOTE * HID Devices are only capable of sending                  */
   /*          hcVirtualCableUnplug Control Operations. All other       */
   /*          Control Operations performed by a Device will return an  */
   /*          error result.                                            */
   /* * NOTE * Control Channel Transfers normally consist of two phases,*/
   /*          a Request by the Host and a Response by the Device.      */
   /*          However, HID Control transactions require no Response    */
   /*          phase.  Note that HID Control Requests are not allowed   */
   /*          while other transactions are being processed unless the  */
   /*          Control Operation Type is hcVirtualCableUnplug which may */
   /*          be sent at any time.                                     */
BTPSAPI_DECLARATION int BTPSAPI HID_Control_Request(unsigned int BluetoothStackID, unsigned int HIDID, HID_Control_Operation_Type_t ControlOperation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Control_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Control_Operation_Type_t ControlOperation);
#endif

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Stack which is to send the*/
   /* request and the HID ID for which the Connection has been          */
   /* established.  The third parameter is the descriptor that indicates*/
   /* how the device is to determine the size of the buffer that the    */
   /* host has allocated.  The fourth parameter is the type of report   */
   /* requested.  The fifth parameter is the Report ID determined by the*/
   /* Device's SDP record.  Passing HID_INVALID_REPORT_ID as the value  */
   /* for this parameter will indicate that this parameter is not used  */
   /* and will exclude the appropriate byte from the transaction        */
   /* payload.  The fifth parameters use is based on the parameter      */
   /* passed as Size.  If the host indicates it has allocated a buffer  */
   /* of a size smaller than the report it is requesting, this parameter*/
   /* will be used as the size of the report returned.  Otherwise, the  */
   /* appropriate bytes will not be included in the transaction payload.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Get Report Confirmation event         */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Report_Request(unsigned int BluetoothStackID, unsigned int HIDID, HID_Get_Report_Size_Type_t Size, HID_Report_Type_Type_t ReportType, Byte_t ReportID, Word_t BufferSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Report_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Get_Report_Size_Type_t Size, HID_Report_Type_Type_t ReportType, Byte_t ReportID, Word_t BufferSize);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_REPORT transaction.  This function */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Statuses are used to  */
   /* respond, a HANDSHAKE response that has a Result Code parameter of */
   /* the specified Error Condition is sent.  If the ResultType         */
   /* specified is rtData, the GET_REPORT transaction is responded to   */
   /* with a DATA Response that has the Report (specified by the final  */
   /* parameter) as its Payload.  The fourth parameter is the type of   */
   /* report being sent.  Note that rtOther is an Invalid Report Type   */
   /* for use with this function.  The final two parameters are the     */
   /* Length of the Report Payload to send and a pointer to the Report  */
   /* Payload that will be sent.  This function returns a zero if       */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Report_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Report_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);
#endif

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* transaction and the HID ID for which the Connection has been      */
   /* established.  The third parameter is the type of report being     */
   /* sent.  Note that rtOther is an Invalid Report Type for use with   */
   /* this function.  The final two parameters to this function are the */
   /* Length of the Report Payload to send and a pointer to the Report  */
   /* Payload that will be sent.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Set Report Confirmation event         */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Report_Request(unsigned int BluetoothStackID, unsigned int HIDID, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Report_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_REPORT transaction.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified, this function responds to the SET_REPORT request with a*/
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Report_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Report_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);
#endif

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Stack which is to   */
   /* send the request and the HID ID for which the Connection has been */
   /* established.  This function returns a zero if successful, or a    */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Get Protocol Confirmation event       */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Protocol_Request(unsigned int BluetoothStackID, unsigned int HIDID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Protocol_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtSuccessful Result Type is Invalid for use with   */
   /* this function.  If the rtNotReady through rtErrFatal Result Types */
   /* are specified, this function will respond to the GET_PROTOCOL     */
   /* request with a HANDSHAKE response that has a Result Code parameter*/
   /* of the specified Error Condition.  If the ResultType specified is */
   /* rtData, the GET_PROTOCOL transaction is responded to with a DATA  */
   /* Response that has the Protocol type specified as the final        */
   /* parameter as its Payload.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Protocol_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, HID_Protocol_Type_t Protocol);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Protocol_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, HID_Protocol_Type_t Protocol);
#endif

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the HID ID for which the Connection has been          */
   /* established.  The last parameter is the protocol to be set.  This */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Set Protocol Confirmation event       */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Protocol_Request(unsigned int BluetoothStackID, unsigned int HIDID, HID_Protocol_Type_t Protocol);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Protocol_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Protocol_Type_t Protocol);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified then this function will respond to the SET_PROTOCOL     */
   /* Transaction with a HANDSHAKE response that has a Result Code      */
   /* parameter that matches the specified Result Type.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Protocol_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Protocol_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);
#endif

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the HID ID for which the Connection has been          */
   /* established.  This function returns a zero if successful, or a    */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Get Idle Confirmation event           */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Idle_Request(unsigned int BluetoothStackID, unsigned int HIDID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Idle_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Types are specified,  */
   /* then this function will respond to the GET_IDLE Transaction with a*/
   /* HANDSHAKE response that has a Result Code parameter of the        */
   /* specified Error Condition.  If the ResultType specified is rtData */
   /* the GET_IDLE transaction is responded to with a DATA Response that*/
   /* has the Idle Rate specified as the final parameter as its Payload.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Idle_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, Byte_t IdleRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Idle_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType, Byte_t IdleRate);
#endif

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts the  */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to send the    */
   /* request and the HID ID for which the Connection has been          */
   /* established.  The last parameter is the Idle Rate to be set.  The */
   /* Idle Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution   */
   /* is 4ms with a range from 4ms to 1.020s).  This function returns a */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Set Idle Confirmation event           */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Idle_Request(unsigned int BluetoothStackID, unsigned int HIDID, Byte_t IdleRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Idle_Request_t)(unsigned int BluetoothStackID, unsigned int HIDID, Byte_t IdleRate);
#endif

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The rtData     */
   /* Result Type is Invalid for use with this function.  If the        */
   /* rtSuccessful through rtErrFatal Result Types are specified, then  */
   /* this function will respond to the SET_IDLE Transaction with a     */
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Idle_Response(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Idle_Response_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Result_Type_t ResultType);
#endif

   /* The following function is responsible for sending Reports over the*/
   /* Interrupt Channel.  This function accepts the Bluetooth Stack ID  */
   /* of the Bluetooth Stack which is to send the Report Data and the   */
   /* HID ID for which the Connection has been established.  The third  */
   /* parameter is the type of report being sent.  The final two        */
   /* parameters are the Length of the Report Payload to send and a     */
   /* pointer to the Report Payload that will be sent.  Note that       */
   /* rtOther and rtFeature are Invalid Report Types for use with this  */
   /* function.  Also note that rtInput Reports must be sent from the   */
   /* Device to the Host, and that rtOutput Reports must be sent from   */
   /* the Host to the Device.  This function returns a zero if          */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns the Error Code:                 */
   /*          BTPS_ERROR_INSUFFICIENT_BUFFER_SPACE then this is a      */
   /*          signal to the caller that the requested data could NOT be*/
   /*          sent because the requested data could not be queued in   */
   /*          the Outgoing L2CAP Queue.  The caller then, must wait for*/
   /*          the etHID_Data_Buffer_Empty_Indication Event before      */
   /*          trying to send any more data.                            */
BTPSAPI_DECLARATION int BTPSAPI HID_Data_Write(unsigned int BluetoothStackID, unsigned int HIDID, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Data_Write_t)(unsigned int BluetoothStackID, unsigned int HIDID, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);
#endif

   /* The following function is responsible for retrieving the current  */
   /* HID Server Connection Mode.  This function accepts as its first   */
   /* parameter the Bluetooth Stack ID of the Bluetooth Stack on which  */
   /* the server exists.  The final parameter to this function is a     */
   /* pointer to a Server Connection Mode variable which will receive   */
   /* the current Server Connection Mode.  This function returns zero if*/
   /* successful, or a negative return error code if an error occurred. */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            hsmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for HID Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Server_Connection_Mode(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t *ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t *ServerConnectionMode);
#endif

   /* The following function is responsible for setting the HID Server  */
   /* Connection Mode.  This function accepts as its first parameter the*/
   /* Bluetooth Stack ID of the Bluetooth Stack in which the server     */
   /* exists.  The second parameter to this function is the new Server  */
   /* Connection Mode to set the Server to use.  Connection requests    */
   /* will not be dispatched unless the Server Mode (second parameter)  */
   /* is set to hsmManualAccept.  In this case the Callback that was    */
   /* registered with the server will be invoked whenever a Remote      */
   /* Bluetooth Device attempts to connect to the Local Device.  If the */
   /* Server Mode (second parameter) is set to anything other than      */
   /* hsmManualAccept then the then no open request indication events   */
   /* will be dispatched.  This function returns zero if successful, or */
   /* a negative return error code if an error occurred.                */
   /* ** NOTE ** The Default Server Connection Mode is                  */
   /*            hsmAutomaticAccept.                                    */
   /* ** NOTE ** This function is used for HID Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Server_Connection_Mode(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t ServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, HID_Server_Connection_Mode_t ServerConnectionMode);
#endif

   /* The following function is a utility function that exists to query */
   /* the currently configured HID Data Queing parameters.  These       */
   /* parameters dictate how the data packets are queued into L2CAP.    */
   /* This mechanism allows for the ability to implement a streaming    */
   /* type interface by limiting the number of packets that can queued. */
   /* This is useful to keep L2CAP from infinitely queing packets which */
   /* can lead to stale data if there is an issue sending the data to   */
   /* the remote device (i.e. interference).  This function accepts the */
   /* Bluetooth Stack ID followed by a pointer to a buffer that is to   */
   /* receive the current L2CAP Queueing parameters that are being used */
   /* by HID.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * This function sets the queing parameters globally for    */
   /*          HID.  Setting the Queing parameters for an individual    */
   /*          connection is currently not supported.                   */
   /* * NOTE * A value of zero for the QueueLimit member of the L2CAP   */
   /*          queing parameters means that there is no queing active   */
   /*          (i.e. all packets are queued, regardless of the queue    */
   /*          depth).                                                  */
BTPSAPI_DECLARATION int BTPSAPI HID_Get_Data_Queueing_Parameters(unsigned int BluetoothStackID, L2CA_Queueing_Parameters_t *QueueingParameters);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Get_Data_Queueing_Parameters_t)(unsigned int BluetoothStackID, L2CA_Queueing_Parameters_t *QueueingParameters);
#endif

   /* The following function is a utility function that exists to change*/
   /* the currently configured HID Data Queing parameters.  These       */
   /* parameters dictate how the data packets are queued into L2CAP.    */
   /* This mechanism allows for the ability to implement a streaming    */
   /* type interface by limiting the number of packets that can queued. */
   /* This is useful to keep L2CAP from infinitely queing packets which */
   /* can lead to stale data if there is an issue sending the data to   */
   /* the remote device (i.e. interference).  This function accepts the */
   /* Bluetooth Stack ID followed by a pointer to a buffer that contains*/
   /* the new L2CAP Queueing parameters that are to be used by HID.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * This function sets the queing parameters globally for    */
   /*          HID.  Setting the Queing parameters for an individual    */
   /*          connection is currently not supported.                   */
   /* * NOTE * A value of zero for the QueueLimit member of the L2CAP   */
   /*          queing parameters means that there is no queing active   */
   /*          (i.e. all packets are queued, regardless of the queue    */
   /*          depth).                                                  */
BTPSAPI_DECLARATION int BTPSAPI HID_Set_Data_Queueing_Parameters(unsigned int BluetoothStackID, L2CA_Queueing_Parameters_t *QueueingParameters);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HID_Set_Data_Queueing_Parameters_t)(unsigned int BluetoothStackID, L2CA_Queueing_Parameters_t *QueueingParameters);
#endif

#endif
