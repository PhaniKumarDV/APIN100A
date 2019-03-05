#ifndef __EMMCBLD_MEMCTL_H
#define __EMMCBLD_MEMCTL_H

/*================================================================================================
 *
 * FILE:        emmcbld_memctl.h
 *
 * SERVICES:    Main memory controller for programming public interface header file
 *
 * DESCRIPTION:
 *    This file contains the interface to the memory controller.  
 *
 *        Copyright © 2009-2010 Qualcomm Technologies Incorporated.
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
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_memctl.h#1 $ 
 *   $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2011-09-21   ab      Add GPP image types, and partition info command
 * 2010-06-23   rh      Simplify the type of partition being programmed
 * 2009-10-27   rh      Initial Revision
 *===============================================================================================*/

/*===================================================================
 *
 *                     Include Files
 *
 ====================================================================*/

#include "comdef.h"

//--------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------
#define MAX_NUMBER_GPP 8
#define GPP_MAGIC_NUMBER 0xDEAD
#define BLOCK_SIZE                0x200

//--------------------------------------------------------------------------
// Type Declarations
//--------------------------------------------------------------------------

/* A ENUM used to mark which image is currently being programmed */
typedef enum
{
  IMAGE_USER_ENTIRE,
  IMAGE_BOOT1_ENTIRE,
  IMAGE_BOOT2_ENTIRE,
  IMAGE_GPP1_ENTIRE,
  IMAGE_GPP2_ENTIRE,
  IMAGE_GPP3_ENTIRE,
  IMAGE_GPP4_ENTIRE,
  IMAGE_NO_PROG                     /* Any image that do not need programming */
} opened_image_type;

/* Public definition for dispatch table structure containing
 * all pertinent information about a particular flash device.
 * Detailed definition is in flash private header file.
 */

struct flash_device_data;
typedef struct flash_device_data *flash_device_t;


/* The Flash device is, of course, memory mapped, so the compiler might
   find the various device manipulations to be nonsense, and optimize
   it all away.  To prevent this, we use pointers to volatile to let
   the compiler know that we're dealing with a special device and that
   it is to leave the operations as written. */

typedef volatile word *flash_ptr_type;


//--------------------------------------------------------------------------
// Extern Definitions
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Function Definitions
//--------------------------------------------------------------------------

/*===========================================================================

  This function writes a provided string of bytes into a specified
  block of memory.

  If the block is in RAM space, the only restriction is that the
  write is not permitted to overlay this program.

  If the block is in Flash space, the write will be attempted.  The
  updated memory will then be verified to confirm that the write
  operation succeeded.  It might fail because the area had not been
  erased, or because it has worn out, or because it is in the
  hardware-lockable boot block.

===========================================================================*/

response_code_type do_simple_write (byte * buf, dword addr, word len);


/*===========================================================================

  This function writes a provided string of bytes into a specified
  block of memory and handles erase of any blocks as necessary
  before writing.


  If the block is in Flash space, the write will be attempted.  The
  updated memory will then be verified to confirm that the write
  operation succeeded.  It might fail because the area had not been
  erased, or because it has worn out, or because it is in the
  hardware-lockable boot block.

===========================================================================*/

response_code_type do_stream_write (byte * buf, dword addr, word len);

/*===============================================================================================

  This function creates General Purpose Partitions

===============================================================================================*/

response_code_type do_create_gpp (dword *gpp_sizes);

/*===============================================================================================

  This function erases the opened partition.

===============================================================================================*/

response_code_type do_erase (byte *erase_buffer, uint32 erase_buffer_length);

/*===========================================================================

  This function sets the active boot partition.

===========================================================================*/

response_code_type set_active_boot(void);

/*===========================================================================

  This function writes a buffer to a given sector.

===========================================================================*/

response_code_type do_write(dword start_sector, dword num_sectors, byte * buf);

/*===========================================================================

  This function reads string of bytes into a specified
  block of memory.

===========================================================================*/

response_code_type do_read (byte * buf, dword addr, word len);

/*===========================================================================

  This function returns info about the opened physical
  partition.

===========================================================================*/

response_code_type do_partition_info (byte * buf, uint32 *response_length);


/*===========================================================================

  This function returns a pointer to a string containing the flash
  device name

===========================================================================*/

char * storage_device_name (void);


/*===========================================================================

  This function fills the indicated handle_hello response buffer with the
  number of sectors, and a length for each sector.

===========================================================================*/

int storage_device_sectors (uint8 *buf);


/*===========================================================================

  This function does one time initialization of data structures used in
  flash device access

===========================================================================*/

void flash_device_init_data (void);


/*===========================================================================

  This function is a stub for a function needed in the next revision of
  the protocol.

===========================================================================*/

response_code_type do_open (byte mode);


/*===========================================================================

  This function is a stub for a function needed in the next revision of
  the protocol.

===========================================================================*/

response_code_type do_close (void);

/*===========================================================================

  This function Sends the security mode to the layer below

===========================================================================*/
response_code_type do_security_mode (byte mode);


/*===========================================================================

  This function Sends the partition table to the layer below

===========================================================================*/
response_code_type do_partition_table (byte *data, byte override, word len);


/*===========================================================================

  This function handles the open for all modes.

===========================================================================*/
response_code_type do_open_multi (byte *data, byte mode, uint32 length);


#endif /* __EMMCBLD_MEMCTL_H */
