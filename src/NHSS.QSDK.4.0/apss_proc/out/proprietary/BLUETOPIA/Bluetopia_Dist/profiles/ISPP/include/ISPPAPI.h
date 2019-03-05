/*****< isppapi.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ISPPAPI - Stonestreet One Bluetooth Stack ISPP API Type Definitions,      */
/*            Constants, and Prototypes.                                      */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/22/11  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __ISPPAPIH__
#define __ISPPAPIH__

#include "SS1BTPS.h"           /* Bluetooth Stack API Prototypes/Constants.   */

#include "IAPTypes.h"          /* Bluetooth iAP Type Definitions/Constants.   */

   /* The following define the error codes that may be returned by this */
   /* module.                                                           */
#define ISPP_ERROR_SUCCESS                                       (0)

#define ISPP_ERROR_INVALID_PARAMETER                         (-1000)
#define ISPP_ERROR_INVALID_BLUETOOTH_STACK_ID                (-1001)
#define ISPP_ERROR_NOT_INITIALIZED                           (-1002)
#define ISPP_ERROR_FAILED_TO_INITIALIZE                      (-1003)
#define ISPP_ERROR_FAILED_TO_REGISTER_SERVER                 (-1004)
#define ISPP_ERROR_INSUFFICIENT_BUFFER_SPACE                 (-1005)
#define ISPP_ERROR_MAX_PAYLOAD_SIZE_EXCEEDED                 (-1006)
#define ISPP_ERROR_INSUFFICIENT_RESOURCES                    (-1007)
#define ISPP_ERROR_NOT_SUPPORTED                             (-1008)
#define ISPP_ERROR_NOT_ALLOWED                               (-1009)
#define ISPP_ERROR_INVALID_SERIAL_PORT_ID                    (-1010)
#define ISPP_ERROR_INVALID_SESSION_ID                        (-1011)
#define ISPP_ERROR_INVALID_PACKET_ID                         (-1012)
#define ISPP_ERROR_AUTHENTICATION_IACP_FAILURE               (-1013)

   /* The following define the status information values that may be    */
   /* supplied in the Process Status Event.                             */
   /* * NOTE * A Process Failure indicates a rejection of a command or  */
   /*          non-support for a feature.  A General Failure indicates a*/
   /*          system failure due to insufficient resources or          */
   /*          communication error with the Authentication Coprocessor. */
#define IAP_PROCESS_STATUS_SUCCESS                                 0
#define IAP_PROCESS_STATUS_ERROR_RETRYING                          1
#define IAP_PROCESS_STATUS_TIMEOUT_HALTING                         2
#define IAP_PROCESS_STATUS_GENERAL_FAILURE                         3
#define IAP_PROCESS_STATUS_PROCESS_FAILURE                         4
#define IAP_PROCESS_STATUS_PROCESS_TIMEOUT_RETRYING                5

   /* The following constants represent the Port Open Status Values that*/
   /* are possible in the ISPP Open Port Confirmation Event Data        */
   /* Information.                                                      */
#define ISPP_OPEN_PORT_STATUS_SUCCESS                 (SPP_OPEN_PORT_STATUS_SUCCESS)
#define ISPP_OPEN_PORT_STATUS_CONNECTION_TIMEOUT      (SPP_OPEN_PORT_STATUS_CONNECTION_TIMEOUT)
#define ISPP_OPEN_PORT_STATUS_CONNECTION_REFUSED      (SPP_OPEN_PORT_STATUS_CONNECTION_REFUSED)
#define ISPP_OPEN_PORT_STATUS_UNKNOWN_ERROR           (SPP_OPEN_PORT_STATUS_UNKNOWN_ERROR)

   /* The following defines the status values that can be returned in   */
   /* the Send Session Data Confirmation or Send Raw Confirmation event.*/
#define PACKET_SEND_STATUS_SENT                                    0
#define PACKET_SEND_STATUS_ACKNOWLEDGED                            1
#define PACKET_SEND_STATUS_FAILED                                  2
#define PACKET_SEND_STATUS_CANCELED                                3
#define PACKET_SEND_STATUS_SESSION_CLOSED                          4

   /* The following defines the values that may be supplied in the      */
   /* Response parameter of the Session Open Response command.          */
#define SESSION_OPEN_RESPONSE_ACCEPT                               0
#define SESSOPN_OPEN_RESPONSE_REJECT                               1

   /* The following define the values that are returned in the ErrorCode*/
   /* parameter of a Transport Error Event.                             */
#define TRANSPORT_ERROR_RECEIVE_BUFFER_OVERRUN                     1
#define TRANSPORT_ERROR_CHECKSUM_FAILURE                           2

   /* The following enumerates the various states that the              */
   /* authentication state machine may be in during the identification  */
   /* and authentication process.  These values are supplied in the     */
   /* status field of a Process Status Event structure.                 */
typedef enum
{
   psStartIdentificationRequest,
   psStartIdentificationProcess,
   psIdentificationProcess,
   psIdentificationProcessComplete,
   psStartAuthenticationProcess,
   psAuthenticationProcess,
   psAuthenticationProcessComplete
} Process_State_t;

   /* The following enumeration defines the operating mode of an active */
   /* port.                                                             */
typedef enum
{
   pomSPP,
   pomiAP,
   pomiAP2
} Port_Operating_Mode_t;

   /* The following structure is used with the                          */
   /* ISPP_Register_SDP_Record() function.  This structure (when        */
   /* specified) contains additional SDP Service Information that will  */
   /* be added to the SDP SPP Service Record Entry.  The first member of*/
   /* this structure specifies the Number of Service Class UUIDs that   */
   /* are present in the SDPUUIDEntries Array.  This member must be at  */
   /* least one, and the SDPUUIDEntries member must point to an array of*/
   /* SDP UUID Entries that contains (at least) as many entries         */
   /* specified by the NumberServiceClassUUID member.  The ProtocolList */
   /* member is an SDP Data Element Sequence that contains a list of    */
   /* Protocol Information that will be added to the generic SDP Service*/
   /* Record.                                                           */
typedef struct _tagISPP_SDP_Service_Record_t
{
   unsigned int        NumberServiceClassUUID;
   SDP_UUID_Entry_t   *SDPUUIDEntries;
   SDP_Data_Element_t *ProtocolList;
} ISPP_SDP_Service_Record_t;

#define ISPP_SDP_SERVICE_RECORD_SIZE                     (sizeof(ISPP_SDP_Service_Record_t))

   /* The following structure is used with the                          */
   /* ISPP_Register_SDP_Record() function.  This structure (when        */
   /* specified) contains additional SDP Service Information that will  */
   /* be added to the SDP SPP Service Record Entry.  The first member of*/
   /* this structure specifies the Number of Service Class UUIDs that   */
   /* are present in the SDPUUIDEntries Array.  This member must be at  */
   /* least one, and the SDPUUIDEntries member must point to an array of*/
   /* SDP UUID Entries that contains (at least) as many entries         */
   /* specified by the NumberServiceClassUUID member.  The              */
   /* NumberOfProtocolDataListUUIDOffsets and                           */
   /* ProtocolDataListUUIDOffsets specify the offsets of the UUIDs in   */
   /* the specified ProtocolDataList data (if any UUIDs).  The          */
   /* ProtocolDataListLength and ProtocolDataList members must contain  */
   /* (if specified) a formatted SDP Data Element Sequence that contains*/
   /* a list of Protocol Information that will be added to the generic  */
   /* SDP Service Record.                                               */
typedef struct _tagISPP_SDP_Raw_Service_Record_t
{
   unsigned int      NumberServiceClassUUID;
   SDP_UUID_Entry_t *SDPUUIDEntries;
   unsigned int      NumberOfProtocolDataListUUIDOffsets;
   Word_t           *ProtocolDataListUUIDOffsets;
   unsigned int      ProtocolDataListLength;
   Byte_t           *ProtocolDataList;
} ISPP_SDP_Raw_Service_Record_t;

#define ISPP_SDP_RAW_SERVICE_RECORD_SIZE                 (sizeof(ISPP_SDP_Raw_Service_Record_t))

   /* ISPP Event API Types.                                             */
typedef enum
{
   etIPort_Open_Indication,
   etIPort_Open_Confirmation,
   etIPort_Close_Port_Indication,
   etIPort_Status_Indication,
   etIPort_Data_Indication,
   etIPort_Transmit_Buffer_Empty_Indication,
   etIPort_Line_Status_Indication,
   etIPort_Send_Port_Information_Indication,
   etIPort_Send_Port_Information_Confirmation,
   etIPort_Query_Port_Information_Indication,
   etIPort_Query_Port_Information_Confirmation,
   etIPort_Open_Request_Indication,
   etIPort_Process_Status,
   etIPort_Open_Session_Indication,
   etIPort_Close_Session_Indication,
   etIPort_Session_Data_Indication,
   etIPort_Send_Session_Data_Confirmation,
   etIPort_Raw_Data_Indication,
   etIPort_Send_Raw_Data_Confirmation,
   etIPort_Transport_Error,
   etIPort_Unhandled_Control_Session_Message
} ISPP_Event_Type_t;

   /* ******************** DEPRECATED IDENTIFIERS ********************* */
   /* ** The following section is for backwards compatibility ONLY.  ** */
   /* ** These should not be used for new designs.                   ** */
   /* ***************************************************************** */

