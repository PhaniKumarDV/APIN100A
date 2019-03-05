/*****< iaptypes.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  IAPTYPE - iOS Accessory Protocol Type Definitions, Prototypes, and        */
/*            Constants.                                                      */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/13/11  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __IAPTYPESH__
#define __IAPTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following MACRO is a utility MACRO that assigns the Apple     */
   /* Device Bluetooth Universally Unique Identifier (SDP_UUID_128) to  */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* APPLE_DEVICE_UUID_128 Constant value.                             */
#define SDP_ASSIGN_APPLE_DEVICE_UUID(_x)      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFE)

   /* The following MACRO is a utility MACRO that assigns the Apple     */
   /* Accessory Bluetooth Universally Unique Identifier (SDP_UUID_128)  */
   /* to the specified UUID_128_t variable.  This MACRO accepts one     */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* APPLE_ACCESSORY_UUID_128 Constant value.                          */
#define SDP_ASSIGN_APPLE_ACCESSORY_UUID(_x)   ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE, 0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF)

   /* iAP Packet format definitions.                                    */
#define SYNC_BYTE                                                       0xFF
#define PACKET_START_BYTE                                               0x55
#define PACKET_SYNC_START_BYTE                                          0x5A
#define PAYLOAD_LENGTH_MARKER_BYTE                                      0x00

#define IAP2_START_OF_PACKET_ID                                       0xFF5A

   /* Maximum allowable payload sizes of iAP packets.                   */
#define MAX_SMALL_PACKET_PAYLOAD_SIZE                                    255
#define MAX_LARGE_PACKET_PAYLOAD_SIZE                                  65529

   /* Size of individual iAP packet members.                            */
#define TRANSACTION_ID_SIZE                            NON_ALIGNED_WORD_SIZE
#define SESSION_ID_SIZE                                NON_ALIGNED_WORD_SIZE
#define CHECKSUM_DATA_SIZE                             NON_ALIGNED_BYTE_SIZE

   /* The following defines the bit masks that are used to process the  */
   /* control byte of the iAP2 packet.                                  */
#define IAP2_CONTROL_BYTE_SYNC_MASK                                     0x80
#define IAP2_CONTROL_BYTE_ACK_MASK                                      0x40
#define IAP2_CONTROL_BYTE_EAK_MASK                                      0x20
#define IAP2_CONTROL_BYTE_RST_MASK                                      0x10
#define IAP2_CONTROL_BYTE_SLP_MASK                                      0x08

   /* Packet structures for command and data packets.                   */

   /* Definition for the header of a small packet (contains no data).   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Small_Packet_Header_t
{
   NonAlignedByte_t SyncByte;
   NonAlignedByte_t StartByte;
   NonAlignedByte_t PayloadLength;
} __PACKED_STRUCT_END__ IAP_Small_Packet_Header_t;

#define IAP_SMALL_PACKET_HEADER_SIZE                              (sizeof(IAP_Small_Packet_Header_t))

   /* Definition for the header of a large packet (contains no data).   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Large_Packet_Header_t
{
   NonAlignedByte_t SyncByte;
   NonAlignedByte_t StartByte;
   NonAlignedByte_t LengthMarker;
   NonAlignedWord_t PayloadLength;
} __PACKED_STRUCT_END__ IAP_Large_Packet_Header_t;

#define IAP_LARGE_PACKET_HEADER_SIZE                              (sizeof(IAP_Large_Packet_Header_t))

   /* Definition for the header of an iAP2 packet (contains no data).   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Link_Packet_Header_t
{
   NonAlignedWord_t StartOfPacket;
   NonAlignedWord_t PacketLength;
   NonAlignedByte_t ControlByte;
   NonAlignedByte_t PacketSequenceNumber;
   NonAlignedByte_t PacketAcknowledgmentNumber;
   NonAlignedByte_t SessionIdentifier;
   NonAlignedByte_t HeaderChecksum;
} __PACKED_STRUCT_END__ IAP2_Link_Packet_Header_t;

#define IAP2_LINK_PACKET_HEADER_SIZE                              (sizeof(IAP2_Link_Packet_Header_t))

   /* Command payload header that is used for both a small and large    */
   /* packet.  This data immediately follows the header and precedes the*/
   /* command payload data and checksum.                                */
   /* * NOTE * ALL new accessories *MUST* support IDPS (Identify Device */
   /*          Preferences and Settings).                               */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Command_Header_t
{
   NonAlignedByte_t LingoID;
   NonAlignedByte_t CommandID;
   NonAlignedWord_t TransactionID;
} __PACKED_STRUCT_END__ IAP_Command_Header_t;

#define IAP_COMMAND_HEADER_SIZE(_x)                               (sizeof(IAP_Command_Header_t))

   /* Command payload that is used for both a small and large packet.   */
   /* This data immediately follows the header and precedes the         */
   /* checksum.                                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Command_Payload_t
{
   NonAlignedByte_t LingoID;
   NonAlignedByte_t CommandID;
   NonAlignedWord_t TransactionID;
   NonAlignedByte_t CommandData[1];
} __PACKED_STRUCT_END__ IAP_Command_Payload_t;

#define IAP_COMMAND_PAYLOAD_SIZE(_x)                              (STRUCTURE_OFFSET(IAP_Command_Payload_t, CommandData) + (_x))

   /* Small packet definition.                                          */
   /* * NOTE * This structure is not directly applicable to a small     */
   /*          packet in that the command payload is variable length.   */
   /*          This structure definition is provided to merely show the */
   /*          format of a small packet.  Because of this, the Checksum */
   /*          member will not be directly accessible in the structure  */
   /*          definition as it will follow the command payload (which  */
   /*          contains variable length data).                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Small_Packet_t
{
   IAP_Small_Packet_Header_t PacketHeader;
   IAP_Command_Payload_t     CommandPayload;
   NonAlignedByte_t          Checksum;
} __PACKED_STRUCT_END__ IAP_Small_Packet_t;

   /* The following MACRO is a utility MACRO that exists to calculate   */
   /* the total size (in bytes) that will be required for a small iAP   */
   /* packet with the specified number of command payload bytes.        */
#define IAP_SMALL_PACKET_SIZE(_x)                                 (IAP_SMALL_PACKET_HEADER_SIZE + IAP_COMMAND_PAYLOAD_SIZE(_x) + CHECKSUM_DATA_SIZE)

   /* Large packet definition.                                          */
   /* * NOTE * This structure is not directly applicable to a large     */
   /*          packet in that the command payload is variable length.   */
   /*          This structure definition is provided to merely show the */
   /*          format of a large packet.  Because of this, the Checksum */
   /*          member will not be directly accessible in the structure  */
   /*          definition as it will follow the command payload (which  */
   /*          contains variable length data).                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Large_Packet_t
{
   IAP_Large_Packet_Header_t PacketHeader;
   IAP_Command_Payload_t     CommandPayload;
   NonAlignedByte_t          Checksum;
} __PACKED_STRUCT_END__ IAP_Large_Packet_t;

   /* The following MACRO is a utility MACRO that exists to calculate   */
   /* the total size (in bytes) that will be required for a large iAP   */
   /* packet with the specified number of command payload bytes.        */
#define IAP_LARGE_PACKET_SIZE(_x)                                 (IAP_LARGE_PACKET_HEADER_SIZE + IAP_COMMAND_PAYLOAD_SIZE(_x) + CHECKSUM_DATA_SIZE)

   /* Full ID String (FID) token packet definition.                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_FID_Token_t
{
   NonAlignedByte_t Length;
   NonAlignedByte_t FID_Type;
   NonAlignedByte_t FID_SubType;
   NonAlignedByte_t FID_Data[1];
} __PACKED_STRUCT_END__ IAP_FID_Token_t;

   /* The following MACRO is a utility MACRO that exists to calculate   */
   /* the total size (in bytes) that will be required for a FID Token   */
   /* with the specified number of command FID String bytes.            */
#define IAP_FID_TOKEN_SIZE(_x)                                    (STRUCTURE_OFFSET(IAP_FID_Token_t, FID_Data) + (_x))

   /* Definition for the Session Types specified in the Session Info    */
   /* structure.                                                        */
#define IAP2_SESSION_TYPE_CONTROL                                          0x00
#define IAP2_SESSION_TYPE_FILE_TRANSFER                                    0x01
#define IAP2_SESSION_TYPE_EXTERNAL_ACCESSORY                               0x02

   /* Definition for Session Information of an iAP2 packet.             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Session_Info_t
{
   NonAlignedByte_t SessionIdentifier;
   NonAlignedByte_t SessionType;
   NonAlignedByte_t SessionVersion;
} __PACKED_STRUCT_END__ IAP2_Session_Info_t;

#define IAP2_SESSION_INFO_DATA_SIZE                            (sizeof(IAP2_Session_Info_t))

#define IAP2_LINK_VERSION                                                   0x01

   /* Definition for the header of an iAP2 Link Synchronous packet.     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Link_Sync_Payload_t
{
   NonAlignedByte_t    LinkVersion;
   NonAlignedByte_t    MaxOutstandingPackets;
   NonAlignedWord_t    MaxPacketLength;
   NonAlignedWord_t    RetransmissionTimeout;
   NonAlignedWord_t    CumulativeAckTimeout;
   NonAlignedByte_t    MaxRetransmissions;
   NonAlignedByte_t    MaxCumulativeAcks;
   IAP2_Session_Info_t SessionInfo[1];
} __PACKED_STRUCT_END__ IAP2_Link_Sync_Payload_t;

   /* The following MACRO calculates the size required to hold an IAP2  */
   /* Link Sync Payload packet, given the number of "additional"        */
   /* sessions that are supported.  "Additional" in this case means     */
   /* sessions that are NOT the control session.                        */
   /* * NOTE * Since the Control Session is required, this packet will  */
   /*          always contain at least one valid IAP2_Session_Info_t    */
   /*          data payload.  If there are no other sessions (besides   */
   /*          the Control Session then zero (0) should be passed to    */
   /*          this MACRO.                                              */
#define IAP2_LINK_SYNC_PAYLOAD_SIZE(_x)                                    (sizeof(IAP2_Link_Sync_Payload_t) + ((_x)*(IAP2_SESSION_INFO_DATA_SIZE)))

