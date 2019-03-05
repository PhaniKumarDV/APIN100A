/*****< anttypes.h >***********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANTTYPES - ANT+ Transport Type Definitions/Constants.                     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/17/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANTTYPESH__
#define __ANTTYPESH__

#include "BTAPITyp.h"
#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following type declaration represents the structure of a      */
   /* single ANT Network Key.                                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Network_Key_t
{
   Byte_t ANT_Network_Key0;
   Byte_t ANT_Network_Key1;
   Byte_t ANT_Network_Key2;
   Byte_t ANT_Network_Key3;
   Byte_t ANT_Network_Key4;
   Byte_t ANT_Network_Key5;
   Byte_t ANT_Network_Key6;
   Byte_t ANT_Network_Key7;
} __PACKED_STRUCT_END__ ANT_Network_Key_t;

#define ANT_NETWORK_KEY_SIZE                                (sizeof(ANT_Network_Key_t))

   /* The following MACRO is a utility MACRO that exists to assign the  */
   /* individual byte values into the specified ANT Network Key         */
   /* variable.  The bytes are NOT in little endian format, however,    */
   /* they are assigned to the ANT Network Key variable in little endian*/
   /* format.  The first parameter is the ANT Network Key variable (of  */
   /* type ANT_Network_Key_t) to assign, and the next 16 parameters are */
   /* the individual ANT Network Key byte values to assign to the ANT   */
   /* Network Key variable.                                             */
#define ASSIGN_NETWORK_KEY(_dest, _a, _b, _c, _d, _e, _f, _g, _h)                                        \
{                                                                                                        \
   (_dest).ANT_Network_Key0  = (_h); (_dest).ANT_Network_Key1  = (_g); (_dest).ANT_Network_Key2  = (_f); \
   (_dest).ANT_Network_Key3  = (_e); (_dest).ANT_Network_Key4  = (_d); (_dest).ANT_Network_Key5  = (_c); \
   (_dest).ANT_Network_Key6  = (_b); (_dest).ANT_Network_Key7  = (_a);                                   \
}

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* comparison of two ANT_Network_Key_t variables.  This MACRO only   */
   /* returns whether the two ANT_Network_Key_t variables are equal     */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* two parameters to this MACRO are both of type ANT_Network_Key_t   */
   /* and represent the ANT_Network_Key_t variables to compare.         */
#define COMPARE_NETWORK_KEY(_x, _y)                                                                                                                                  \
(                                                                                                                                                                    \
   ((_x).ANT_Network_Key0  == (_y).ANT_Network_Key0)  && ((_x).ANT_Network_Key1  == (_y).ANT_Network_Key1)  && ((_x).ANT_Network_Key2  == (_y).ANT_Network_Key2)  && \
   ((_x).ANT_Network_Key3  == (_y).ANT_Network_Key3)  && ((_x).ANT_Network_Key4  == (_y).ANT_Network_Key4)  && ((_x).ANT_Network_Key5  == (_y).ANT_Network_Key5)  && \
   ((_x).ANT_Network_Key6  == (_y).ANT_Network_Key6)  && ((_x).ANT_Network_Key7  == (_y).ANT_Network_Key7)                                                           \
)

   /* The following MACRO is a utility MACRO that exists to aid in the  */
   /* comparison of a ANT_Network_Key_t variables to the NULL ANT       */
   /* Network Key.  This MACRO only returns whether the the             */
   /* ANT_Network_Key_t variable is equal to the NULL ANT Network Key   */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* parameter to this MACRO is the ANT_Network_Key_t structure to     */
   /* compare to the NULL ANT Network Key.                              */
#define COMPARE_NULL_NETWORK_KEY(_x)                                                                                                           \
(                                                                                                                                              \
   ((_x).ANT_Network_Key0 == 0x00) && ((_x).ANT_Network_Key1 == 0x00) && ((_x).ANT_Network_Key2 == 0x00) && ((_x).ANT_Network_Key3 == 0x00) && \
   ((_x).ANT_Network_Key4 == 0x00) && ((_x).ANT_Network_Key5 == 0x00) && ((_x).ANT_Network_Key6 == 0x00) && ((_x).ANT_Network_Key7 == 0x00)    \
)

   /* The following type declaration represents the structure of the    */
   /* Header of an ANT message packet.  This header information is      */
   /* contained in every defined ANT message packet.  This structure    */
   /* forms the basis of additional defined ANT message packets.  Since */
   /* this structure is present at the begining of every defined ANT    */
   /* message packet, this structure will be the first element of every */
   /* defined ANT message packet in this file.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Message_Header_t
{
   NonAlignedByte_t Message_Sync;
   NonAlignedByte_t Message_Length;
   NonAlignedByte_t Message_ID;
} __PACKED_STRUCT_END__ ANT_Message_Header_t;

#define ANT_MESSAGE_HEADER_SIZE                             (sizeof(ANT_Message_Header_t))

   /* The following constant represents the maximum size (in bytes) that*/
   /* can ever be contained in an ANT Extension message (this value     */
   /* represents all Extension Flags enabled).                          */
#define ANT_MESSAGE_EXTENSION_MAXIMUM_SIZE                  10

   /* The following constant represents the maximum allowable data      */
   /* payload (in bytes) that can exist in a single ANT Message.        */
#define ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE                     8

   /* The following constant represents the maximum allowable buffer    */
   /* space required to hold the worst case ANT Message.  The worst     */
   /* case (i.e. largest) ANT Message will be made up of the following: */
   /*    - ANT Header                                                   */
   /*    - ANT Data Payload (of length ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE)*/
   /*    - ANT Message Extension (largest possible size)                */
   /*    - Checksum                                                     */
