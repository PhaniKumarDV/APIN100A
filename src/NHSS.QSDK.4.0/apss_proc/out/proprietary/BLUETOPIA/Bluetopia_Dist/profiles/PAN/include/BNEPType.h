/*****< bneptype.h >***********************************************************/
/*      Copyright 2004 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BNEPTYPE - Bluetooth Network Encapsulation Protocol (BNEP) Type           */
/*             Definitions/Constants.                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/25/04  R. Sledge      Initial creation.                               */
/*   08/01/09  D. Lange       Moved into seperate file.                       */
/******************************************************************************/
#ifndef __BNEPTYPEH__
#define __BNEPTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* SDP Bluetooth Network Encapsulation Protocol (BNEP) UUID's.       */

   /* The following MACRO is a utility MACRO that assigns the Bluetooth */
   /* Network Encapsulation Protocol (BNEP) Bluetooth Universally Unique*/
   /* Identifier (SDP_UUID_16) to the specified UUID_16_t variable.     */
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the BNEP_PROTOCOL_UUID_16 Constant value.      */
#define SDP_ASSIGN_BNEP_PROTOCOL_UUID_16(_x)                        ASSIGN_SDP_UUID_16((_x), 0x00, 0x0F)

   /* The following MACRO is a utility MACRO that assigns the Bluetooth */
   /* Network Encapsulation Protocol (BNEP) Bluetooth Universally Unique*/
   /* Identifier (SDP_UUID_32) to the specified UUID_32_t variable.     */
   /* This MACRO accepts one parameter which is the UUID_32_t variable  */
   /* that is to receive the BNEP_PROTOCOL_UUID_32 Constant value.      */
#define SDP_ASSIGN_BNEP_PROTOCOL_UUID_32(_x)                        ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x00, 0x0F)

   /* The following MACRO is a utility MACRO that assigns the Bluetooth */
   /* Network Encapsulation Protocol (BNEP) Bluetooth Universally Unique*/
   /* Identifier (SDP_UUID_128) to the specified UUID_128_t variable.   */
   /* This MACRO accepts one parameter which is the UUID_128_t variable */
   /* that is to receive the BNEP_PROTOCOL_UUID_128 Constant value.     */
#define SDP_ASSIGN_BNEP_PROTOCOL_UUID_128(_x)                       ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following constant defines the BNEP Protocol Version Number   */
   /* that this file supports (for SDP record purposes).                */
#define BNEP_PROTOCOL_VERSION                                       (0x0100)

   /* The following value represents the minimum supported L2CAP MTU    */
   /* that a BNEP implementation *MUST* support.  The L2CAP MTU can be  */
   /* larger, but it cannot be smaller than this number.                */
#define BNEP_MINIMUM_L2CAP_MTU                                      (1691)

   /* The following type definition represents the container class for a*/
   /* BNEP Ethernet Address.                                            */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Ethernet_Address_t
{
   unsigned char Address0;
   unsigned char Address1;
   unsigned char Address2;
   unsigned char Address3;
   unsigned char Address4;
   unsigned char Address5;
} __PACKED_STRUCT_END__ BNEP_Ethernet_Address_t;

#define BNEP_ETHERNET_ADDRESS_SIZE                      (sizeof(BNEP_Ethernet_Address_t))

   /* The following MACRO is a utility MACRO that exists to assign the  */
   /* individual Byte values into the specified BNEP_Ethernet_Address   */
   /* variable.  The Bytes are NOT in Little Endian Format and are      */
   /* assigned to the BNEP_Ethernet_Address Variable in BIG Endian      */
   /* Format.  The first parameter is the BNEP_Ethernet_Address         */
   /* Variable (of type BNEP_Ethernet_Address_t) to assign, and the     */
   /* next six parameters are the Individual Ethernet Address Byte      */
   /* values to assign to the variable.                                 */
