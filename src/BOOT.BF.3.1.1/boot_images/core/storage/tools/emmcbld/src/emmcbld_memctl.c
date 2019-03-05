/*================================================================================================
 *
 * FILE:        emmcbld_memctl.c
 *
 * SERVICES:    Control routines for programming the memory device
 *
 * DESCRIPTION:
 *    This module implements the commands to program the MMC/SD memory card.  The code is 
 *    written with reference to dload_flash code.
 *   
 *
 *        Copyright © 2009-2011 Qualcomm Technologies Incorporated.
 *               All Rights Reserved.
 *            QUALCOMM Proprietary/GTDR
 *===============================================================================================*/

/*================================================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_memctl.c#1 $ 
 *   $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2011-09-21   ab      Add support for writing to GPPs, sector addresses, partition info command
 * 2011-03-11   vj      Made do_open_device() static
 * 2010-08-26   rh      Adding the newly allocated image ID for eMMC
 * 2010-06-23   rh      Add Single binary programming and partition update
 * 2010-03-03   vj      Detect the table entry based on partition type
 * 2010-01-12   vj      Added Support for 7x27 target
 * 2010-01-27   rh      Adjust the boot loader allocated partition size
 * 2009-10-27   rh      Initial Revision
 *===============================================================================================*/

/*================================================================================================
 *
 *                     Include Files
 *
 ================================================================================================*/

#include "comdef.h"
#include "emmcbld.h"
#include "emmcbld_memctl.h"
#include "emmcbld_packet.h"
#include "emmcbld_msm.h"
#include "emmcbld_debug.h"
#include "emmcbld_bsp.h"
#include "sdcc_api.h"
#include <string.h>

//-----------------------------------------------------------------------------------------------
// External Declarations
//-----------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------------------------
/* This is the number of sectors which will fit in the reply buffer.
 * This really should be done by calculations, but due to the nature of
 * the parameter request reply, it must be done by estimation.  This is
 * because one part of the reply is variable length, and that is the flash
 * name.  So, take all the fixed length fields and add up their length, then
 * take a very long length for the flash name and add that to it.  Subtract
 * this value from the reply length buffer size.  Now you have the number of
 * bytes that are available for storing sector sizes.  Divide this by 4
 * to get the number of sectors we can put in, and then subtract some to be
 * absolutely sure we stay below the limit.
 */

#define MAX_BLOCKS_IN_REPLY 200

/* Some simple macro for accessing memory */
#define GET_LWORD_FROM_BYTE(x)    ((uint32)*(x) | \
                                  ((uint32)*(x+1) << 8) | \
                                  ((uint32)*(x+2) << 16) | \
                                  ((uint32)*(x+3) << 24))

#define PUT_LWORD_TO_BYTE(x, y)   do{*(x) = y & 0xff;     \
                                     *(x+1) = (y >> 8) & 0xff;     \
                                     *(x+2) = (y >> 16) & 0xff;     \
                                     *(x+3) = (y >> 24) & 0xff; }while(0)    

/* Some constants for partition table construction */
#define TABLE_ENTRY_0             0x1BE
#define TABLE_ENTRY_1             0x1CE
#define TABLE_ENTRY_2             0x1DE
#define TABLE_ENTRY_3             0x1EE
#define TABLE_SIGNATURE           0x1FE
#define TABLE_ENTRY_SIZE          0x010

#define OFFSET_STATUS             0x00
#define OFFSET_TYPE               0x04
#define OFFSET_FIRST_SEC          0x08
#define OFFSET_SIZE               0x0C

#define MBR_SIZE                  512
#define SBL_HEADER_SIZE           512
#define ACTIVE                    0x80

#define LOCATE_FAIL               0
#define SIZE_IN_BLOCKS(x)         ( ((x % BLOCK_SIZE) == 0) ? x/BLOCK_SIZE : x/BLOCK_SIZE+1 )

