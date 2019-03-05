/*****< hcrtypes.h >***********************************************************/
/*      Copyright 2002 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HCRTYPES - Bluetooth Hardcopy Cable Replacement Type                      */
/*             Definitions/Constants.                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/30/02  D. Lange       Initial creation.                               */
/*   08/01/09  D. Lange       Moved into seperate file.                       */
/******************************************************************************/
#ifndef __HCRTYPESH__
#define __HCRTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following definitions represent the PDU ID values of the      */
   /* defined Hard Copy Cable Replacement Control Channel PDU's.        */
#define HCR_PDU_ID_CR_DATA_CHANNEL_CREDIT_GRANT                    0x0001
#define HCR_PDU_ID_CR_DATA_CHANNEL_CREDIT_REQUEST                  0x0002
#define HCR_PDU_ID_CR_DATA_CHANNEL_CREDIT_RETURN                   0x0003
#define HCR_PDU_ID_CR_DATA_CHANNEL_CREDIT_QUERY                    0x0004
#define HCR_PDU_ID_CR_GET_LPT_STATUS                               0x0005
#define HCR_PDU_ID_CR_GET_1284_ID                                  0x0006
#define HCR_PDU_ID_CR_SOFT_RESET                                   0x0007
#define HCR_PDU_ID_CR_HARD_RESET                                   0x0008
#define HCR_PDU_ID_CR_REGISTER_NOTIFICATION                        0x0009
#define HCR_PDU_ID_CR_NOTIFICATION_CONNECTION_ALIVE                0x000A

   /* The following definitions represent the PDU ID values of the      */
   /* defined Hard Copy Cable Replacement Notification Channel PDU's.   */
#define HCR_PDU_ID_N_NOTIFICATION                                  0x0001

   /* The following constants represent the Constants that can be used  */
   /* in the Register Notification PDU to Register or Un-Register a     */
   /* Notification Callback.                                            */
#define HCR_REGISTER_NOTIFICATION_PDU_REGISTER                     0x01
#define HCR_REGISTER_NOTIFICATION_PDU_UNREGISTER                   0x00

   /* The following type declaration represents the structure of the    */
   /* Header of a Hard Copy Cable Replacement Profile Data Packet.  This*/
   /* Header Information is contained in Every Defined Hard Copy Cable  */
   /* Replacement Profile Packet.  This structure forms the basis of    */
   /* additional defined Hard Copy Cable Profile Packets.  Since this   */
   /* structure is present at the begining of Every Defined Hard Copy   */
   /* Cable Profile Packet, this structure will be the first element of */
   /* Every Defined Hard Copy Cable Profile Packet in this file.        */
   /* * NOTE * The Hard Copy Cable Profile is a Big Endian protocol.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_Protocol_Data_Unit_Header_t
{
   NonAlignedWord_t PDU_ID;
   NonAlignedWord_t Transaction_ID;
   NonAlignedWord_t Parameter_Length;
} __PACKED_STRUCT_END__ HCR_Protocol_Data_Unit_Header_t;

#define HCR_PROTOCOL_DATA_UNIT_HEADER_SIZE                        (sizeof(HCR_Protocol_Data_Unit_Header_t))

   /* The following structure represents the CR_DataChannelCreditGrant  */
   /* Request PDU.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Grant_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedDWord_t               Credit_Granted;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Grant_Request_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_GRANT_REQUEST_SIZE             (sizeof(HCR_CR_Data_Channel_Credit_Grant_Request_t))

   /* The following structure represents the CR_DataChannelCreditGrant  */
   /* Reply PDU.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Grant_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Grant_Reply_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_GRANT_REPLY_SIZE               (sizeof(HCR_CR_Data_Channel_Credit_Grant_Reply_t))

   /* The following structure represents the CR_DataChannelCreditRequest*/
   /* Request PDU.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Request_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Request_Request_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_REQUEST_REQUEST_SIZE           (sizeof(HCR_CR_Data_Channel_Credit_Request_Request_t))

   /* The following structure represents the CR_DataChannelCreditRequest*/
   /* Reply PDU.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Request_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedDWord_t               Credit_Granted;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Request_Reply_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_REQUEST_REPLY_SIZE             (sizeof(HCR_CR_Data_Channel_Credit_Request_Reply_t))

   /* The following structure represents the CR_DataChannelCreditReturn */
   /* Request PDU.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Return_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedDWord_t               Client_Credit_Return;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Return_Request_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_RETURN_REQUEST_SIZE            (sizeof(HCR_CR_Data_Channel_Credit_Return_Request_t))

   /* The following structure represents the CR_DataChannelCreditReturn */
   /* Reply PDU.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Return_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedDWord_t               Server_Credit_Return;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Return_Reply_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_RETURN_REPLY_SIZE              (sizeof(HCR_CR_Data_Channel_Credit_Return_Reply_t))

   /* The following structure represents the CR_DataChannelCreditQuery  */
   /* Request PDU.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Query_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedDWord_t               Client_Credit;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Query_Request_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_QUERY_REQUEST_SIZE             (sizeof(HCR_CR_Data_Channel_Credit_Query_Request_t))

   /* The following structure represents the CR_DataChannelCreditQuery  */
   /* Reply PDU.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Data_Channel_Credit_Query_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedDWord_t               Server_Credit;
} __PACKED_STRUCT_END__ HCR_CR_Data_Channel_Credit_Query_Reply_t;

#define HCR_CR_DATA_CHANNEL_CREDIT_QUERY_REPLY_SIZE               (sizeof(HCR_CR_Data_Channel_Credit_Query_Reply_t))

   /* The following structure represents the CR_GetLPTStatus Request    */
   /* PDU.                                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Get_LPT_Status_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
} __PACKED_STRUCT_END__ HCR_CR_Get_LPT_Status_Request_t;

#define HCR_CR_GET_LPT_STATUS_REQUEST_SIZE                        (sizeof(HCR_CR_Get_LPT_Status_Request_t))

   /* The following structure represents the CR_GetLPTStatus Reply PDU. */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Get_LPT_Status_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedByte_t                LPT_Status;
} __PACKED_STRUCT_END__ HCR_CR_Get_LPT_Status_Reply_t;