#define IAP_LINGO_GENERAL                             IAP_LINGO_ID_GENERAL
#define IAP_LINGO_MICROPHONE                          IAP_LINGO_ID_MICROPHONE
#define IAP_LINGO_SIMPLE_REMOTE                       IAP_LINGO_ID_SIMPLE_REMOTE
#define IAP_LINGO_DISPLAY_REMOTE                      IAP_LINGO_ID_DISPLAY_REMOTE
#define IAP_LINGO_ENHANCED_INTERFACE                  IAP_LINGO_ID_ENHANCED_INTERFACE
#define IAP_LINGO_ACCESSORY_POWER                     IAP_LINGO_ID_ACCESSORY_POWER
#define IAP_LINGO_USB_HOST_MODE                       IAP_LINGO_ID_USB_HOST_MODE
#define IAP_LINGO_RF_TUNER                            IAP_LINGO_ID_RF_TUNER
#define IAP_LINGO_ACCESSORY_EQUALIZER                 IAP_LINGO_ID_ACCESSORY_EQUALIZER
#define IAP_LINGO_SPORTS                              IAP_LINGO_ID_SPORTS
#define IAP_LINGO_DIGITAL_AUDIO                       IAP_LINGO_ID_DIGITAL_AUDIO
#define IAP_LINGO_STORAGE                             IAP_LINGO_ID_STORAGE
#define IAP_LINGO_IPOD_OUT                            IAP_LINGO_ID_IPOD_OUT
#define IAP_LINGO_LOCATION                            IAP_LINGO_ID_LOCATION

#define pomWiAP                                       pomiAP

#define ietPort_Open_Indication                       etIPort_Open_Indication
#define ietPort_Open_Confirmation                     etIPort_Open_Confirmation
#define ietPort_Close_Port_Indication                 etIPort_Close_Port_Indication
#define ietPort_Status_Indication                     etIPort_Status_Indication
#define ietPort_Data_Indication                       etIPort_Data_Indication
#define ietPort_Transmit_Buffer_Empty_Indication      etIPort_Transmit_Buffer_Empty_Indication
#define ietPort_Line_Status_Indication                etIPort_Line_Status_Indication
#define ietPort_Send_Port_Information_Indication      etIPort_Send_Port_Information_Indication
#define ietPort_Send_Port_Information_Confirmation    etIPort_Send_Port_Information_Confirmation
#define ietPort_Query_Port_Information_Indication     etIPort_Query_Port_Information_Indication
#define ietPort_Query_Port_Information_Confirmation   etIPort_Query_Port_Information_Confirmation
#define ietPort_Open_Request_Indication               etIPort_Open_Request_Indication
#define ietPort_Process_Status                        etIPort_Process_Status
#define ietPort_Open_Session_Indication               etIPort_Open_Session_Indication
#define ietPort_Close_Session_Indication              etIPort_Close_Session_Indication
#define ietPort_Session_Data_Indication               etIPort_Session_Data_Indication
#define ietPort_Send_Session_Data_Confirmation        etIPort_Send_Session_Data_Confirmation
#define ietPort_Raw_Data_Indication                   etIPort_Raw_Data_Indication
#define ietPort_Send_Raw_Data_Confirmation            etIPort_Send_Raw_Data_Confirmation
#define ietPort_Transport_Error                       etIPort_Transport_Error
#define ietPort_Unhandled_Control_Session_Message     etIPort_Unhandled_Control_Session_Message

   /* ******************** DEPRECATED IDENTIFIERS ********************* */
   /* ** End backwards compatiblity section.                         ** */
   /* ***************************************************************** */

   /* The following Constants represent the Default Packet Retransmit   */
   /* Attempts Values that are supported by the ISPP Module.  These     */
   /* Constants can be used with the ISPP_Set_Configuration_Parameters()*/
   /* and the ISPP_Get_Configuration_Parameters() functions.  The       */
   /* special Constant ISPP_PACKET_TRANSMIT_ATTEMPTS_CURRENT is used    */
   /* with the ISPP_Set_Configuration_Parameters() function to inform   */
   /* the function NOT to change the Default Value                      */
#define ISPP_PACKET_TRANSMIT_ATTEMPTS_MINIMUM                      1
#define ISPP_PACKET_TRANSMIT_ATTEMPTS_MAXIMUM                      255
#define ISPP_PACKET_TRANSMIT_ATTEMPTS_DEFAULT                      5
#define ISPP_PACKET_TRANSMIT_ATTEMPTS_CURRENT                      0

   /* The following Constants represent the Minimum, Maximum and Default*/
   /* Packet Timeout Values that are supported by the ISPP Module.  The */
   /* time values for these constants are specified in Milliseconds.    */
   /* These Constants can be used with the                              */
   /* ISPP_Set_Configuration_Parameters() and the                       */
   /* ISPP_Get_Configuration_Parameters() functions.  The special       */
   /* Constant ISPP_PACKET_TIMEOUT_CURRENT is used with the             */
   /* ISPP_Set_Configuration_Parameters() function to inform the        */
   /* function NOT to change the Default Timeout Value                  */
#define ISPP_PACKET_TIMEOUT_MINIMUM                                500
#define ISPP_PACKET_TIMEOUT_MAXIMUM                                15000
#define ISPP_PACKET_TIMEOUT_DEFAULT                                1500
#define ISPP_PACKET_TIMEOUT_CURRENT                                0

   /* The following Constants represent the Minimum, Maximum and Default*/
   /* Maximum Number of Outstanding Packets that are supported by the   */
   /* ISPP Module.  The value is only used for IAP2 connections.  For   */
   /* IAP connections, this number is fixed at 1.  These Constants can  */
   /* be used with the ISPP_Set_Configuration_Parameters() and the      */
   /* ISPP_Get_Configuration_Parameters() functions.  The special       */
   /* Constant ISPP_PACKET_TIMEOUT_CURRENT is used with the             */
   /* ISPP_Set_Configuration_Parameters() function to inform the        */
   /* function NOT to change the Default Value                          */
#define ISPP_MAX_OUTSTANDING_PACKETS_MINIMUM                       1
#define ISPP_MAX_OUTSTANDING_PACKETS_MAXIMUM                       127
#define ISPP_MAX_OUTSTANDING_PACKETS_DEFAULT                       1
#define ISPP_MAX_OUTSTANDING_PACKETS_CURRENT                       0

   /* The following Constants represent the Minimum, Maximum and Default*/
   /* Buffer Sizes that are supported for Transmit/Receive Buffers.     */
   /* These Constants can be used with the                              */
   /* ISPP_Set_Configuration_Parameters(), and the                      */
   /* ISPP_Get_Configuration_Parameters() functions.  The special       */
   /* Constant ISPP_BUFFER_SIZE_CURRENT is used with the                */
   /* ISPP_Set_Configuration_Parameters() function to inform the        */
   /* function NOT to change the Buffer Size.                           */
#define ISPP_BUFFER_SIZE_MINIMUM                                   (SPP_BUFFER_SIZE_MINIMUM)
#define ISPP_BUFFER_SIZE_MAXIMUM                                   (SPP_BUFFER_SIZE_MAXIMUM)
#define ISPP_BUFFER_SIZE_DEFAULT_TRANSMIT                          (SPP_BUFFER_SIZE_DEFAULT_TRANSMIT)
#define ISPP_BUFFER_SIZE_DEFAULT_RECEIVE                           (SPP_BUFFER_SIZE_DEFAULT_RECEIVE)
#define ISPP_BUFFER_SIZE_CURRENT                                   (SPP_BUFFER_SIZE_CURRENT)

   /* The following Constants represent the Minimum, Maximum and Default*/
   /* RFCOMM Frame Sizes that are supported by the ISPP Module.  These  */
   /* Constants can be used with the ISPP_Set_Configuration_Parameters()*/
   /* and the ISPP_Get_Configuration_Parameters() functions.  The       */
   /* special Constant ISPP_FRAME_SIZE_CURRENT is used with the         */
   /* ISPP_Set_Configuration_Parameters() function to inform the        */
   /* function NOT to change the currently configured Frame Size.       */