//-----------------------------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------------------------
opened_image_type curr_opened_image;         /* Current image being programmed */
uint32 curr_opened_offset;                   /* Offset of the image being programmed */
struct sdcc_device *hsdev_handle_user = NULL;       /* Memory device partition handle - User space */
struct sdcc_device *hsdev_handle_boot1 = NULL;      /* Memory device partition handle - Boot1 */
struct sdcc_device *hsdev_handle_boot2 = NULL;      /* Memory device partition handle - Boot2 */
struct sdcc_device *hsdev_handle_gpp1 = NULL;
struct sdcc_device *hsdev_handle_gpp2 = NULL;
struct sdcc_device *hsdev_handle_gpp3 = NULL;
struct sdcc_device *hsdev_handle_gpp4 = NULL;
byte mbr_buffer[MBR_SIZE];                   /* Memory buffer for construction MBR */
byte sblhd_buffer[SBL_HEADER_SIZE];          /* Memory buffer for construction SBL Header */
boolean sector_addresses_enabled;

extern char log_msg_buffer[];
char *sdcc_err_string[] = {
    "SDCC_NO_ERROR",
    "SDCC_ERR_UNKNOWN",
    "SDCC_ERR_CMD_TIMEOUT",
    "SDCC_ERR_TIMEOUT",
    "SDCC_ERR_CMD_CRC_FAIL",
    "SDCC_ERR_DATA_CRC_FAIL",
    "SDCC_ERR_CMD_SENT",
    "SDCC_ERR_PROG_DONE",
    "SDCC_ERR_CARD_READY",
    "SDCC_ERR_INVALID_TX_STATE",
    "SDCC_ERR_SET_BLKSZ",
    "SDCC_ERR_SDIO_R5_RESP",
    "SDCC_ERR_DMA",
    "SDCC_ERR_READ_FIFO",
    "SDCC_ERR_WRITE_FIFO",
    "SDCC_ERR_ERASE",
    "SDCC_ERR_SDIO",
    "SDCC_ERR_SDIO_READ",
    "SDCC_ERR_SDIO_WRITE",
    "SDCC_ERR_SWITCH",
    "SDCC_ERR_INVALID_PARAM",
    "SDCC_ERR_CARD_UNDETECTED",
    "SDCC_ERR_FEATURE_UNSUPPORTED",
    "SDCC_ERR_SECURE_COMMAND_IN_PROGRESS",
    "SDCC_ERR_READ_SEC_CMD_NOT_ALLOWED",
    "SDCC_ERR_ABORT_READ_SEC_CMD",
    "SDCC_ERR_CARD_INIT",
    "SDCC_ERR_CARD_REMOVED",
    "SDCC_ERR_PWR_ON_WRITE_PROT",
    "SDCC_ERR_WP_VIOLATION",
    "SDCC_ERR_SPS_MODE_USED",
    "SDCC_ERR_DML_INIT",
    "SDCC_ERR_SPS_GET_EVENT",
    "SDCC_ERR_SPS_WRITING_DESCRIPTOR"
};

//-----------------------------------------------------------------------------------------------
// Function Declarations
//-----------------------------------------------------------------------------------------------


