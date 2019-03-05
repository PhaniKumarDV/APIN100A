#ifndef __EMMCBLD_H_
#define __EMMCBLD_H_

/*==================================================================
 *
 * FILE:        emmcbld.h
 *
 * SERVICES:    Definitions for the DMSS Flash Programmer
 *
 * DESCRIPTION:
 *    This file contains common definitions for DMSS Flash Programmer
 *    Utility.
 *
 *
 *        Copyright © 2008-2010 Qualcomm Technologies Incorporated.
 *               All Rights Reserved.
 *            QUALCOMM Proprietary/GTDR
 *==================================================================*/

/*===================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld.h#1 $ $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2011-09-21   ab      Adding partition types for boot and GPP partitions
 * 2010-08-26   rh      Adding the newly allocated image ID for eMMC
 * 2010-06-23   rh      Adding define for partition type
 * 2009-03-20   jz      Moved the dispatch struct def from hostdl.h to flash_prog.h
 * 2008-10-29   mm      Initial Revision
 *==================================================================*/

/*===================================================================
 *
 *                     Include Files
 *
 ====================================================================*/

#include "flash_prog.h"

//--------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------

/* For transport layer */
#define CHECK_FOR_DATA() (*disptbl->check)()
#define DRAIN_TRANSMIT_FIFO() (*disptbl->drain)()
#define TRANSMIT_BYTE(c) (*disptbl->xmit)(c)

/* Stub out FLASH log support because it does not exist */
#define FLASHLOG(x, y)

//--------------------------------------------------------------------------
// Type Declarations
//--------------------------------------------------------------------------

/* List to include all the supported interfaces for the software download */
typedef enum
{
  INTERFACE_INVALID,
  INTERFACE_UART,
  INTERFACE_USB
} interface_id_type;

/* List to include all the return types for partition table command */
typedef enum
{
  PARTI_TBL_ACCEPTED,
  PARTI_TBL_DIFFERS,
  PARTI_TBL_BAD_FORMAT,
  PARTI_TBL_ERASE_FAIL,
  PARTI_TBL_UNKNOWN_ERROR
} parti_tbl_type;

/* Defines for Open Multi command */
#define OPEN_MULTI_SUCCESS             0x00
#define OPEN_MULTI_LENGTH_EXCEEDED     0x01
#define OPEN_MULTI_PAYLOAD_NOT_ALLOWED 0x02
#define OPEN_MULTI_PAYLOAD_REQUIRED    0x03
#define OPEN_MULTI_UNKNOWN_ERROR       0x04
#define OPEN_MULTI_UNKNOWN_PARTITION   0x05

/* List to use for which image to open in Open Multi mode */
typedef enum
{
  OPEN_MULTI_MODE_NONE        = 0x00,    /* Not opened yet                    */
  OPEN_MULTI_MODE_PBL         = 0x01,    /* Primary Boot Loader               */
  OPEN_MULTI_MODE_QCSBLHDCFG  = 0x02,    /* QC 2ndary Boot Loader Header and  */
                                         /*      Config Data                  */
  OPEN_MULTI_MODE_QCSBL       = 0x03,    /* QC 2ndary Boot Loader             */
  OPEN_MULTI_MODE_OEMSBL      = 0x04,    /* OEM 2ndary Boot Loader            */
  OPEN_MULTI_MODE_AMSS        = 0x05,    /* AMSS modem executable             */
  OPEN_MULTI_MODE_APPS        = 0x06,    /* APPS executable                   */
  OPEN_MULTI_MODE_OBL         = 0x07,    /* OTP Boot Loader                   */
  OPEN_MULTI_MODE_FOTAUI      = 0x08,    /* FOTA UI binarh                    */
  OPEN_MULTI_MODE_CEFS        = 0x09,    /* Modem CEFS image                  */
  OPEN_MULTI_MODE_APPSBL      = 0x0A,    /* APPS Boot Loader                  */
  OPEN_MULTI_MODE_APPS_CEFS   = 0x0B,    /* APPS CEFS image                   */
  OPEN_MULTI_MODE_FLASH_BIN   = 0x0C,    /* Flash.bin image for Windows mobile*/
  OPEN_MULTI_MODE_DSP1        = 0x0D,    /* DSP1 runtime image                */
  OPEN_MULTI_MODE_CUSTOM      = 0x0E,    /* Image for user defined partition  */
  OPEN_MULTI_MODE_DBL         = 0x0F,    /* DBL Image for SB Architecture 2.0 */
  OPEN_MULTI_MODE_OSBL        = 0x10,    /* OSBL Image for SB Architecture 2.0*/
  OPEN_MULTI_MODE_FSBL        = 0x11,    /* FSBL Image for SB Architecture 2.0*/
  OPEN_MULTI_MODE_DSP2        = 0x12,    /* DSP2 executable                   */
  OPEN_MULTI_MODE_RAW         = 0x13,    /* APPS EFS2 RAW image               */
  OPEN_MULTI_MODE_EMMC_USER   = 0x21,    /* EMMC USER partition               */
  OPEN_MULTI_MODE_EMMC_BOOT0  = 0x22,    /* EMMC BOOT partition 0             */
  OPEN_MULTI_MODE_EMMC_BOOT1  = 0x23,    /* EMMC BOOT partition 1             */
  OPEN_MULTI_MODE_EMMC_RPMB   = 0x24,    /* EMMC BOOT partition 1             */
  OPEN_MULTI_MODE_EMMC_GPP1   = 0x25,    /* EMMC GPP partition 1              */
  OPEN_MULTI_MODE_EMMC_GPP2   = 0x26,    /* EMMC GPP partition 2              */
  OPEN_MULTI_MODE_EMMC_GPP3   = 0x27,    /* EMMC GPP partition 3              */
  OPEN_MULTI_MODE_EMMC_GPP4   = 0x28     /* EMMC GPP partition 4              */

} open_multi_mode_type;