#define ANT_MESSAGE_MAXIMUM_SIZE                            (ANT_MESSAGE_HEADER_SIZE + (sizeof(NonAlignedByte_t)) + ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE + ANT_MESSAGE_EXTENSION_MAXIMUM_SIZE)

   /* The following structure represents the data structure that holds  */
   /* the information for a raw ANT packet that is to be transmitted or */
   /* that has been received.  The only detail to note is that the      */
   /* packet data field is an array of unsigned char's, and NOT a       */
   /* pointer to an array of unsigned char's.  This is very important,  */
   /* because this mechanism will allow an arbitrary memory buffer to be*/
   /* typecast to this structure and all elements will be accessible in */
   /* the same memory block (i.e.  NO other pointer operation is        */
   /* required).  The side effect of this is that when the memory for a */
   /* raw ANT packet is to be allocated, the allocated size required    */
   /* will be (sizeof(ANT_Packet_t)-1) + length of the packet data.     */
   /* After this is completed, the data elements in the packet data     */
   /* array can be accessed by simple array logic, aiding code          */
   /* readability.  It might appear confusing to the user because array */
   /* elements greater than zero will be indexed, however, as long as   */
   /* the programmer is aware of this design decision, the code should  */
   /* be much more simple to read.  MACRO's and definitions will be     */
   /* provided following this structure definition to alleviate the     */
   /* programmer from having to remember the above formula when         */
   /* allocating memory of the correct size.                            */
   /* * NOTE * It should be noted that the ANTPacketData field will     */
   /*          contain both the variable length data field AND the      */
   /*          checksum (1 byte) that is fixed length AND is always     */
   /*          present in any ANT Packet.                               */
typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Message_Data[1];
} __PACKED_STRUCT_END__ ANT_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the number of data bytes that */
   /* will be required to hold a ANT Message Information Header and the */
   /* raw ANT Message Data (of the specified length).  See notes and    */
   /* discussion above for the reason for this MACRO definition.        */
   /* * NOTE * We will add 1 byte to the length because every ANT       */
   /*          packet contains a 1 byte checksum AFTER the variable     */
   /*          length data.                                             */
#define ANT_CALCULATE_MESSAGE_SIZE(_x)                      (ANT_MESSAGE_HEADER_SIZE + sizeof(NonAlignedByte_t) + (unsigned int)(_x))

   /* The following MACRO is a utility MACRO provided to allow a        */
   /* programmer of quickly determining the length that should be       */
   /* specified in the Message_Length field of an ANT Message Header    */
   /* (defined by ANT_Message_Header_t).  The parameter to this MACRO   */
   /* is the size of the entire packet (normally should use one of the  */
   /* message size MACROs of form ANT_XXX_MESSAGE_SIZE that are defined */
   /* below) INCLUDING the message header and the 1 byte checksum that  */
   /* is appended to the end of every ANT message.                      */
#define ANT_CALCULATE_MESSAGE_LENGTH(_x)                    ((unsigned int)(_x) - ANT_MESSAGE_HEADER_SIZE - sizeof(NonAlignedByte_t))

   /* The following MACRO is a utility MACRO provided to allow a        */
   /* programmer of quickly determining the length of a message that    */
   /* should be used to calculate the checksum of a packet buffer.  The */
   /* parameter to this MACRO is the size of the entire packet (normally*/
   /* should use one of the message size MACROs of form                 */
   /* ANT_XXX_MESSAGE_SIZE that are defined below) INCLUDING the message*/
   /* header and the 1 byte checksum that is appended to the end of     */
   /* every ANT message.                                                */
#define ANT_CALCULATE_CHECKSUM_LENGTH(_x)                    ((unsigned int)(_x) - sizeof(NonAlignedByte_t))

   /* ANT Message Declarations/Types/Constants.                         */
   /* * NOTE * Since all ANT Messages from host to ANT and ANT to host  */
   /*          use a common packet structure, meaning they are all      */
   /*          identified by the message ID, we will use the naming     */
   /*          convention ANT_MESSAGE_ID_HOST_XXX for messages that     */
   /*          can only be sent host to ANT and                         */
   /*          ANT_MESSAGE_ID_CONTROLLER_XXX for messages that may only */
   /*          be sent from ANT to host.  For messages that may be sent */
   /*          in either direction we will use                          */
   /*          ANT_MESSAGE_ID_PACKET_XXX for packets that may be sent   */
   /*          in either direction.                                     */

   /* ANT Configuration Message ID Definitions/Constants.               */