#define IAP2_CONTROL_SESSION_MESSAGE_START_OF_MESSAGE_ID                   0x4040

  /* Definition for Session Information of an iAP2 packet.              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Control_Session_Message_Parameter_t
{
   NonAlignedWord_t ParameterLength;
   NonAlignedWord_t ParameterID;
   NonAlignedByte_t ParameterData[1];
} __PACKED_STRUCT_END__ IAP2_Control_Session_Message_Parameter_t;

#define IAP2_CONTROL_SESSION_MESSAGE_PARAMETER_HEADER_SIZE        (STRUCTURE_OFFSET(IAP2_Control_Session_Message_Parameter_t, ParameterData))
#define IAP2_CONTROL_SESSION_MESSAGE_PARAMATER_SIZE(_x)           (IAP2_CONTROL_SESSION_MESSAGE_PARAMETER_HEADER_SIZE + (_x))

  /* Definition for the header of an iAP2 Link Synchronous packet.      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Control_Session_Message_t
{
   NonAlignedWord_t                          StartOfMessage;
   NonAlignedWord_t                          MessageLength;
   NonAlignedWord_t                          MessageID;
   IAP2_Control_Session_Message_Parameter_t  Parameter[1];
} __PACKED_STRUCT_END__ IAP2_Control_Session_Message_t;

#define IAP2_CONTROL_SESSION_MESSAGE_SIZE(_x)                     (STRUCTURE_OFFSET(IAP2_Control_Session_Message_t, Parameter) + (_x))
#define IAP2_CONTROL_SESSION_MESSAGE_START_SESSION_DATA_SIZE      (IAP2_CONTROL_SESSION_MESSAGE_SIZE(IAP2_CONTROL_SESSION_MESSAGE_PARAMATER_SIZE(1)+IAP2_CONTROL_SESSION_MESSAGE_PARAMATER_SIZE(2)))
#define IAP2_CONTROL_SESSION_MESSAGE_STOP_SESSION_DATA_SIZE       (IAP2_CONTROL_SESSION_MESSAGE_SIZE(IAP2_CONTROL_SESSION_MESSAGE_PARAMATER_SIZE(2)))

   /* Definition foe the File Transfer State of a File Transfer Session */
   /* Message datagram.                                                 */
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_START                  0x01
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_CANCEL                 0x02
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_PAUSE                  0x03
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_SETUP                  0x04
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_SUCCESS                0x05
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_FAILURE                0x06
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_FIRST_DATA             0x80
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_FIRST_AND_ONLY_DATA    0xC0
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_CONTINUE_DATA          0x00
#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_STATE_LAST_DATA              0x40

   /* Definition for the header of an iAP2 File Transfer Session        */
   /* Message.                                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_File_Transfer_Session_Datagram_Header_t
{
   NonAlignedByte_t FileTransferID;
   NonAlignedByte_t FileTransferState;
} __PACKED_STRUCT_END__ IAP2_File_Transfer_Session_Datagram_Header_t;

#define IAP2_FILE_TRANSFER_SESSION_DATAGRAM_HEADER_SIZE          (sizeof(IAP2_File_Transfer_Session_Datagram_Header_t))

   /* Definition for the header of an iAP2 External Accessory Session   */
   /* Message.                                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_External_Accessory_Session_Datagram_t
{
   NonAlignedWord_t ExternalAccessoryTransferID;
   NonAlignedByte_t ExternalAccessoryData[1];
} __PACKED_STRUCT_END__ IAP2_External_Accessory_Session_Datagram_t;

#define IAP2_EXTERNAL_ACCESSORY_SESSION_DATAGRAM_SIZE(_x)        (NON_ALIGNED_WORD_SIZE+(_x))

   /* Definition for the header of an iAP2 Link Synchronous packet.     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP2_Session_Packet_Payload_t
{
   IAP2_External_Accessory_Session_Datagram_t Datagram;
   NonAlignedByte_t                           PacketChecksum;
} __PACKED_STRUCT_END__ IAP2_Session_Packet_Payload_t;

#define IAP2_SESSION_PACKET_PAYLOAD_SIZE(_x)                     (IAP2_EXTERNAL_ACCESSORY_SESSION_DATAGRAM_SIZE(_x) + NON_ALIGNED_WORD_SIZE)

#define IAP2_ACCESSORY_REQUEST_TYPE(_x)                          (((_x) >> 8) & 0xFF)
#define IAP2_ACCESSORY_REQUEST_OPTION(_x)                        ((_x) & 0xFF)
#define IAP2_MAKE_ACCESSORY_REQUEST(_x, _y)                      ((((_x) & 0xFF) << 8) | ((_y) & 0xFF))

#define IAP2_ACCESSORY_REQUEST_TYPE_IDENTIFICATION                         0x1D
#define IAP2_ACCESSORY_REQUEST_TYPE_BLUETOOTH_INFORMATION                  0x4E
#define IAP2_ACCESSORY_REQUEST_TYPE_ASSISTIVE_TOUCH                        0x54
#define IAP2_ACCESSORY_REQUEST_TYPE_AUTHENTICATION                         0xAA
#define IAP2_ACCESSORY_REQUEST_TYPE_EXTERNAL_ACCESSORY                     0xEA

#define IAP2_ACCESSORY_AUTHENTICATION_REQUEST_CERTIFICATE                0xAA00
#define IAP2_ACCESSORY_AUTHENTICATION_CERTIFICATE                        0xAA01
#define IAP2_ACCESSORY_AUTHENTICATION_CHALLENGE_RESPONSE                 0xAA02
#define IAP2_ACCESSORY_AUTHENTICATION_RESPONSE                           0xAA03
#define IAP2_ACCESSORY_AUTHENTICATION_FAILURE                            0xAA04
#define IAP2_ACCESSORY_AUTHENTICATION_SUCCESS                            0xAA05

#define IAP2_ACCESSORY_IDENTIFICATION_START_IDENTIFICATION               0x1D00
#define IAP2_ACCESSORY_IDENTIFICATION_INFORMATION                        0x1D01
#define IAP2_ACCESSORY_IDENTIFICATION_ACCEPTED                           0x1D02
#define IAP2_ACCESSORY_IDENTIFICATION_REJECTED                           0x1D03
#define IAP2_ACCESSORY_IDENTIFICATION_CANCEL                             0x1D05
#define IAP2_ACCESSORY_IDENTIFICATION_INFORMATION_UPDATE                 0x1D06

#define IAP2_ACCESSORY_ASSISTIVE_TOUCH_START                             0x5400
#define IAP2_ACCESSORY_ASSISTIVE_TOUCH_STOP                              0x5401
#define IAP2_ACCESSORY_ASSISTIVE_TOUCH_START_INFORMATION                 0x5402
#define IAP2_ACCESSORY_ASSISTIVE_TOUCH_INFORMATION                       0x5403
#define IAP2_ACCESSORY_ASSISTIVE_TOUCH_STOP_INFORMATION                  0x5404

#define IAP2_ACCESSORY_BLUETOOTH_COMPONENT_INFORMATION                   0x4E01
#define IAP2_ACCESSORY_BLUETOOTH_START_CONNECTION_UPDATES                0x4E03
#define IAP2_ACCESSORY_BLUETOOTH_CONNECTION_UPDATE                       0x4E04
#define IAP2_ACCESSORY_BLUETOOTH_STOP_CONNECTION_UPDATEs                 0x4E05

#define IAP2_ACCESSORY_HID_START                                         0x6800
#define IAP2_ACCESSORY_HID_RECEIVED_HID_REPORT                           0x6801
#define IAP2_ACCESSORY_HID_SEND_HID_REPORT                               0x6802
#define IAP2_ACCESSORY_HID_STOP                                          0x6803

#define IAP2_ACCESSORY_EXTERNAL_ACCESSORY_START_SESSION                  0xEA00
#define IAP2_ACCESSORY_EXTERNAL_ACCESSORY_STOP_SESSION                   0xEA01
#define IAP2_ACCESSORY_EXTERNAL_ACCESSORY_LAUNCH_APP                     0xEA02

#define IAP2_SUPPORTED_EXTERNAL_ACCESSORY_PROTOCOL_PARAMETER_IDENTIFIER  0x000A
#define IAP2_SUPPORTED_EXTERNAL_ACCESSORY_PARAMETER_PROTOCOL_IDENTIFIER  0x0000

   /* Defined iAP Lingo ID types.                                       */
#define IAP_LINGO_ID_GENERAL                                               0x00
#define IAP_LINGO_ID_MICROPHONE                                            0x01
#define IAP_LINGO_ID_SIMPLE_REMOTE                                         0x02
#define IAP_LINGO_ID_DISPLAY_REMOTE                                        0x03
#define IAP_LINGO_ID_ENHANCED_INTERFACE                                    0x04
#define IAP_LINGO_ID_ACCESSORY_POWER                                       0x05
#define IAP_LINGO_ID_USB_HOST_MODE                                         0x06
#define IAP_LINGO_ID_RF_TUNER                                              0x07
#define IAP_LINGO_ID_ACCESSORY_EQUALIZER                                   0x08
#define IAP_LINGO_ID_SPORTS                                                0x09
#define IAP_LINGO_ID_DIGITAL_AUDIO                                         0x0A
#define IAP_LINGO_ID_STORAGE                                               0x0C
#define IAP_LINGO_ID_IPOD_OUT                                              0x0D
#define IAP_LINGO_ID_LOCATION                                              0x0E

   /* Identify Device Preferences and Settings (IDPS).                  */
#define IAP_IDPS_STATUS_CONTINUE_WITH_AUTHENTICATION                          0
#define IAP_IDPS_STATUS_RESET_ALL_IDPS_IFORMATION                             1
#define IAP_IDPS_STATUS_ABORT_PROCESS                                         2

   /* General Lingo command ID types.                                   */