#define BNEP_ASSIGN_ETHERNET_ADDRESS(_dest, _a, _b, _c, _d, _e, _f)  \
{                                                                    \
   (_dest).Address0 = (_a);                                          \
   (_dest).Address1 = (_b);                                          \
   (_dest).Address2 = (_c);                                          \
   (_dest).Address3 = (_d);                                          \
   (_dest).Address4 = (_e);                                          \
   (_dest).Address5 = (_f);                                          \
}

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* Comparison of two BNEP_Ethernet_Address_t variables.  This MACRO  */
   /* only returns whether the two BNEP_Ethernet_Address_t variables    */
   /* are equal (MACRO returns Boolean_t result) NOT less than/greater  */
   /* than.  The two parameters to this MACRO are both of type          */
   /* BNEP_Ethernet_Address_t and represent the                         */
   /* BNEP_Ethernet_Address_t variables to compare.                     */
#define BNEP_COMPARE_ADDRESSES(_x, _y)                                                                          \
(                                                                                                               \
   ((_x).Address0 == (_y).Address0) && ((_x).Address1 == (_y).Address1) && ((_x).Address2  == (_y).Address2) && \
   ((_x).Address3 == (_y).Address3) && ((_x).Address4 == (_y).Address4) && ((_x).Address5  == (_y).Address5)    \
)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Packet Type Values.                        */
#define BNEP_GENERAL_ETHERNET_PACKET_TYPE                           (0x00)
#define BNEP_CONTROL_PACKET_TYPE                                    (0x01)
#define BNEP_COMPRESSED_ETHERNET_PACKET_TYPE                        (0x02)
#define BNEP_COMPRESSED_ETHERNET_SOURCE_ONLY_PACKET_TYPE            (0x03)
#define BNEP_COMPRESSED_ETHERNET_DESTINATION_ONLY_PACKET_TYPE       (0x04)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol BNEP Packet Header Mask which may be used  */
   /* to obtain the BNEP Packet Type Value and BNEP Extension Flag Value*/
   /* from the first byte of the BNEP Packet Header.                    */
#define BNEP_PACKET_TYPE_MASK                                       (0x7F)
#define BNEP_EXTENSION_FLAG_MASK                                    (0x80)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Control Packet Type Control Type Values.   */
#define BNEP_CONTROL_COMMAND_NOT_UNDERSTOOD_CONTROL_TYPE            (0x00)
#define BNEP_SETUP_CONNECTION_REQUEST_MSG_CONTROL_TYPE              (0x01)
#define BNEP_SETUP_CONNECTION_RESPONSE_MSG_CONTROL_TYPE             (0x02)
#define BNEP_FILTER_NET_TYPE_SET_MSG_CONTROL_TYPE                   (0x03)
#define BNEP_FILTER_NET_TYPE_RESPONSE_MSG_CONTROL_TYPE              (0x04)
#define BNEP_FILTER_MULTICAST_ADDRESS_SET_MSG_CONTROL_TYPE          (0x05)
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MSG_CONTROL_TYPE     (0x06)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Connection Response Message Values.        */
#define BNEP_CONNECTION_RESPONSE_MESSAGE_OPERATION_SUCCESSFUL       (0x0000)
#define BNEP_CONNECTION_RESPONSE_MESSAGE_OPERATION_FAILED_INVALID_DESTINATION_SERVICE_UUID (0x0001)
#define BNEP_CONNECTION_RESPONSE_MESSAGE_OPERATION_FAILED_INVALID_SOURCE_SERVICE_UUID (0x0002)
#define BNEP_CONNECTION_RESPONSE_MESSAGE_OPERATION_FAILED_INVALID_SERVICE_UUID_SIZE (0x0003)
#define BNEP_CONNECTION_RESPONSE_MESSAGE_OPERATION_FAILED_CONNECTION_NOT_ALLOWED (0x0004)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Filter Net Type Response Message Values.   */
#define BNEP_FILTER_NET_TYPE_RESPONSE_MESSAGE_OPERATION_SUCCESSFUL  (0x0000)
#define BNEP_FILTER_NET_TYPE_RESPONSE_MESSAGE_UNSUPPORTED_REQUEST   (0x0001)
#define BNEP_FILTER_NET_TYPE_RESPONSE_MESSAGE_OPERATION_FAILED_INVALID_NETWORK_PROTOCOL_TYPE_RANGE (0x0002)
#define BNEP_FILTER_NET_TYPE_RESPONSE_MESSAGE_OPERATION_FAILED_TOO_MANY_FILTERS (0x0002)
#define BNEP_FILTER_NET_TYPE_RESPONSE_MESSAGE_OPERATION_FAILED_UNABLE_TO_FULFILL_REQUEST_SECURITY (0x0004)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Filter Multicast Address Response Message  */
   /* Value.                                                            */
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MESSAGE_OPERATION_SUCCESSFUL (0X0000)
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MESSAGE_UNSUPPORTED_REQUEST (0x00001)
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MESSAGE_OPERATION_FAILED_INVALID_MULTICAST_ADDRESS (0x0002)
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MESSAGE_OPERATION_FAILED_TOO_MANY_FILTERS (0x0003)
#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_MESSAGE_OPERATION_FAILED_UNABLE_TO_FULFILL_REQUEST_SECURITY (0x0004)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol Extension Type Values.                     */
#define BNEP_EXTENSION_CONTROL_PACKET_TYPE                          (0x00)

   /* The following constants represent the Bluetooth Network           */
   /* Encapsulation Protocol BNEP Extension Packet Header Mask which may*/
   /* be used to obtain the BNEP Extension Packet Type Value from the   */
   /* first byte of the BNEP Extension Packet Header.                   */