#define ANT_MESSAGE_ID_HOST_UNASSIGN_CHANNEL                      0x41
#define ANT_MESSAGE_ID_HOST_ASSIGN_CHANNEL                        0x42
#define ANT_MESSAGE_ID_HOST_SET_CHANNEL_ID                        0x51
#define ANT_MESSAGE_ID_HOST_CHANNEL_MESSAGING_PERIOD              0x43
#define ANT_MESSAGE_ID_HOST_CHANNEL_SEARCH_TIMEOUT                0x44
#define ANT_MESSAGE_ID_HOST_CHANNEL_RF_FREQUENCY                  0x45
#define ANT_MESSAGE_ID_HOST_SET_NETWORK_KEY                       0x46
#define ANT_MESSAGE_ID_HOST_TRANSMIT_POWER                        0x47
#define ANT_MESSAGE_ID_HOST_ADD_CHANNEL_ID                        0x59
#define ANT_MESSAGE_ID_HOST_CONFIG_LIST_ID                        0x5A
#define ANT_MESSAGE_ID_HOST_SET_CHANNEL_TX_POWER                  0x60
#define ANT_MESSAGE_ID_HOST_CHANNEL_LOW_PRIORITY_SEARCH_TIMEOUT   0x63
#define ANT_MESSAGE_ID_HOST_SERIAL_NUMBER_CHANNEL_ID              0x65
#define ANT_MESSAGE_ID_HOST_ENABLE_EXTENDED_MESSAGES              0x66
#define ANT_MESSAGE_ID_HOST_ENABLE_LED                            0x68
#define ANT_MESSAGE_ID_HOST_ENABLE_CRYSTAL                        0x6D
#define ANT_MESSAGE_ID_HOST_LIB_CONFIG                            0x6E
#define ANT_MESSAGE_ID_HOST_FREQUENCY_AGILITY                     0x70
#define ANT_MESSAGE_ID_HOST_PROXIMITY_SEARCH                      0x71
#define ANT_MESSAGE_ID_HOST_CHANNEL_SEARCH_PRIORITY               0x75
#define ANT_MESSAGE_ID_HOST_SET_USB_DESCRIPTOR_STRING             0xC7

   /* ANT Notification Message ID Definitions/Constants.                */
#define ANT_MESSAGE_ID_CONTROLLER_STARTUP_MESSAGE                 0x6F
#define ANT_MESSAGE_ID_CONTROLLER_SERIAL_ERROR_MESSAGE            0xAE

   /* ANT Control Message ID Definitions/Constants.                     */
#define ANT_MESSAGE_ID_HOST_RESET_SYSTEM                          0x4A
#define ANT_MESSAGE_ID_HOST_OPEN_CHANNEL                          0x4B
#define ANT_MESSAGE_ID_HOST_CLOSE_CHANNEL                         0x4C
#define ANT_MESSAGE_ID_HOST_REQUEST_MESSAGE                       0x4D
#define ANT_MESSAGE_ID_HOST_OPEN_RX_SCAN_MODE                     0x5B
#define ANT_MESSAGE_ID_HOST_SLEEP_MESSAGE                         0xC5

   /* ANT Data Message ID Definitions/Constants.                        */
#define ANT_MESSAGE_ID_PACKET_BROADCAST_DATA                      0x4E
#define ANT_MESSAGE_ID_PACKET_ACKNOWLEDGED_DATA                   0x4F
#define ANT_MESSAGE_ID_PACKET_BURST_DATA                          0x50

   /* ANT Channel Response/Event Message ID Definitions/Constants.      */
#define ANT_MESSAGE_ID_CONTROLLER_CHANNEL_RESPONSE                0x40

   /* ANT Requested Response Message ID Definitions/Constants.          */
#define ANT_MESSAGE_ID_CONTROLLER_CHANNEL_STATUS_RESPONSE         0x52
#define ANT_MESSAGE_ID_CONTROLLER_CHANNEL_ID_RESPONSE             0x51
#define ANT_MESSAGE_ID_CONTROLLER_ANT_VERSION_RESPONSE            0x3E
#define ANT_MESSAGE_ID_CONTROLLER_CAPABILITIES_RESPONSE           0x54
#define ANT_MESSAGE_ID_CONTROLLER_DEVICE_SERIAL_NUMBER_RESPONSE   0x61

   /* ANT Test Mode Message ID Definitions/Constants.                   */
#define ANT_MESSAGE_ID_HOST_INIT_CW_TEST_MODE                     0x53
#define ANT_MESSAGE_ID_HOST_CW_TEST_MODE                          0x48

   /* ANT Legacy Extended Data Message ID Definitions/Constants.        */
#define ANT_MESSAGE_ID_PACKET_EXTENDED_BROADCAST_DATA             0x5D
#define ANT_MESSAGE_ID_PACKET_EXTENDED_ACKNOWLEDGED_DATA          0x5E
#define ANT_MESSAGE_ID_PACKET_EXTENDED_BURST_DATA                 0x5F

   /* ANT Configuration Message Definitions/Constants.                  */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_UnAssign_Channel_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_UnAssign_Channel_Message_t;

#define ANT_HOST_UNASSIGN_CHANNEL_MESSAGE_SIZE              (sizeof(ANT_Host_UnAssign_Channel_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Assign_Channel_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Channel_Type;
   NonAlignedByte_t     Network_Number;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Assign_Channel_Message_t;

#define ANT_HOST_ASSIGN_CHANNEL_MESSAGE_SIZE                (sizeof(ANT_Host_Assign_Channel_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Assign_Channel_Extended_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Channel_Type;
   NonAlignedByte_t     Network_Number;
   NonAlignedByte_t     Extended_Assignment;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Assign_Channel_Extended_Message_t;

#define ANT_HOST_ASSIGN_CHANNEL_EXTENDED_MESSAGE_SIZE       (sizeof(ANT_Host_Assign_Channel_Extended_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Set_Channel_ID_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Set_Channel_ID_Message_t;

#define ANT_HOST_SET_CHANNEL_ID_MESSAGE_SIZE                (sizeof(ANT_Host_Set_Channel_ID_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Channel_Messaging_Period_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Messaging_Period;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Channel_Messaging_Period_Message_t;

#define ANT_HOST_CHANNEL_MESSAGING_PERIOD_MESSAGE_SIZE      (sizeof(ANT_Host_Channel_Messaging_Period_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Channel_Search_Timeout_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Search_Timeout;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Channel_Search_Timeout_Message_t;

#define ANT_HOST_CHANNEL_SEARCH_TIMEOUT_MESSAGE_SIZE        (sizeof(ANT_Host_Channel_Search_Timeout_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Channel_RF_Frequency_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Channel_RF_Frequency;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Channel_RF_Frequency_Message_t;