#define ISPP_FRAME_SIZE_MINIMUM                                    (SPP_FRAME_SIZE_MINIMUM)
#define ISPP_FRAME_SIZE_MAXIMUM                                    (SPP_FRAME_SIZE_MAXIMUM)
#define ISPP_FRAME_SIZE_DEFAULT                                    (SPP_FRAME_SIZE_DEFAULT)
#define ISPP_FRAME_SIZE_CURRENT                                    (SPP_FRAME_SIZE_CURRENT)

   /* The following constants represent the session types supported by  */
   /* the IAP2 module.  These Constants can be used with the            */
   /* ISPP_Set_Configuration_Parameters() and the                       */
   /* ISPP_Get_Configuration_Parameters() functions.  The special       */
   /* Constant ISPP_SESSION_TYPE_SUPPORTED_CURRENT is used with the     */
   /* ISPP_Set_Configuration_Parameters() function to inform the        */
   /* function NOT to change the currently configured supported session */
   /* types.                                                            */
   /* * NOTE * The IAP2 specification currently defines three session   */
   /*          types that can be supported:                             */
   /*             - Control            - mandatory                      */
   /*             - Extended Accessory - optional                       */
   /*             - File Transfer      - optional                       */
   /*          Because the control session is mandatory it cannot be    */
   /*          be specified.  The remaining session types can be        */
   /*          configured according to the requirements of the product. */
   /* * NOTE * The default configuration specifies the Extended         */
   /*          Accessory Session Type as supported.  If this session    */
   /*          type is not required, then the                           */
   /*          ISPP_Set_Configuration_Parameters() function must called */
   /*          to remove the session type from the currently configured */
   /*          supported sessions.                                      */
#define ISPP_SESSION_TYPE_SUPPORTED_EXTERNAL_ACCESSORY             0x0001
#define ISPP_SESSION_TYPE_SUPPORTED_FILE_TRANSFER                  0x0002
#define ISPP_SESSION_TYPE_SUPPORTED_DEFAULT                        (ISPP_SESSION_TYPE_SUPPORTED_EXTERNAL_ACCESSORY)
#define ISPP_SESSION_TYPE_SUPPORTED_CURRENT                        ((unsigned int)(-1))

   /* The following structure is used to contain information about the  */
   /* accessory.  The device information required for IAP and IAP2 are  */
   /* different.                                                        */
typedef struct _tagISPP_Identification_Info_t
{
   unsigned int   FIDInfoLength;
   unsigned char *FIDInfo;
   unsigned int   IAP2InfoLength;
   unsigned char *IAP2Info;
} ISPP_Identification_Info_t;

   /* The following structure represents the structure of the User      */
   /* configurable parameter structure that can be used to change the   */
   /* default behaviour of ISPP Clients and Servers.  This structure is */
   /* used with the ISPP_Get_Configuration_Parameters() and the         */
   /* ISPP_Set_Configuration_Parameters() functions.                    */
typedef struct _tagISPP_Configuration_Params_t
{
   unsigned int RetransmitAttempts;
   unsigned int PacketTimeout;
   Word_t       MaximumFrameSize;
   unsigned int TransmitBufferSize;
   unsigned int ReceiveBufferSize;
   unsigned int MaxOutstandingPackets;
   unsigned int SessionTypesSupported;
   Word_t       SupportedMessages;
} ISPP_Configuration_Params_t;

#define ISPP_CONFIGURATION_PARAMS_SIZE                (sizeof(ISPP_Configuration_Params_t))

   /* The following structure contains information that is obtained from*/
   /* the Authentication Coprocessor.                                   */
typedef struct _tagISPP_Coprocessor_Info_t
{
   Byte_t  DeviceVersion;
   Byte_t  FirmwareVersion;
   Byte_t  AuthenticationProtocolMajorVersion;
   Byte_t  AuthenticationProtocolMinorVersion;
   DWord_t DeviceID;
   Byte_t  ErrorCode;
} ISPP_Coprocessor_Info_t;

   /* The following defines the Bit Mask for Control Messages that are  */
   /* supported over IAP2 connections.                                  */
#define ISPP_IAP2_CONTROL_MESSAGE_APP_LAUNCH_SUPPORTED         0x0001
#define ISPP_IAP2_CONTROL_MESSAGE_ASSISTIVE_TOUCH_SUPPORTED    0x0002
#define ISPP_IAP2_CONTROL_MESSAGE_BLUETOOTH_STATUS_SUPPORTED   0x0004
#define ISPP_IAP2_CONTROL_MESSAGE_EXTERNAL_ACCESSORY_SUPPORTED 0x0008
#define ISPP_IAP2_CONTROL_MESSAGE_HUMAN_INTERFACE_SUPPORTED    0x0010
#define ISPP_IAP2_CONTROL_MESSAGE_LOCATION_SUPPORTED           0x0020
#define ISPP_IAP2_CONTROL_MESSAGE_MEDIA_LIBRARY_SUPPORTED      0x0040
#define ISPP_IAP2_CONTROL_MESSAGE_NOW_PLAYING_SUPPORTED        0x0080
#define ISPP_IAP2_CONTROL_MESSAGE_POWER_SUPPORTED              0x0100
#define ISPP_IAP2_CONTROL_MESSAGE_USB_AUDIO_SUPPORTED          0x0200
#define ISPP_IAP2_CONTROL_MESSAGE_VOICE_OVER_SUPPORTED         0x0400
#define ISPP_IAP2_CONTROL_MESSAGE_WIFI_SHARING_SUPPORTED       0x0800

   /* The following defines the default set of supported control        */
   /* messages.                                                         */
#define ISPP_IAP2_SUPPORTED_MESSAGES_DEFAULT                   0x000D

   /* The following structure is dispatched to inform the upper layer   */
   /* about the status of the authentication process.                   */
typedef struct _tagISPP_Process_Status_Data_t
{
   unsigned int    SerialPortID;
   Process_State_t ProcessState;
   Byte_t          Status;
} ISPP_Process_Status_Data_t;

   /* The following defines the structure of the Open Session event.    */
   /* This event is dispatched when a Open Session request is received  */
   /* from the Apple Device.                                            */
   /* * NOTE * The user must Accept/Reject this connection by calling   */
   /*          the ISPP_Open_Session_Request_Response function.         */
   /* * NOTE * The MaxTxSessionPayloadSize defines that maximum number  */
   /*          of bytes that can be specified when sending session data */
   /*          to the connected Apple device.                           */
typedef struct _tagISPP_Session_Open_Indication_Data_t
{
   unsigned int SerialPortID;
   Word_t       SessionID;
   Byte_t       ProtocolIndex;
   Word_t       MaxTxSessionPayloadSize;
} ISPP_Session_Open_Indication_Data_t;

   /* The following defines the structure of the Close Session event.   */
   /* This event is dispatched when a Close Session request is received */
   /* from the Apple Device.                                            */
typedef struct _tagISPP_Session_Close_Indication_Data_t
{
   unsigned int SerialPortID;
   Word_t       SessionID;
} ISPP_Session_Close_Indication_Data_t;

   /* The following defines the structure of the Session Data event.    */
   /* This event is dispatched when Session Data is received from the   */
   /* Apple Device.                                                     */
   /* * NOTE * During normal operation the data dispatched in this      */
   /*          callback will be consumed.  If there are no available    */
   /*          resources to consume the packet, the PacketConsumed flag */
   /*          should be set to FALSE.  If a packet is not consumed, the*/
   /*          lower layer to buffer the session data and postpone the  */
   /*          sending of an ACK for the session data.  The pointer to  */
   /*          the data and the data length must be maintained by the   */
   /*          upper layer.  The pointer and length will be valid until */
   /*          the packet is later ACKed.  If the packet is not ACKed is*/
   /*          a reasonable amount of time the remote iOS device might  */
   /*          consider the channel unusable and drop the connection.   */
typedef struct _tagISPP_Session_Data_Indication_Data_t
{
   unsigned int  SerialPortID;
   Word_t        SessionID;
   Word_t        DataLength;
   Byte_t       *DataPtr;
   Boolean_t     PacketConsumed;
} ISPP_Session_Data_Indication_Data_t;

   /* The following defines the structure of the Session Send Data      */
   /* Confirmation event.  This event is dispatched when Session Data   */
   /* has sent to the remote device has been completed.                 */
   /* * NOTE * The PacketID value identifies the packet that has been   */
   /*          sent.  This will be the value that was returned to the   */
   /*          caller when the ISPP_Send_Session_Data function was      */
   /*          called.                                                  */
typedef struct _tagISPP_Send_Session_Data_Confirmation_Data_t
{
   unsigned int SerialPortID;
   unsigned int PacketID;
   Word_t       SessionID;
   Byte_t       Status;
} ISPP_Send_Session_Data_Confirmation_Data_t;

   /* The following structure is used to dispatch raw data to the upper */
   /* layer.  This is dispatched when a packet is received that is not  */
   /* handled by this module.  The data is dispatched to the upper layer*/
   /* for further processing.                                           */