#define IAP_GENERAL_LINGO_REQUEST_IDENTIFY                                 0x00
#define IAP_GENERAL_LINGO_ACK                                              0x02
#define IAP_GENERAL_LINGO_REQUEST_REMOTE_UI_MODE                           0x03
#define IAP_GENERAL_LINGO_REQUEST_EXTENDED_INTERFACE_MOD                   0x03
#define IAP_GENERAL_LINGO_RETURN_REMOTE_UI_MODE                            0x04
#define IAP_GENERAL_LINGO_RETURN_EXTENDED_INTERFACE_MODE_MODE              0x04
#define IAP_GENERAL_LINGO_ENTER_REMOTE_UI_MODE                             0x05
#define IAP_GENERAL_LINGO_ENTER_EXTENDED_INTERFACE_MODE                    0x05
#define IAP_GENERAL_LINGO_EXIT_REMOTE_UI_MODE                              0x06
#define IAP_GENERAL_LINGO_EXIT_EXTENDED_INTERFACE_MODE                     0x06
#define IAP_GENERAL_LINGO_REQUEST_IPOD_NAME                                0x07
#define IAP_GENERAL_LINGO_RETURN_IPOD_NAME                                 0x08
#define IAP_GENERAL_LINGO_REQUEST_IPOD_SOFTWARE_VERSION                    0x09
#define IAP_GENERAL_LINGO_RETURN_IPOD_SOFTWARE_VERSION                     0x0A
#define IAP_GENERAL_LINGO_REQUEST_IPOD_SERIAL_NUMBER                       0x0B
#define IAP_GENERAL_LINGO_RETURN_IPOD_SERIAL_NUMBER                        0x0C
#define IAP_GENERAL_LINGO_REQUEST_IPOD_MODEL_NUMBER                        0x0D
#define IAP_GENERAL_LINGO_RETURN_IPOD_MODEL_NUMBER                         0x0E
#define IAP_GENERAL_LINGO_REQUEST_LINGO_PROTOCOL_VERSION                   0x0F
#define IAP_GENERAL_LINGO_RETURN_LINGO_PROTOCOL_VERSION                    0x10
#define IAP_GENERAL_LINGO_REQUEST_TRANSPORT_MAX_PACKET_SIZE                0x11
#define IAP_GENERAL_LINGO_RETURN_TRANSPORT_MAX_PACKET_SIZE                 0x12
#define IAP_GENERAL_LINGO_IDENTIFY_DEVICE_LINGOS                           0x13
#define IAP_GENERAL_LINGO_IDENTIFY_ACCESSORY_LINGOS                        0x13
#define IAP_GENERAL_LINGO_GET_DEVICE_AUTHENTIFICATION_INFO                 0x14
#define IAP_GENERAL_LINGO_GET_ACCESSORY_AUTHENTIFICATION_INFO              0x14
#define IAP_GENERAL_LINGO_RET_DEVICE_AUTHENTIFICATION_INFO                 0x15
#define IAP_GENERAL_LINGO_RET_ACCESSORY_AUTHENTIFICATION_INFO              0x15
#define IAP_GENERAL_LINGO_ACK_DEVICE_AUTHENTIFICATION_INFO                 0x16
#define IAP_GENERAL_LINGO_ACK_ACCESSORY_AUTHENTIFICATION_INFO              0x16
#define IAP_GENERAL_LINGO_GET_DEVICE_AUTHENTIFICATION_SIGNATURE            0x17
#define IAP_GENERAL_LINGO_GET_ACCESSORY_AUTHENTIFICATION_SIGNATURE         0x17
#define IAP_GENERAL_LINGO_RET_DEVICE_AUTHENTIFICATION_SIGNATURE            0x18
#define IAP_GENERAL_LINGO_RET_ACCESSORY_AUTHENTIFICATION_SIGNATURE         0x18
#define IAP_GENERAL_LINGO_ACK_DEVICE_AUTHENTIFICATION_STATUS               0x19
#define IAP_GENERAL_LINGO_ACK_ACCESSORY_AUTHENTIFICATION_STATUS            0x19
#define IAP_GENERAL_LINGO_GET_IPOD_AUTHENTIFICATION_INFO                   0x1A
#define IAP_GENERAL_LINGO_RET_IPOD_AUTHENTIFICATION_INFO                   0x1B
#define IAP_GENERAL_LINGO_ACK_IPOD_AUTHENTIFICATION_INFO                   0x1C
#define IAP_GENERAL_LINGO_GET_IPOD_AUTHENTIFICATION_SIGNATURE              0x1D
#define IAP_GENERAL_LINGO_RET_IPOD_AUTHENTIFICATION_SIGNATURE              0x1E
#define IAP_GENERAL_LINGO_ACK_IPOD_AUTHENTIFICATION_STATUS                 0x1F
#define IAP_GENERAL_LINGO_NOTIFY_IPOD_STATE_CHANGE                         0x23
#define IAP_GENERAL_LINGO_GET_IPOD_OPTIONS                                 0x24
#define IAP_GENERAL_LINGO_RET_IPOD_OPTIONS                                 0x25
#define IAP_GENERAL_LINGO_GET_ACCESSORY_INFO                               0x27
#define IAP_GENERAL_LINGO_RET_ACCESSORY_INFO                               0x28
#define IAP_GENERAL_LINGO_GET_IPOD_PREFERENCES                             0x29
#define IAP_GENERAL_LINGO_RET_IPOD_PREFERENCES                             0x2A
#define IAP_GENERAL_LINGO_SET_IPOD_PREFERENCES                             0x2B
#define IAP_GENERAL_LINGO_GET_MODE                                         0x35
#define IAP_GENERAL_LINGO_RET_MODE                                         0x36
#define IAP_GENERAL_LINGO_SET_MODE                                         0x37
#define IAP_GENERAL_LINGO_START_IDPS                                       0x38
#define IAP_GENERAL_LINGO_FID_TOKEN_VALUES                                 0x39
#define IAP_GENERAL_LINGO_RET_FID_TOKEN_VALUE_ACKS                         0x3A
#define IAP_GENERAL_LINGO_ACK_FID_TOKEN_VALUES                             0x3A
#define IAP_GENERAL_LINGO_END_IDPS                                         0x3B
#define IAP_GENERAL_LINGO_IDPS_STATUS                                      0x3C
#define IAP_GENERAL_LINGO_OPEN_DATA_SESSION_FOR_PROTOCOL                   0x3F
#define IAP_GENERAL_LINGO_CLOSE_DATA_SESSION                               0x40
#define IAP_GENERAL_LINGO_DEVICE_ACK                                       0x41
#define IAP_GENERAL_LINGO_ACCESSORY_ACK                                    0x41
#define IAP_GENERAL_LINGO_DEVICE_DATA_TRANSFER                             0x42
#define IAP_GENERAL_LINGO_ACCESSORY_DATA_TRANSFER                          0x42
#define IAP_GENERAL_LINGO_IPOD_DATA_TRANSFER                               0x43
#define IAP_GENERAL_LINGO_SET_ACC_STATUS_NOTIFICATION                      0x46
#define IAP_GENERAL_LINGO_SET_ACCESSORY_STATUS_NOTIFICATION                0x46
#define IAP_GENERAL_LINGO_RET_ACC_STATUS_NOTIFICATION                      0x47
#define IAP_GENERAL_LINGO_RET_ACCESSORY_STATUS_NOTIFICATION                0x47
#define IAP_GENERAL_LINGO_ACCESSORY_STATUS_NOTIFICATION                    0x48
#define IAP_GENERAL_LINGO_SET_EVENT_NOTIFICATION                           0x49
#define IAP_GENERAL_LINGO_IPOD_NOTIFICATION                                0x4A
#define IAP_GENERAL_LINGO_GET_IPOD_OPTIONS_FOR_LINGO                       0x4B
#define IAP_GENERAL_LINGO_RET_IPOD_OPTIONS_FOR_LINGO                       0x4C
#define IAP_GENERAL_LINGO_GET_EVENT_NOTIFICATION                           0x4D
#define IAP_GENERAL_LINGO_RET_EVENT_NOTIFICATION                           0x4E
#define IAP_GENERAL_LINGO_GET_SUPPORTED_EVENT_NOTIFICATION                 0x4F
#define IAP_GENERAL_LINGO_CANCEL_COMMAND                                   0x50
#define IAP_GENERAL_LINGO_RET_SUPPORTED_EVENT_NOTIFICATION                 0x51
#define IAP_GENERAL_LINGO_REQUEST_APPLICATION_LAUNCH                       0x64
#define IAP_GENERAL_LINGO_GET_NOW_PLAYING_FOCUS_APP                        0x65
#define IAP_GENERAL_LINGO_RET_NOW_PLAYING_FOCUS_APP                        0x66
#define IAP_GENERAL_LINGO_IAP2_SUPPORT_QUERY                               0xEE

   /* Specific General Lingo packet definitions.                        */
   /* * NOTE * These definitions do not include ANY packet headers.     */
   /*          This means that the definitions will ONLY include the    */
   /*          variable data contained in the Command Payload.  The     */
   /*          checksum will be included immediately after the payload  */
   /*          data.                                                    */

   /* General Lingo ACK command (command data payload portion).         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_ACK_Data_t
{
   NonAlignedByte_t Status;
   NonAlignedByte_t CommmandID;
} __PACKED_STRUCT_END__ IAP_General_ACK_Data_t;

#define IAP_GENERAL_ACK_DATA_SIZE                              (sizeof(IAP_General_ACK_Data_t))

   /* General Lingo ACK command status values.                          */