#define ANT_HOST_CHANNEL_RF_FREQUENCY_MESSAGE_SIZE          (sizeof(ANT_Host_Channel_RF_Frequency_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Set_Network_Key_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Network_Number;
   ANT_Network_Key_t    Network_Key;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Set_Network_Key_Message_t;

#define ANT_HOST_SET_NETWORK_KEY_MESSAGE_SIZE               (sizeof(ANT_Host_Set_Network_Key_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Transmit_Power_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Transmit_Power;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Transmit_Power_Message_t;

#define ANT_HOST_TRANSMIT_POWER_MESSAGE_SIZE                (sizeof(ANT_Host_Transmit_Power_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Add_Channel_ID_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type_ID;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     List_Index;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Add_Channel_ID_Message_t;

#define ANT_HOST_ADD_CHANNEL_ID_MESSAGE_SIZE                (sizeof(ANT_Host_Add_Channel_ID_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Config_List_ID_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     List_Size;
   NonAlignedByte_t     Exclude;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Config_List_ID_Message_t;

#define ANT_HOST_CONFIG_LIST_ID_MESSAGE_SIZE                (sizeof(ANT_Host_Config_List_ID_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Set_Channel_Tx_Power_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Transmit_Power;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Set_Channel_Tx_Power_Message_t;

#define ANT_HOST_SET_CHANNEL_TX_POWER_MESSAGE_SIZE          (sizeof(ANT_Host_Set_Channel_Tx_Power_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Channel_Low_Priority_Search_Timeout_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Search_Timeout;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Channel_Low_Priority_Search_Timeout_Message_t;

#define ANT_HOST_CHANNEL_LOW_PRIORITY_SEARCH_TIMEOUT_MESSAGE_SIZE (sizeof(ANT_Host_Channel_Low_Priority_Search_Timeout_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Serial_Number_Channel_ID_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Device_Type_ID;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Serial_Number_Channel_ID_Message_t;

#define ANT_HOST_SERIAL_NUMBER_CHANNEL_ID_MESSAGE_SIZE      (sizeof(ANT_Host_Serial_Number_Channel_ID_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Enable_Extended_Messages_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Enable;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Enable_Extended_Messages_Message_t;

#define ANT_HOST_ENABLE_EXTENDED_MESSAGES_MESSAGE_SIZE      (sizeof(ANT_Host_Enable_Extended_Messages_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Enable_LED_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Enable;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Enable_LED_Message_t;

#define ANT_HOST_ENABLE_LED_MESSAGE_SIZE                    (sizeof(ANT_Host_Enable_LED_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Enable_Crystal_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Enable_Crystal_Message_t;

#define ANT_HOST_ENABLE_CRYSTAL_MESSAGE_SIZE                (sizeof(ANT_Host_Enable_Crystal_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Lib_Config_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Lib_Config;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Lib_Config_Message_t;

#define ANT_HOST_LIB_CONFIG_MESSAGE_SIZE                    (sizeof(ANT_Host_Lib_Config_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Frequency_Agility_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Frequency_One;
   NonAlignedByte_t     Frequency_Two;
   NonAlignedByte_t     Frequency_Three;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Frequency_Agility_Message_t;

#define ANT_HOST_FREQUENCY_AGILITY_MESSAGE_SIZE             (sizeof(ANT_Host_Frequency_Agility_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Proximity_Search_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Search_Threshold;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Proximity_Search_Message_t;

#define ANT_HOST_PROXIMITY_SEARCH_MESSAGE_SIZE              (sizeof(ANT_Host_Proximity_Search_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Channel_Search_Priority_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Search_Priority;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Channel_Search_Priority_Message_t;

#define ANT_HOST_CHANNEL_SEARCH_PRIORITY_MESSAGE_SIZE              (sizeof(ANT_Host_Channel_Search_Priority_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Set_USB_Descriptor_String_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     String_Number;
   NonAlignedByte_t     Variable_Data[1];
} __PACKED_STRUCT_END__ ANT_Host_Set_USB_Descriptor_String_Message_t;

   /* The following MACRO is provided to allow the programmer a         */
   /* mechanism of calculating the                                      */
   /* ANT_Host_Set_USB_Descriptor_String_Message_t message size for a   */
   /* given string length.  The only argument to this MACRO is the      */
   /* string length of the USB Descriptor String (EXCLUDING the NULL    */
   /* terminater character).                                            */
#define ANT_HOST_SET_USB_DESCRIPTOR_STRING_MESSAGE_SIZE(_x) (BTPS_STRUCTURE_OFFSET(ANT_Host_Set_USB_Descriptor_String_Message_t, Variable_Data) + (sizeof(NonAlignedByte_t)*3) + (unsigned int)(_x))

   /* ANT Notification Message Definitions/Constants.                   */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Startup_Message_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Startup_Message;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Startup_Message_Message_t;

#define ANT_CONTROLLER_STARTUP_MESSAGE_MESSAGE_SIZE        (sizeof(ANT_Controller_Startup_Message_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Serial_Error_Message_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Serial_Error_Message;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Serial_Error_Message_Message_t;

#define ANT_CONTROLLER_SERIAL_ERROR_MESSAGE_MESSAGE_SIZE        (sizeof(ANT_Controller_Serial_Error_Message_Message_t))

   /* ANT Control Message Definitions/Constants.                        */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Reset_System_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Reset_System_Message_t;

#define ANT_HOST_RESET_SYSTEM_MESSAGE_SIZE                  (sizeof(ANT_Host_Reset_System_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Open_Channel_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Open_Channel_Message_t;