typedef struct _tagISPP_Raw_Data_Indication_Data_t
{
   unsigned int  SerialPortID;
   Byte_t        Lingo;
   Byte_t        Command;
   Word_t        DataLength;
   Byte_t       *DataPtr;
} ISPP_Raw_Data_Indication_Data_t;

   /* The following structure is used to dispatch the data concerned    */
   /* with a Control Session message that was not handled internally by */
   /* ISPP.  The data is dispatched to the upper layer for further      */
   /* processing.                                                       */
typedef struct _tagISPP_Unhandled_Control_Session_Message_Data_t
{
   unsigned int  SerialPortID;
   Word_t        MessageID;
   Word_t        DataLength;
   Byte_t       *DataPtr;
} ISPP_Unhandled_Control_Session_Message_Data_t;

   /* The following defines the structure of the Raw Data Send          */
   /* Confirmation event.  This event is dispatched when processing of  */
   /* Raw Data sent to the remote device has been completed.  The status*/
   /* value indicate whether the packet was successfully received and   */
   /* acknowledged by the remote device.                                */
   /* * NOTE * The PacketID value identifies the packet that has been   */
   /*          sent.  This will be the value that was returned to the   */
   /*          caller when the ISPP_Send_Raw_Data function was called.  */
typedef struct _tagISPP_Send_Raw_Data_Confirmation_Data_t
{
   unsigned int   SerialPortID;
   unsigned int   PacketID;
   Word_t         TransactionID;
   Byte_t         Status;
} ISPP_Send_Raw_Data_Confirmation_Data_t;

   /* The following defines the structure of the Transport Error event. */
   /* This event is dispatched when a packet is received and there is a */
   /* problem processing the data.                                      */
typedef struct _tagISPP_Transport_Error_Data_t
{
   unsigned int SerialPortID;
   Byte_t       ErrorCode;
} ISPP_Transport_Error_Data_t;

   /* The following structure represents the container structure for    */
   /* Holding all ISPP Event Data Data.                                 */
typedef struct _tagISPP_Event_Data_t
{
   ISPP_Event_Type_t Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      /* The following event structures are directly mapped to SPP Event*/
      /* Data Structures.                                               */
      SPP_Open_Port_Indication_Data_t                 *ISPP_Open_Port_Indication_Data;
      SPP_Open_Port_Confirmation_Data_t               *ISPP_Open_Port_Confirmation_Data;
      SPP_Close_Port_Indication_Data_t                *ISPP_Close_Port_Indication_Data;
      SPP_Port_Status_Indication_Data_t               *ISPP_Port_Status_Indication_Data;
      SPP_Data_Indication_Data_t                      *ISPP_Data_Indication_Data;
      SPP_Transmit_Buffer_Empty_Indication_Data_t     *ISPP_Transmit_Buffer_Empty_Indication_Data;
      SPP_Line_Status_Indication_Data_t               *ISPP_Line_Status_Indication_Data;
      SPP_Send_Port_Information_Indication_Data_t     *ISPP_Send_Port_Information_Indication_Data;
      SPP_Send_Port_Information_Confirmation_Data_t   *ISPP_Send_Port_Information_Confirmation_Data;
      SPP_Query_Port_Information_Indication_Data_t    *ISPP_Query_Port_Information_Indication_Data;
      SPP_Query_Port_Information_Confirmation_Data_t  *ISPP_Query_Port_Information_Confirmation_Data;
      SPP_Open_Port_Request_Indication_Data_t         *ISPP_Open_Port_Request_Indication_Data;

      /* The following event structures are specific to ISPP Events.    */
      ISPP_Process_Status_Data_t                      *ISPP_Process_Status;
      ISPP_Session_Open_Indication_Data_t             *ISPP_Session_Open_Indication;
      ISPP_Session_Close_Indication_Data_t            *ISPP_Session_Close_Indication;
      ISPP_Session_Data_Indication_Data_t             *ISPP_Session_Data_Indication;
      ISPP_Send_Session_Data_Confirmation_Data_t      *ISPP_Send_Session_Data_Confirmation;
      ISPP_Raw_Data_Indication_Data_t                 *ISPP_Raw_Data_Indication;
      ISPP_Send_Raw_Data_Confirmation_Data_t          *ISPP_Send_Raw_Data_Confirmation;
      ISPP_Transport_Error_Data_t                     *ISPP_Transport_Error;
      ISPP_Unhandled_Control_Session_Message_Data_t   *ISPP_Unhandled_Control_Session_Message;
   } Event_Data;
} ISPP_Event_Data_t;

#define ISPP_EVENT_DATA_SIZE                             (sizeof(ISPP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an iSPP Event Receive Data Callback.  This function will be called*/
   /* whenever a iSPP Event occurs that is associated with the specified*/
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the iSPP Event Data that occurred and the iSPP*/
   /* Event Callback Parameter that was specified when this Callback was*/
   /* installed.  The caller is free to use the contents of the iSPP    */
   /* iSPP Event Data ONLY in the context of this callback.  If the     */
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
   /* (this argument holds anyway because another iSPP Event will not be*/
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving iSPP Event Packets.  A    */
   /*          Deadlock WILL occur because NO iSPP Event Callbacks will */
   /*          be issued while this function is currently outstanding.  */
