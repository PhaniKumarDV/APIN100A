/*****< ss1btusb.h >***********************************************************/
/* Copyright (c) 2015, The Linux Foundation. All rights reserved.             */
/*                                                                            */
/* Permission to use, copy, modify, and/or distribute this software for       */
/* any purpose with or without fee is hereby granted, provided that           */
/* the above copyright notice and this permission notice appear in all        */
/* copies.                                                                    */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL              */
/* WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED              */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE           */
/* AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL       */
/* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA         */
/* OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER          */
/* TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR           */
/* PERFORMANCE OF THIS SOFTWARE.                                              */
/*                                                                            */
/* Copyright 2007 - 2014 Qualcomm Atheros, Inc.                               */
/******************************************************************************/
/*                                                                            */
/*  SS1BTUSB - Linux USB Device Driver Constants for the Stonestreet One      */
/*             Bluetooth HCI USB Device Driver (Linux Only).                  */
/*                                                                            */
/******************************************************************************/
#ifndef __SS1BTUSBH__
#define __SS1BTUSBH__

#include <linux/ioctl.h>                 /* Included for IOCTL MACRO's.       */

   /* The following constant defines the actual device name prefix of   */
   /* the USB Device.  This prefix will have an index number appended to*/
   /* it to generate the actual physical device name.                   */
#define SS1BTUSB_DEVICE_NAME_PREFIX                   "SS1BTUSB"

   /* The following constant defines the 'seed' value that is used to   */
   /* generate the IOCTL's defined for the device.                      */
#define BTUSB_IOCTL_MAGIC                             'b'

   /* Wait for an asynchronous USB Device Event (ACL/SCO/Event Data)    */
   /* received.                                                         */
#define BTUSB_IOCTL_WAIT_DATA                         _IOR(BTUSB_IOCTL_MAGIC,  0, void *)

   /* Send/Receive HCI Packets to/from USB Bluetooth Device.            */
#define BTUSB_IOCTL_SEND_PACKET                       _IOW(BTUSB_IOCTL_MAGIC,  1, void *)

#define BTUSB_IOCTL_READ_PACKET                       _IOWR(BTUSB_IOCTL_MAGIC, 2, void *)

   /* Enable/Disable SCO Data transfers.                                */
#define BTUSB_IOCTL_ENABLE_SCO_DATA                   _IOW(BTUSB_IOCTL_MAGIC,  3, int)

   /* The following constants are used to identify a packet type for the*/
   /* USBTransferInformation_t structure.                               */
   /* * NOTE * These correspond to the Packet Type values used by HCI   */
   /*          UART protocol.                                           */
#define BTUSB_PACKET_TYPE_COMMAND                     0x01
#define BTUSB_PACKET_TYPE_ACL                         0x02
#define BTUSB_PACKET_TYPE_SCO                         0x03
#define BTUSB_PACKET_TYPE_EVENT                       0x04

   /* The following Constants are used as an Event Mask for the IOCTL   */
   /* function IOCTL_WAIT_DATA and represent the event(s) that occurred */
   /* causing the wait event to return.  These constants are also used  */
   /* as valid packet types in the ioctl_info_t structure.              */
   /* * NOTE * The packet type values were specifically chosen so that  */
   /*          the value would equal (1 << HCI Packet Type).  This was  */
   /*          done so that the bits in the event mask can be           */
   /*          set/cleared using the packet type as the bit number.     */
#define BTUSB_WAIT_DRIVER_EVENT_ACL_DATA              (1 << BTUSB_PACKET_TYPE_ACL)
#define BTUSB_WAIT_DRIVER_EVENT_SCO_DATA              (1 << BTUSB_PACKET_TYPE_SCO)
#define BTUSB_WAIT_DRIVER_EVENT_HCI_EVENT             (1 << BTUSB_PACKET_TYPE_EVENT)

   /* The following Constants represent the maximum packet size for the */
   /* various Bluetooth Packets.                                        */
#define BTUSB_HCI_MAX_ACL_LENGTH                      1024
#define BTUSB_HCI_MAX_SCO_LENGTH                      256
#define BTUSB_HCI_MAX_EVENT_LENGTH                    260
#define BTUSB_HCI_MAX_COMMAND_LENGTH                  260

   /* This structure is used for for transferring data to/from the USB  */
   /* Driver.                                                           */
typedef struct _tagUSBPacketInformation_t
{
   unsigned int   PacketType;
   unsigned int   BufferSize;
   unsigned char *Buffer;
} USBPacketInformation_t;

   /* The following constants represent the constants that can be used  */
   /* with the IOCTL_ENABLE_SCO_DATA IOCTL to enable/disable SCO traffic*/
   /* on the USB Bus.                                                   */
#define BTUSB_SCO_DATA_NO_CHANNELS                                         0x00
#define BTUSB_SCO_DATA_ONE_EIGHT_BIT_CHANNEL                               0x01
#define BTUSB_SCO_DATA_TWO_EIGHT_BIT_CHANNELS                              0x02
#define BTUSB_SCO_DATA_THREE_EIGHT_BIT_CHANNELS                            0x03
#define BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_CHANNEL                             0x04
#define BTUSB_SCO_DATA_TWO_SIXTEEN_BIT_CHANNELS                            0x05
#define BTUSB_SCO_DATA_THREE_SIXTEEN_BIT_CHANNELS                          0x06
#define BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_WBS_CHANNEL                         0x07
#define BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_CHANNEL_ONE_SIXTEEN_BIT_WBS_CHANNEL 0x08
#define BTUSB_SCO_DATA_ONE_SIXTEEN_BIT_MSBC_CHANNEL                        0x09

#endif