#define HCR_CR_GET_LPT_STATUS_REPLY_SIZE                          (sizeof(HCR_CR_Get_LPT_Status_Reply_t))

   /* The following structure represents the CR_Get1284ID Request PDU.  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Get_1284_ID_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Start_Byte;
   NonAlignedWord_t                Number_Of_Bytes;
} __PACKED_STRUCT_END__ HCR_CR_Get_1284_ID_Request_t;

#define HCR_CR_GET_1284_ID_REQUEST_SIZE                           (sizeof(HCR_CR_Get_1284_ID_Request_t))

   /* The following structure represents the CR_Get1284ID Reply PDU.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Get_1284_ID_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   Byte_t                          Variable_Data[1];
} __PACKED_STRUCT_END__ HCR_CR_Get_1284_ID_Reply_t;

   /* The following MACRO is a utility MACRO that exists to aid code    */
   /* readability to Determine the size (in Bytes) of a Hard Copy Cable */
   /* Replacement Get 1284ID String Packet Data structure based upon the*/
   /* length of bytes that are required for the 1284ID portion of the   */
   /* Reply.  The first parameter to this MACRO is the size (in Bytes)  */
   /* of the 1284ID String Data that is part of the Reply Packet.       */
#define HCR_CR_GET_1284_ID_REPLY_SIZE(_x)                         (sizeof(HCR_CR_Get_1284_ID_Reply_t) - sizeof(Byte_t) + (unsigned int)(_x))

   /* The following structure represents the CR_SoftReset Request PDU.  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Soft_Reset_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
} __PACKED_STRUCT_END__ HCR_CR_Soft_Reset_Request_t;

#define HCR_CR_SOFT_RESET_REQUEST_SIZE                            (sizeof(HCR_CR_Soft_Reset_Request_t))

   /* The following structure represents the CR_SoftReset Reply PDU.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Soft_Reset_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
} __PACKED_STRUCT_END__ HCR_CR_Soft_Reset_Reply_t;

#define HCR_CR_SOFT_RESET_REPLY_SIZE                              (sizeof(HCR_CR_Soft_Reset_Reply_t))

   /* The following structure represents the CR_HardReset Request PDU.  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Hard_Reset_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
} __PACKED_STRUCT_END__ HCR_CR_Hard_Reset_Request_t;

#define HCR_CR_HARD_RESET_REQUEST_SIZE                            (sizeof(HCR_CR_Hard_Reset_Request_t))

   /* The following structure represents the CR_HardReset Reply PDU.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Hard_Reset_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
} __PACKED_STRUCT_END__ HCR_CR_Hard_Reset_Reply_t;

#define HCR_CR_HARD_RESET_REPLY_SIZE                              (sizeof(HCR_CR_Soft_Reset_Reply_t))

   /* The following structure represents the CR_RegisterNotification    */
   /* Request PDU.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Register_Notification_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedByte_t                Register;
   NonAlignedDWord_t               Callback_Context_ID;
   NonAlignedDWord_t               Callback_Time_Out;
} __PACKED_STRUCT_END__ HCR_CR_Register_Notification_Request_t;

#define HCR_CR_REGISTER_NOTIFICATION_REQUEST_SIZE                 (sizeof(HCR_CR_Register_Notification_Request_t))

   /* The following structure represents the CR_RegisterNotification    */
   /* Reply PDU.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Register_Notification_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedDWord_t               Time_Out;
   NonAlignedDWord_t               Callback_Time_Out;
} __PACKED_STRUCT_END__ HCR_CR_Register_Notification_Reply_t;

#define HCR_CR_REGISTER_NOTIFICATION_REPLY_SIZE                   (sizeof(HCR_CR_Register_Notification_Reply_t))

   /* The following structure represents the                            */
   /* CR_NotificationConnectionAlive Request PDU.                       */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Notification_Connection_Alive_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
} __PACKED_STRUCT_END__ HCR_CR_Notification_Connection_Alive_Request_t;