/*===========================================================================

DESCRIPTION
  This function does a search though the software partition table to 
  locate the starting point of the given partition type.  A return of 0 
  will indicate partition not found.

RETURN VALUE
  If success, return the starting sector of the givin partition, if
  failed, return 0

===========================================================================*/
uint32 partition_locate_by_type (uint32 srch_type)
{
   byte sector_buf[512];
   SDCC_STATUS rc;
   uint32 type = 0;
   uint32 start_sec = 0;
   uint32 next_ebr_sec = 0;
   uint32 first_ebr_sec = 0;
   uint32 curr_prt = PARTITION_IMAGE_TYPE_EXTENDED;

   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PART_LOCATE);

   /* Read in the MBR sector first */
   rc = sdcc_handle_read (hsdev_handle_user, 0, sector_buf, 1);
   if (rc != SDCC_NO_ERROR) 
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
      return LOCATE_FAIL;
   }

   /* See if the partition trying to locate is the first 4 partition */
   /* Read the first partition location */
   type = sector_buf[TABLE_ENTRY_0 + OFFSET_TYPE];
   start_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_0 + OFFSET_FIRST_SEC);
   if (type == srch_type)
      return start_sec;

   /* Continue... 2nd */
   type = sector_buf[TABLE_ENTRY_1 + OFFSET_TYPE];
   start_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_1 + OFFSET_FIRST_SEC);
   if (type == srch_type)
      return start_sec;
   
   /* Continue... 3rd */
   type = sector_buf[TABLE_ENTRY_2 + OFFSET_TYPE];
   start_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_2 + OFFSET_FIRST_SEC);
   if (type == srch_type)
      return start_sec;

   /* Continue... 4th */
   type = sector_buf[TABLE_ENTRY_3 + OFFSET_TYPE];
   start_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_3 + OFFSET_FIRST_SEC);
   if (type == srch_type)
      return start_sec;

   /* Traverse the link list of EBR to find the correct partition */
   curr_prt = PARTITION_IMAGE_TYPE_EXTENDED;

   /* If the 4th primary partition is not EBR, this is an error */
   type = sector_buf[TABLE_ENTRY_3 + OFFSET_TYPE];
   first_ebr_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_3 + OFFSET_FIRST_SEC);
   next_ebr_sec = 0;

   if(type != PARTITION_IMAGE_TYPE_EXTENDED)
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
      return LOCATE_FAIL;
   }

   next_ebr_sec = 0;
   for(;;)
   {
      /* Read in the EBR pointed by the MBR or last EBR */
      rc = sdcc_handle_read (hsdev_handle_user, next_ebr_sec + first_ebr_sec, sector_buf, 1);
      if (rc != SDCC_NO_ERROR) 
      {
         BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
         return LOCATE_FAIL;
      }

      /* Check the current partition to see if this is what we wanted */
      type = sector_buf[TABLE_ENTRY_0 + OFFSET_TYPE];
      start_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_0 + OFFSET_FIRST_SEC);
      start_sec += first_ebr_sec + next_ebr_sec;
      if (type == srch_type)
         return start_sec;

      /* Time to find the next EBR */
      type = sector_buf[TABLE_ENTRY_1 + OFFSET_TYPE];
      next_ebr_sec = GET_LWORD_FROM_BYTE (sector_buf + TABLE_ENTRY_1 + OFFSET_FIRST_SEC);
      curr_prt++;
      if(type != PARTITION_IMAGE_TYPE_EXTENDED)
      {
         /* Reaching the end of the EBR table link list without finding the wanted partition */
         BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
         return LOCATE_FAIL;
      }
   }
}  /* partition_locate_by_type */



/*===============================================================================================

  This function does one time initialization of data structures used in
  flash device access

===============================================================================================*/

void flash_device_init_data (void)
{
   /* Don't have anything that need to be done this point in time */
}


/*===============================================================================================

  This function returns a pointer to a string containing the flash
  device name

===============================================================================================*/

char * storage_device_name (void)
{
   char *storage_name={"eMMC"};
   return storage_name;
}

/*===============================================================================================

  This function fills the indicated handle_hello response buffer with the
  number of sectors, and a length for each sector.

===============================================================================================*/

int storage_device_sectors (uint8 *reply_buffer)
{
   uint32 nbytes;
   int i, p;
   int numblocks, pages, pagesize;

   p = 0;

   /* Hard code the device sector information - as it is not really used */
   numblocks = 0x0800;
   pages = 0x40;
   pagesize = 0x0800;

   /*  Adjust blocks and pages so we don't overflow reply buffer.
    *  The number of blocks and the size of the blocks in reality
    *  only need to add up to the actual space available.  The host tool
    *  only uses these in a progress message, and never uses this
    *  information in reality to manipulate anything else.  So, we keep
    *  shifting down the number blocks while shifting up the size of
    *  the blocks until the number of blocks will fit in the reply buffer.
    */
   while (numblocks > MAX_BLOCKS_IN_REPLY)
   {
      numblocks = numblocks >> 1;
      pages = pages << 1;
   }

   /* Calculate new "sector size from adjusted number of pages */
   nbytes = pages * pagesize;

   /* Store the adjusted block count */
   reply_buffer[p++] = (numblocks) & 0xFF;
   reply_buffer[p++] = (numblocks >> 8) & 0xFF;

   /* Finally, store one block size unit for every "adjusted block" */
   for (i = 0; i < numblocks; i++)
   {
      reply_buffer[p++] = (nbytes) & 0xFF;
      reply_buffer[p++] = (nbytes >> 8) & 0xFF;
      reply_buffer[p++] = (nbytes >> 16) & 0xFF;
      reply_buffer[p++] = (nbytes >> 24) & 0xFF;
   }

   return p;
}


/*===============================================================================================

  This function Sends the security mode - Not required for MMC

===============================================================================================*/

response_code_type do_security_mode (byte mode)
{
   return ACK;
}