#define ANT_HOST_OPEN_CHANNEL_MESSAGE_SIZE                  (sizeof(ANT_Host_Open_Channel_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Close_Channel_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Close_Channel_Message_t;

#define ANT_HOST_CLOSE_CHANNEL_MESSAGE_SIZE                  (sizeof(ANT_Host_Close_Channel_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Request_Message_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Message_ID;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Request_Message_Message_t;

#define ANT_HOST_REQUEST_MESSAGE_MESSAGE_SIZE                 (sizeof(ANT_Host_Request_Message_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Open_Rx_Scan_Mode_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Open_Rx_Scan_Mode_Message_t;

#define ANT_HOST_OPEN_RX_SCAN_MODE_MESSAGE_SIZE             (sizeof(ANT_Host_Open_Rx_Scan_Mode_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Sleep_Message_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Sleep_Message_Message_t;

#define ANT_HOST_SLEEP_MESSAGE_MESSAGE_SIZE                 (sizeof(ANT_Host_Sleep_Message_Message_t))

   /* ANT Data Message Definitions/Constants.                           */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Broadcast_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Broadcast_Data_Packet_t;

#define ANT_BROADCAST_DATA_PACKET_SIZE                      (sizeof(ANT_Broadcast_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Flagged_Broadcast_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Flag_Byte;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Flagged_Broadcast_Data_Packet_t;

#define ANT_FLAGGED_BROADCAST_DATA_PACKET_SIZE              (sizeof(ANT_Flagged_Broadcast_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Extended_Broadcast_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Extended_Broadcast_Data_Packet_t;

#define ANT_EXTENDED_BROADCAST_DATA_PACKET_SIZE             (sizeof(ANT_Extended_Broadcast_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Acknowledged_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Acknowledged_Data_Packet_t;

#define ANT_ACKNOWLEDGED_DATA_PACKET_SIZE                   (sizeof(ANT_Acknowledged_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Flagged_Acknowledged_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Flag_Byte;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Flagged_Acknowledged_Data_Packet_t;

#define ANT_FLAGGED_ACKNOWLEDGED_DATA_PACKET_SIZE           (sizeof(ANT_Flagged_Acknowledged_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Extended_Acknowledged_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Extended_Acknowledged_Data_Packet_t;

#define ANT_EXTENDED_ACKNOWLEDGED_DATA_PACKET_SIZE          (sizeof(ANT_Extended_Acknowledged_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Burst_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Sequence_Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Burst_Data_Packet_t;

#define ANT_BURST_DATA_PACKET_SIZE                          (sizeof(ANT_Burst_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Flagged_Burst_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Sequence_Channel_Number;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Flag_Byte;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Flagged_Burst_Data_Packet_t;

#define ANT_FLAGGED_BURST_DATA_PACKET_SIZE                  (sizeof(ANT_Flagged_Burst_Data_Packet_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Extended_Burst_Data_Packet_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Sequence_Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Data[ANT_MESSAGE_PAYLOAD_MAXIMUM_SIZE];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Extended_Burst_Data_Packet_t;

#define ANT_EXTENDED_BURST_DATA_PACKET_SIZE                 (sizeof(ANT_Extended_Burst_Data_Packet_t))

   /* The following MACRO is provided to allow the programmer a         */
   /* of assigning a 3 bit sequence number in the upper 3 bits of a     */
   /* Burst Data Packet Sequence_Channel_Number member.  The first      */
   /* parameter to this MACRO is a pointer to the                       */
   /* Sequence_Channel_Number member.  The second parameter is the      */
   /* requested Sequence Number.                                        */
#define ANT_BURST_DATA_PACKET_ASSIGN_SEQUENCE_NUMBER(_x, _y)   \
   ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(_x, READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x) | (((_y) << 5) & 0xE0))

   /* The following MACRO is provided to allow the programmer a         */
#define ANT_BURST_DATA_PACKET_ASSIGN_CHANNEL_NUMBER(_x, _y)   \
   ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(_x, READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x) | ((_y) & 0x1F))

   /* ANT Channel Response/Event Message Definitions/Constants.         */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Channel_Response_Event_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Message_ID;
   NonAlignedByte_t     Message_Code;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Channel_Response_Event_Message_t;

#define ANT_CONTROLLER_CHANNEL_RESPONSE_EVENT_MESSAGE_SIZE          (sizeof(ANT_Controller_Channel_Response_Event_Message_t))

   /* ANT Requested Response Message Definitions/Constants.             */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Channel_Status_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedByte_t     Channel_Status;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Channel_Status_Message_t;

#define ANT_CONTROLLER_CHANNEL_STATUS_MESSAGE_SIZE          (sizeof(ANT_Controller_Channel_Status_Message_t))

#define ANT_READ_CHANNEL_STATUS(_x)                         (READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x) & 0x3)

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Channel_ID_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Channel_Number;
   NonAlignedWord_t     Device_Number;
   NonAlignedByte_t     Device_Type_ID;
   NonAlignedByte_t     Transmission_Type;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Channel_ID_Message_t;

#define ANT_CONTROLLER_CHANNEL_ID_MESSAGE_SIZE              (sizeof(ANT_Controller_Channel_ID_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_ANT_Version_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Version_Data[11];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_ANT_Version_Message_t;

#define ANT_CONTROLLER_ANT_VERSION_MESSAGE_SIZE             (sizeof(ANT_Controller_ANT_Version_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Capabilities_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Max_ANT_Channels;
   NonAlignedByte_t     Max_Networks;
   NonAlignedByte_t     Standard_Options;
   NonAlignedByte_t     Advanced_Options;
   NonAlignedByte_t     Advanced_Options_Two;
   NonAlignedByte_t     Reserved;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Capabilities_Message_t;