#define HCR_CR_NOTIFICATION_CONNECTION_ALIVE_REQUEST_SIZE         (sizeof(HCR_CR_Notification_Connection_Alive_Request_t))

   /* The following structure represents the                            */
   /* CR_NotificationConnectionAlive Reply PDU.                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Notification_Connection_Alive_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   NonAlignedDWord_t               Time_Out_Increment;
} __PACKED_STRUCT_END__ HCR_CR_Notification_Connection_Alive_Reply_t;

#define HCR_CR_NOTIFICATION_CONNECTION_ALIVE_REPLY_SIZE           (sizeof(HCR_CR_Notification_Connection_Alive_Reply_t))

   /* The following structure represents the CR_VendorSpecific Request  */
   /* PDU.                                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Vendor_Specific_Request_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   Byte_t                          Variable_Data[1];
} __PACKED_STRUCT_END__ HCR_CR_Vendor_Specific_Request_t;

   /* The following MACRO is a utility MACRO that exists to aid code    */
   /* readability to Determine the size (in Bytes) of a Hard Copy Cable */
   /* Replacement Vendor Specific Request Packet Data structure based   */
   /* upon the length of bytes that are required for the Vendor Specific*/
   /* Request portion of the Request.  The first parameter to this MACRO*/
   /* is the size (in Bytes) of the Vendor Specific Data that is part of*/
   /* the Request Packet.                                               */
#define HCR_CR_VENDOR_SPECIFIC_REQUEST_SIZE(_x)                   (sizeof(HCR_CR_Vendor_Specific_Request_t) - sizeof(Byte_t) + (unsigned int)(_x))

   /* The following structure represents the CR_VendorSpecific Reply    */
   /* PDU.                                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Vendor_Specific_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
   Byte_t                          Variable_Data[1];
} __PACKED_STRUCT_END__ HCR_CR_Vendor_Specific_Reply_t;

   /* The following MACRO is a utility MACRO that exists to aid code    */
   /* readability to Determine the size (in Bytes) of a Hard Copy Cable */
   /* Replacement Vendor Specific Reply Packet Data structure based upon*/
   /* the length of bytes that are required for the Vendor Specific     */
   /* Reply portion of the Reply.  The first parameter to this MACRO is */
   /* the size (in Bytes) of the Vendor Specific Data that is part of   */
   /* the Reply Packet.                                                 */
