/*****< tdstypes.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSTYPES - Bluetooth 3D Synchronization (TDS) Type Definitions/Constants. */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/01/13  R. Byrne       Initial creation.                               */
/*   04/15/14  T. Cook        Changed some structure naming.                  */
/******************************************************************************/
#ifndef __TDSTYPESH__
#define __TDSTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following defines the L2CAP Fixed Channel that is used by 3DS */
   /* devices to communicate association information.                   */
#define TDS_COMMUNICATION_CHANNEL_PROTOCOL_SERVICE_MULTIPLEXOR       (0x0021)

   /* The following define the possible OpCodes for a 3DS Communication */
   /* Channel message.                                                  */
#define TDS_COMMUNICATION_CHANNEL_OPCODE_3DG_CONNECTION_ANNOUNCEMENT 0

   /* The following defines the TDS Broadcast LT_ADDR that is used on   */
   /* the Connectionless Slave Broadcast channel.                       */
#define TDS_BROADCAST_LT_ADDR                                        1

   /* The following section defines the Legacy (Broadcom) 3DG opcodes.  */
#define TDS_LEGACY_OCF_ENABLE_3D_REPORTING                           0x0B7

   /* The following section defines the Vendor Sub-Codes for the        */
   /* Broadcom 3DG Events that are returned in the HCI Vendor Specific  */
   /* Event.                                                            */
#define TDS_LEGACY_VENDOR_SUB_CODE_3D_MODE_CHANGE_EVENT              0x21
#define TDS_LEGACY_VENDOR_SUB_CODE_3D_FRAME_PERIOD_EVENT             0x22
#define TDS_LEGACY_VENDOR_SUB_CODE_EIR_HANDSHAKE_EVENT               0x29

   /* The following section defines the QCA Vendor Specific 3DD opcodes.*/
#define TDS_QCA_OCF_ISCAN_ASSOCIATION                                0x13

   /* The following section defines the Vendor Sub-Codes for the QCA 3DD*/
   /* Events returned in the HCI Vendor Specific Event.                 */
#define TDS_QCA_VENDOR_SUB_CODE_ASSOCIATION_NOTIFICATION             0x14

   /* The following type declaration represents the structure of a      */
   /* broadcast message transmitted by the 3D Display.                  */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Broadcast_Message_t
{
   NonAlignedDWord_t FrameSyncVideoMode;
   NonAlignedWord_t  ClockPhase;
   NonAlignedWord_t  LeftLensShutterOpenOffset;
   NonAlignedWord_t  LeftLensShutterCloseOffset;
   NonAlignedWord_t  RightLensShutterOpenOffset;
   NonAlignedWord_t  RightLensShutterCloseOffset;
   NonAlignedWord_t  FrameSyncPeriod;
   NonAlignedByte_t  FrameSyncPeriodFraction;
} __PACKED_STRUCT_END__ TDS_Broadcast_Message_t;

#define TDS_BROADCAST_MESSAGE_SIZE                             (sizeof(TDS_Broadcast_Message_t))

   /* The following type declaration represents the structure of a      */
   /* broadcast message transmitted by the 3D Display operating in      */
   /* Legacy mode.                                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Legacy_Broadcast_Message_t
{
   NonAlignedWord_t  LeftLensShutterOpenOffset;
   NonAlignedWord_t  LeftLensShutterCloseOffset;
   NonAlignedWord_t  RightLensShutterOpenOffset;
   NonAlignedWord_t  RightLensShutterCloseOffset;
   NonAlignedWord_t  Delay;
   NonAlignedByte_t  ModeFlags;
} __PACKED_STRUCT_END__ TDS_Legacy_Broadcast_Message_t;

#define TDS_LEGACY_BROADCAST_MESSAGE_SIZE                      (sizeof(TDS_Legacy_Broadcast_Message_t))

   /* The following can be used to set special fields within the        */
   /* TDS_Broadcast_Message_t structure.                                */
#define TDS_BROADCAST_MESSAGE_FRAME_SYNC_BITMASK               (0x03FFFFFF)
#define TDS_BROADCAST_MESSAGE_VIDEO_MODE_BITMASK               (0x40000000)

   /* The following define the valid Video Mode values.                 */
#define TDS_BROADCAST_MESSAGE_VIDEO_MODE_3D                    (0x00000000)
#define TDS_BROADCAST_MESSAGE_VIDEO_MODE_DUAL_VIEW             (0x40000000)

   /* The following define the valid Video Mode values for the legacy   */
   /* TDS Broadcast Message.                                            */
#define TDS_LEGACY_BROADCAST_MESSAGE_VIDEO_MODE_3D             (0x00)
#define TDS_LEGACY_BROADCAST_MESSAGE_VIDEO_MODE_DUAL_VIEW      (0x01)

   /* The following type declaration represents the structure of the    */
   /* header for all 3DS Communication Channel messages.                */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Communication_Channel_Message_Header_t
{
   NonAlignedByte_t OpCode;
} __PACKED_STRUCT_END__ TDS_Communication_Channel_Message_Header_t;