#define IAP_ACK_COMMAND_ERROR_SUCCESS                                       0x00
#define IAP_ACK_COMMAND_ERROR_UNKNOWN_DATABASE_CATAGORY                     0x01
#define IAP_ACK_COMMAND_ERROR_COMMAND_FAILED                                0x02
#define IAP_ACK_COMMAND_ERROR_OUT_OF_RESOURCES                              0x03
#define IAP_ACK_COMMAND_ERROR_BAD_PARAMETER                                 0x04
#define IAP_ACK_COMMAND_ERROR_UNKNOWN_ID                                    0x05
#define IAP_ACK_COMMAND_ERROR_COMMAND_PENDING                               0x06
#define IAP_ACK_COMMAND_ERROR_NOT_AUTHENTICATED                             0x07
#define IAP_ACK_COMMAND_ERROR_BAD_AUTHENTICATION_VERSION                    0x08
#define IAP_ACK_COMMAND_ERROR_ACCESSORY_POWER_MODE_REQUEST_FAILED           0x09
#define IAP_ACK_COMMAND_ERROR_CERTIFICATE_INVALID                           0x0A
#define IAP_ACK_COMMAND_ERROR_CERTIFICATE_PERMISSIONS_INVALID               0x0B
#define IAP_ACK_COMMAND_ERROR_FILE_IN_USE                                   0x0C
#define IAP_ACK_COMMAND_ERROR_INVALID_FILE_HANDLE                           0x0D
#define IAP_ACK_COMMAND_ERROR_DIRECTORY_NOT_EMPTY                           0x0E
#define IAP_ACK_COMMAND_ERROR_OPERATION_TIMED_OUT                           0x0F
#define IAP_ACK_COMMAND_ERROR_COMMAND_UNAVAILABE_IN_THIS_IPOD_MODE          0x10
#define IAP_ACK_COMMAND_ERROR_INVALID_ACCESSORY_REGISTER_ID_VALUE           0x11
#define IAP_ACK_COMMAND_ERROR_MAX_NUMBER_OF_ACCESSORY_CONNECTIONS_REACHED   0x15
#define IAP_ACK_COMMAND_ERROR_SESSION_WRITE_FAILURE                         0x17

   /* General Lingo ACK command when status type is command pending     */
   /* (command data payload portion).                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_ACK_Command_Pending_Data_t
{
   NonAlignedByte_t  Status;
   NonAlignedByte_t  CommmandID;
   NonAlignedDWord_t MaximumPendingWait;
} __PACKED_STRUCT_END__ IAP_General_ACK_Command_Pending_Data_t;

#define IAP_GENERAL_ACK_COMMAND_PENDING_DATA_SIZE              (sizeof(IAP_General_ACK_Command_Pending_Data_t))

   /* General Lingo ACK command when status type is session write       */
   /* failure (command data payload portion).                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_ACK_Session_Write_Failure_Data_t
{
   NonAlignedByte_t  Status;
   NonAlignedByte_t  CommmandID;
   NonAlignedWord_t  SessionID;
   NonAlignedDWord_t NumberBytesDropped;
} __PACKED_STRUCT_END__ IAP_General_ACK_Session_Write_Failure_Data_t;

#define IAP_GENERAL_ACK_SESSION_WRITE_FAILURE_DATA_SIZE        (sizeof(IAP_General_ACK_Session_Write_Failure_Data_t))

   /* General Lingo Return Remote UI Mode command (command data payload */
   /* portion).                                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_Remote_UI_Mode_Data_t
{
   NonAlignedByte_t Mode;
} __PACKED_STRUCT_END__ IAP_General_Return_Remote_UI_Mode_Data_t;

#define IAP_GENERAL_RETURN_REMOTE_UI_MODE_DATA_SIZE            (sizeof(IAP_General_Return_Remote_UI_Mode_Data_t))

   /* Values for Mode member of the Return Remote UI Mode command.      */
#define IAP_GENERAL_RETURN_REMOTE_UI_MODE_STANDARD_UI_MODE                 0x00

   /* General Lingo Return iPod Name command (command data payload      */
   /* portion).                                                         */
   /* * NOTE * iPodName is zero or more bytes (UTF-8 encoded).          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_iPod_Name_Data_t
{
   NonAlignedByte_t iPodName[1];
} __PACKED_STRUCT_END__ IAP_General_Return_iPod_Name_Data_t;

   /* The following MACRO is a utility MACRO that is provided to        */
   /* determine the number of bytes that will be occupied by the iAP    */
   /* General Lingo Return iPod Name command payload give the specified */
   /* length (in bytes) of the iPod Name.                               */
#define IAP_GENERAL_RETURN_IPOD_NAME_DATA_SIZE(_x)             (STRUCTURE_OFFSET(IAP_General_Return_iPod_Name_Data_t, iPodName) + (_x))

   /* General Lingo Return iPod Version command (command data payload   */
   /* portion).                                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_iPod_Version_Data_t
{
   NonAlignedByte_t iPodMajorVersion;
   NonAlignedByte_t iPodMinorVersion;
   NonAlignedByte_t iPodRevisionVersion;
} __PACKED_STRUCT_END__ IAP_General_Return_iPod_Version_Data_t;

#define IAP_GENERAL_RETURN_IPOD_VERSION_DATA_SIZE              (sizeof(IAP_General_Return_iPod_Version_Data_t))

   /* General Lingo Return iPod Serial Number command (command data     */
   /* payload portion).                                                 */
   /* * NOTE * iPodSerialNumber is zero or more bytes (UTF-8 encoded).  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_iPod_Serial_Number_Data_t
{
   NonAlignedByte_t iPodSerialNumber[1];
} __PACKED_STRUCT_END__ IAP_General_Return_iPod_Serial_Number_Data_t;

   /* The following MACRO is a utility MACRO that is provided to        */
   /* determine the number of bytes that will be occupied by the iAP    */
   /* General Lingo Return iPod Serial Number command payload give the  */
   /* specified length (in bytes) of the iPod Serial Number.            */
#define IAP_GENERAL_RETURN_IPOD_SERIAL_NUMBER_DATA_SIZE(_x)    (STRUCTURE_OFFSET(IAP_General_Return_iPod_Serial_Number_Data_t, iPodSerialNumber) + (_x))

   /* General Lingo Return iPod Model Number command (command data      */
   /* payload portion).                                                 */
   /* * NOTE * iPodModelNumber is zero or more bytes (UTF-8 encoded).   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_iPod_Model_Number_Data_t
{
   NonAlignedDWord_t iPodModelID;
   NonAlignedByte_t  iPodModelNumber[1];
} __PACKED_STRUCT_END__ IAP_General_Return_iPod_Model_Number_Data_t;

   /* The following MACRO is a utility MACRO that is provided to        */
   /* determine the number of bytes that will be occupied by the iAP    */
   /* General Lingo Return iPod Model Number command payload give the   */
   /* specified length (in bytes) of the iPod Model Number.             */
#define IAP_GENERAL_RETURN_IPOD_MODEL_NUMBER_DATA_SIZE(_x)     (STRUCTURE_OFFSET(IAP_General_Return_iPod_Model_Number_Data_t, iPodModelNumber) + (_x))

   /* The following bit-mask constants are used with the iPodModelID    */
   /* member to denote the iPod model number of the device.             */
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_MINI_ORIGINAL_4GB               0x00040000
#define IAP_IPOD_MODEL_NUMBER_MASK_4G_IPOD_WHITE_GRAY_CLICK_WHEEL       0x00050000
#define IAP_IPOD_MODEL_NUMBER_MASK_4G_IPOD_COLOR_DISPLAY                0x00060000
#define IAP_IPOD_MODEL_NUMBER_MASK_2ND_GEN_IPOD_MINI_M9800_M9807        0x00070000
#define IAP_IPOD_MODEL_NUMBER_MASK_5G_IPOD                              0x000B0000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_NANO                            0x000C0000
#define IAP_IPOD_MODEL_NUMBER_MASK_2G_IPOD_NANO                         0x00100000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPHONE                               0x00110000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_CLASSIC                         0x00130000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_CLASSIC_120GB                   0x00130100
#define IAP_IPOD_MODEL_NUMBER_MASK_3G_IPOD_NANO                         0x00140000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_TOUCH                           0x00150000
#define IAP_IPOD_MODEL_NUMBER_MASK_4G_IPOD_NANO                         0x00170000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPHONE_3G                            0x00180000
#define IAP_IPOD_MODEL_NUMBER_MASK_2G_TOUCH                             0x00190000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPHONE_3GS                           0x001B0000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPOD_CLASSIC_160GB                   0x00130200
#define IAP_IPOD_MODEL_NUMBER_MASK_5G_IPOD_NANO                         0x001C0000
#define IAP_IPOD_MODEL_NUMBER_MASK_2G_TOUCH_2009                        0x001D0000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPAD_IPAD_3G                         0x001F0000
#define IAP_IPOD_MODEL_NUMBER_MASK_IPHONE_4                             0x00200000

   /* General Lingo Request Lingo Protocol Version command (command data*/
   /* payload portion).                                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Request_Lingo_Protocol_Version_Data_t
{
   NonAlignedByte_t LingoID;
} __PACKED_STRUCT_END__ IAP_General_Request_Lingo_Protocol_Version_Data_t;

#define IAP_GENERAL_REQUEST_LINGO_PROTOCOL_VERSION_DATA_SIZE   (sizeof(IAP_General_Request_Lingo_Protocol_Version_Data_t))

   /* General Lingo Return Lingo Protocol Version command (command data */
   /* payload portion).                                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_General_Return_Lingo_Protocol_Version_Data_t
{
   NonAlignedByte_t LingoID;
   NonAlignedByte_t MajorProtocolVersion;
   NonAlignedByte_t MinorProtocolVersion;
} __PACKED_STRUCT_END__ IAP_General_Return_Lingo_Protocol_Version_Data_t;

#define IAP_GENERAL_RETURN_LINGO_PROTOCOL_VERSION_DATA_SIZE    (sizeof(IAP_General_Return_Lingo_Protocol_Version_Data_t))

   /* Microphone Lingo command packet types.                            */
#define IAP_MICROPHONE_LINGO_BEGIN_RECORD                                   0x00
#define IAP_MICROPHONE_LINGO_END_RECORD                                     0x01
#define IAP_MICROPHONE_LINGO_BEGIN_PLAYBACK                                 0x02
#define IAP_MICROPHONE_LINGO_END_PLAYBACK                                   0x03
#define IAP_MICROPHONE_LINGO_ACK                                            0x04
#define IAP_MICROPHONE_LINGO_GET_DEVICE_ACK                                 0x05
#define IAP_MICROPHONE_LINGO_IPOD_MOTE_CHANGE                               0x06
#define IAP_MICROPHONE_LINGO_GET_DEVICE_CAPS                                0x07
#define IAP_MICROPHONE_LINGO_RET_DEVICE_CAPS                                0x08
#define IAP_MICROPHONE_LINGO_GET_DEVICE_CTRL                                0x09
#define IAP_MICROPHONE_LINGO_RET_DEVICE_CTRL                                0x0A
#define IAP_MICROPHONE_LINGO_SET_DEVICE_CTRL                                0x0B

   /* Simple Remote Control Lingo command packet types.                 */