#define HCR_CR_VENDOR_SPECIFIC_REPLY_SIZE(_x)                     (sizeof(HCR_CR_Vendor_Specific_Reply_t) - sizeof(Byte_t) + (unsigned int)(_x))

   /* The following structure represents the Reply PDU that is issued   */
   /* when a Response Error Code of Unsupported is issued.  This PDU is */
   /* simply the Status Code and Protocol Data Unit Header (i.e. no     */
   /* Parameters are present in the Response).  This is a special case, */
   /* all other Status Codes return PDU's that have the correct number  */
   /* or Parameters.                                                    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_CR_Feature_Unsupported_Reply_t
{
   HCR_Protocol_Data_Unit_Header_t HCR_Protocol_Data_Unit_Header;
   NonAlignedWord_t                Status_Code;
} __PACKED_STRUCT_END__ HCR_CR_Feature_Unsupported_Reply_t;

#define HCR_CR_FEATURE_UNSUPPORTED_REPLY_SIZE                     (sizeof(HCR_CR_Feature_Unsupported_Reply_t))

   /* The following type declaration represents the structure of the    */
   /* Header of a Hard Copy Cable Replacement Profile Notification Data */
   /* Packet.  This Header Information is contained in Every Defined    */
   /* Hard Copy Cable Replacement Profile Notification Packet.  This    */
   /* structure forms the basis of additional defined Hard Copy Cable   */
   /* Profile Notification Packets.  Since this structure is present at */
   /* the begining of Every Defined Hard Copy Cable Profile Packet, this*/
   /* structure will be the first element of Every Defined Hard Copy    */
   /* Cable Profile Notification Packet in this file.                   */
   /* * NOTE * The Hard Copy Cable Profile is a Big Endian protocol.    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_Notification_Data_Unit_Header_t
{
   NonAlignedWord_t PDU_ID;
} __PACKED_STRUCT_END__ HCR_Notification_Data_Unit_Header_t;

#define HCR_NOTIFICATION_DATA_UNIT_HEADER_SIZE                    (sizeof(HCR_Notification_Data_Unit_Header_t))

   /* The following structure represents the N_Notification Request PDU.*/
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_N_Notification_Request_t
{
   HCR_Notification_Data_Unit_Header_t HCR_Notification_Data_Unit_Header;
   NonAlignedDWord_t                   Callback_Context_ID;
} __PACKED_STRUCT_END__ HCR_N_Notification_Request_t;

#define HCR_N_NOTIFICATION_REQUEST_SIZE                           (sizeof(HCR_N_Notification_Request_t))

   /* The following structure represents the N_VendorSpecific Request   */
   /* PDU.                                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_N_Vendor_Specific_Request_t
{
   HCR_Notification_Data_Unit_Header_t HCR_Notification_Data_Unit_Header;
   Byte_t                              Variable_Data[1];
} __PACKED_STRUCT_END__ HCR_N_Vendor_Specific_Request_t;

   /* The following MACRO is a utility MACRO that exists to aid code    */
   /* readability to Determine the size (in Bytes) of a Hard Copy Cable */
   /* Replacement Vendor Specific Notification Request Packet Data      */
   /* structure based upon the length of bytes that are required for the*/
   /* Vendor Specific Request portion of the Request.  The first        */
   /* parameter to this MACRO is the size (in Bytes) of the Vendor      */
   /* Specific Data that is part of the Request Packet.                 */
#define HCR_N_VENDOR_SPECIFIC_REQUEST_SIZE(_x)                    (sizeof(HCR_N_Vendor_Specific_Request_t) - sizeof(Byte_t) + (unsigned int)(_x))

   /* The following structure represents the N_VendorSpecific Reply PDU.*/
typedef __PACKED_STRUCT_BEGIN__ struct _tagHCR_N_Vendor_Specific_Reply_t
{
   HCR_Notification_Data_Unit_Header_t HCR_Notification_Data_Unit_Header;
   Byte_t                              Variable_Data[1];
} __PACKED_STRUCT_END__ HCR_N_Vendor_Specific_Reply_t;

   /* The following MACRO is a utility MACRO that exists to aid code    */
   /* readability to Determine the size (in Bytes) of a Hard Copy Cable */
   /* Replacement Vendor Specific Notification Reply Packet Data        */
   /* structure based upon the length of bytes that are required for the*/
   /* Vendor Specific Reply portion of the Reply.  The first parameter  */
   /* to this MACRO is the size (in Bytes) of the Vendor Specific Data  */
   /* that is part of the Reply Packet.                                 */
#define HCR_N_VENDOR_SPECIFIC_REPLY_SIZE(_x)                      (sizeof(HCR_N_Vendor_Specific_Reply_t) - sizeof(Byte_t) + (unsigned int)(_x))

#endif