#define TDS_COMMUNICATION_CHANNEL_MESSAGE_HEADER_SIZE          (sizeof(TDS_Communication_Channel_Message_Header_t))

   /* The following type declaration represents the structure of the    */
   /* Connection Announcement 3DS Communication Channel message.        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Connection_Announcement_Message_t
{
   TDS_Communication_Channel_Message_Header_t Header;
   NonAlignedByte_t                           Flags;
   NonAlignedByte_t                           BatteryLevel;
} __PACKED_STRUCT_END__ TDS_Connection_Announcement_Message_t;

#define TDS_CONNECTION_ANNOUNCEMENT_MESSAGE_SIZE               (sizeof(TDS_Connection_Announcement_Message_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Enable_Legacy_3D_Reporting_Command_Header_t
{
   NonAlignedByte_t Operation;
} __PACKED_STRUCT_END__ TDS_Enable_Legacy_3D_Reporting_Command_Header_t;

#define TDS_ENABLE_LEGACY_3D_REPORTING_COMMAND_HEADER_SIZE     (sizeof(TDS_Enable_Legacy_3D_Reporting_Command_Header_t))

   /* The following defines the valid values for the Operation field of */
   /* the TDS Legacy Enable 3D Reporting Command.                       */
#define TDS_ENABLE_LEGACY_3D_REPORTING_DISABLE                          0x00
#define TDS_ENABLE_LEGACY_3D_REPORTING_GLASSES_ONLY                     0x01
#define TDS_ENABLE_LEGACY_3D_REPORTING_SLAVE                            0x02
#define TDS_ENABLE_LEGACY_3D_REPORTING_MASTER                           0x03
#define TDS_ENABLE_LEGACY_3D_REPORTING_MASTER_MULTICAST_GLASSES_UNICAST 0x04
#define TDS_ENABLE_LEGACY_3D_REPORTING_MIP                              0x05
#define TDS_ENABLE_LEGACY_3D_REPORTING_MULTICAST_AND_MIP                0x06
#define TDS_ENABLE_LEGACY_3D_REPORTING_MULTICAST_AND_MIP_HYBRID         0x07

   /* The following type declaration represents the structure of the    */
   /* Header of an Legacy (Broadcom) 3DG Event Packet.  This Header     */
   /* Information is contained in Every Defined Legacy Broadcom 3DG     */
   /* Event Packet.  This structure forms the basis of additional       */
   /* defined HCI Event Packets.  Since this structure is present at the*/
   /* begining of Every Defined HCI Event Packet, this structure will be*/
   /* the first element of Every Defined HCI Event Packet in this file. */
typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Legacy_Event_Header_t
{
   NonAlignedByte_t SubCode;
} __PACKED_STRUCT_END__ TDS_Legacy_Event_Header_t;

#define TDS_LEGACY_EVENT_HEADER_SIZE                           (sizeof(TDS_Legacy_Event_Header_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Legacy_Frame_Period_Event_t
{
   TDS_Legacy_Event_Header_t TDS_Legacy_Event_Header;
   NonAlignedWord_t          Frame_Period;
   NonAlignedByte_t          Frame_Period_Fraction;
   NonAlignedByte_t          Unstable_Measurement;
} __PACKED_STRUCT_END__ TDS_Legacy_Frame_Period_Event_t;

#define TDS_LEGACY_FRAME_PERIOD_EVENT_SIZE                     (sizeof(TDS_Legacy_Frame_Period_Event_t))

   /* The following defines the valid value of the Unstable_Measurement */
   /* field of the 3D Frame Period Event.                               */
#define TDS_LEGACY_FRAME_PERIOD_EVENT_FRAME_PERIOD_STABLE      0x00
#define TDS_LEGACY_FRAME_PERIOD_EVENT_FRAME_PERIOD_UNSTABLE    0x01

typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_Legacy_EIR_Handshake_Event_t
{
   TDS_Legacy_Event_Header_t TDS_Legacy_Event_Header;
   BD_ADDR_t                 BD_ADDR;
   NonAlignedByte_t          Device_Data;
   NonAlignedByte_t          Reserved[10];
} __PACKED_STRUCT_END__ TDS_Legacy_EIR_Handshake_Event_t;

#define TDS_LEGACY_EIR_HANDSHAKE_EVENT_SIZE                    (sizeof(TDS_Legacy_EIR_Handshake_Event_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_QCA_Event_Header_t
{
   NonAlignedByte_t SubCode;
} __PACKED_STRUCT_END__ TDS_QCA_Event_Header_t;

#define TDS_QCA_EVENT_HEADER_SIZE                              (sizeof(TDS_QCA_Event_Header_t))

typedef __PACKED_STRUCT_BEGIN__ struct _tagTDS_QCA_Legacy_Association_Event_t
{
   TDS_QCA_Event_Header_t Header;
   BD_ADDR_t              BD_ADDR;
   NonAlignedByte_t       Device_Data;
   NonAlignedByte_t       Reserved[10];
} __PACKED_STRUCT_END__ TDS_QCA_Legacy_Association_Event_t;

#define TDS_QCA_LEGACY_ASSOCIATION_EVENT_SIZE                  (sizeof(TDS_QCA_Legacy_Association_Event_t))

#endif