#define IAP_SIMPLE_REMOTE_LINGO_CONTEXT_BUTTON_STATUS                       0x00
#define IAP_SIMPLE_REMOTE_LINGO_ACK                                         0x01
#define IAP_SIMPLE_REMOTE_LINGO_IMAGE_BUTTON_STATUS                         0x02
#define IAP_SIMPLE_REMOTE_LINGO_VIDEO_BUTTON_STATUS                         0x03
#define IAP_SIMPLE_REMOTE_LINGO_AUDIO_BUTTON_STATUS                         0x04
#define IAP_SIMPLE_REMOTE_LINGO_IPOD_OUT_BUTTON_STATUS                      0x0B
#define IAP_SIMPLE_REMOTE_LINGO_ROTATION_INPUT_STATUS                       0x0C
#define IAP_SIMPLE_REMOTE_LINGO_RADIO_BUTTON_STATUS                         0x0D
#define IAP_SIMPLE_REMOTE_LINGO_CAMERA_BUTTON_STATUS                        0x0E
#define IAP_SIMPLE_REMOTE_LINGO_REGISTER_DISCRIPTOR                         0x0F
#define IAP_SIMPLE_REMOTE_LINGO_SEND_HID_REPORT_TO_IPOD                     0x10
#define IAP_SIMPLE_REMOTE_LINGO_IPOD_HID_REPORT                             0x10
#define IAP_SIMPLE_REMOTE_LINGO_SEND_HID_REPORT_TO_ACC                      0x11
#define IAP_SIMPLE_REMOTE_LINGO_ACCESSORY_HID_REPORT                        0x11
#define IAP_SIMPLE_REMOTE_LINGO_UNREGISTER_DESCRIPTOR                       0x12
#define IAP_SIMPLE_REMOTE_LINGO_ACCESSIBILITY_EVENT                         0x13
#define IAP_SIMPLE_REMOTE_LINGO_GET_ACCESSIBILITY_PARAMETER                 0x14
#define IAP_SIMPLE_REMOTE_LINGO_RET_ACCESSIBILITY_PARAMETER                 0x15
#define IAP_SIMPLE_REMOTE_LINGO_SET_ACCESSIBILITY_PARAMETER                 0x16
#define IAP_SIMPLE_REMOTE_LINGO_GET_CURRENT_PROPERTY                        0x17
#define IAP_SIMPLE_REMOTE_LINGO_RET_CURRENT_PROPERTY                        0x18
#define IAP_SIMPLE_REMOTE_LINGO_SET_CONTEXT                                 0x19
#define IAP_SIMPLE_REMOTE_LINGO_DEVICE_ACK                                  0x81
#define IAP_SIMPLE_REMOTE_LINGO_ACCESSORY_ACK                               0x81

   /* Display Lingo command packet types.                               */
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_ACK                                0x00
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_CURRENT_EQ_PROFILE_INDEX       0x01
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_CURRENT_EQ_PROFILE_INDEX       0x02
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_CURRENT_EQ_PROFILE_INDEX       0x03
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_NUM_EQ_PROFILES                0x04
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_NUM_EQ_PROFILES                0x05
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_INDEXED_EQ_PROFILES_NAME       0x06
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_INDEXED_EQ_PROFILES_NAME       0x07
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_REMOTE_EVENT_NOTIFICATION      0x08
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_REMOTE_EVENT_NOTIFICATION          0x09
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_EVENT_STATUS                   0x0A
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_EVENT_STATUS                   0x0B
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_IPOD_STATUS                    0x0C
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_IPOD_STATUS                    0x0D
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_IPOD_STATUS                    0x0E
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_PLAYBACK_STATUS                0x0F
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_PLAYBACK_STATUS                0x10
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_CURRENT_PLAYING_TRACK          0x11
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_INDEXED_PLAYING_TRACK_INFO     0x12
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_INDEXED_PLAYING_TRACK_INFO     0x13
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_NUM_PLAYING_TRACKS             0x14
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_NUM_PLAYING_TRACKS             0x15
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_ATRWORK_FORMATS                0x16
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_ATRWORK_FORMATS                0x17
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_TRACK_ATRWORK_DATA             0x18
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_TRACK_ATRWORK_DATA             0x19
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_POWER_BATTERY_STATE            0x1A
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_POWER_BATTERY_STATE            0x1B
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_SOUND_CHECK_STATE              0x1C
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_SOUND_CHECK_STATE              0x1D
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_SET_SOUND_CHECK_STATE              0x1E
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_GET_TRACK_ATRWORK_TIMES            0x1F
#define IAP_DISPLAY_DISPLAY_REMOTE_LINGO_RET_TRACK_ATRWORK_TIMES            0x20

   /* Accessory Power Lingo command packet types.                       */
#define IAP_ACCESSORY_POWER_LINGO_BEGIN_HIGH_POWER                          0x02
#define IAP_ACCESSORY_POWER_LINGO_END_HIGH_POWER                            0x03

   /* USB Host Lingo command packet types.                              */
#define IAP_USB_HOST_MODE_LINGO_DEVICE_ACK                                  0x00
#define IAP_USB_HOST_MODE_LINGO_ACCESSORY_ACK                               0x00
#define IAP_USB_HOST_MODE_LINGO_NOTIFY_USB_MODE                             0x04
#define IAP_USB_HOST_MODE_LINGO_IPOD_ACK                                    0x80
#define IAP_USB_HOST_MODE_LINGO_GET_IPOD_USB_MODE                           0x81
#define IAP_USB_HOST_MODE_LINGO_RET_IPOD_USB_MODE                           0x82
#define IAP_USB_HOST_MODE_LINGO_SET_IPOD_USB_MODE                           0x83

   /* RF Tuner Lingo command packet types.                              */
#define IAP_RF_TUNER_LINGO_ACK                                              0x00
#define IAP_RF_TUNER_LINGO_GET_TUNER_CAPS                                   0x01
#define IAP_RF_TUNER_LINGO_RET_TUNER_CAPS                                   0x02
#define IAP_RF_TUNER_LINGO_GET_TUNER_CTRL                                   0x03
#define IAP_RF_TUNER_LINGO_RET_TUNER_CTRL                                   0x04
#define IAP_RF_TUNER_LINGO_SET_TUNER_CTRL                                   0x05
#define IAP_RF_TUNER_LINGO_GET_TUNER_BAND                                   0x06
#define IAP_RF_TUNER_LINGO_RET_TUNER_BAND                                   0x07
#define IAP_RF_TUNER_LINGO_SET_TUNER_BAND                                   0x08
#define IAP_RF_TUNER_LINGO_GET_TUNER_FREQ                                   0x09
#define IAP_RF_TUNER_LINGO_RET_TUNER_FREQ                                   0x0A
#define IAP_RF_TUNER_LINGO_SET_TUNER_FREQ                                   0x0B
#define IAP_RF_TUNER_LINGO_GET_TUNER_MODE                                   0x0C
#define IAP_RF_TUNER_LINGO_RET_TUNER_MODE                                   0x0D
#define IAP_RF_TUNER_LINGO_SET_TUNER_MODE                                   0x0E
#define IAP_RF_TUNER_LINGO_GET_TUNER_SEEK_RSSI                              0x0F
#define IAP_RF_TUNER_LINGO_RET_TUNER_SEEK_RSSI                              0x00
#define IAP_RF_TUNER_LINGO_SET_TUNER_SEEK_RSSI                              0x11
#define IAP_RF_TUNER_LINGO_TUNER_SEEK_START                                 0x12
#define IAP_RF_TUNER_LINGO_TUNER_SEEK_DONE                                  0x13
#define IAP_RF_TUNER_LINGO_GET_TUNER_STATUS                                 0x14
#define IAP_RF_TUNER_LINGO_RET_TUNER_STATUS                                 0x15
#define IAP_RF_TUNER_LINGO_GET_STATUS_NOTIFY_MASK                           0x16
#define IAP_RF_TUNER_LINGO_RET_STATUS_NOTIFY_MASK                           0x17
#define IAP_RF_TUNER_LINGO_SET_STATUS_NOTIFY_MASK                           0x18
#define IAP_RF_TUNER_LINGO_SET_CHANGE_NOTIFY                                0x19
#define IAP_RF_TUNER_LINGO_GET_RDS_READY_STATUS                             0x1A
#define IAP_RF_TUNER_LINGO_RET_RDS_READY_STATUS                             0x1B
#define IAP_RF_TUNER_LINGO_GET_RDS_DATA                                     0x1C
#define IAP_RF_TUNER_LINGO_RET_RDS_DATA                                     0x1D
#define IAP_RF_TUNER_LINGO_GET_RDS_NOTIFY_MASK                              0x1E
#define IAP_RF_TUNER_LINGO_RET_RDS_NOTIFY_MASK                              0x1F
#define IAP_RF_TUNER_LINGO_SET_RDS_NOTIFY_MASK                              0x20
#define IAP_RF_TUNER_LINGO_RDS_READY_NOTIFY                                 0x21
#define IAP_RF_TUNER_LINGO_GET_HDP_PROGRAM_SERVICE_COUNT                    0x25
#define IAP_RF_TUNER_LINGO_RET_HDP_PROGRAM_SERVICE_COUNT                    0x26
#define IAP_RF_TUNER_LINGO_GET_HDP_PROGRAM_SERVICE                          0x27
#define IAP_RF_TUNER_LINGO_RET_HDP_PROGRAM_SERVICE                          0x28
#define IAP_RF_TUNER_LINGO_SET_HDP_PROGRAM_SERVICE                          0x29
#define IAP_RF_TUNER_LINGO_GET_HDD_DATA_READY_SERVICE                       0x2A
#define IAP_RF_TUNER_LINGO_RET_HDD_DATA_READY_SERVICE                       0x2B
#define IAP_RF_TUNER_LINGO_GET_HDD_DATA                                     0x2C
#define IAP_RF_TUNER_LINGO_RET_HDD_DATA                                     0x2D
#define IAP_RF_TUNER_LINGO_GET_HDD_DATA_NOTIFY_MASK                         0x2E
#define IAP_RF_TUNER_LINGO_RET_HDD_DATA_NOTIFY_MASK                         0x2F
#define IAP_RF_TUNER_LINGO_SET_HDD_DATA_NOTIFY_MASK                         0x30
#define IAP_RF_TUNER_LINGO_SET_HDD_DATA_READY_NOTIFY                        0x31

   /* Accessory Equalizer Lingo command packet types.                   */
#define IAP_ACCESSORY_EQUALIZER_LINGO_ACK                                   0x00
#define IAP_ACCESSORY_EQUALIZER_LINGO_GET_CURRENT_EQ_INDEX                  0x01
#define IAP_ACCESSORY_EQUALIZER_LINGO_RET_CURRENT_EQ_INDEX                  0x02
#define IAP_ACCESSORY_EQUALIZER_LINGO_SET_CURRENT_EQ_INDEX                  0x03
#define IAP_ACCESSORY_EQUALIZER_LINGO_GET_SETTINGS_COUNT                    0x04
#define IAP_ACCESSORY_EQUALIZER_LINGO_RET_SETTINGS_COUNT                    0x05
#define IAP_ACCESSORY_EQUALIZER_LINGO_GET_EQ_INDEX_NAME                     0x06
#define IAP_ACCESSORY_EQUALIZER_LINGO_RET_EQ_INDEX_NAME                     0x07

   /* Sports Lingo command packet types.                                */