/*===============================================================================================

  This function Sends the partition table - Use this chance to setup
  a simple partition table

===============================================================================================*/

response_code_type do_partition_table (byte *data, byte override, word size)
{

   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_DO_PARTITION);


   return ACK;
}


/*===============================================================================================

  This function handles the open for factory image mode - Not really used.

===============================================================================================*/

response_code_type do_open (byte mode)
{
   FREEZE_WATCHDOG();
   
   /* Set status with the command to initialize */
   switch (mode)
   {
      case OPEN_MODE_FACTORY:
         break;
      case OPEN_BOOTLOADER:
      case OPEN_BOOTABLE:
      case OPEN_CEFS:
      case OPEN_MODE_NONE:
      default:
      {
          FLASHLOG(2,("do_open: open for INVALID mode\n"));
          return NAK_FAILED;
      }
   }
   
   return ACK;
}


/*===============================================================================================

  This function opens the memory card device

===============================================================================================*/

static response_code_type do_open_device ()
{
   SDCC_STATUS rc;
   int16   driveno = EMMCBLD_USE_DRIVENO;

	/* Only continue with the open process if handle structure is NULL */
   if(hsdev_handle_user != NULL)
	{
      return ACK;
	}

   /* Initialize the SDCC controller */
   FREEZE_WATCHDOG();

   hsdev_handle_user = sdcc_handle_open (driveno, 0);

   if(hsdev_handle_user != NULL)
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_SUCCESS);
   }
   else
   {
      /* Opening the device failed */
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
      return NAK_FAILED;
   }

   /* Set the active boot partition to default to user partition */
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_SET_ACTIVE_PRTI);
   FREEZE_WATCHDOG();
   rc = sdcc_handle_set_active_bootable_partition (hsdev_handle_user);

   if (rc == SDCC_ERR_FEATURE_UNSUPPORTED) 
   {
      /* If the chip does not support partition setting - not opening boot partition */
      hsdev_handle_boot1 = NULL;
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_SUCCESS);
      return ACK;
   }
   if (rc != SDCC_NO_ERROR) 
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
      return NAK_FAILED;
   }

   /* Open the boot partition 1 */
   FREEZE_WATCHDOG();
   hsdev_handle_boot1 = sdcc_handle_open (driveno, 1);
   
   /* Open the boot partition 2 */
   FREEZE_WATCHDOG();
   hsdev_handle_boot2 = sdcc_handle_open (driveno, 2);
   
   /* Open the GPP 1 */
   FREEZE_WATCHDOG();
   hsdev_handle_gpp1 = sdcc_handle_open (driveno, 4);
   
   /* Open the GPP 2 */
   FREEZE_WATCHDOG();
   hsdev_handle_gpp2 = sdcc_handle_open (driveno, 5);
   
   /* Open the GPP 3 */
   FREEZE_WATCHDOG();
   hsdev_handle_gpp3 = sdcc_handle_open (driveno, 6);
   
   /* Open the GPP 4 */
   FREEZE_WATCHDOG();
   hsdev_handle_gpp4 = sdcc_handle_open (driveno, 7);

   if(hsdev_handle_user != NULL)
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_SUCCESS);
   }
   else
   {
      /* Opening the boot partition failed - continue as
       * the code will detect to see if boot partition is available later */
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
   }

   return ACK;
}

/*===============================================================================================

  This function handles the open for all modes.

===============================================================================================*/