#define BNEP_EXTENSION_PACKET_TYPE_MASK                             (0x7F)

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Packet Header.  This header exists  */
   /* at the start of all BNEP Packets.                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Packet_Header_t
{
   Byte_t BNEP_Type_And_Extension_Flag;
} __PACKED_STRUCT_END__ BNEP_Packet_Header_t;

#define BNEP_PACKET_HEADER_SIZE                         (sizeof(BNEP_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Control Packet Header.  This header */
   /* exists at the start of all BNEP Packets with BNEP_CONTROL type.   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Control_Packet_Header_t
{
   BNEP_Packet_Header_t BNEP_Packet_Header;
   Byte_t               BNEP_Control_Type;
} __PACKED_STRUCT_END__ BNEP_Control_Packet_Header_t;

#define BNEP_CONTROL_PACKET_HEADER_SIZE                 (sizeof(BNEP_Control_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Control Command Not Understood      */
   /* Packet.                                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Control_Command_Not_Understood_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   Byte_t                       Unknown_Control_Type;
} __PACKED_STRUCT_END__ BNEP_Control_Command_Not_Understood_t;

#define BNEP_CONTROL_COMMAND_NOT_UNDERSTOOD_SIZE        (sizeof(BNEP_Control_Command_Not_Understood_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Setup Connection Request Header.    */
   /* This header exists at the start of all BNEP Setup Connection      */
   /* Request Packets.                                                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Setup_Connection_Request_Header_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   Byte_t                       UUID_Size;
} __PACKED_STRUCT_END__ BNEP_Setup_Connection_Request_Header_t;

#define BNEP_SETUP_CONNECTION_REQUEST_HEADER_SIZE       (sizeof(BNEP_Setup_Connection_Request_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Setup Connection Request Packet with*/
   /* 16-bit UUIDs.                                                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Setup_Connection_Request_UUID_16_t
{
   BNEP_Setup_Connection_Request_Header_t BNEP_Setup_Connection_Request_Header;
   UUID_16_t                              Destination_Service_UUID;
   UUID_16_t                              Source_Service_UUID;
} __PACKED_STRUCT_END__ BNEP_Setup_Connection_Request_UUID_16_t;

#define BNEP_SETUP_CONNECTION_REQUEST_UUID_16_SIZE      (sizeof(BNEP_Setup_Connection_Request_UUID_16_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Setup Connection Request Packet with*/
   /* 32-bit UUIDs.                                                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Setup_Connection_Request_UUID_32_t
{
   BNEP_Setup_Connection_Request_Header_t BNEP_Setup_Connection_Request_Header;
   UUID_32_t                              Destination_Service_UUID;
   UUID_32_t                              Source_Service_UUID;
} __PACKED_STRUCT_END__ BNEP_Setup_Connection_Request_UUID_32_t;

#define BNEP_SETUP_CONNECTION_REQUEST_UUID_32_SIZE      (sizeof(BNEP_Setup_Connection_Request_UUID_32_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Setup Connection Request Packet with*/
   /* 128-bit UUIDs.                                                    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Setup_Connection_Request_UUID_128_t
{
   BNEP_Setup_Connection_Request_Header_t BNEP_Setup_Connection_Request_Header;
   UUID_128_t                             Destination_Service_UUID;
   UUID_128_t                             Source_Service_UUID;
} __PACKED_STRUCT_END__ BNEP_Setup_Connection_Request_UUID_128_t;

#define BNEP_SETUP_CONNECTION_REQUEST_UUID_128_SIZE     (sizeof(BNEP_Setup_Connection_Request_UUID_128_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Setup Connection Response Packet.   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Setup_Connection_Response_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   NonAlignedWord_t             Response_Message;
} __PACKED_STRUCT_END__ BNEP_Setup_Connection_Response_t;

#define BNEP_SETUP_CONNECTION_RESPONSE_SIZE             (sizeof(BNEP_Setup_Connection_Response_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Filter Net Type Set Header.  This   */
   /* header exists at the start of all BNEP Filter Net Type Set        */
   /* Packets.                                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Filter_Net_Type_Set_Header_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   NonAlignedWord_t             List_Length;
} __PACKED_STRUCT_END__ BNEP_Filter_Net_Type_Set_Header_t;

#define BNEP_FILTER_NET_TYPE_SET_HEADER_SIZE            (sizeof(BNEP_Filter_Net_Type_Set_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Filter Net Type Response Packet.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Filter_Net_Type_Response_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   NonAlignedWord_t             Response_Message;
} __PACKED_STRUCT_END__ BNEP_Filter_Net_Type_Response_t;

#define BNEP_FILTER_NET_TYPE_RESPONSE_SIZE              (sizeof(BNEP_Filter_Net_Type_Response_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Filter Multicast Address Set Header.*/
   /* This header exists at the start of all BNEP Filter Multicast      */
   /* Address Set Packets.                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Filter_Multicast_Address_Set_Header_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   NonAlignedWord_t             List_Length;
} __PACKED_STRUCT_END__ BNEP_Filter_Multicast_Address_Set_Header_t;

#define BNEP_FILTER_MULTICAST_ADDRESS_SET_HEADER_SIZE   (sizeof(BNEP_Filter_Multicast_Address_Set_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Filter Multicast Address Response   */
   /* Packet.                                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Filter_Multicast_Address_Response_t
{
   BNEP_Control_Packet_Header_t BNEP_Control_Packet_Header;
   NonAlignedWord_t             Response_Message;
} __PACKED_STRUCT_END__ BNEP_Filter_Multicast_Address_Response_t;

#define BNEP_FILTER_MULTICAST_ADDRESS_RESPONSE_SIZE     (sizeof(BNEP_Filter_Multicast_Address_Response_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) General Ethernet Packet Header.     */
   /* This header exists at the start of all BNEP General Ethernet      */
   /* Packets.                                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_General_Ethernet_Packet_Header_t
{
   BNEP_Packet_Header_t    BNEP_Packet_Header;
   BNEP_Ethernet_Address_t Destination_Address;
   BNEP_Ethernet_Address_t Source_Address;
   NonAlignedWord_t        Networking_Protocol_Type;
} __PACKED_STRUCT_END__ BNEP_General_Ethernet_Packet_Header_t;

#define BNEP_GENERAL_ETHERNET_PACKET_HEADER_SIZE        (sizeof(BNEP_General_Ethernet_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) General Ethernet Packet.            */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_General_Ethernet_Packet_t
{
   BNEP_General_Ethernet_Packet_Header_t BNEP_General_Ethernet_Packet_Header;
   Byte_t                                Payload[1];
} __PACKED_STRUCT_END__ BNEP_General_Ethernet_Packet_t;