#define IAP_SPORTS_LINGO_DEVICE_ACK                                         0x00
#define IAP_SPORTS_LINGO_ACCESSORY_ACK                                      0x00
#define IAP_SPORTS_LINGO_GET_DEVICE_VERSION                                 0x01
#define IAP_SPORTS_LINGO_GET_ACCESSORY_VERSION                              0x01
#define IAP_SPORTS_LINGO_RET_DEVICE_VERSION                                 0x02
#define IAP_SPORTS_LINGO_RET_ACCESSORY_VERSION                              0x02
#define IAP_SPORTS_LINGO_GET_DEVICE_CAPS                                    0x03
#define IAP_SPORTS_LINGO_GET_ACCESSORY_CAPS                                 0x03
#define IAP_SPORTS_LINGO_RET_DEVICE_CAPS                                    0x04
#define IAP_SPORTS_LINGO_RET_ACCESSORY_CAPS                                 0x04
#define IAP_SPORTS_LINGO_IPOD_ACK                                           0x80
#define IAP_SPORTS_LINGO_GET_IPOD_CAPS                                      0x83
#define IAP_SPORTS_LINGO_RET_IPOD_CAPS                                      0x84
#define IAP_SPORTS_LINGO_GET_USER_INDEX                                     0x85
#define IAP_SPORTS_LINGO_RET_USER_INDEX                                     0x86
#define IAP_SPORTS_LINGO_GET_USER_DATA                                      0x88
#define IAP_SPORTS_LINGO_RET_USER_DATA                                      0x89
#define IAP_SPORTS_LINGO_SET_USER_DATA                                      0x8A

   /* Digital Audio Lingo command packet types.                         */
#define IAP_DIGITAL_AUDIO_LINGO_ACC_ACK                                     0x00
#define IAP_DIGITAL_AUDIO_LINGO_ACCESSPRY_ACK                               0x00
#define IAP_DIGITAL_AUDIO_LINGO_IPOD_ACK                                    0x01
#define IAP_DIGITAL_AUDIO_LINGO_GET_ACC_SAMPLE_RATE_CAPS                    0x02
#define IAP_DIGITAL_AUDIO_LINGO_GET_ACCESSORY_SAMPLE_RATE_CAPS              0x02
#define IAP_DIGITAL_AUDIO_LINGO_RET_ACC_SAMPLE_RATE_CAPS                    0x03
#define IAP_DIGITAL_AUDIO_LINGO_RET_ACCESSORY_SAMPLE_RATE_CAPS              0x03
#define IAP_DIGITAL_AUDIO_LINGO_NEW_IPOD_TRACK_INFO                         0x04
#define IAP_DIGITAL_AUDIO_LINGO_SET_VIDEO_DELAY                             0x05

   /* Storage Lingo command packet types.                               */
#define IAP_STORAGE_LINGO_IPOD_ACK                                          0x00
#define IAP_STORAGE_LINGO_GET_IPOD_CAPS                                     0x01
#define IAP_STORAGE_LINGO_RET_IPOD_CAPS                                     0x02
#define IAP_STORAGE_LINGO_RET_IPOD_FILE_HANDLE                              0x04
#define IAP_STORAGE_LINGO_WRITE_IPOD_FILE_DATA                              0x07
#define IAP_STORAGE_LINGO_CLOSE_IPOD_FILE                                   0x08
#define IAP_STORAGE_LINGO_GET_IPOD_FREE_SPACE                               0x10
#define IAP_STORAGE_LINGO_RET_IPOD_FREE_SPACE                               0x11
#define IAP_STORAGE_LINGO_OPEN_IPOD_FEATURE_FILE                            0x12
#define IAP_STORAGE_LINGO_DEVICE_ACK                                        0x80
#define IAP_STORAGE_LINGO_ACCESSORY_ACK                                     0x80
#define IAP_STORAGE_LINGO_GET_DEVICE_CAPS                                   0x81
#define IAP_STORAGE_LINGO_GET_ACCESSORY_CAPS                                0x81
#define IAP_STORAGE_LINGO_RET_DEVICE_CAPS                                   0x82
#define IAP_STORAGE_LINGO_RET_ACCESSORY_CAPS                                0x82

   /* IPOD Out command packet types.                                    */
#define IAP_IPOD_OUT_LINGO_IPOD_ACK                                         0x00
#define IAP_IPOD_OUT_LINGO_GET_IPOD_OUT_OPTIONS                             0x01
#define IAP_IPOD_OUT_LINGO_RET_IPOD_OUT_OPTIONS                             0x02
#define IAP_IPOD_OUT_LINGO_SET_IPOD_OUT_OPTIONS                             0x03
#define IAP_IPOD_OUT_LINGO_DEVICE_STATE_CHANGE_EVENT                        0x04
#define IAP_IPOD_OUT_LINGO_ACCESSORY_STATE_CHANGE_EVENT                     0x04
#define IAP_IPOD_OUT_LINGO_DEVICE_VIDEO_SCREEN_INFO                         0x06
#define IAP_IPOD_OUT_LINGO_ACCESSORY_VIDEO_SCREEN_INFO                      0x06

   /* Location command packet types.                                    */
#define IAP_LOCATION_LINGO_DEVICE_ACK                                       0x00
#define IAP_LOCATION_LINGO_ACCESSORY_ACK                                    0x00
#define IAP_LOCATION_LINGO_GET_DEVICE_CAPS                                  0x01
#define IAP_LOCATION_LINGO_GET_ACCESSORYPS                                  0x01
#define IAP_LOCATION_LINGO_RET_DEVICE_CAPS                                  0x02
#define IAP_LOCATION_LINGO_RET_ACCESSORY_CAPS                               0x02
#define IAP_LOCATION_LINGO_GET_DEVICE_CONTROL                               0x03
#define IAP_LOCATION_LINGO_GET_ACCESSORY_CONTROL                            0x03
#define IAP_LOCATION_LINGO_RET_DEVICE_CONTROL                               0x04
#define IAP_LOCATION_LINGO_RET_ACCESSORY_CONTROL                            0x04
#define IAP_LOCATION_LINGO_SET_DEVICE_CONTROL                               0x05
#define IAP_LOCATION_LINGO_SET_ACCESSORY_CONTROL                            0x05
#define IAP_LOCATION_LINGO_GET_DEVICE_DATA                                  0x06
#define IAP_LOCATION_LINGO_GET_ACCESSORY_DATA                               0x06
#define IAP_LOCATION_LINGO_RET_DEVICE_DATA                                  0x07
#define IAP_LOCATION_LINGO_RET_ACCESSORY_DATA                               0x07
#define IAP_LOCATION_LINGO_SET_DEVICE_DATA                                  0x08
#define IAP_LOCATION_LINGO_SET_ACCESSORY_DATA                               0x08
#define IAP_LOCATION_LINGO_ASYNC_DEVICE_DATA                                0x09
#define IAP_LOCATION_LINGO_ASYNC_ACCESSORY_DATA                             0x09
#define IAP_LOCATION_LINGO_IPOD_ACK                                         0x80

   /* Full ID String (FID) type/sub-types.                              */
#define IAP_FID_TYPE_GENERAL                                                0x00
#define IAP_FID_SUBTYPE_TOKEN_IDENTIFY                                      0x00
#define IAP_FID_SUBTYPE_TOKEN_ACC_CAPS                                      0x01
#define IAP_FID_SUBTYPE_TOKEN_ACCESSORY_CAPS                                0x01
#define IAP_FID_SUBTYPE_TOKEN_ACC_INFO                                      0x02
#define IAP_FID_SUBTYPE_TOKEN_ACCESSORY_INFO                                0x02
#define IAP_FID_SUBTYPE_TOKEN_IPOD_PREFERENCE                               0x03
#define IAP_FID_SUBTYPE_TOKEN_SDK_PROTOCOL                                  0x04
#define IAP_FID_SUBTYPE_TOKEN_BUNDLE_SEED_ID_PREF                           0x05
#define IAP_FID_SUBTYPE_TOKEN_SCREEN_INFO                                   0x07
#define IAP_FID_SUBTYPE_TOKEN_SDK_PROTOCOL_METADATA                         0x08

#define IAP_FID_TYPE_MICROPHONE                                             0x01
#define IAP_FID_SUBTYPE_TOKEN_MICROPHONE_CAPS                               0x00

   /* Identify Device options.                                          */
#define IAP_DEVICE_OPTIONS_AUTHENTICATE_IMMEDIATLY                        0x0002
#define IAP_DEVICE_OPTIONS_LOW_POWER_ONLY                                 0x0000
#define IAP_DEVICE_OPTIONS_INTERMEDIATE_HIGH_POWER                        0x0004
#define IAP_DEVICE_OPTIONS_CONSTANT_HIGH_POWER                            0x000C

   /* Accessory capabilities.                                           */
#define IAP_ACCESSORY_CAPABILITIES_ANALOG_LINE_OUT                     (1 <<  0)
#define IAP_ACCESSORY_CAPABILITIES_ANALOG_LINE_IN                      (1 <<  1)
#define IAP_ACCESSORY_CAPABILITIES_ANALOG_VIDEO_OUT                    (1 <<  2)
#define IAP_ACCESSORY_CAPABILITIES_USB_AUDIO                           (1 <<  4)
#define IAP_ACCESSORY_CAPABILITIES_SUPPORTS_COMM_WITH_APP              (1 <<  9)
#define IAP_ACCESSORY_CAPABILITIES_CHECKS_IPOD_VOLUME                  (1 << 11)
#define IAP_ACCESSORY_CAPABILITIES_USES_IPOD_ACCESSABILITY             (1 << 17)
#define IAP_ACCESSORY_CAPABILITIES_HANDLES_MULTI_PACKET_RESPONSE       (1 << 19)

   /* Accessory Screen feature options.                                 */
#define IAP_ACCESSORY_SCREEN_FEATURE_HAS_COLOR_DISPLAY                 (1 <<  0)

   /* Accessory Info type definitions.                                  */