#define ANT_CONTROLLER_CAPABILITIES_MESSAGE_SIZE            (sizeof(ANT_Controller_Capabilities_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Controller_Device_Serial_Number_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Serial_Number[4];
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Controller_Device_Serial_Number_Message_t;

#define ANT_CONTROLLER_DEVICE_SERIAL_NUMBER_MESSAGE_SIZE    (sizeof(ANT_Controller_Device_Serial_Number_Message_t))

   /* ANT Test Mode Message Definitions/Constants.                      */

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_Init_CW_Test_Mode_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_Init_CW_Test_Mode_Message_t;

#define ANT_HOST_INIT_CW_TEST_MODE_MESSAGE_SIZE       (sizeof(ANT_Host_Init_CW_Test_Mode_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagANT_Host_CW_Test_Mode_Message_t
{
   ANT_Message_Header_t Message_Header;
   NonAlignedByte_t     Filler;
   NonAlignedByte_t     Transmit_Power;
   NonAlignedByte_t     Channel_RF_Frequency;
   NonAlignedByte_t     Checksum;
} __PACKED_STRUCT_END__ ANT_Host_CW_Test_Mode_Message_t;

#define ANT_HOST_CW_TEST_MODE_MESSAGE_SIZE            (sizeof(ANT_Host_CW_Test_Mode_Message_t))

   /* The following constants represent the defined Sync values that are*/
   /* included in each ANT message.                                     */
#define ANT_SYNC_VALUE                                            0xA4
#define ANT_SYNC_VALUE_ALT                                        0xA5

   /* The following constant represents the defined filler value that is*/
   /* used in various ANT Messages.                                     */
#define ANT_FILLER_VALUE                                          0x00

   /* The following constant represents the defined length of the data  */
   /* field when sending Broadcast, Acknowledged, or Burst data.        */
#define ANT_MESSAGE_DATA_LENGTH                                   0x08

   /* The following constants represent the defined Channel Types that  */
   /* may be specified ANT Host Assign Channel message.                 */
#define ANT_CHANNEL_TYPE_RECEIVE                                  0x00
#define ANT_CHANNEL_TYPE_TRANSMIT                                 0x10
#define ANT_CHANNEL_TYPE_RECEIVE_ONLY                             0x40
#define ANT_CHANNEL_TYPE_TRANSMIT_ONLY                            0x50
#define ANT_CHANNEL_TYPE_SHARED_BIDIRECTION_RECEIVE_CHANNEL       0x20
#define ANT_CHANNEL_TYPE_SHARED_BIDIRECTION_TRANSMIT_CHANNEL      0x30

   /* The following constants represent the defined Extended Assignment */
   /* values that may be specified in the optional Extended Assignment  */
   /* Byte of the ANT Host Assign Channel message.                      */
#define ANT_EXTENDED_ASSIGNMENT_BACKGROUND_SCANNING_CHANNEL_ENABLE 0x01
#define ANT_EXTENDED_ASSIGNMENT_FREQUENCY_AGILITY_ENABLE           0x04

   /* The following constants reprsents special Search Timeout values   */
   /* that may be specified when setting both the high priority and low */
   /* priority search timeouts (ANT Set Channel Search Timeout and ANT  */
   /* Set Low Priority Search Timeout messages).                        */
#define ANT_SEARCH_TIMEOUT_DISABLE_SEARCH_MODE                    0x00
#define ANT_SEARCH_TIMEOUT_INFINITE_SEARCH_TIMEOUT                0xFF

   /* The following constants represent the defined Transmit Power      */
   /* Values in the ANT Messages that have a Transmit_Power member.     */
#define ANT_TRANSMIT_POWER_NEGATIVE_TWENTY_DBM                    0x00
#define ANT_TRANSMIT_POWER_NEGATIVE_TEN_DBM                       0x01
#define ANT_TRANSMIT_POWER_NEGATIVE_FIVE_DBM                      0x02
#define ANT_TRANSMIT_POWER_ZERO_DBM                               0x03
#define ANT_TRANSMIT_POWER_FOUR_DBM                               0x04

   /* The following constants represent the minimum and maximum Config  */
   /* List size that can be specified in the ANT Config List message.   */
#define ANT_CONFIG_LIST_INCLUSION_SIZE_MINIMUM                    0x00
#define ANT_CONFIG_LIST_INCLUSION_SIZE_MAXIMUM                    0x04

   /* The following constants represent the inclusion/exclusion list    */
   /* types of Config List that can be specified in the ANT Config List */
   /* message.                                                          */
#define ANT_CONFIG_LIST_INCLUDE_LIST                              0x00
#define ANT_CONFIG_LIST_EXCLUDE_LIST                              0x01

   /* The following constants represent the allowable values that may   */
   /* in the ANT Host Enable Extended Messages message.                 */
#define ANT_EXTENDED_MESSAGES_DISABLE                             0x00
#define ANT_EXTENDED_MESSAGES_ENABLE                              0x01

   /* The following constants represent the allowable values that may   */
   /* in the ANT Host Enable LED message.                               */
#define ANT_LED_DISABLE                                           0x00
#define ANT_LED_ENABLE                                            0x01

   /* The following constant represents the valid value for the Lib     */
   /* Config message byte to disable extended Rx messages.              */
#define ANT_LIB_CONFIG_DISABLE                                    0x00

   /* The following constants represent the valid bit positions in the  */
   /* Lib Config message byte to enable various extended Rx messages.   */