/* List to use for which image to open in Open mode. Modes
 * Bootloader download (0x01), Bootable image download (0x02),
 * CEFS Image Download (0x03) are not supported in OPEN Mode.
 */

typedef enum
{
  OPEN_MODE_NONE        = 0x00,    /* Not opened yet                    */
  OPEN_BOOTLOADER = 0x01,    /* Bootloader Image                  */
  OPEN_BOOTABLE   = 0x02,    /* Bootable Image                    */
  OPEN_CEFS       = 0x03,    /* CEFS Image                        */
  OPEN_MODE_FACTORY = 0x04   /* Factory Image                     */

} open_mode_type;

/* Status Code Enumeration
 * This lists the status result codes passed around in the program.
 * This enum is used to index a table of response packets, so these
 * values map exactly to possible responses.
 */

typedef enum
{
  ACK = 0x00,                 /* Successful                                  */
  RESERVED_1 = 0x01,          /* Reserved                                    */
  NAK_INVALID_DEST = 0x02,    /* Failure: destination address is invalid.    */
  NAK_INVALID_LEN = 0x03,     /* Failure: operation length is invalid.       */
  NAK_EARLY_END = 0x04,       /* Failure: packet was too short for this cmd. */
  NAK_INVALID_CMD = 0x05,     /* Failure: invalid command                    */
  RESERVED_6 = 0x06,          /* Reserved                                    */
  NAK_FAILED = 0x07,          /* Failure: operation did not succeed.         */
  NAK_WRONG_IID = 0x08,       /* Failure: intelligent ID code was wrong.     */
  NAK_BAD_VPP = 0x09,         /* Failure: programming voltage out of spec    */
  NAK_VERIFY_FAILED = 0x0A,   /* Failure: readback verify did not match      */
  RESERVED_0xB = 0x0B,        /* Reserved */
  NAK_INVALID_SEC_CODE = 0x0C,/* Failure: Incorrect security code            */
  NAK_CANT_POWER_DOWN = 0x0D, /* Failure: Cannot power down phone            */
  NAK_NAND_NOT_SUPP = 0x0E,   /* Failure: Download to NAND not supported     */
  NAK_CMD_OUT_SEQ = 0x0F,     /* Failure: Command out of sequence            */
  NAK_CLOSE_FAILED = 0x10,    /* Failure: Close command failed               */
  NAK_BAD_FEATURE_BITS = 0x11,/* Failure: Incompatible Feature Bits          */
  NAK_NO_SPACE = 0x12,        /* Failure: Out of space                       */
  NAK_INVALID_SEC_MODE = 0x13,/* Failure: Multi-Image invalid security mode  */
  NAK_MIBOOT_NOT_SUPP = 0x14, /* Failure: Multi-Image boot not supported     */
  NAK_PWROFF_NOT_SUPP = 0x15  /* Failure: Power off not supported            */
} response_code_type;

//--------------------------------------------------------------------------
// Extern Definitions
//--------------------------------------------------------------------------
/* global pointer to current function dispatch table */
extern DISPPTR disptbl;

//--------------------------------------------------------------------------
// Function Definitions
//--------------------------------------------------------------------------

/*===========================================================================

  This function is the main loop implementing the DMSS Async Download
  Protocol.  It loops forever, processing packets as they arrive.

===========================================================================*/

extern void packet_loop (void);
extern void packet_init (void);

/*===========================================================================

  This function causes the MSM to reset

===========================================================================*/
extern void target_reset (void);

#ifdef USE_BUFFERED_TERMIO
  extern void jprintf_init (void);
  extern void term_flush (void);
#endif

//--------------------------------------------------------------------------
// Mapping of QPST image ID to partition type
//--------------------------------------------------------------------------
#define PARTITION_IMAGE_TYPE_EXTENDED    0x05
#define PARTITION_IMAGE_TYPE_FAT32       0x0b
#define PARTITION_IMAGE_TYPE_FAT32L      0x0c
#define PARTITION_IMAGE_TYPE_FAT16       0x0e
#endif /* __EMMCBLD_H_ */