#define IAP_ACCESSORY_INFO_NAME                                             0x01
#define IAP_ACCESSORY_INFO_FIRMWARE_VERSION                                 0x04
#define IAP_ACCESSORY_INFO_HARDWARE_VERSION                                 0x05
#define IAP_ACCESSORY_INFO_MANUFACTURER                                     0x06
#define IAP_ACCESSORY_INFO_MODEL_NUMBER                                     0x07
#define IAP_ACCESSORY_INFO_SERIAL_NUMBER                                    0x08
#define IAP_ACCESSORY_INFO_IN_MAX_PACKET_SIZE                               0x09
#define IAP_ACCESSORY_INFO_STATUS                                           0x0B
#define IAP_ACCESSORY_INFO_RF_CERTIFICATIONS                                0x0C

   /* Accessory RF Certification values.                                */
#define IAP_ACCESSORY_RF_CERTIFICATION_CLASS_1                              0x00
#define IAP_ACCESSORY_RF_CERTIFICATION_CLASS_2                              0x01
#define IAP_ACCESSORY_RF_CERTIFICATION_CLASS_3                              0x02

   /* Accessory Meta-data types.                                        */
#define IAP_META_DATA_TYPE_MATCHING_PROTOCOL_HIDE_MAX                       0x00
#define IAP_META_DATA_TYPE_MATCHING_PROTOCOL_DEFAULT                        0x01
#define IAP_META_DATA_TYPE_MATCHING_PROTOCOL_OVERRIDE                       0x02
#define IAP_META_DATA_TYPE_MATCHING_PROTOCOL_HIDE                           0x03

   /* Microphone capability values.                                     */
#define IAP_MICROPHONE_CAPABILITIES_STEREO_LINE_IN                     (1 <<  0)
#define IAP_MICROPHONE_CAPABILITIES_STEREO_OR_MONO_LINE_IN             (1 <<  1)
#define IAP_MICROPHONE_CAPABILITIES_RECORDING_LEVEL_PRESENT            (1 <<  2)
#define IAP_MICROPHONE_CAPABILITIES_RECORDING_LEVEL_LIMIT_PRESENT      (1 <<  3)
#define IAP_MICROPHONE_CAPABILITIES_SUPPORTS_DUPLEX_AUDIO              (1 <<  4)

#define IAP_TOKEN_STATUS_TOKEN_VALUE_ACCEPTED                               0x00
#define IAP_TOKEN_STATUS_REQUIRED_TOKEN_VALUE_FAILED                        0x01
#define IAP_TOKEN_STATUS_TOKEN_VALUE_RECOGNIZED_BUT_FAILED                  0x02
#define IAP_TOKEN_STATUS_TOKEN_NOT_SUPPORTED                                0x03
#define IAP_TOKEN_STATUS_LINGO_BUSY                                         0x04
#define IAP_TOKEN_STATUS_MAX_CONNECTIONS_REACHED                            0x05

#define IAP_ACCESSORY_END_IDPS_STATUS_CONTINUE_WITH_AUTHENTICATION          0x00
#define IAP_ACCESSORY_END_IDPS_STATUS_RESET_ALL_IDPS_INFORMATION            0x01
#define IAP_ACCESSORY_END_IDPS_STATUS_ADANDON_PROCESS                       0x02

#define IAP_IDPS_STATUS_NORMAL_AUTHENTICATION_WILL_PROCEED                  0x00
#define IAP_IDPS_STATUS_NORMAL_REQUIRED_TOKEN_REJECTED                      0x01
#define IAP_IDPS_STATUS_NORMAL_REQUIRED_TOKEN_MISSING                       0x02
#define IAP_IDPS_STATUS_NORMAL_REQUIRED_TOKEN_REJECTED_AND_TOKEN_MISSING    0x03
#define IAP_IDPS_STATUS_RESET_RETRY_IDPS                                    0x04
#define IAP_IDPS_STATUS_RESET_IDPS_TIME_LIMIT_EXCEEDED                      0x05
#define IAP_IDPS_STATUS_ABANDOM_IDPS_PROCESS_STOPPING                       0x06
#define IAP_IDPS_STATUS_NORMAL_IDPS_PROCESS_FAILURE                         0x07

#define IAP_ACCESSORY_STATUS_TYPE_FAULT                                     0x02

#define IAP_ACCESSORY_STATUS_FAULT_NO_FAULT                                 0x00
#define IAP_ACCESSORY_STATUS_FAULT_VOLTAGE_FAULT                            0x01
#define IAP_ACCESSORY_STATUS_FAULT_CURRENT_FAULT                            0x02

#define IAP_ACCESSORY_STATUS_FAULT_CONDITION_NO_FAULT                       0x00
#define IAP_ACCESSORY_STATUS_FAULT_CONDITION_RECOVERABLE                    0x01
#define IAP_ACCESSORY_STATUS_FAULT_CONDITION_NON_RECOVERABLE                0x02

#define IAP_ACCESSORY_STATE_CHANGE_HIBERNATE_NO_CONTEXT_PRESERVED           0x01
#define IAP_ACCESSORY_STATE_CHANGE_HIBERNATE_CONTEXT_PRESERVED              0x02
#define IAP_ACCESSORY_STATE_CHANGE_SLEEP_STATE                              0x03
#define IAP_ACCESSORY_STATE_CHANGE_POWER_ON_STATE                           0x04

#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_LINE_OUT_USAGE                 (1 <<  0)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_VIDEO_OUTPUT                   (1 <<  1)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_NTSC_VIDEO_FORMAT              (1 <<  2)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_PAL_VIDEO_FORMAT               (1 <<  3)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_COMPOSITE_VIDEO_OUT            (1 <<  4)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_S_VIDEO_OUT                    (1 <<  5)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_COMPONENT_VIDEO_OUT            (1 <<  6)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_CLOSED_CAPTION                 (1 <<  7)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_VIDEO_ASPECT_RATIO_FULL_SCREEN (1 <<  8)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_VIDEO_ASPECT_RATIO_WIDE_SCREEN (1 <<  9)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_SUBTITLES                      (1 << 10)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_VIDEO_ALTERNATE_AUDIO_CHANNEL  (1 << 11)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_COMMICATION_WITH_OS3X_APPS     (1 << 13)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_IPOD_NOTIFICATIONS             (1 << 14)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_PAUSE_ON_POWER_REMOVAL         (1 << 19)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_EXTENDED_IDPS_TOKEN_HANDLING   (1 << 23)
#define IAP_IPOD_GENERAL_LINGO_OPTION_BIT_REQUEST_APPLICATION_LAUNCH     (1 << 24)

#define IAP_IPOD_MICROPHONE_LINGO_OPTION_BIT_ANALOG_RECORDING_CAPABLE      (1 <<  3)
#define IAP_IPOD_MICROPHONE_LINGO_OPTION_BIT_USB_DIGITAL_RECORDING_CAPABLE (1 <<  4)

#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_CONTEXT_SPECIFIC_CONTROLS (1 <<  0)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_AUDIO_MEDIA_CONTROLS      (1 <<  1)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_VIDEO_MEDIA_CONTROLS      (1 <<  2)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_IMAGE_MEDIA_CONTROLS      (1 <<  3)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_SPORTS_MEDIA_CONTROLS     (1 <<  4)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_CAMERA_MEDIA_CONTROLS     (1 <<  8)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_USB_HID_COMMANDS          (1 <<  9)
#define IAP_IPOD_SIMPLE_REMOTE_LINGO_OPTION_BIT_ACCESSIBILITY_CONTROLS    (1 << 10)

#define IAP_IPOD_DISPLAY_REMOTE_LINGO_OPTION_BIT_UI_VOLUME_CONTROL       (1 <<  0)
#define IAP_IPOD_DISPLAY_REMOTE_LINGO_OPTION_BIT_ABSOLUTE_VOLUME_CONTROL (1 <<  1)

#define IAP_IPOD_EXTENDED_INTERFACE_LINGO_OPTION_BIT_VIDEO_BROWSING      (1 <<  0)
#define IAP_IPOD_EXTENDED_INTERFACE_LINGO_OPTION_BIT_EXTENDED_INTERFACE_ENHANCMENTS (1 <<  1)
#define IAP_IPOD_EXTENDED_INTERFACE_LINGO_OPTION_BIT_NESTED_PLAYLIST     (1 <<  2)
#define IAP_IPOD_EXTENDED_INTERFACE_LINGO_OPTION_BIT_DISPLAY_IMAGES      (1 <<  4)

#define IAP_IPOD_USB_HOST_MODE_LINGO_OPTION_BIT_MODE_INVOKED_BY_HARDWARE (1 <<  0)
#define IAP_IPOD_USB_HOST_MODE_LINGO_OPTION_BIT_MODE_INVOKED_BY_FIRMWARE (1 <<  1)

#define IAP_IPOD_RF_TUNER_LINGO_OPTION_BIT_RDS_RAW_MODE_SUPPORT        (1 <<  0)
#define IAP_IPOD_RF_TUNER_LINGO_OPTION_BIT_HD_RADIO_TUNING_SUPPORT     (1 <<  1)
#define IAP_IPOD_RF_TUNER_LINGO_OPTION_BIT_AM_RADIO_TUNING_SUPPORT     (1 <<  2)

#define IAP_IPOD_SPORTS_LINGO_OPTION_BIT_NIKE_IPOD_CARDIO_EQUIPMENT    (1 <<  1)

#define IAP_IPOD_DIGITAL_AUDIO_LINGO_OPTION_BIT_AV_SYNCHRONIZATION     (1 <<  0)

#define IAP_IPOD_STORAGE_LINGO_OPTION_BIT_ITUNE_TAGGING                (1 <<  0)
#define IAP_IPOD_STORAGE_LINGO_OPTION_BIT_NIKE_IPOD_CARDIO_EQUIPMENT   (1 <<  1)

#define IAP_IPOD_IPOD_OUT_LINGO_OPTION_BIT_IPOD_OUT_AVAILABLE          (1 <<  0)

#define IAP_IPOD_LOCATION_LINGO_MAJOR_VERSION                           1
#define IAP_IPOD_LOCATION_LINGO_MINOR_VERSION                           0

#define IAP_IPOD_LOCATION_LINGO_DEVICE_CAPABILITIES_TYPE_SYSTEM        (0)
#define IAP_IPOD_LOCATION_LINGO_DEVICE_CAPABILITIES_TYPE_NMEA          (1)
#define IAP_IPOD_LOCATION_LINGO_DEVICE_CAPABILITIES_TYPE_ASSISTANCE    (2)