response_code_type do_open_multi (byte *data, byte mode, uint32 length)
{
   uint32 startloc = 0;

	if (do_open_device () == NAK_FAILED)
      return NAK_FAILED;

   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_HIGHLIGHT);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_HIGHLIGHT);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_DO_OPEN_MULTI_PL);
   BPROFILE_WRITE_BMRK (mode);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PACKET_DDUMP);
   BPROFILE_DUMP_DATA (length, data);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PACKET_DDUMP_END);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PACKET_DDUMP_END);
      
   switch (mode)
   {
      case OPEN_MULTI_MODE_QCSBLHDCFG:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_QCSBL:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_OEMSBL:
         curr_opened_image = IMAGE_BOOT1_ENTIRE;
         curr_opened_offset = 0;
         break;
      
      case OPEN_MULTI_MODE_DBL:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_FSBL:
         curr_opened_image = IMAGE_USER_ENTIRE;
         /* FSBL is being remapped to FAT image update */
         startloc = partition_locate_by_type (PARTITION_IMAGE_TYPE_FAT32);
         if (startloc != LOCATE_FAIL)
         {
            curr_opened_offset = startloc;
            break;
         }
         startloc = partition_locate_by_type (PARTITION_IMAGE_TYPE_FAT32L);
         if (startloc != LOCATE_FAIL)
         {
            curr_opened_offset = startloc;
            break;
         }
         startloc = partition_locate_by_type (PARTITION_IMAGE_TYPE_FAT16);
         if (startloc != LOCATE_FAIL)
         {
            curr_opened_offset = startloc;
            break;
         }
         return NAK_FAILED;
      
      case OPEN_MULTI_MODE_OSBL:
         curr_opened_image = IMAGE_BOOT1_ENTIRE;
         curr_opened_offset = 0;
         break;
      
      case OPEN_MULTI_MODE_AMSS:
         curr_opened_image = IMAGE_USER_ENTIRE;
         curr_opened_offset = 0;
         break;
      
      case OPEN_MULTI_MODE_APPSBL:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_APPS:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_CEFS:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_APPS_CEFS:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_DSP1:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_CUSTOM:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_FLASH_BIN:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_DSP2:
         curr_opened_image = IMAGE_NO_PROG;
         break;
      
      case OPEN_MULTI_MODE_RAW:
         curr_opened_image = IMAGE_NO_PROG;
         break;

      case OPEN_MULTI_MODE_EMMC_USER:
         curr_opened_image = IMAGE_USER_ENTIRE;
         curr_opened_offset = 0;
         break;
      
      case OPEN_MULTI_MODE_EMMC_BOOT0:
         curr_opened_image = IMAGE_BOOT1_ENTIRE;
         curr_opened_offset = 0;
         break;

      case OPEN_MULTI_MODE_EMMC_BOOT1:
         curr_opened_image = IMAGE_BOOT2_ENTIRE;
         curr_opened_offset = 0;
         break;

      case OPEN_MULTI_MODE_EMMC_GPP1:
         curr_opened_image = IMAGE_GPP1_ENTIRE;
         curr_opened_offset = 0;
         break;

      case OPEN_MULTI_MODE_EMMC_GPP2:
         curr_opened_image = IMAGE_GPP2_ENTIRE;
         curr_opened_offset = 0;
         break;

      case OPEN_MULTI_MODE_EMMC_GPP3:
         curr_opened_image = IMAGE_GPP3_ENTIRE;
         curr_opened_offset = 0;
         break;

      case OPEN_MULTI_MODE_EMMC_GPP4:
         curr_opened_image = IMAGE_GPP4_ENTIRE;
         curr_opened_offset = 0;
         break;
      
      /* Unknown mode, failure */
      default:
      {
         FLASHLOG(2,("do_open_multi: open for INVALID mode\n"));
         return NAK_FAILED;
      }
   }
   
   return ACK;
}

/*===============================================================================================

  This function is usually called after programming - 
  Don't need to do anything for RAM

===============================================================================================*/

response_code_type do_close ()
{
   return ACK;
}


/*===============================================================================================

  This function writes a provided string of bytes into a specified
  block of memory, by calling the appropriate device-specific function.
  For NOR flash, this is not really necessary, however, the interface is here
  to support other flash types that need the differentation between write,
  which requires a prior erase and simple_write which does not.

===============================================================================================*/

response_code_type do_simple_write (byte *buf, dword addr, word len)
{
   /* We do not do simple_write, but do not want to fail when it is
    * requested.
    */
   return ACK;
}

/*===============================================================================================

  This function writes a provided string of bytes into a specified
  block of memory.  Depending on the value of curr_opened_image
  the data is programmed into differet location in memory.

===============================================================================================*/