typedef void (BTPSAPI *ISPP_Event_Callback_t)(unsigned int BluetoothStackID, ISPP_Event_Data_t *ISPP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for initializing an ISPP    */
   /* Context Layer for the specified Bluetooth Protocol Stack.  This   */
   /* function will allocate and initialize a ISPP Context Information  */
   /* structure and initialize the structure members.  The function     */
   /* receives a void pointer to information that is passed to the Apple*/
   /* Authentication Coprocessor handling function and is considered    */
   /* opaque information to this function.  This function returns zero  */
   /* if successful, or a non-zero value if there was an error.         */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Initialize(unsigned int BluetoothStackID, void *ACP_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Initialize_t)(unsigned int BluetoothStackID, void *ACP_Params);
#endif

   /* The following function is responsible for releasing any resources */
   /* that the ISPP Layer, associated with the Bluetooth Protocol Stack,*/
   /* specified by the Bluetooth Stack ID, has allocated.  Upon         */
   /* completion of this function, ALL ISPP functions will fail if used */
   /* on the specified Bluetooth Protocol Stack.                        */
BTPSAPI_DECLARATION void BTPSAPI ISPP_Cleanup(unsigned int BluetoothStackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ISPP_Cleanup_t)(unsigned int BluetoothStackID);
#endif

   /* The following function is responsible for establishing a Serial   */
   /* Port Server (will wait for a connection to occur on the port      */
   /* established by this function).  The Server is capable of          */
   /* supporting the standard Bluetooth SPP Profile as well as the Apple*/
   /* Accessory protocol.  This function accepts as input the Bluetooth */
   /* Stack ID of the Bluetooth Protocol Stack that this Serial Port is */
   /* to be established with.  The second parameter is the Port Number  */
   /* to establish.  This number *MUST* be between                      */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  The last two*/
   /* parameters specify the ISPP Event Callback function and Callback  */
   /* Parameter, respectively, that will be called with ISPP Events that*/
   /* occur on the specified Serial Port.  This function returns a      */
   /* non-zero, positive, number on success or a negative return error  */
   /* code if an error occurred (see BTERRORS.H).  A successful return  */
   /* code will be a Serial Port ID that can be used to reference the   */
   /* Opened Serial Port in ALL other functions in this module (except  */
   /* the ISPP_Open_Remote_Port() function).  Once a Server Serial Port */
   /* is opened, it can only be Un-Registered via a call to the         */
   /* ISPP_Close_Server_Port() function (passing the return value from  */
   /* this function).  The ISPP_Close_Port() function can be used to    */
   /* Disconnect a Client from the Server Port (if one is connected, it */
   /* will NOT Un-Register the Server Port however.                     */
   /* * NOTE * All ISPP Server Ports are opened in a Manual Accept mode.*/
   /*          When a remote device attempts to connect to the port a   */
   /*          Callback is made to the upper layer to Accept/Reject the */
   /*          connection.  This provides an opportunity to query the   */
   /*          connecting device to determine if the device supports    */
   /*          iAP.                                                     */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Open_Server_Port(unsigned int BluetoothStackID, unsigned int ServerPort, ISPP_Event_Callback_t ISPP_Event_Callback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Open_Server_Port_t)(unsigned int BluetoothStackID, unsigned int ServerPort, ISPP_Event_Callback_t ISPP_Event_Callback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a Serial */
   /* Port Server (which was Registered by a successful call to the     */
   /* ISPP_Open_Server_Port() function).  This function accepts as input*/
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* port is associated with.  The second parameter is the Serial Port */
   /* ID that identifies the Server that is to be closed.  This function*/
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred (see BTERRORS.H).  Note that this function does NOT*/
   /* delete any SDP Service Record Handles.                            */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Close_Server_Port(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Close_Server_Port_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is responsible for responding to requests  */
   /* to connect to a Serial Port Server.  This function accepts as     */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol Stack*/
   /* and the Serial Port ID (which *MUST* have been obtained by calling*/
   /* the ISPP_Open_Server_Port() function).  The third parameter       */
   /* indicates whether the connection should be accepted or rejected.  */
   /* If the connection is rejected, then all further parameters are    */
   /* ignored.  The connection is accepted, the remaining parameters    */
   /* define whether the port will operate in standard SPP mode or iAP  */
   /* mode.  If the following parameters are non-zero/non-NULL, the port*/
   /* will operating in iAP mode.  Identification Information is a      */
   /* pointer to a structure that contains FID information that is      */
   /* passed to the Apple device during the IAP Identification process  */
   /* and possibly the IAP2 information that is passed to the Apple     */
   /* device during the IAP2 Identification process.  .  The            */
   /* MaxRxPacketSize defines the maximum amount of payload, in bytes,  */
   /* that the Apple device is allowed to send in a single packet.  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Open_Port_Request_Response(unsigned int BluetoothStackID, unsigned int SerialPortID, Boolean_t AcceptConnection, ISPP_Identification_Info_t *IdentificationInformation, unsigned int MaxRxPacketSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Open_Port_Request_Response_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Boolean_t AcceptConnection, ISPP_Identification_Info_t *IdentificationInformation, unsigned int MaxRxPacketSize);
#endif

   /* The following function is provided to allow a means to add an iAP */
   /* capable SDP Service Record to the SDP Database.  This function    */
   /* takes as input the Bluetooth Stack ID of the Local Bluetooth      */
   /* Protocol Stack, the Serial Port ID (which *MUST* have been        */
   /* obtained by calling the ISPP_Open_Server_Port() function.  The    */
   /* third parameter (if specified) specifies any additional SDP       */
   /* Information to add to the record.  The fourth parameter specifies */
   /* the Service Name to associate with the SDP Record.  The final     */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the SerialPortID*/
   /*          that was returned from the ISPP_Open_Server_Port()       */
   /*          function.  This function should NEVER be used with the   */
   /*          Serial Port ID returned from the ISPP_Open_Remote_Port() */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the ISDP_Delete_Service_Record()   */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          ISPP_Un_Register_SDP_Record() to                         */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * If NO UUID Information is specified in the               */
   /*          SDPServiceRecord Parameter, then the default SPP Service */
   /*          Classes are added.  Any Protocol Information that is     */
   /*          specified (if any) will be added in the Protocol         */
   /*          Attribute AFTER the default SPP Protocol List (L2CAP and */
   /*          RFCOMM).                                                 */
   /* * NOTE * The UUID that identifies the port as iAP capable is      */
   /*          automatically added to the database..                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI ISPP_Register_SDP_Record(unsigned int BluetoothStackID, unsigned int SerialPortID, ISPP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Register_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, ISPP_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply registers a    */
   /* generic SPP SDP Service Record.  This MACRO simply maps to the    */
   /* ISPP_Register_Raw_SDP_Record() function.  This MACRO is only      */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the Serial Port ID (returned from a */
   /* successful call to the ISPP_Open_Server_Port() function), the     */
   /* Service Name and a pointer to return the SDP Service Record       */
   /* Handle.  See the ISPP_Register_Raw_SDP_Record() function for more */
   /* information.  This MACRO returns the result of the                */
   /* ISPP_Register_Raw_SDP_Record() function, which is zero for success*/
   /* or a negative return error code (see BTERRORS.H).                 */
#define ISPP_Register_Generic_SDP_Record(__BluetoothStackID, __SerialPortID, __ServiceName, __SDPServiceRecordHandle) \
   (ISPP_Register_Raw_SDP_Record(__BluetoothStackID, __SerialPortID, NULL, __ServiceName, __SDPServiceRecordHandle))

   /* The following function is provided to allow a means to add an iAP */
   /* capable SDP Raw Service Record to the SDP Database.  This function*/
   /* takes as input the Bluetooth Stack ID of the Local Bluetooth      */
   /* Protocol Stack, the Serial Port ID (which *MUST* have been        */
   /* obtained by calling the ISPP_Open_Server_Port() function.  The    */
   /* third parameter (if specified) specifies any additional SDP       */
   /* Information to add to the record.  The fourth parameter specifies */
   /* the Service Name to associate with the SDP Record.  The final     */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the SerialPortID*/
   /*          that was returned from the ISPP_Open_Server_Port()       */
   /*          function.  This function should NEVER be used with the   */
   /*          Serial Port ID returned from the ISPP_Open_Remote_Port() */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the ISDP_Delete_Service_Record()   */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          ISPP_Un_Register_SDP_Record() to                         */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * If NO UUID Information is specified in the               */
   /*          SDPServiceRecord Parameter, then the default SPP Service */
   /*          Classes are added.  Any Protocol Information that is     */
   /*          specified (if any) will be added in the Protocol         */
   /*          Attribute AFTER the default SPP Protocol List (L2CAP and */
   /*          RFCOMM).                                                 */
   /* * NOTE * The UUID that identifies the port as iAP capable is      */
   /*          automatically added to the database..                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI ISPP_Register_Raw_SDP_Record(unsigned int BluetoothStackID, unsigned int SerialPortID, ISPP_SDP_Raw_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Register_Raw_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, ISPP_SDP_Raw_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the SPP*/
   /* SDP Service Record (specified by the third parameter) from SDP    */
   /* Database.  This MACRO simply maps to the                          */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on, the Serial Port ID (returned from a     */
   /* successful call to the ISPP_Open_Server_Port() function), and the */
   /* SDP Service Record Handle.  The SDP Service Record Handle was     */
   /* returned via a successful call to the ISPP_Register_SDP_Record()  */
   /* function.  See the ISPP_Register_SDP_Record() function for more   */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define ISPP_Un_Register_SDP_Record(__BluetoothStackID, __SerialPortID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for Opening a Remote Serial */
   /* Port on the specified Remote Device.  This function accepts the   */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to open the    */
   /* Serial Connection as the first parameter.  The second parameter   */
   /* specifies the Board Address (NON NULL) of the Remote Bluetooth    */
   /* Device to connect with.  The next parameter specifies the Remote  */
   /* Server Channel ID to connect.  The following three parameters are */
   /* use to indicate if the port is to operate in standard SPP mode or */
   /* iAP mode.  If the parameters are non-zero/non-NULL, the port will */
   /* operating in iAP mode.  FIDInfo is a pointer to the FID           */
   /* information that is passed to the Apple device during the         */
   /* Identification process.  FIDLength defines the size in bytes of   */
   /* the FID information.  The MaxRxPacketSize defines the maximum     */
   /* amount of payload, in bytes, that the Apple device is allowed to  */
   /* send in a single packet.  The final two parameters specify the    */
   /* ISPP Event Callback function, and callback parameter,             */
   /* respectively, of the ISPP Event Callback that is to process any   */
   /* further interaction with the specified Remote Port (Opening       */
   /* Status, Data Writes, etc).  This function returns a non-zero,     */
   /* positive, value if successful, or a negative return error code if */
   /* this function is unsuccessful.  If this function is successful,   */
   /* the return value will represent the Serial Port ID that can be    */
   /* passed to all other functions that require it.  Once a Serial Port*/
   /* is opened, it can only be closed via a call to the                */
   /* ISPP_Close_Port() function (passing the return value from this    */
   /* function).                                                        */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Open_Remote_Port(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, ISPP_Identification_Info_t *IdentificationInformation, unsigned int MaxRxPacketSize, ISPP_Event_Callback_t ISPP_Event_Callback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Open_Remote_Port_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, ISPP_Identification_Info_t *IdentificationInformation, unsigned int MaxRxPacketSize, ISPP_Event_Callback_t ISPP_Event_Callback, unsigned long CallbackParameter);
#endif

   /* The following function exists to close a Serial Port that was     */
   /* previously opened with the ISPP_Open_Server_Port() function OR the*/
   /* ISPP_Open_Remote_Port() function.  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Stack which the Open      */
   /* Serial Port resides and the Serial Port ID (return value from one */
   /* of the above mentioned Open functions) of the Port to Close.  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.  This function does NOT Un-Register a SPP     */
   /* Server Port from the system, it ONLY disconnects any connection   */
   /* that is currently active on the Server Port.  The                 */
   /* ISPP_Close_Server_Port() function can be used to Un-Register the  */
   /* ISPP Server Port.                                                 */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Close_Port(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Close_Port_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is responsible for Reading Serial Data from*/
   /* the specified Serial Connection.  The SerialPortID that is passed */
   /* to this function MUST have been established by either Accepting a */
   /* Serial Port Connection (callback from the ISPP_Open_Server_Port() */
   /* function) or by initiating a Serial Port Connection (via calling  */
   /* the ISPP_Open_Remote_Port() function and having the remote side   */
   /* accept the Connection).  The input parameters to this function are*/
   /* the Bluetooth Stack ID of the Bluetooth Stack that the second     */
   /* parameter is valid for (Serial Port Identifier), the Size of the  */
   /* Data Buffer to be used for reading and a pointer to the Data      */
   /* Buffer.  This function returns the number of data bytes that were */
   /* successfully read (zero if there were no Data Bytes ready to be   */
   /* read), or a negative return error code if unsuccessful.           */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Data_Read(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t DataBufferSize, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Data_Read_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t DataBufferSize, Byte_t *DataBuffer);
#endif

   /* The following function is responsible for Sending Serial Data to  */
   /* the specified Serial Connection.  The SerialPortID that is passed */
   /* to this function MUST have been established by either Accepting a */
   /* Serial Port Connection (callback from the ISPP_Open_Server_Port() */
   /* function) or by initiating a Serial Port Connection (via calling  */
   /* the ISPP_Open_Remote_Port() function and having the remote side   */
   /* accept the Connection).  The input parameters to this function are*/
   /* the Bluetooth Stack ID of the Bluetooth Stack that the second     */
   /* parameter is valid for (Serial Port Identifier), the Length of the*/
   /* Data to send and a pointer to the Data Buffer to Send.  This      */
   /* function returns the number of data bytes that were successfully  */
   /* sent, or a negative return error code if unsuccessful.            */
   /* * NOTE * If this function is unable to send all of the data that  */
   /*          was specified (via the DataLength parameter), this       */
   /*          function will return the number of bytes that were       */
   /*          actually sent (zero or more, but less than the DataLength*/
   /*          parameter value).  When this happens (and ONLY when this */
   /*          happens), the user can expect to be notified when the    */
   /*          Serial Port is able to send data again via the           */
   /*          ietPort_Transmit_Buffer_Empty_Indication ISPP Event.     */
   /*          This will allow the user a mechanism to know when the    */
   /*          Transmit Buffer is empty so that more data can be sent.  */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Data_Write(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Data_Write_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t DataLength, Byte_t *DataBuffer);
#endif

   /* The following function is provided to allow the programmer a      */
   /* means to change the default Transmit and Receive Buffer Sizes.    */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the specified Serial Port has been  */
   /* previously opened (second parameter), and the next two parameters */
   /* represent the requested Buffer size to change the Receive and     */
   /* Transmit Buffer to (respectively).  The special constant          */
   /* SPP_BUFFER_SIZE_CURRENT can be used to specify that the requested */
   /* Buffer Size (either Transmit and/or Receive) NOT be changed.      */
   /* This function returns zero if the specified Buffer Size(s) were   */
   /* changed, or a negative return error code if there was an error.   */
   /* * NOTE * This function causes ALL Data in each Buffer to be       */
   /*          lost.  This function clears the each Data Buffer so that */
   /*          all the available data buffer is available to be used.   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Change_Buffer_Size(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int ReceiveBufferSize, unsigned int TransmitBufferSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Change_Buffer_Size_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int ReceiveBufferSize, unsigned int TransmitBufferSize);
#endif

   /* The following function exists to allow the user a mechanism for   */
   /* either aborting ALL Data present in either an Input or an Output  */
   /* Buffer, or a means to wait until a ALL Data present in either an  */
   /* Input or Output buffer has been removed.  This function takes as  */
   /* input the Bluetooth Stack ID of the Bluetooth Stack that contains */
   /* the Serial Port that was opened previously (specified by the next */
   /* parameter).  The final parameter is a BIT MASK that represents    */
   /* type of operation to perform.  This function returns zero if      */
   /* successful, or a negative return error code if unsuccessful.      */
   /* * NOTE * When using a PurgeBufferMask of                          */
   /*          SPP_PURGE_MASK_TRANSMIT_FLUSH_BIT, if the SPP Transmit   */
   /*          Buffer is already empty this function will return        */
   /*          BTPS_ERROR_SPP_BUFFER_EMPTY.                             */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Purge_Buffer(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PurgeBufferMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Purge_Buffer_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PurgeBufferMask);
#endif

   /* The following function is provided to allow the programmer a      */
   /* means to notify the remote side of the Serial Connection of a     */
   /* Break Condition.  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Protocol Stack that the Serial Port     */
   /* is valid with (specified by the second parameter).  The final     */
   /* parameter (if specified specifies the length of time that the     */
   /* Break was detected.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Break(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int BreakTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Break_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int BreakTimeout);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to send the existing state of the Line Status to the       */
   /* remote side.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the specified Serial Port */
   /* has been opened on (the second parameter is the Serial Port), and */
   /* the Current Line Status State.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Line_Status(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int SPPLineStatusMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Line_Status_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int SPPLineStatusMask);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to send the existing state of ALL Modem/Port Control       */
   /* Signals to the remote side.  This function accepts as input the   */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* specified Serial Port has been opened on (the second parameter is */
   /* the Serial Port), and the Current State of all the Modem Control  */
   /* Signals.  This function returns zero if successful, or a negative */
   /* return value if there was an error.                               */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Port_Status(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PortStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Port_Status_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PortStatus);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to inform the remote Side of the Serial Port Parameters    */
   /* that are to be used.  This function accepts as input the          */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack the specified  */
   /* Serial Port has been opened on (the second parameter is the       */
   /* Serial Port).  The final parameter to this function is the        */
   /* Requested Serial Port Information and cannot be NULL.  This       */
   /* function returns zero if successful, or a negative return value   */
   /* if there was an error.                                            */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Port_Information(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Port_Information_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to respond to a Serial Port Parameters Indication from the */
   /* remote side.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the specified Serial Port */
   /* has been opened on (the second parameter is the Serial Port), and */
   /* the status of the specified Port Information (acceptable or       */
   /* unacceptable).  This function returns zero if successful, or a    */
   /* negative return value if there was an error.                      */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Respond_Port_Information(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Respond_Port_Information_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to query the existing Serial Port Parameters from the      */
   /* remote side.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the specified Serial Port */
   /* has been opened on (the second parameter is the Serial Port).     */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Query_Remote_Port_Information(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Query_Remote_Port_Information_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is provided to allow the programmer a      */
   /* method to respond to a Serial Port Parameters Request from the    */
   /* remote side.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the specified Serial Port */
   /* has been opened on (the second parameter is the Serial Port), and */
   /* the Current Local Port Information.  This function returns zero   */
   /* if successful, or a negative return value if there was an error.  */
   /* * NOTE * This function is only available when operating in        */
   /*          standard SPP Mode.  This function is not allowed when    */
   /*          operating in iAP mode.                                   */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Respond_Query_Port_Information(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Respond_Query_Port_Information_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, SPP_Port_Information_t *ISPPPortInformation);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* query the current SPP Configuration Parameters.  These parameters */
   /* are the parameters that control the RFCOMM Frame Size (and the    */
   /* default Transmit and Receive Buffer sizes) that SPP will use when */
   /* opening/accepting SPP connections.  The first parameter to this   */
   /* function is the Bluetooth Stack ID of the Bluetooth Stack which   */
   /* this function is to query the SPP Configuration of.  The second   */
   /* parameter is a pointer to structure that will receive the current */
   /* SPP Configuration Information that is in use.  This function      */
   /* returns zero if successful or a negative return error code if an  */
   /* error occurs.                                                     */
   /* * NOTE * These parameters are set globally for the entire SPP     */
   /*          entity (per Bluetooth Stack Instance).  These values     */
   /*          can only be changed when NO SPP Clients or Servers are   */
   /*          open.                                                    */
   /* * NOTE * The Transmit and Receive Buffer sizes *MUST* be AT LEAST */
   /*          the size of the Maximum Frame Size that is being set.    */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Get_Configuration_Parameters(unsigned int BluetoothStackID, ISPP_Configuration_Params_t *ISPPConfigurationParams);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Get_Configuration_Parameters_t)(unsigned int BluetoothStackID, ISPP_Configuration_Params_t *ISPPConfigurationParams);
#endif

   /* The following function is responsible for allowing a mechanism    */
   /* to change the default SPP Configuration Parameters.  These        */
   /* parameters are the parameters that control the RFCOMM Frame Size  */
   /* (and the default Transmit and Receive Buffer sizes) that SPP will */
   /* use when opening/accepting SPP connections.  The first parameter  */
   /* to this function is the Bluetooth Stack ID of the Bluetooth Stack */
   /* which this function is to change the SPP Configuration of.  The   */
   /* second parameter is a pointer to structure that contains the new  */
   /* SPP Configuration Information to use.  This function returns zero */
   /* if successful or a negative return error code if an error occurs. */
   /* * NOTE * These parameters are set globally for the entire SPP     */
   /*          entity (per Bluetooth Stack Instance).  These values     */
   /*          can only be changed when NO SPP Clients or Servers are   */
   /*          open (i.e. this function can only be called when no      */
   /*          SPP Clients or Servers are active).                      */
   /* * NOTE * The Transmit and Receive Buffer sizes *MUST* be AT LEAST */
   /*          the size of the Maximum Frame Size that is being set.    */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Set_Configuration_Parameters(unsigned int BluetoothStackID, ISPP_Configuration_Params_t *ISPPConfigurationParams);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Set_Configuration_Parameters_t)(unsigned int BluetoothStackID, ISPP_Configuration_Params_t *ISPPConfigurationParams);
#endif

   /* The following function is a utility function that exists to       */
   /* determine the current SPP (RFCOMM) Connection state for a specific*/
   /* SPP/RFCOMM Connection.  This function accepts as input, the       */
   /* Bluetooth Stack ID of the Bluetooth Stack for which the request is*/
   /* valid for, followed by the Bluetooth Device Address of the        */
   /* connection in question (required), followed by the SPP/RFCOMM     */
   /* Server Port Number of the connection in question, followed by a   */
   /* flag that specifies whether or not the connection is to a Local   */
   /* SPP Server (TRUE) or a Remote SPP Server (FALSE), followed by a   */
   /* pointer to state variable that is to receive the state            */
   /* information.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function will only supply a value in the SPP Port   */
   /*          Connection State parameter if the return value from this */
   /*          function is success (0).  If this function returns an    */
   /*          error, then the contents of this variable will be        */
   /*          undefined.                                               */
   /* * NOTE * The Bluetooth Address is a required parameter and is used*/
   /*          to determine the TEI of the underlying RFCOMM Connection.*/
   /* * NOTE * The ServerPort parameter must be one of the following: - */
   /*          0 (queries if a connection to the remote device specifies*/
   /*          is possible) - SPP_PORT_NUMBER_MINIMUM -                 */
   /*          SPP_PORT_NUMBER_MAXIMUM Note that the above values are   */
   /*          NOT the SPP Port ID values returned from the             */
   /*          SPP_Open_Server_Port() or the SPP_Open_Remote_Port()     */
   /*          functions, but rather are the actual SPP/RFCOMM Port     */
   /*          Channel Numbers !!!!!!!!!!!!!!!!!                        */
   /* * NOTE * This is a very low level function and exists solely to   */
   /*          allow a mechanism to determine the current Control       */
   /*          Message State for the underlying RFCOMM Transport.  This */
   /*          is needed in some circumstances when trying to connect or*/
   /*          disconnect SPP Ports and the user has no way of knowing  */
   /*          knowing the current Status (keep in mind that there can  */
   /*          only be a single Control Message outstanding on any given*/
   /*          RFCOMM Channel).                                         */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Get_Port_Connection_State(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, Boolean_t LocalPort, SPP_Port_Connection_State_t *ISPP_Port_Connection_State);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Get_Port_Connection_State_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, Boolean_t LocalPort, SPP_Port_Connection_State_t *ISPP_Port_Connection_State);
#endif

   /* The following function is responsible for setting the lower level */
   /* queuing parameters.  These parameters are used to control aspects */
   /* of the amount of data packets that can be queued into the lower   */
   /* level (per individual channel).  This mechanism allows for the    */
   /* flexibility to limit the amount of RAM that is used for streaming */
   /* type applications (where the remote side has a large number of    */
   /* credits that were granted).  This function accepts as input the   */
   /* Bluetooth Stack ID of the Bluetooth stack in which to set the     */
   /* system wide queuing parameters, followed by the maximum number of */
   /* queued data packets (per DLCI), followed by the low threshold     */
   /* (used be the lower layer to inform RFCOMM when it can send another*/
   /* data packet).  This function returns zero if successful or a      */
   /* negative return error code if there is an error.                  */
   /* * NOTE * This function can only be called when there are NO active*/
   /*          connections.                                             */
   /* * NOTE * Setting both parameters to zero will disable the queuing */
   /*          mechanism.  This means that the amount of queued packets */
   /*          will only be limited via the amount of available RAM.    */
   /* * NOTE * These parameters do not affect the transmit and receive  */
   /*          buffers and do not affect any frame sizes and/or credit  */
   /*          logic.  These parameters ONLY affect the number of       */
   /*          data packets queued into the lower level.                */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Set_Queuing_Parameters(unsigned int BluetoothStackID, unsigned int MaximumNumberDataPackets, unsigned int QueuedDataPacketsThreshold);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Set_Queuing_Parameters_t)(unsigned int BluetoothStackID, unsigned int MaximumNumberDataPackets, unsigned int QueuedDataPacketsThreshold);
#endif

   /* The following function is responsible for setting the lower level */
   /* queuing parameters.  These parameters are used to control aspects */
   /* of the amount of data packets that can be queued into the lower   */
   /* level (per individual channel).  This mechanism allows for the    */
   /* flexibility to limit the amount of RAM that is used for streaming */
   /* type applications (where the remote side has a large number of    */
   /* credits that were granted).  This function accepts as input the   */
   /* Bluetooth Stack ID of the Bluetooth stack in which to get the     */
   /* lower level queuing parameters, followed by a pointer to a        */
   /* variable that is to receive the maximum number of queued data     */
   /* packets (per channel), followed by the low threshold (used be the */
   /* lowest layer to inform the lower layer when it can send another   */
   /* data packet).  This function returns zero if successful or a      */
   /* negative return error code if there is an error.                  */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Get_Queuing_Parameters(unsigned int BluetoothStackID, unsigned int *MaximumNumberDataPackets, unsigned int *QueuedDataPacketsThreshold);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Get_Queuing_Parameters_t)(unsigned int BluetoothStackID, unsigned int *MaximumNumberDataPackets, unsigned int *QueuedDataPacketsThreshold);
#endif

   /* The following function is a utility function that exists to       */
   /* determine the current Operating Mode of an active SPP Port.  This */
   /* function accepts as its first parameter, the Bluetooth Stack ID of*/
   /* the Bluetooth Stack for which the request is valid for.  The      */
   /* second parameter is the Serial Port ID that identifies the active */
   /* port whose mode is being queried.  The last parameter is a pointer*/
   /* to a variable that will receive the current operating mode.  This */
   /* function returns zero if successful or a negative return error    */
   /* code if an error occurs.                                          */
   /* * NOTE * If the function returns zero, then OperatingMode will    */
   /*          contain the current mode of the active port specified.   */
   /* * NOTE * The SerialPortID must reference an Active Port.  If the  */
   /*          port is not connected or in the process of being         */
   /*          connected, the return value from this function will      */
   /*          indicate an invalid serial port ID.                      */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Get_Port_Operating_Mode(unsigned int BluetoothStackID, unsigned int SerialPortID, Port_Operating_Mode_t *OperatingMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Get_Port_Operating_Mode_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Port_Operating_Mode_t *OperatingMode);
#endif

   /* The following function is a utility function that exists to       */
   /* determine the current Timeout Value that is used for outgoing     */
   /* packets.  This function accepts as its first parameter, the       */
   /* Bluetooth Stack ID of the Bluetooth Stack for which the request is*/
   /* valid for.  The second parameter is the Serial Port ID that       */
   /* identifies the active port whose timeout value is being queried.  */
   /* The last parameter is a pointer to a variable that will receive   */
   /* the current timeout value.  This function returns zero if         */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * If the function returns zero, then TimeoutValue will     */
   /*          contain the current timeout value set for the port.      */
   /* * NOTE * The SerialPortID must reference an Active Port.  If the  */
   /*          port is not connected or in the process of being         */
   /*          connected, the return value from this function will      */
   /*          indicate an invalid serial port ID.                      */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Get_Port_Timeout_Value(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int *TimeoutValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Get_Port_Timeout_Value_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int *TimeoutValue);
#endif

   /* The following function is a utility function that exists to change*/
   /* the current Timeout Value that is used for outgoing packets.  This*/
   /* function accepts as its first parameter, the Bluetooth Stack ID of*/
   /* the Bluetooth Stack for which the request is valid for.  The      */
   /* second parameter is the Serial Port ID that identifies the active */
   /* port whose timeout value is being queried.  The last parameter is */
   /* the timeout value in Milliseconds that will be assigned to all    */
   /* packets that are sent on the port.  This function returns zero if */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The SerialPortID must reference an Active Port.  If the  */
   /*          port is not connected or in the process of being         */
   /*          connected, the return value from this function will      */
   /*          indicate an invalid serial port ID.                      */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Set_Port_Timeout_Value(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int TimeoutValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Set_Port_Timeout_Value_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int TimeoutValue);
#endif

   /* The following function is used to start the Authentication Process*/
   /* with a remote Apple device.  This function accepts as its first   */
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the request is valid for.  The second parameter is the            */
   /* SerialPortID that identifies the Port on which to start the       */
   /* authentication process.  The function returns Zero if successful  */
   /* and will return a negative value if the process could not be      */
   /* started.                                                          */
   /* * NOTE * The process may fail to start if the authentication      */
   /*          process is already in progress.  A call to               */
   /*          ISPP_CancelAuthorization() can be made prior to calling  */
   /*          this function to restart the process.                    */
   /* * NOTE * It is suggested that a timer be started upon the receipt */
   /*          of the ietPort_Open_Indication or                        */
   /*          ietPort_Open_Confirmation to allows time for the Apple   */
   /*          device to start the authentication process.  If the      */
   /*          process is not started by expiration of the timer, then  */
   /*          this function should be called to start the process.  A  */
   /*          recommended timeout is 2-5 seconds.                      */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Start_Authorization(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Start_Authorization_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is used to abort an Authentication Process */
   /* that is currently in progress.  This function accepts as its first*/
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the request is valid for.  The second parameter is the            */
   /* SerialPortID that identifies the Port on which to cancel the      */
   /* authentication process.  The function returns Zero if the process */
   /* was successfully canceled and a negative number if the function   */
   /* fails.                                                            */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Cancel_Authorization(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Cancel_Authorization_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is used to submit a response to an Open    */
   /* Session request from the remote device.  This function accepts as */
   /* its first parameter, the Bluetooth Stack ID of the Bluetooth Stack*/
   /* for which the request is valid for.  The second parameter is the  */
   /* Serial Port ID that identifies the connection on which the session*/
   /* is to be established.  The third parameter identifies the ID of   */
   /* the Session that is being established.  The last parameter        */
   /* indicates whether the user wants to accept or reject the session  */
   /* request.  The function return Zero if the function was successful.*/
   /* * NOTE * This function must be called in response to the reception*/
   /*          of an ietPort_Open_Session_Indication event.             */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Open_Session_Request_Response(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Open_Session_Request_Response_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Boolean_t Accept);
#endif

   /* The following function is used to send session data to a remote   */
   /* device that is referenced by the specified SessionID.  This       */
   /* function accepts as its first parameter, the Bluetooth Stack ID of*/
   /* the Bluetooth Stack for which the request is valid for.  The      */
   /* second parameter is the Serial Port ID of Port on which the       */
   /* session resides.  The third parameter specifies the Session ID    */
   /* that the data is associated with.  The fourth parameter specifies */
   /* the number of bytes of data that is to be sent and the last       */
   /* parameter is a pointer to the data that is to be sent.  The       */
   /* function returns a negative value on failure.  If the packet was  */
   /* successfully queued, the function return a positive non-Zero      */
   /* PacketID value that is used to uniquely identify the packet.  When*/
   /* the data packet ultimately sent and acknowledged, a Send          */
   /* Confirmation event is dispatched with the PacketID and he status  */
   /* of the transaction.                                               */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Session_Data(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Word_t DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Session_Data_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Word_t DataLength, Byte_t *Data);
#endif

   /* The following function is used to send session data to a remote   */
   /* device that is referenced by the specified SessionID.  This       */
   /* function accepts as its first parameter, the Bluetooth Stack ID of*/
   /* the Bluetooth Stack for which the request is valid for.  The      */
   /* second parameter is the Serial Port ID of Port on which the       */
   /* session resides.  The third parameter specifies the Session ID    */
   /* that the data is associated with.  The fourth parameter specifies */
   /* the number of bytes of data that is to be sent and the last       */
   /* parameter is a pointer to the data that is to be sent.  The last  */
   /* parameter specified the length of time that this module will wait */
   /* for an acknowledgment from the iOS device before retransmitting   */
   /* the packet.  The function returns a negative value on failure.  If*/
   /* the packet was successfully queued, the function return a positive*/
   /* non-Zero PacketID value that is used to uniquely identify the     */
   /* packet.  When the data packet ultimately sent and acknowledged, a */
   /* Send Confirmation event is dispatched with the PacketID and he    */
   /* status of the transaction.                                        */
   /* * NOTE * The Timeout value is specified in Milliseconds.          */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Session_Data_With_Timeout(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Word_t DataLength, Byte_t *Data, unsigned int Timeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Session_Data_With_Timeout_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID, Word_t DataLength, Byte_t *Data, unsigned int Timeout);
#endif

   /* The following function is used to allow for the submission of     */
   /* Control Session messages.  This function accepts as its first     */
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the request is valid for.  The second parameter is the Serial Port*/
   /* ID that identifies the connection on which the session is to be   */
   /* established.  The third parameter identifies the ID of the Control*/
   /* Session Command to be sent.  The last two parameters indicates the*/
   /* Command Payload Length and a pointer to the Command Payload.  The */
   /* Command Payload consists of the list of parameters that are       */
   /* specified with the command being sent.  The function return an    */
   /* identifier that references the command sent.                      */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Control_Message(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t ControlMessageID, Word_t MessageDataLength, unsigned char *MessageData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Control_Message_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t ControlMessageID, Word_t MessageDataLength, unsigned char *MessageData);
#endif

   /* The following function is no longer provided due to protocol      */
   /* differences between iAP versions.  Any attempt to invoke this     */
   /* function will result in a BTPS_ERROR_FEATURE_NOT_AVAILABLE error  */
   /* code, and no operation will be performed.                         */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Cancel_Packet(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PacketID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Cancel_Packet_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, unsigned int PacketID);
#endif

   /* The following function is used to acknowledge a session data      */
   /* packet that was not automatically acknowledged in the iSPP Event  */
   /* Callback.  This function accepts as its first parameter, the      */
   /* Bluetooth Stack ID of the Bluetooth Stack for which the packet is */
   /* to be received.  The second parameter is the Serial Port ID of    */
   /* Port on which the data was received.  The third parameter is the  */
   /* Session ID of the session that received the packet.  The function */
   /* returns Zero on success and a negative number on error.           */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Ack_Last_Session_Data_Packet(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Ack_Last_Session_Data_Packet_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Word_t SessionID);
#endif

   /* The following function is used to send application formatted Lingo*/
   /* Packets to a remote device.  This function accepts as its first   */
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the packet is to be sent.  The second parameter is the Serial Port*/
   /* ID of Port on which the data is to be sent.  The third and fourth */
   /* parameter specifies the Lingo and Command ID for which the data is*/
   /* associated with.  The fifth parameter is the TransactionID that is*/
   /* associated with the data.  The last two parameters identify the   */
   /* length of the Lingo Packet data and a pointer to the data.  The   */
   /* function returns a negative value on failure.  If the packet was  */
   /* successfully queued, the function return a positive non-Zero      */
   /* PacketID value that is used to uniquely identify the packet.  When*/
   /* the data packet ultimately sent and acknowledged, a Send          */
   /* Confirmation event is dispatched with the PacketID and he status  */
   /* of the transaction.                                               */
   /* * NOTE * If the Packet Data represents a Response to a previously */
   /*          received Request packet, then the TransactionID must     */
   /*          match the TransactionID of the Request Packet.  If the   */
   /*          Packet is not a Response Packet, then the TransactionID  */
   /*          must be 0 to indicate that a TransactionID needs to be   */
   /*          assigned at the time the packet is sent.                 */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Send_Raw_Data(unsigned int BluetoothStackID, unsigned int SerialPortID, Byte_t Lingo, Byte_t CommandID, Word_t TransactionID, Word_t PacketDataLength, Byte_t *PacketData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Send_Raw_Data_t)(unsigned int BluetoothStackID, unsigned int SerialPortID, Byte_t Lingo, Byte_t CommandID, Word_t TransactionID, Word_t PacketDataLength, Byte_t *PacketData);
#endif

   /* The following function is used to obtain the Max Payload Size     */
   /* reported from the iOS device during the identification process.   */
   /* Packets to a remote device.  This function accepts as its first   */
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the packet is to be sent.  The second parameter is the Serial Port*/
   /* ID of Port on which the data is to be sent.  The function returns */
   /* the value that was received from the iOS device on success.  The  */
   /* function returns a negative value on failure.                     */
   /* * NOTE * This function is only available when operating in iAP    */
   /*          mode.  This function is not allowed when operating in    */
   /*          standard SPP Mode.                                       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Query_Max_Tx_Packet_Size(unsigned int BluetoothStackID, unsigned int SerialPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Query_Max_Tx_Packet_Size_t)(unsigned int BluetoothStackID, unsigned int SerialPortID);
#endif

   /* The following function is used to obtain information from the     */
   /* Authentication Coprocessor.  This function accepts as its first   */
   /* parameter, the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the packet is to be sent.  The second parameter is a pointer to a */
   /* Coprocessor_Info_t structure to receive the data.  The function   */
   /* returns the value that was received from the iOS device on        */
   /* success.  The function returns a negative value on failure.       */
BTPSAPI_DECLARATION int BTPSAPI ISPP_Query_Coprocessor_Info(unsigned int BluetoothStackID, ISPP_Coprocessor_Info_t *CoprocessorInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ISPP_Query_Coprocessor_Info_t)(unsigned int BluetoothStackID, ISPP_Coprocessor_Info_t *CoprocessorInfo);
#endif

#endif