#define BNEP_CALCULATE_GENERAL_ETHERNET_PACKET_SIZE(_x) (sizeof(BNEP_General_Ethernet_Packet_t)-sizeof(Byte_t)+(unsigned int)(_x))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Packet Header.  */
   /* This header exists at the start of all BNEP Compressed Ethernet   */
   /* Packets.                                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Packet_Header_t
{
   BNEP_Packet_Header_t BNEP_Packet_Header;
   NonAlignedWord_t     Networking_Protocol_Type;
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Packet_Header_t;

#define BNEP_COMPRESSED_ETHERNET_PACKET_HEADER_SIZE     (sizeof(BNEP_Compressed_Ethernet_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Packet.         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Packet_t
{
   BNEP_Compressed_Ethernet_Packet_Header_t BNEP_Compressed_Ethernet_Packet_Header;
   Byte_t                                   Payload[1];
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Packet_t;

#define BNEP_CALCULATE_COMPRESSED_ETHERNET_PACKET_SIZE(_x) (sizeof(BNEP_Compressed_Ethernet_Packet_t)-sizeof(Byte_t)+(unsigned int)(_x))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Source Only     */
   /* Packet Header.  This header exists at the start of all BNEP       */
   /* Compressed Ethernet Source Only Packets.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Source_Only_Packet_Header_t
{
   BNEP_Packet_Header_t    BNEP_Packet_Header;
   BNEP_Ethernet_Address_t Source_Address;
   NonAlignedWord_t        Networking_Protocol_Type;
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Source_Only_Packet_Header_t;

#define BNEP_COMPRESSED_ETHERNET_SOURCE_ONLY_PACKET_HEADER_SIZE (sizeof(BNEP_Compressed_Ethernet_Source_Only_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Source Only     */
   /* Packet.                                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Source_Only_Packet_t
{
   BNEP_Compressed_Ethernet_Source_Only_Packet_Header_t BNEP_Compressed_Ethernet_Source_Only_Packet_Header;
   Byte_t                                               Payload[1];
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Source_Only_Packet_t;

#define BNEP_CALCULATE_COMPRESSED_ETHERNET_SOURCE_ONLY_PACKET_SIZE(_x) (sizeof(BNEP_Compressed_Ethernet_Source_Only_Packet_t)-sizeof(Byte_t)+(unsigned int)(_x))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Destination Only*/
   /* Packet Header.  The header exists at the start of all BNEP        */
   /* Compressed Ethernet Destination Only Packets.                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Destination_Only_Packet_Header_t
{
   BNEP_Packet_Header_t    BNEP_Packet_Header;
   BNEP_Ethernet_Address_t Destination_Address;
   NonAlignedWord_t        Networking_Protocol_Type;
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Destination_Only_Packet_Header_t;

#define BNEP_COMPRESSED_ETHERNET_DESTINATION_ONLY_PACKET_HEADER_SIZE (sizeof(BNEP_Compressed_Ethernet_Destination_Only_Packet_Header_t))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Compressed Ethernet Destination Only*/
   /* Packet.                                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Compressed_Ethernet_Destination_Only_Packet_t
{
   BNEP_Compressed_Ethernet_Destination_Only_Packet_Header_t BNEP_Compressed_Ethernet_Destination_Only_Packet_Header;
   Byte_t                                                    Payload[1];
} __PACKED_STRUCT_END__ BNEP_Compressed_Ethernet_Destination_Only_Packet_t;

#define BNEP_CALCULATE_COMPRESSED_ETHERNET_DESTINATION_ONLY_PACKET_SIZE(_x) (sizeof(BNEP_Compressed_Ethernet_Destination_Only_Packet_t)-sizeof(Byte_t)+(unsigned int)(_x))

   /* The following structure represents the Bluetooth Network          */
   /* Encapsulation Protocol (BNEP) Extension Header.  A chain of       */
   /* extension headers may follow the initial BNEP Packet Type Header  */
   /* before the actual packet payload.                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBNEP_Extension_Header_t
{
   Byte_t Extension_Type_And_Extension_Flag;
   Byte_t Extension_Length;
} __PACKED_STRUCT_END__ BNEP_Extension_Header_t;

#define BNEP_EXTENSION_HEADER_SIZE                      (sizeof(BNEP_Extension_Header_t))

#endif