response_code_type do_stream_write (byte *buf, dword addr, word len)
{
#ifndef FEATURE_EMMCBLD_ENABLE_GPP
   dword start_sec = 0;
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;
   boolean boot_part_bootable = FALSE;       /* Possible to boot off the boot partition? */

   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_HIGHLIGHT);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_HIGHLIGHT);
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_DO_STREAM_WRITE);
   BPROFILE_WRITE_BMRK ((uint16)addr);

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return ACK;
      case IMAGE_BOOT1_ENTIRE:
         /* OSBL image is designed to be loaded into boot partition */
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         /* AMSS image is designed to be loaded into user partition */
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   /* If incoming data is the MBR block and BOOT partition is being programmed */
   if (curr_opened_image == IMAGE_BOOT1_ENTIRE && addr == 0)
   {
      /* Check to see if boot partition contain software partition that is bootable */
      boot_part_bootable = (buf[OFFSET_STATUS + TABLE_ENTRY_0] == ACTIVE ||
                            buf[OFFSET_STATUS + TABLE_ENTRY_1] == ACTIVE ||
                            buf[OFFSET_STATUS + TABLE_ENTRY_2] == ACTIVE ||
                            buf[OFFSET_STATUS + TABLE_ENTRY_3] == ACTIVE);
      if (boot_part_bootable && hsdev_handle_boot1 != NULL)
      {
         /* If the boot partition contain a bootable software partition, enable booting */
         rc = sdcc_handle_set_active_bootable_partition (hsdev_handle_boot1);
         if (rc != SDCC_NO_ERROR) 
         {
            BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
            return NAK_FAILED;
         }
      }
   }
   
   /* Address is given in sectors */
   if (TRUE == sector_addresses_enabled)
      start_sec += (addr);
   else
      start_sec += (addr / BLOCK_SIZE);
   start_sec += curr_opened_offset;

   /* Blast the data down into the memory card */
   /* If length to be written is not a multiple of 512 bytes, round it up to multiple of 512 */
   /* Disable the watchdog timer prior to entering SDCC function call */
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PACKET_WRITE);
   FREEZE_WATCHDOG();
   rc = sdcc_handle_write (hsdev_handle, start_sec, buf, 
                           (len / BLOCK_SIZE) + (((len % BLOCK_SIZE) > 0) ? 1 : 0));
   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_PACKET_WRITE_DONE);

   if (rc != SDCC_NO_ERROR) 
   {
      BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_FAIL);
      return NAK_FAILED;
   }

   BPROFILE_WRITE_BMRK (EMMCBLD_BMRK_RC_SUCCESS);
   return ACK;
#else
   return NAK_FAILED;
#endif
}

/*===============================================================================================

  This function creates General Purpose Partitions

===============================================================================================*/

response_code_type do_create_gpp (dword *gpp_sizes)
{
#ifdef FEATURE_EMMCBLD_ENABLE_GPP
   boolean rc;
   sdcc_emmc_gpp_enh_desc_type desc;
   struct sdcc_device *hsdev_handle = NULL;

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return ACK;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   desc.GPP_size[0] = gpp_sizes[0];
   desc.GPP_size[1] = gpp_sizes[1];
   desc.GPP_size[2] = gpp_sizes[2];
   desc.GPP_size[3] = gpp_sizes[3];
   desc.ENH_size = 0;
   desc.ENH_start_addr = 0;
   desc.GPP_enh_flag = 0;

   FREEZE_WATCHDOG();
   rc = sdcc_config_emmc_gpp_enh (hsdev_handle, &desc);

   if (rc != SDCC_NO_ERROR)
   {
     snprintf(log_msg_buffer, 128, "GPP creation failed. SDCC Error %d: %s", rc, sdcc_err_string[rc]);
     return NAK_FAILED;
   }
   return ACK;
#else
   return NAK_FAILED;
#endif
}

/*===============================================================================================

  This function erases the opened partition.

===============================================================================================*/

response_code_type do_erase (byte *erase_buffer, uint32 erase_buffer_length)
{
#ifndef FEATURE_EMMCBLD_ENABLE_GPP
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;
   sdcc_mem_info_type mem_info;

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return ACK;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   FREEZE_WATCHDOG();
   rc = sdcc_handle_mem_get_device_info(hsdev_handle, &mem_info);
   if (rc != SDCC_NO_ERROR) 
   {
       return NAK_FAILED;
   }
   if (curr_opened_image == IMAGE_USER_ENTIRE)
        rc = sdcc_handle_erase(hsdev_handle, 0, mem_info.card_size_in_sectors - 1);
   else {
        /* erase first and last sectors of the boot partition */
        memset(erase_buffer, 0x00, BLOCK_SIZE);
        rc = sdcc_handle_write(hsdev_handle, 0x0, erase_buffer, 0x1);
        if (rc != SDCC_NO_ERROR) 
        {
            return NAK_FAILED;
        }
        rc = sdcc_handle_write(hsdev_handle, mem_info.card_size_in_sectors-1, erase_buffer, 0x1);
        if (rc != SDCC_NO_ERROR) 
        {
            return NAK_FAILED;
        }
   }
   if (rc != SDCC_NO_ERROR)
   {
       return NAK_FAILED;
   }
   return ACK;
#else
   return NAK_FAILED;
#endif
}