#define IAP_IPOD_LOCATION_LINGO_SYSTEM_CAPABILITY_POWER_MANAGEMENT     (1 << 0)
#define IAP_IPOD_LOCATION_LINGO_SYSTEM_CAPABILITY_ASYNC_LOCATION_NOTIFICATION (1 << 2)

#define IAP_IPOD_LOCATION_LINGO_LOCATION_CAPABILITY_SYSTEM_SUPPORT     (1 << 0)
#define IAP_IPOD_LOCATION_LINGO_LOCATION_CAPABILITY_NEMA_GPS_SUPPORT   (1 << 1)
#define IAP_IPOD_LOCATION_LINGO_LOCATION_CAPABILITY_LOCATION_ASSISTANCE_SUPPORT (1 << 2)

#define IAP_IPOD_LOCATION_LINGO_NEMA_GPS_SENTANCE_FILTERING_SUPPORT    (1 << 0)

#define IAP_IPOD_LOCATION_LINGO_OPTION_BIT_IPOD_ACCEPTS_NMEA_GPS_DATA  (1 <<  0)
#define IAP_IPOD_LOCATION_LINGO_OPTION_BIT_IPOD_CAN_SEND_LOCATION_ASSISTANCE_DATA (1 <<  1)

#define IAP_IPOD_LOCATION_LINGO_DEVICE_CONTROL_TYPE_SYSTEM                   (0)
#define IAP_IPOD_LOCATION_LINGO_DEVICE_CONTROL_TYPE_NMEA                     (1)

#define IAP_IPOD_LOCATION_LINGO_SYSTEM_CONTROL_GPS_POWER_OFF                0x00
#define IAP_IPOD_LOCATION_LINGO_SYSTEM_CONTROL_GPS_POWER_ON                 0x03
#define IAP_IPOD_LOCATION_LINGO_SYSTEM_CONTROL_ASYNC_NOTIFICATION_ENABLED   0x04
#define IAP_IPOD_LOCATION_LINGO_NEMA_CONTROL_FILTERING_ENABLED         (1 <<  0)

#define IAP_EVENT_NOTIFICATION_FLOW_CONTROL                            (1 <<  2)
#define IAP_EVENT_NOTIFICATION_RADIO_TAGGING_STATUS                    (1 <<  3)
#define IAP_EVENT_NOTIFICATION_CAMERA_STATUS                           (1 <<  4)
#define IAP_EVENT_NOTIFICATION_NOW_PLAYING_FOCUS_APP_STATUS            (1 << 10)
#define IAP_EVENT_NOTIFICATION_SESSION_SPACE_AVAILABLE                 (1 << 11)

#define IAP_IPOD_RADIO_TAGGING_NOTIFICATION_TAGGING_OPERATION_SUCCESSFUL          0x00
#define IAP_IPOD_RADIO_TAGGING_NOTIFICATION_TAGGING_OPERATION_FAILED              0x01
#define IAP_IPOD_RADIO_TAGGING_NOTIFICATION_INFORMATION_AVAILABLE_FOR_TAGGING     0x02
#define IAP_IPOD_RADIO_TAGGING_NOTIFICATION_INFORMATION_NOT_AVAILABLE_FOR_TAGGING 0x03

#define IAP_IPOD_CAMERA_STATUS_NOTIFICATION_CAMERA_APP_OFF                  0x00
#define IAP_IPOD_CAMERA_STATUS_NOTIFICATION_PREVIEW                         0x03
#define IAP_IPOD_CAMERA_STATUS_NOTIFICATION_RECORDING                       0x04

   /* Specific Location Lingo packet definitions.                       */
   /* * NOTE * These definitions do not include ANY packet headers.     */
   /*          This means that the definitions will ONLY include the    */
   /*          variable data contained in the Command Payload.  The     */
   /*          checksum will be included immediately after the payload  */
   /*          data.                                                    */

   /* Location Device Acknowledgement definition.                       */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Device_ACK_Data_t
{
   NonAlignedByte_t Status;
   NonAlignedByte_t CommmandID;
} __PACKED_STRUCT_END__ IAP_Location_Device_ACK_Data_t;

#define IAP_LOCATION_DEVICE_ACK_DATA_SIZE                         (sizeof(IAP_Location_Device_ACK_Data_t))

   /* Location system capabilities definition.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_System_Capabilities_t
{
   NonAlignedByte_t  DevMajorVersion;
   NonAlignedByte_t  DevMinorVersion;
   NonAlignedQWord_t SystemCapsMask;
   NonAlignedQWord_t LocCapsMask;
} __PACKED_STRUCT_END__ IAP_Location_System_Capabilities_t;

#define IAP_LOCATION_SYSTEM_CAPABILITIES_SIZE                     (sizeof(IAP_Location_System_Capabilities_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_NEMA_Capabilities_t
{
   NonAlignedQWord_t NemaGPSCaps;
} __PACKED_STRUCT_END__ IAP_Location_NEMA_Capabilities_t;

#define IAP_LOCATION_NEMS_CAPABILITIES_SIZE                       (sizeof(IAP_Location_NEMA_Capabilities_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Assistance_Capabilities_t
{
   NonAlignedQWord_t LocAsstData;
} __PACKED_STRUCT_END__ IAP_Location_Assistance_Capabilities_t;

#define IAP_LOCATION_ASSISTANCE_CAPABILITIES_SIZE                 (sizeof(IAP_Location_Assistance_Capabilities_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Capabilities_t
{
   Byte_t                                    LocationType;
   __PACKED_STRUCT_BEGIN__ union
   {
      IAP_Location_System_Capabilities_t     SystemCapabilities;
      IAP_Location_NEMA_Capabilities_t       NEMACapabilities;
      IAP_Location_Assistance_Capabilities_t AssistanceCapabilities;
   } __PACKED_STRUCT_END__ Capabilities;
} __PACKED_STRUCT_END__ IAP_Location_Capabilities_t;

#define IAP_LOCATION_CAPABILITIES_SIZE                            (sizeof(IAP_Location_Capabilities_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_System_Control_t
{
   NonAlignedQWord_t SystemControlData;
} __PACKED_STRUCT_END__ IAP_Location_System_Control_t;

#define IAP_LOCATION_SYSTEM_CONTROL_SIZE                          (sizeof(IAP_Location_System_Control_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_NEMA_Control_t
{
   NonAlignedQWord_t NemaGPSControlData;
} __PACKED_STRUCT_END__ IAP_Location_NEMA_Control_t;

#define IAP_LOCATION_NEMA_CONTROL_SIZE                            (sizeof(IAP_Location_NEMA_Control_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Control_t
{
   Byte_t                           LocationType;
   __PACKED_STRUCT_BEGIN__ union
   {
      IAP_Location_System_Control_t SystemControl;
      IAP_Location_NEMA_Control_t   NEMAControl;
   } __PACKED_STRUCT_END__ ControlType;
} __PACKED_STRUCT_END__ IAP_Location_Control_t;

#define IAP_LOCATION_CONTROL_SIZE                                 (sizeof(IAP_Location_Control_t))

#define IAP_LOCATION_GET_DEVICE_DATA_LOCATION_TYPE_SATELLITE_EPHEMERIS_MAXIMUM_REFRESH_INTERVAL     0x0203
#define IAP_LOCATION_GET_DEVICE_DATA_LOCATION_TYPE_SATELLITE_EPHEMERIS_RECOMMENDED_REFRESH_INTERVAL 0x0204

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Get_Device_Data_t
{
   Word_t LocationType;
} __PACKED_STRUCT_END__ IAP_Location_Get_Device_Data_t;

#define IAP_LOCATION_GET_DEVICE_DATA_DATA_SIZE                    (sizeof(IAP_Location_Get_Device_Data_t))

#define IAP_LOCATION_SET_DEVICE_DATA_LOCATION_TYPE_NEMA                     0x0100
#define IAP_LOCATION_SET_DEVICE_DATA_LOCATION_TYPE_CURRENT_POINT            0x0200
#define IAP_LOCATION_SET_DEVICE_DATA_LOCATION_TYPE_SATELLITE_EPHEMERIS      0x0201
#define IAP_LOCATION_SET_DEVICE_DATA_LOCATION_TYPE_CURRENT_TIME             0x0205

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_NEMA_Filter_Data_t
{
   Byte_t StringLength;
   char   FilterString[1];
} __PACKED_STRUCT_END__ IAP_Location_NEMA_Filter_Data_t;

#define IAP_LOCATION_NEMA_FILTER_DATA_SIZE                     (sizeof(IAP_Location_NEMA_Filter_Data_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Point_Location_Data_t
{
   NonAlignedWord_t  Week;
   NonAlignedDWord_t Time;
   NonAlignedDWord_t Latitude;
   NonAlignedDWord_t Longitude;
   NonAlignedWord_t  Accuracy;
} __PACKED_STRUCT_END__ IAP_Location_Point_Location_Data_t;

#define IAP_LOCATION_POINT_LOCATION_DATA_SIZE                  (sizeof(IAP_Location_Point_Location_Data_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Current_GPS_Time_t
{
   NonAlignedWord_t  Week;
   NonAlignedDWord_t Time;
} __PACKED_STRUCT_END__ IAP_Location_Current_GPS_Time_t;

#define IAP_LOCATION_CURRENT_GPS_TIME_SIZE                     (sizeof(IAP_Location_Current_GPS_Time_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagIAP_Location_Set_Device_Data_t
{
   Word_t                                LocationType;
   NonAlignedWord_t                      CurrentSection;
   NonAlignedWord_t                      MaxSection;
   NonAlignedQWord_t                     TotalSize;
   __PACKED_STRUCT_BEGIN__ union
   {
      IAP_Location_NEMA_Filter_Data_t    SentanceFilter;
      IAP_Location_Point_Location_Data_t PointLocationData;
      Byte_t                             EphemerisDataStart[1];
      IAP_Location_Current_GPS_Time_t    CurrentTime;
   } __PACKED_STRUCT_END__ Device_Data;
} __PACKED_STRUCT_END__ IAP_Location_Set_Device_Data_t;

#define IAP_LOCATION_SET_DEVICE_DATA_SIZE                      (sizeof(IAP_Location_Set_Device_Data_t))

#endif