#define ANT_LIB_CONFIG_ENABLE_RX_TIMESTAMP_OUTPUT_BIT             0x20
#define ANT_LIB_CONFIG_ENABLE_RSSI_OUTPUT_BIT                     0x40
#define ANT_LIB_CONFIG_ENABLE_CHANNEL_ID_OUTPUT_BIT               0x80

   /* The following constants represent the minimum and maximum         */
   /* allowable Frequency Agility values used with the ANT Frequency    */
   /* Agility message.                                                  */
#define ANT_FREQUENCY_AGILITY_MINIMUM                             0x00
#define ANT_FREQUENCY_AGILITY_MAXIMUM                             0x7C

   /* The following constants represent the allowable Proximity Search  */
   /* Threshold values that are used with the ANT Set Proximity Search  */
   /* message.                                                          */
#define ANT_PROXIMITY_SEARCH_THRESHOLD_DISABLE                    0x00
#define ANT_PROXIMITY_SEARCH_THRESHOLD_MINIMUM_CLOSEST            0x01
#define ANT_PROXIMITY_SEARCH_THRESHOLD_MAXIMUM_FARTHEST           0x0A

   /* The following constants represent the valid values for the USB    */
   /* Descriptor String Number byte to specify the type of string in the*/
   /* variable length data of the Lib Config message.                   */
#define ANT_USB_DESCRIPTOR_STRING_NUMBER_PID_VID                  0x00
#define ANT_USB_DESCRIPTOR_STRING_NUMBER_MANUFACTURER             0x01
#define ANT_USB_DESCRIPTOR_STRING_NUMBER_DEVICE                   0x02
#define ANT_USB_DESCRIPTOR_STRING_NUMBER_SERIAL_NUMBER            0x03

   /* The following constants represent the special value in the the    */
   /* ANT Controller Startup message that represents that a Power On    */
   /* Reset has occurred.                                               */
#define ANT_STARTUP_MESSAGE_POWER_ON_RESET                        0x00

   /* The following constants represent the defined bit positions in the*/
   /* ANT Controller Startup message.                                   */
#define ANT_STARTUP_MESSAGE_HARDWARE_RESET_LINE_BIT               0x01
#define ANT_STARTUP_MESSAGE_WATCH_DOG_RESET_BIT                   0x02
#define ANT_STARTUP_MESSAGE_COMMAND_RESET_BIT                     0x20
#define ANT_STARTUP_MESSAGE_SYNCHRONOUS_RESET_BIT                 0x40
#define ANT_STARTUP_MESSAGE_SUSPEND_RESET_BIT                     0x80

   /* The following constant represents the value that is present in    */
   /* the Flag byte of a Extended Data Packet message.                  */
#define ANT_FLAGGED_EXTENDED_DATA_PACKET_FLAG                     0x80

   /* The following constants represent the defined values that may be  */
   /* returned in the Message Code of the ANT Controller Channel        */
   /* Response Event message.                                           */
#define ANT_CHANNEL_RESPONSE_CODE_RESPONSE_NO_ERROR               0x00
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_RX_SEARCH_TIMEOUT         0x01
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_RX_FAIL                   0x02
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_TX                        0x03
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_TRANSFER_RX_FAILED        0x04
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_TRANSFER_TX_COMPLETED     0x05
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_TRANSFER_TX_FAILED        0x06
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_CLOSED            0x07
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_RX_FAIL_GO_TO_SEARCH      0x08
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_CHANNEL_COLLISION         0x09
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_TRANSFER_TX_START         0x0A
#define ANT_CHANNEL_RESPONSE_CODE_CHANNEL_IN_WRONG_STATE          0x15
#define ANT_CHANNEL_RESPONSE_CODE_CHANNEL_NOT_OPENED              0x16
#define ANT_CHANNEL_RESPONSE_CODE_CHANNEL_ID_NOT_SET              0x18
#define ANT_CHANNEL_RESPONSE_CODE_CLOSE_ALL_CHANNELS              0x19
#define ANT_CHANNEL_RESPONSE_CODE_TRANSFER_IN_PROGRESS            0x1F
#define ANT_CHANNEL_RESPONSE_CODE_TRANSFER_SEQUENCE_NUMBER_ERROR  0x20
#define ANT_CHANNEL_RESPONSE_CODE_TRANSFER_IN_ERROR               0x21
#define ANT_CHANNEL_RESPONSE_CODE_MESSAGE_SIZE_EXCEEDS_LIMIT      0x27
#define ANT_CHANNEL_RESPONSE_CODE_INVALID_MESSAGE                 0x28
#define ANT_CHANNEL_RESPONSE_CODE_INVALID_NETWORK_NUMBER          0x29
#define ANT_CHANNEL_RESPONSE_CODE_INVALID_LIST_ID                 0x30
#define ANT_CHANNEL_RESPONSE_CODE_INVALID_SCAN_TX_CHANNEL         0x31
#define ANT_CHANNEL_RESPONSE_CODE_INVALID_PARAMETER_PROVIDED      0x33
#define ANT_CHANNEL_RESPONSE_CODE_EVENT_QUEUE_OVERFLOW            0x35
#define ANT_CHANNEL_RESPONSE_CODE_NVM_FULL_ERROR                  0x40
#define ANT_CHANNEL_RESPONSE_CODE_NVM_WRITE_ERROR                 0x41
#define ANT_CHANNEL_RESPONSE_CODE_USB_STRING_WRITE_FAIL           0x70
#define ANT_CHANNEL_RESPONSE_CODE_MSG_SERIAL_ERROR_ID             0xAE

   /* The following constants represent the values that may be present  */
   /* in the data portion of the serial error response code.            */