/*===========================================================================

  This function sets the active boot partition.

===========================================================================*/

response_code_type set_active_boot(void)
{
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return NAK_FAILED;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   FREEZE_WATCHDOG();
   rc = sdcc_handle_set_active_bootable_partition( hsdev_handle );
   if (rc != SDCC_NO_ERROR) 
   {
       return NAK_FAILED;
   }

   return ACK;
}

/*===========================================================================

  This function writes a buffer to a given sector.

===========================================================================*/
response_code_type do_write(dword start_sector, dword num_sectors, byte * buf)
{
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return NAK_FAILED;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   FREEZE_WATCHDOG();
   rc = sdcc_handle_write(hsdev_handle, start_sector, buf, num_sectors);
   if (rc != SDCC_NO_ERROR) 
   {
     return NAK_FAILED;
   }

   return ACK;
}

/*===============================================================================================

  This function reads string of bytes into a specified
  block of RAM memory, not FLASH.  It is used by QPST to read the mobile
  model number from the running build.

===============================================================================================*/

response_code_type do_read (byte * buf, dword addr, word length)
{
   /*int i, j;

   for (i=0, j=0; j < length; j++)
   {
      buf[i++] = ((byte *) addr)[j];
   }*/
#ifdef FEATURE_EMMCBLD_ENABLE_READ
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;
   
   length = SIZE_IN_BLOCKS(length);
   if (FALSE == sector_addresses_enabled) {
       addr = SIZE_IN_BLOCKS(addr);
   }
   
   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return ACK;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   FREEZE_WATCHDOG();
   rc = sdcc_handle_read(hsdev_handle, addr, buf, length);
   if (rc != SDCC_NO_ERROR) 
   {
       return NAK_FAILED;
   }
   return ACK;
#else
   return NAK_FAILED;
#endif
}

response_code_type do_partition_info (byte * buf, uint32 *response_length)
{
   SDCC_STATUS rc;
   struct sdcc_device *hsdev_handle = NULL;
   sdcc_mem_info_type mem_info;

   switch (curr_opened_image)
   {
      default:
      case IMAGE_NO_PROG:
         /* In the case where programming is not done - return right away */
         return ACK;
      case IMAGE_BOOT1_ENTIRE:
         hsdev_handle = hsdev_handle_boot1;
         break;
      case IMAGE_BOOT2_ENTIRE:
         hsdev_handle = hsdev_handle_boot2;
         break;
      case IMAGE_GPP1_ENTIRE:
         hsdev_handle = hsdev_handle_gpp1;
         break;
      case IMAGE_GPP2_ENTIRE:
         hsdev_handle = hsdev_handle_gpp2;
         break;
      case IMAGE_GPP3_ENTIRE:
         hsdev_handle = hsdev_handle_gpp3;
         break;
      case IMAGE_GPP4_ENTIRE:
         hsdev_handle = hsdev_handle_gpp4;
         break;
      case IMAGE_USER_ENTIRE:
         hsdev_handle = hsdev_handle_user;
         break;
   }

   /* Check - Fail with error if handle is not available */
   if (hsdev_handle == NULL)
   {
      return NAK_FAILED;
   }

   /* Check whether the info will fit in the reply buffer while leaving */
   /* 1 byte for the response command and 2 bytes for the CRC */
   if (sizeof(sdcc_mem_info_type) > REPLY_BUFFER_SIZE-3)
      return NAK_FAILED;

   FREEZE_WATCHDOG();
   rc = sdcc_handle_mem_get_device_info(hsdev_handle, &mem_info);
   if (rc != SDCC_NO_ERROR) 
   {
      return NAK_FAILED;
   }
   alignmemcpy(buf, &mem_info, sizeof(mem_info));
   *response_length =  *response_length + sizeof(mem_info);
   return ACK;
}
