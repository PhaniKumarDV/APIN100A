#ifndef _HOSTDL_PACKET_H_
#define _HOSTDL_PACKET_H_

/*==================================================================
 *
 * FILE:        hostdl_packet.h
 *
 * SERVICES:    None
 *
 * DESCRIPTION:
 *   This header file contains externalized definitions from the packet layer.
 *   It is used by the stream buffering layer.
 *
 * Copyright (c) 2009-2011 Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *==================================================================*/

/*===================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_packet.h#1 $ $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why 
 * ----------   ---     ----------------------------------------------
 * 2011-09-21   ab      Add support for partition info command
 * 2011-18-10   rh      Increase the max allowed streamDL VER to 0x05
 * 2010-07-15   rh      Reduce memory footprint, smaller packet buffer
 * 2009-09-15   sv      Revert max data length to 1024
 * 2009-05-07   sv      Hostdl Optimization - Increase max data length
 * 2008-10-29   mm      Initial version
 *==================================================================*/

/*===================================================================
 *
 *                     Include Files
 *
 ====================================================================*/

/* Maximum data length. */
/* Currently QPST tool supports a maximum packet length of 1024. This 
 * value can be tuned to use bigger packet sizes for better performance 
 * if the QPST tool supports bigger size packets */
#ifndef FEATURE_EMMCBLD_NO_HDLC_RX
#define MAX_DATA_LENGTH 1024
#else
#define MAX_DATA_LENGTH 15*1024
#endif

/* Number of packets.  NUMBER_OF_PACKETS * MAX_DATA_LENGTH will be our
   maximum window size. */
#ifndef FEATURE_EMMCBLD_NO_HDLC_RX
#define NUMBER_OF_PACKETS 2
//#define NUMBER_OF_PACKETS 30
#else
#define NUMBER_OF_PACKETS 2
#endif

/* ----------------------------------------------------------------------
 * Defines used to calculate size of reply buffer to QPST and also
 * number of Flash part sectors that will fit into that reply buffer.
 * Code in packet layer will cause a fatal error if the size of the
 * buffer is exceeded at run time.  Code in NOR flash layer will cause
 * a fatal error at compile time if number of sectors is exceeded.
  ----------------------------------------------------------------------*/

#define HOST_REPLY_BUFFER_SIZE  2048

/* Fixed size elements of Parameter Request Reply packet */
#define PACKET_OVERHEAD_SIZE    7
#define CMD_SIZE                1
#define MAGIC_SIZE              32
#define VERSION_SIZE            1
#define COMPAT_VERSION_SIZE     1
#define BLOCK_SIZE_SIZE         4
#define FLASH_BASE_SIZE         4
#define FLASH_ID_LEN_SIZE       1
#define WINDOW_SIZE_SIZE        2
#define NUM_SECTORS_SIZE        2
#define FEATURE_BITS_SIZE       4

/* Variable size element of Parameter Request Reply packet.  The length
 * of the Flash ID string is indeterminate, but currently the largest is
 * 16 bytes, so allow double this size for growth. */
#define FLASH_ID_STRING_SIZE    32


/* Add up all the parts except sectors */
#define REPLY_FIXED_SIZE (PACKET_OVERHEAD_SIZE+CMD_SIZE+MAGIC_SIZE+ \
  VERSION_SIZE+COMPAT_VERSION_SIZE+BLOCK_SIZE_SIZE+FLASH_BASE_SIZE+ \
  FLASH_ID_LEN_SIZE+WINDOW_SIZE_SIZE+NUM_SECTORS_SIZE+ \
  FEATURE_BITS_SIZE+FLASH_ID_STRING_SIZE)


#define REPLY_BUFFER_SIZE   HOST_REPLY_BUFFER_SIZE

/* Calculate amount of 4 byte sector sizes which fit in remaining
 * portion of parameter request reply
 */
#define MAX_SECTORS      ((REPLY_BUFFER_SIZE - REPLY_FIXED_SIZE) / 4)


#define DEVICE_UNKNOWN  0xFF

/* Maximum packet size.  1 for packet type.  4 for length.  2 for CRC. */
#define MAX_PACKET_SIZE (MAX_DATA_LENGTH + 1 + 4 + 2)

#define STREAM_DLOAD_MAX_VER         0x05
#define STREAM_DLOAD_MIN_VER         0x02

#define FEATURE_UNCOMPRESSED_DLOAD   0x00000001

/* We only support these on NAND targets */
#define FEATURE_NAND_PRIMARY_IMAGE      0x00000002
#define FEATURE_NAND_BOOTLOADER_IMAGE   0x00000004
#define FEATURE_NAND_MULTI_IMAGE        0x00000008

#define FEATURE_SECTOR_ADDRESSES   0x00000010

/* This version of the downloader does not support the old NAND two
 * image mode, only Multi-Image
 */
#define SUPPORTED_FEATURES \
  (FEATURE_UNCOMPRESSED_DLOAD | FEATURE_NAND_MULTI_IMAGE | FEATURE_SECTOR_ADDRESSES)

#define READ_LEN                     7
#define PARTITION_INFO_LEN           6+(4*MAX_NUMBER_GPP)



/* Command/Rsp codes, DUMMY_RSP should alwaus be the last one */
#define HELLO_RSP                    0x02
#define READ_RSP                     0x04
#define WRITE_RSP                    0x06
#define STREAM_WRITE_RSP             0x08
#define NOP_RSP                      0x0A
#define RESET_RSP                    0x0C
#define ERROR_RSP                    0x0D
#define CMD_LOG                      0x0E
#define UNLOCK_RSP                   0x10
#define PWRDOWN_RSP                  0x12
#define OPEN_RSP                     0x14
#define CLOSE_RSP                    0x16
#define SECURITY_MODE_RSP            0x18
#define PARTITION_TABLE_RSP          0x1A
#define OPEN_MULTI_IMAGE_RSP         0x1C
#define ERASE_RSP                    0x1E
#define INFO_RSP                     0x24
#define DUMMY_RSP                    0x25

/* Only dispatch commands if they fall in the valid command range. */
#define FIRST_COMMAND 0x01
#define LAST_COMMAND  0x24

/* Check to make sure that LAST_COMMAND is in agreement */
#if (DUMMY_RSP != (LAST_COMMAND + 1))
#error LAST_COMMAND and DUMMY_RSP mismatch. Bailing out!
#endif


/* Length of buffer for size message back to host.  Increase this
 * value if you change the size of size_msg_fixed.  Used by the
 * function log_size_msg().
 */
#define SIZE_MSG_LEN  64


//--------------------------------------------------------------------------
// Global Data
//--------------------------------------------------------------------------

struct incoming_data
{
  struct incoming_data *next;   /* Chain appropriately. */
  word length;                  /* Number of valid bytes in buffer. */
  byte buffer[MAX_PACKET_SIZE];
};

typedef struct incoming_data *incoming_t;
void* alignmemcpy (void* dest, void const* src, uint32 len);

#endif /* _HOSTDL_PACKET_H_ */