#define ANT_MSG_SERIAL_ERROR_ID_TYPE_INVALID_SYNC_BYTE            0x00
#define ANT_MSG_SERIAL_ERROR_ID_TYPE_INVALID_CHECKSUM             0x02
#define ANT_MSG_SERIAL_ERROR_ID_TYPE_MESSAGE_TOO_LARGE            0x03

   /* The following constants represent the values that may be present  */
   /* in the lower 2 bits of the Channel Status byte in the ANT         */
   /* Controller Channel Status message.                                */
#define ANT_CHANNEL_STATUS_CHANNEL_UNASSIGNED                     0x00
#define ANT_CHANNEL_STATUS_CHANNEL_ASSIGNED                       0x01
#define ANT_CHANNEL_STATUS_CHANNEL_SEARCHING                      0x02
#define ANT_CHANNEL_STATUS_CHANNEL_TRACKING                       0x03

   /* The following constants represent the valid bit positions in the  */
   /* Standard Options of the Capabilities Request Response message.    */
#define ANT_CAPABILITIES_STD_NO_RECEIVE_CHANNELS_BIT              0x00
#define ANT_CAPABILITIES_STD_NO_TRANSMIT_CHANNELS_BIT             0x01
#define ANT_CAPABILITIES_STD_NO_RECEIVE_MESSAGES_BIT              0x02
#define ANT_CAPABILITIES_STD_NO_TRANSMIT_MESSAGES_BIT             0x03
#define ANT_CAPABILITIES_STD_NO_ACKNOWLEDGED_MESSAGES_BIT         0x04
#define ANT_CAPABILITIES_STD_NO_BURST_MESSAGES_BIT                0x05

   /* The following constants represent the valid bit values in the     */
   /* Standard Options of the Capabilities Request Response message.    */
#define ANT_CAPABILITIES_STD_NO_RECEIVE_CHANNELS_VALUE            0x01
#define ANT_CAPABILITIES_STD_NO_TRANSMIT_CHANNELS_VALUE           0x02
#define ANT_CAPABILITIES_STD_NO_RECEIVE_MESSAGES_VALUE            0x04
#define ANT_CAPABILITIES_STD_NO_TRANSMIT_MESSAGES_VALUE           0x08
#define ANT_CAPABILITIES_STD_NO_ACKNOWLEDGED_MESSAGES_VALUE       0x10
#define ANT_CAPABILITIES_STD_NO_BURST_MESSAGES_VALUE              0x20

   /* The following constants represent the valid bit positions in the  */
   /* Advanced Options 1 of the Capabilities Request Response message.  */
#define ANT_CAPABILITIES_ADV_NETWORK_ENABLED_BIT                  0x01
#define ANT_CAPABILITIES_ADV_SERIAL_NUMBER_ENABLED_BIT            0x03
#define ANT_CAPABILITIES_ADV_PER_CHANNEL_TX_POWER_ENABLED_BIT     0x04
#define ANT_CAPABILITIES_ADV_LOW_PRIORITY_SEARCH_ENABLED_BIT      0x05
#define ANT_CAPABILITIES_ADV_SCRIPT_ENABLED_BIT                   0x06
#define ANT_CAPABILITIES_ADV_SEARCH_LIST_ENABLED_BIT              0x07

   /* The following constants represent the valid bit values in the     */
   /* Advanced Options 1 of the Capabilities Request Response message.  */
#define ANT_CAPABILITIES_ADV_NETWORK_ENABLED_VALUE                0x02
#define ANT_CAPABILITIES_ADV_SERIAL_NUMBER_ENABLED_VALUE          0x08
#define ANT_CAPABILITIES_ADV_PER_CHANNEL_TX_POWER_ENABLED_VALUE   0x10
#define ANT_CAPABILITIES_ADV_LOW_PRIORITY_SEARCH_ENABLED_VALUE    0x20
#define ANT_CAPABILITIES_ADV_SCRIPT_ENABLED_VALUE                 0x40
#define ANT_CAPABILITIES_ADV_SEARCH_LIST_ENABLED_VALUE            0x80

   /* The following constants represent the valid bit positions in the  */
   /* Advanced Options 2 of the Capabilities Request Response message.  */
#define ANT_CAPABILITIES_ADV2_LED_ENABLED_BIT                     0x00
#define ANT_CAPABILITIES_ADV2_EXTENDED_MESSAGE_ENABLED_BIT        0x01
#define ANT_CAPABILITIES_ADV2_SCAN_MODE_ENABLED_BIT               0x02
#define ANT_CAPABILITIES_ADV2_PROXIMITY_SEARCH_ENABLED_BIT        0x04
#define ANT_CAPABILITIES_ADV2_EXTENDED_ASSIGN_ENABLED_BIT         0x05
#define ANT_CAPABILITIES_ADV2_FS_ANTFS_ENABLED_BIT                0x06

   /* The following constants represent the valid bit values in the     */
   /* Advanced Options 2 of the Capabilities Request Response message.  */
#define ANT_CAPABILITIES_ADV2_LED_ENABLED_VALUE                   0x01
#define ANT_CAPABILITIES_ADV2_EXTENDED_MESSAGE_ENABLED_VALUE      0x02
#define ANT_CAPABILITIES_ADV2_SCAN_MODE_ENABLED_VALUE             0x04
#define ANT_CAPABILITIES_ADV2_PROXIMITY_SEARCH_ENABLED_VALUE      0x10
#define ANT_CAPABILITIES_ADV2_EXTENDED_ASSIGN_ENABLED_VALUE       0x20
#define ANT_CAPABILITIES_ADV2_FS_ANTFS_ENABLED_VALUE              0x40

#endif
