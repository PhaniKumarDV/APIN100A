/*==================================================================
 *
 * FILE:        emmcbld_packet.c
 *
 * SERVICES:    Logging for NAND client via T32 DCC
 *
 * DESCRIPTION:
 *   This module handles the DMSS Async Download Protocol to download
 *   code using simple generic UART/USB services.  This consists of an
 *   infinite loop of receiving a command packet through the UART/USB,
 *   acting on it, and returning a response packet.
 *
 *   Actual block memory operations are not part of this module.
 *   These functions (named do_write and do_erase) are responsible
 *   for checking the received fields for validity.
 *
 *   Everything in this file is MSM independent.
 *
 * INITIALIZATION AND SEQUENCING REQUIREMENTS
 *   All necessary initialization for normal CPU operation must have
 *   been performed, and the UART/USB must have been initialized, before
 *   entering this module.  Dispatch table to correct transport (UART or USB)
 *   must have been filled in.
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
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_packet.c#1 $ 
 *   $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2011-09-21   ab      Add support for partition info command, and sector addresses
 * 2011-06-27   ab      Added alignmemcpy() to fix RVCT4.1 memcpy() issues
 * 2011-02-14   rh      Removed "inc" in include path since now handled by scons
 * 2010-10-29   rh      Adding support for loading external modules
 * 2010-06-23   rh      Moving watchdog related function to BSP
 * 2010-01-29   rh      Use the watchdog timer to reset the phone
 * 2010-01-12   vj      Added Support for 7x27 target
 * 2009-11-23   rh      Ported the code for emmc Boot Loader Downloader
 * 2009-05-07   sv      Hostdl Optimization
 * 2008-10-29   mm      Clean up
 *==================================================================*/

/*===================================================================
 *
 *                     Include Files
 *
 ====================================================================*/

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "stringl.h"
#include "comdef.h"
#include "emmcbld.h"
#include "emmcbld_memctl.h"
#include "crc.h"
#include "emmcbld_packet.h"
#include "emmcbld_msm.h"
#include "emmcbld_bsp.h"
#include "emmcbld_debug.h"
#include "emmcbld_packet_api.h"

struct incoming_data packet_data[NUMBER_OF_PACKETS];

/* We need 4 bytes extra for adjustment purposes. */
ALIGN(4) byte aligned_buffer[MAX_DATA_LENGTH + 4];

/* Lists. */
incoming_t free_list;           /* Unused buffers. */
incoming_t waiting;             /* Waiting to be processed. */
incoming_t waiting_last;        /* Last waiting packet (for queuing). */
incoming_t current;             /* The current packet for incoming
                                   data. */
/* Set to true after an escape character. */
static byte escape_state = 0;

/* True indicates that we need our hello packet before seeing
   anything. */
static byte need_hello = 1;

/* Reply packet. */
byte reply_buffer[REPLY_BUFFER_SIZE];
uint32 reply_length;
byte reply_buffer_temp[REPLY_BUFFER_SIZE];

/* Flag to indicate active packet processing, only used by USB */
boolean process_packet_flag = FALSE;

/* Used by debug only code that simulates dropped ACKS */
#ifdef DROP_ACKS
static uint32 ack_drop_count = 0;
#endif

/* Used by debug only code that simulates dropped packets */
#ifdef DROP_PACKETS
static uint32 drop_count = 0;
#endif

/* Variables used in Multi-Image mode extended commands */
static int security_mode_rcvd = FALSE;
static int partition_table_rcvd = TRUE; // By default set to true since this is optional does nothing
static open_multi_mode_type open_multi_mode = OPEN_MULTI_MODE_NONE;
static open_mode_type open_mode = OPEN_MODE_NONE;

/* Global to take status of partition table command from layer below */
parti_tbl_type partition_status;

/* Global for status of open multi from layer below */
byte open_multi_status;

/* Variable to keep track of how many bytes received */
static uint32 bytes_recd = 0;

/* Data to produce size message back to host.  The length
 * of size_msg_fixed plus 9 must not exceed SIZE_MSG_LEN or the code
 * which checks for this will deal with it by sending a fixed message
 * indicating an overrun was detected.
 */
char size_msg[SIZE_MSG_LEN];
char *size_msg_fixed = "Total bytes recd by ARMPRG 0x";
char *size_msg_overrun = "ARMPRG size msg overrun";
const char digs[] = "0123456789ABCDEF";

char log_msg_buffer[128];

/* Flag used to enable/disable CRC check in the received data.
 * 1 - enable crc check
 * 0 - disable crc check
 * This will be set to 1 during interface initialization in hostdl.c if UART 
 * transport is used since CRC check is required for UART transport.
 * Set this to 1 using Trace32 if CRC check needs to be enabled in the received data 
 * for USB transport during debugging
 */
dword enable_crc_check = 0;

extern struct emmcbld_init_info emmcbld_api_pack[EMMCBLD_MAX_NUM_EXT_INIT_FN];


//--------------------------------------------------------------------------
// Extern Definitions
//--------------------------------------------------------------------------

extern void process_current_packet (void);
extern boolean sector_addresses_enabled;

//--------------------------------------------------------------------------
// Function Prototypes
//--------------------------------------------------------------------------

void transmit_error (dword code, char *message);
void xmit_log_message (char *message);

//--------------------------------------------------------------------------
// Function Declarations
//--------------------------------------------------------------------------

void* alignmemcpy (void* dest, void const* src, uint32 len)
{
    if (((uintptr_t)src & 0x03 == 0) && ((uintptr_t)dest & 0x03 == 0))
    {
        memscpy(dest, len, src, len);
        return dest;
    }
    else {
        char const * byte_src = (char const *) src;
        char *byte_dest = (char *) dest;
        while (len && (((uintptr_t)byte_src & 0x03 != 0) || ((uintptr_t)byte_dest & 0x03 != 0)))
        {
            len--;
            *byte_dest++ = *byte_src++;
        }
        if (len) {
            memscpy(byte_dest, len, byte_src, len);
        }
        return (dest);
    }
}

/*===========================================================================

  This function initializes the receive packet list.
  It also copies some data from the global identification block so that
  the data in the block will be included by the linker and will be able
  to be seen in the resulting binary.

===========================================================================*/

void packet_init (void)
{
  int i;

  FLASHLOG(2,("packet_init: entry\n"));
  packet_data[0].next = NULL;
  packet_data[0].length = 0;

  /* Initialize free list. */
  for (i = 1; i < NUMBER_OF_PACKETS; i++)
  {
    packet_data[i].next = &packet_data[i - 1];
    packet_data[i].length = 0;
  }

  free_list = &packet_data[NUMBER_OF_PACKETS - 1];
  waiting = 0;
  current = 0;

  escape_state = 0;
  waiting_last = 0;
  need_hello = 1;

  FLASHLOG(2,("packet_init: exit\n"));
  return;
}

#ifndef FEATURE_EMMCBLD_NO_HDLC_RX
/*===========================================================================

  This function stores a character received over the serial link in the
  receive buffer. It handles the escape character 0x7D appropriately.

===========================================================================*/

void packet_handle_incoming (int ch)
{
  bytes_recd++;

  /* Get us a buffer if needed. */
  if (current == NULL)
  {
    if (free_list == NULL)
    {
      xmit_log_message ("ERR: Window overrun detected...");
      return;
    }

    current = free_list;
    free_list = free_list->next;

    current->length = 0;
    current->next = NULL;       /* Pull it off of the list. */
  }

  switch (ch)
  {
    case 0x7E:
      process_current_packet ();
      escape_state = 0;
      break;

    case 0x7D:
      escape_state = 1;
      break;

    default:
      if (current->length < MAX_PACKET_SIZE)
      {
        if (escape_state)
        {
          ch ^= 0x20;
          escape_state = 0;
        }

        current->buffer[current->length] = ch;  /*lint !e734 */
      }

      /* Increment the count anyway.  We will automatically discard
         packets that are too long. */
      current->length++;
  }
}
#endif

/*===========================================================================

  This function stores characters received over the serial link in the
  receive buffer. It handles the escape character 0x7D appropriately.

===========================================================================*/

void
packet_handle_incoming_buf (byte* buf, int len)
{
  word length;
  byte *out_buf = NULL;
  dword data;
  int num_chars;

  bytes_recd += len;

  /* Get us a buffer if needed. */

  if (current == NULL)
  {
    if (free_list == NULL)
    {
      xmit_log_message ("ERR: Window overrun detected...");
      return;
    }

    current = free_list;
    free_list = free_list->next;

    current->length = 0;
    current->next = NULL;       /* Pull it off of the list. */
  }
#ifndef FEATURE_EMMCBLD_NO_HDLC_RX
  
  length = current->length;
  out_buf = current->buffer;

  /* Handle in dwords until length is > 4 */
  while(len > 4) 
  {
    data = *((dword *)buf);
    
    /* Check for escape sequence character or end of frame character
     * in the 32 bit data 
     */
    if (((data & 0xFF000000) == 0x7D000000) ||
        ((data & 0x00FF0000) == 0x007D0000) ||
        ((data & 0x0000FF00) == 0x00007D00) ||
        ((data & 0x000000FF) == 0x0000007D) ||
        ((data & 0xFF000000) == 0x7E000000) ||
        ((data & 0x00FF0000) == 0x007E0000) ||
        ((data & 0x0000FF00) == 0x00007E00) ||
        ((data & 0x000000FF) == 0x0000007E))            
    {
      /* Escape sequence or end of frame character in data. Process it
       * character by character 
       */
      num_chars = 4;
      while(num_chars--)
      {    
        switch (*buf)
        {
          case 0x7E:
            if(length !=0 ) 
            {
              current->length = length;
              process_current_packet ();
            }
            escape_state = 0;
            break;

          case 0x7D:
            escape_state = 1;
            break;

          default:
            if (length < MAX_PACKET_SIZE)
            {
              if (escape_state)
              {
                out_buf[length] = *buf ^ 0x20;
                escape_state = 0;
              }
              else
              {
                out_buf[length] = *buf;  
              }
            }

          /* Increment the count anyway.  We will automatically discard
             packets that are too long. */
          length++;
        }
        buf++;    
      }
    }
    else
    {
      /* No escape sequence or end of frame character. Just copy the 32 bit 
       * data into  receive buffer
       */
      alignmemcpy((void *)&out_buf[length], (void *)&data, 4);
      if (escape_state)
      {    
        out_buf[length] ^= 0x20;
        escape_state = 0;
      }    
      length += 4;
      buf += 4;
    }

    len -= 4;
  }

  /* Process the remaining characters */
  while(len--) 
  {
    switch (*buf)
    {
      case 0x7E:
        if(length !=0 ) 
        {
          current->length = length;
          process_current_packet ();
        }
        escape_state = 0;
        break;

      case 0x7D:
        escape_state = 1;
        break;

      default:
        if (length < MAX_PACKET_SIZE)
        {
          if (escape_state)
          {
            out_buf[length] = *buf ^ 0x20;
            escape_state = 0;
          }
          else
          {    
            out_buf[length] = *buf;  
          }
        }

        /* Increment the count anyway. We will automatically discard
           packets that are too long. */
        length++;
    }
    buf++;    
  }
#else
  out_buf = current->buffer;
  alignmemcpy((void *)&out_buf[0], (void *)buf, len);
  current->length = len;
  process_current_packet ();
#endif
}


/*===========================================================================

  This function calculates the CRC for the reply message. It updates the
  reply length by 2 to account for the 2 bytes of CRC.

===========================================================================*/

void compute_reply_crc ()
{
  word crc = crc_16_l_calc (reply_buffer, reply_length * 8);
  reply_buffer[reply_length] = crc & 0xFF;
  reply_buffer[reply_length + 1] = crc >> 8;

  reply_length += 2;
}


/*===========================================================================

  This function transmits a reply packet that has been filled up previously.

===========================================================================*/

void force_xmit_reply ()
{
  int i;
  int ch;
  int j;

  j = 0;


  CHECK_FOR_DATA ();
#ifndef FEATURE_EMMCBLD_NEW_TX
  /* Since we don't know how long it's been. */
  TRANSMIT_BYTE (0x7E);
  j++;
#endif
  for (i = 0; i < reply_length; i++)
  {
    /* we only need to check once every 31 characters, since RX and TX
     * run at about the same speed, and our RX FIFO is 64 characters
     */
    if ((i & 31) == 31)
      CHECK_FOR_DATA ();

    ch = reply_buffer[i];
#ifndef FEATURE_EMMCBLD_NEW_TX
    if (ch == 0x7E || ch == 0x7D)
    {
      TRANSMIT_BYTE (0x7D);
      TRANSMIT_BYTE (0x20 ^ ch);        /*lint !e734 */
      j += 2;
    }
    else
    {
#endif
      TRANSMIT_BYTE (ch);       /*lint !e734 */
      j++;
#ifndef FEATURE_EMMCBLD_NEW_TX
    }
#endif
  }

  CHECK_FOR_DATA ();
#ifndef FEATURE_EMMCBLD_NEW_TX
  TRANSMIT_BYTE (0x7E);
  j++;

  /* Hack for USB protocol.  If we have an exact multiple of the USB frame
   * size, then the last frame will not be sent out.  The USB standard says
   * that a "short packet" needs to be sent to flush the data.  Two flag
   * characters can serve as the short packet.  Doing it this way, we only
   * perform this test once on every entire packet from the target, so the
   * over head is not too much.
   */
#endif
  if ((j%64) == 0)
  {
    TRANSMIT_BYTE (0x7E);
    TRANSMIT_BYTE (0x7E);
  }

}

/*===========================================================================

  This function processes a received packet. It validates the packet contents
  and stores the packet in the receive list to be processed on a FIFO basis.

===========================================================================*/

void process_current_packet (void)
{
  /* Begin with some simple sanity checks. */
#ifndef FEATURE_EMMCBLD_NO_HDLC_RX
  if (current->length > MAX_PACKET_SIZE || current->length < 3)
#else
  if (current->length > MAX_PACKET_SIZE)
#endif
  {
    current->length = 0;
    return;
  }

  /* Queue it up to be processed. */
  if (waiting_last == NULL)
  {
    waiting = waiting_last = current;
    current = NULL;
  }
  else
  {
    waiting_last->next = current;
    waiting_last = current;
    current = NULL;
  }
}


/*===========================================================================

  This function computes the length of the received string.

===========================================================================*/

int my_strlen (char *s)
{
  int i = 0;
  while (*s++)
    i++;
  return i;
}



/*===========================================================================

  This function sends a message back to the host containing the number of
  bytes received.  It embodies a very simple sprintf for hex digits.

===========================================================================*/

void log_size_msg(void)
{
  int i;
  int shift_count = 28;
  unsigned int nibble;
  int fixed_size;

  fixed_size = my_strlen(size_msg_fixed);

  alignmemcpy(size_msg, size_msg_fixed, fixed_size);

  for (i=fixed_size; i<(fixed_size+8); i++)
  {
    /* prevent buffer overrun */
    if (i >= SIZE_MSG_LEN)
    {
      FLASHLOG(1,("log_size_msg() Overrun\n"));
      fixed_size = my_strlen(size_msg_overrun);
      alignmemcpy(size_msg, size_msg_overrun, fixed_size);
      size_msg[fixed_size] = 0;
      break;
    }
    nibble = (bytes_recd>>shift_count) & 0xF;
    size_msg[i] = digs[nibble];
    shift_count-=4;
  }
  size_msg[i] = 0;

  xmit_log_message(size_msg);
}




/*===========================================================================

  This function transmits an error response with the specified error code and
  specified error message.

===========================================================================*/

void transmit_error (dword code, char *message)
{
  int length = my_strlen (message);

  FLASHLOG(2,("transmit_error:  %s\n", message));

  /* Make sure error message gets in log */
  xmit_log_message (message);

  reply_buffer[0] = ERROR_RSP;
  reply_buffer[1] = code & 0xFF;
  reply_buffer[2] = (code >> 8) & 0xFF;
  reply_buffer[3] = (code >> 16) & 0xFF;
  reply_buffer[4] = (code >> 24) & 0xFF;
  alignmemcpy (&reply_buffer[5], message, length);   /*lint !e732 */

  reply_length = 5 + length;    /*lint !e734 */

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("transmit_error: exit\n"));
  return;
}


/*===========================================================================

  This function transmits the specified log message.

===========================================================================*/

void xmit_log_message (char *message)
{
  int length = my_strlen (message);

  reply_buffer[0] = CMD_LOG;
  alignmemcpy (&reply_buffer[1], message, length);   /*lint !e732 */

  reply_length = 1 + length;    /*lint !e734 */

  /* Host tool does not add a CR to the log after the message,
   * so we must
   */
  reply_buffer[reply_length++] = '\n';

  compute_reply_crc ();
  force_xmit_reply ();

  /* If printing is on, also print all messages transmitted to log */
  FLASHLOG(1,("XMIT_ERR:  %s\n", message));
}


/*===========================================================================

  This function transmits an *Invalid Command* message.

===========================================================================*/

int handle_invalid (incoming_t buffer)
{
  FLASHLOG(2,("handle_invalid: entry\n"));
  transmit_error (0x0005, "Invalid Command");
  FLASHLOG(2,("handle_invalid: exit\n"));

  return 0;
} /*lint !e715 */

/*===========================================================================

  This function handles the *hello command* packet and transmits the
  appropriate response packet. It sets a flag to indicate that flash driver
  commands could be processed after validating the hello packet.

===========================================================================*/

int handle_hello (incoming_t buffer)
{
  static const byte host_header[] = "QCOM fast download protocol host";
  static const byte target_header[] = "QCOM fast download protocol targ";
  dword wsize;
  int p;
  char *flashname;
  uint8 requested_feature_bits;
  int i;

  FLASHLOG(2,("handle_hello: entry\n"));

  if (memcmp (&buffer->buffer[1], host_header, 32) != 0)
  {
    transmit_error (NAK_INVALID_CMD, "Invalid magic number");
    return 0;
  }

  FLASHLOG(2,("handle_hello: Stream min ver is 0x%x\n", buffer->buffer[34]));
  FLASHLOG(2,("handle_hello: Stream max ver is 0x%x\n", buffer->buffer[33]));


  /* OK to accept versions higher than we know about */
  if (buffer->buffer[34] < STREAM_DLOAD_MIN_VER)
  {
    transmit_error (NAK_INVALID_CMD, "Invalid protocol version");
    return 0;
  }

  FLASHLOG(2,("handle_hello: buffer length %d  feature bits 0x%x\n",
             buffer->length, buffer->buffer[35]));

  /*
   * Allow more than 1 byte of feature bits, even though we do not use
   * more than one byte of feature bits.  This allows the host to send
   * as many bytes as it cares, and we will only deal with the ones
   * we know about.
   */
  if (buffer->length < 36)
  {
    transmit_error (NAK_INVALID_LEN, "Parameter Request Packet Length Error");
    return 0;
  }


  need_hello = FALSE;

  /* Generate reply. */
  reply_buffer[0] = HELLO_RSP;

  alignmemcpy (&reply_buffer[1], target_header, 32);
  reply_buffer[33] = STREAM_DLOAD_MAX_VER;
  reply_buffer[34] = STREAM_DLOAD_MIN_VER;

  /* Preferred block size. */
  reply_buffer[35] = (MAX_DATA_LENGTH) & 0xFF;  /*lint !e778 */
  reply_buffer[36] = (MAX_DATA_LENGTH >> 8) & 0xFF;
  reply_buffer[37] = 0;
  reply_buffer[38] = 0;

  /* Base address of flash  */
  reply_buffer[39] = 0;
  reply_buffer[40] = 0;
  reply_buffer[41] = 0;
  reply_buffer[42] = 0;

  /* Flash id len and string */
  flashname = storage_device_name();
  if (flashname == 0)
  {
    transmit_error (NAK_WRONG_IID, "Unrecognized flash device");
    return 0;
  }

  reply_buffer[43] = my_strlen (flashname);
  alignmemcpy (&reply_buffer[44], flashname, reply_buffer[43]);

  p = 44 + reply_buffer[43];    /* index into WindowSize field */

  /* Window size. */
  wsize = NUMBER_OF_PACKETS;

  reply_buffer[p++] = (wsize) & 0xFF;
  reply_buffer[p++] = (wsize >> 8) & 0xFF;

  p += storage_device_sectors (&reply_buffer[p]);


  /*
   * Feature bits! Currently, we only deal with the first byte of
   * feature bits, but above, we do not error if more bytes are sent
   * by the host.  If we eventually start recognizing more bytes
   * of feature bits, then these few lines of code need to change
   * to handle each byte on its own.
   */
  requested_feature_bits = buffer->buffer[35] & SUPPORTED_FEATURES;
  reply_buffer[p++] = requested_feature_bits;

  /* Check the major version number and enable sector addresses accordingly */
  if (requested_feature_bits & FEATURE_SECTOR_ADDRESSES) {
     sector_addresses_enabled = TRUE;
  }
  else {
     sector_addresses_enabled = FALSE;
  }

  reply_length = p;             /*lint !e734 */


  /*
   *  Check to see if we overran the end of the reply_buffer.  If so,
   *  loop here forever.  The fix is to increase the size of the
   *  reply_buffer, however, this may have consequences with regards
   *  to the host side.  This should be checked thoroughly.  This
   *  probably would have occurred by someone adding a flash part
   *  that is larger than one we have ever used before.
   */
  if (p > sizeof(reply_buffer))
  {
    transmit_error (NAK_FAILED, "Internal error - reply_buffer overrun");
    FLASHLOG(1,("handle_hello: exceeded buffer length\n"));
    for (;;)
    {
    }
  }

  compute_reply_crc ();
  force_xmit_reply ();

  /* Call any external subsystems that need notification when hello command is received */
  for (i = 0; i < EMMCBLD_MAX_NUM_EXT_INIT_FN; i++)
  {
     if((emmcbld_api_pack[i]).handle_hello_command != NULL)
        (emmcbld_api_pack[i]).handle_hello_command();
  }

  FLASHLOG(2,("handle_hello: exit\n"));

  return 0;                     /* Don't queue. */
}


/*===========================================================================

  This function reads the specified number of bytes from the specified
  location.

===========================================================================*/

int handle_simple_read (incoming_t buffer)
{
#ifdef FEATURE_EMMCBLD_ENABLE_READ
  dword addr;
  word length, i = 0;
  byte *data = buffer->buffer;
  response_code_type retVal;

  FLASHLOG(2,("handle_simple_read: entry\n"));

  if (buffer->length != READ_LEN)
  {
    transmit_error (NAK_INVALID_LEN, "Invalid packet length");
    return 0;
  }

  addr = (data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24));
  length = (data[5] | (data[6] << 8));

  if (length > MAX_DATA_LENGTH)
  {
    transmit_error (NAK_INVALID_LEN, "Packet too large");
    return 0;
  }

  reply_buffer[i++] = READ_RSP;
  reply_buffer[i++] = data[1];
  reply_buffer[i++] = data[2];
  reply_buffer[i++] = data[3];
  reply_buffer[i++] = data[4];

  if ((retVal = do_read (&reply_buffer[i], addr, length)) != ACK)
  {
    transmit_error ((dword) retVal, "Read unsuccessful");
    return 0;
  }
  reply_length = i + length;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_simple_read: exit\n"));
#else
  transmit_error (NAK_INVALID_CMD, "Feature not compiled in");
#endif
  return 0;
}

/*===========================================================================

  This function returns deatails about the physical partition opened

===========================================================================*/

int handle_partition_info (incoming_t buffer)
{
#ifdef FEATURE_EMMCBLD_ENABLE_GPP
  dword gpp_sizes[MAX_NUMBER_GPP];
#endif
  response_code_type retVal = NAK_INVALID_CMD;
  word i;
  byte *data = buffer->buffer;
  byte sub_cmd;
  word magic, cmd_crc, calc_crc;
  dword zeroout_num_sectors;
  dword zeroout_start_sector;
  dword erase_blocks;
  byte * buf;
  
  if (buffer->length != PARTITION_INFO_LEN)
  {
    transmit_error (NAK_INVALID_LEN, "Invalid packet length");
    return 0;
  }
  
  sub_cmd = data[1];
  magic = data[1+1+(MAX_NUMBER_GPP*4)] | (data[1+1+(MAX_NUMBER_GPP*4)+1] << 8);
  cmd_crc = data[1+1+(MAX_NUMBER_GPP*4)+2] | (data[1+1+(MAX_NUMBER_GPP*4)+2+1] << 8);
  calc_crc = crc_16_l_calc (&data[1], (1+(MAX_NUMBER_GPP*4)+2) * 8);

  if (magic != GPP_MAGIC_NUMBER) {
    transmit_error ((dword) retVal, "GPP/Info command failed. Invalid Magic Number");
    return 0;
  }
  if (cmd_crc != calc_crc) {
    transmit_error ((dword) retVal, "GPP/Info command failed. Invalid CRC");
    return 0;
  }

  /* Sub command 0x1 indicates GPP creation */
  /* Irrespective of sub command, partition information is returned to host */
  if (sub_cmd == 0x1) {
#ifdef FEATURE_EMMCBLD_ENABLE_GPP
    for (i = 0; i < MAX_NUMBER_GPP; i++)
        gpp_sizes[i] = (data[(4*i)+2] | (data[(4*i)+3] << 8) | (data[(4*i)+4] << 16) | (data[(4*i)+5] << 24));
    retVal = do_create_gpp(gpp_sizes);
    if (retVal != ACK)
    {
      xmit_log_message(log_msg_buffer);
      transmit_error (retVal, "GPP creation failed");
      return 0;
    }
#else
    transmit_error (NAK_INVALID_CMD, "Feature not compiled in");
#endif
  }
  else if (sub_cmd == 0x2) {
    retVal = set_active_boot();
    if (retVal != ACK)
    {
      transmit_error (retVal, "Set active boot partition failed");
      return 0;
    }
    xmit_log_message("Set active boot partition successfully");
  }
  else if (sub_cmd == 0x3) {
    i = 0;
    zeroout_start_sector = (data[(4*i)+2] | (data[(4*i)+3] << 8) | (data[(4*i)+4] << 16) | (data[(4*i)+5] << 24));
    i++;
    zeroout_num_sectors = (data[(4*i)+2] | (data[(4*i)+3] << 8) | (data[(4*i)+4] << 16) | (data[(4*i)+5] << 24));
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "zeroout start %d, len %d", zeroout_start_sector, zeroout_num_sectors);
    xmit_log_message(log_msg_buffer);
    snprintf(log_msg_buffer, sizeof(log_msg_buffer), "zeroing out %d bytes at a time", MAX_DATA_LENGTH);
    xmit_log_message(log_msg_buffer);
    buf = &aligned_buffer[0];
    /* Zeroout 'MAX_DATA_LENGTH' bytes at a time because that is currently the
     * largest contiguous buffer available to us
     */
    memset(buf, 0x0, (zeroout_num_sectors*BLOCK_SIZE) < MAX_DATA_LENGTH ? (zeroout_num_sectors*BLOCK_SIZE) : MAX_DATA_LENGTH);

    i=0;
    while(zeroout_num_sectors > 0)
    {
        erase_blocks = zeroout_num_sectors < (MAX_DATA_LENGTH/BLOCK_SIZE) ? zeroout_num_sectors : (MAX_DATA_LENGTH/BLOCK_SIZE);
        retVal = do_write(zeroout_start_sector, erase_blocks, buf);
        if (retVal != ACK)
        {
          transmit_error (retVal, "Zeroing out failed");
          return 0;
        }
        zeroout_start_sector += erase_blocks;
        zeroout_num_sectors -= erase_blocks;
        i+=1;
    }
    xmit_log_message("Zeroed out successfully");
  }
  reply_buffer[0] = INFO_RSP;
  reply_length = 1;
  retVal = do_partition_info(&reply_buffer[1], &reply_length);
  if (retVal != ACK)
  {
    transmit_error (retVal, "Partition info failed");
    return 0;
  }
  compute_reply_crc ();
  force_xmit_reply ();

  return 0;
}

/*===========================================================================

  This function causes a watchdog reset to occur.

===========================================================================*/

int handle_reset (incoming_t buffer)
{
   FLASHLOG(2,("handle_reset: entry\n"));
   
   /* Do not allow reset after open but before close */
   if ((open_multi_mode != OPEN_MULTI_MODE_NONE) ||
       (open_mode != OPEN_MODE_NONE))
   {
     transmit_error (NAK_CMD_OUT_SEQ, "Cannot reset before close");
     return 0;
   }
   
   reply_buffer[0] = RESET_RSP;
   reply_length = 1;
   
   compute_reply_crc ();
   force_xmit_reply ();
   
   DRAIN_TRANSMIT_FIFO ();
   
   FLASHLOG(2,("handle_reset: exit\n"));

   KICK_WATCHDOG ();
   /* Force a reset */
   emmcbld_bsp_target_reset();
   
   return 0;                     /*lint !e527 */
}                               /*lint !e715 */

/*===========================================================================

  This function is used to write the specified data at a specified location. It
  can be only be used to change 1's to 0's.

===========================================================================*/

int
handle_simple_write (incoming_t buffer)
{
  FLASHLOG(2,("handle_simple_write: entry\n"));

  /* NAND programming does not support this command */
  transmit_error (NAK_INVALID_CMD, "Simple write not allowed");
  return 0;
}

/*===========================================================================

  This function write a stream of bytes at the specified location in flash
  memory. It validates that the command is valid before writing into flash.

===========================================================================*/

int handle_stream_write (incoming_t buffer)
{
#ifndef FEATURE_EMMCBLD_ENABLE_GPP
  byte *data = buffer->buffer;
  word len = buffer->length - 5;
  dword addr = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
  response_code_type retVal;
  byte* src_ptr;

  FLASHLOG(4,("handle_stream_write: addr 0x%x  len 0x%x\n", addr, len));

  src_ptr = &data[5];

  if ((addr | (uint32)src_ptr | len) & 0x01)
  {
    byte *buffer_ptr; 
    uint32 actual_len_to_write;
#ifdef BUILD_NORPRG
    byte *dest_ptr;
#endif    
  
    actual_len_to_write = len;

    /* Base address is assumed as 0. Refer to the do_stream_write implementation. */
    buffer_ptr = &aligned_buffer[0];

#ifdef BUILD_NORPRG
    dest_ptr = (byte*)addr;
    if (((uint32)dest_ptr) & 0x01)
    {
      dest_ptr = (byte*)((uint32)dest_ptr & (~0x01));
      *buffer_ptr++ = *dest_ptr;
      --addr;
      ++actual_len_to_write;
    }

    if (actual_len_to_write & 0x01)
    {
      aligned_buffer[actual_len_to_write] = dest_ptr[actual_len_to_write];
      ++actual_len_to_write;
    }
#endif

    alignmemcpy(buffer_ptr, src_ptr, len);

    retVal = do_stream_write (aligned_buffer, addr, actual_len_to_write);
  }
  else
  {
    retVal = do_stream_write (src_ptr, addr, len);
  }

  if (retVal != ACK)
  {
    transmit_error ((dword) retVal, "Write unsuccessful");
    return 0;
  }

#ifdef DROP_ACKS
  /* Simulate dropping of ACKs to force false retransmit by host */
  ack_drop_count++;
  if (ack_drop_count%ACK_DROP_MOD == 0)
  {
    term_put('a');
    FLASHLOG(1,("drop ACK %d\n", ack_drop_count));
    return 0;
  }
#endif


  reply_buffer[0] = STREAM_WRITE_RSP;
  reply_buffer[1] = addr & 0xFF;
  reply_buffer[2] = (addr >> 8) & 0xFF;
  reply_buffer[3] = (addr >> 16) & 0xFF;
  reply_buffer[4] = (addr >> 24) & 0xFF;
  reply_length = 5;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(4,("handle_stream_write: exit\n"));

#else
  transmit_error (NAK_INVALID_CMD, "Feature not compiled in");
#endif
  return 0;
}

/*===========================================================================

  This function erases the opened partition.

===========================================================================*/

int handle_erase (incoming_t buffer)
{
#ifndef FEATURE_EMMCBLD_ENABLE_GPP
  response_code_type retVal;
  FLASHLOG(2,("handle_erase: entry\n"));

  if ((open_multi_mode == OPEN_MULTI_MODE_NONE) &&
      (open_mode == OPEN_MODE_NONE))
  {
    transmit_error (NAK_CMD_OUT_SEQ, "Cannot erase when not previously opened");
    return 0;
  }

  retVal = do_erase(reply_buffer, REPLY_BUFFER_SIZE);
  if (retVal != ACK)
  {
    transmit_error (retVal, "Erase failed");
    return 0;
  }

  reply_buffer[0] = ERASE_RSP;
  reply_length = 1;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_erase: exit\n"));
#else
  transmit_error (NAK_INVALID_CMD, "Feature not compiled in");
#endif
  return 0;
  }

/*===========================================================================

  This function responds to a NOP command packet with a NOP response packet.

===========================================================================*/
int handle_sync (incoming_t buffer)
{
  byte *data = buffer->buffer;
  word len = buffer->length - 1;

  FLASHLOG(2,("handle_sync: entry\n"));

  reply_buffer[0] = NOP_RSP;
  alignmemcpy (&reply_buffer[1], &data[1], len);
  reply_length = len + 1;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_sync: exit\n"));

  return 0;
}



/*===========================================================================

  This function sends a message that this command is not supported.

===========================================================================*/

int handle_power_off (incoming_t buffer)
{
  reply_buffer[0] = PWRDOWN_RSP;
  reply_length = 1;

  FLASHLOG(2,("handle_power_off: entry\n"));

  /* Do not allow power down after open but before close */
  transmit_error (NAK_PWROFF_NOT_SUPP, "Power off not supported");
  return 0;
}



/*===========================================================================

  This function is used to open a NAND device for writing.
  Can be used for either Primary Image or BootLoader Image.

===========================================================================*/

int handle_open (incoming_t buffer)
{
  byte mode = buffer->buffer[1];
  response_code_type retVal;

  FLASHLOG(2,("handle_open: entry\n"));

  if (open_mode != OPEN_MODE_NONE)
  {
    if (open_mode != mode)
    {
      transmit_error (NAK_CMD_OUT_SEQ, "Already opened in different mode");
      return 0;
    }
    else
    {
      /* Not a failure, but needs to get in host side log */
      xmit_log_message("Already opened, ignoring open in same mode");
      return 0;
    }
  }

  retVal = do_open(mode);

  if ( retVal != ACK )
  {
    transmit_error (NAK_FAILED, "Open failed");
    FLASHLOG(2,("handle_open: exit\n"));
    return 0;
  }

  /* Success, store mode and reply */
  open_mode = (open_mode_type) mode;
  reply_buffer[0] = OPEN_RSP;
  reply_length = 1;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_open: exit\n"));

  return 0;
}


/*===========================================================================

  This function is used to close a NAND device for writing.
  Can be used for either Primary Image or BootLoader Image.

===========================================================================*/

int handle_close (incoming_t buffer)
{
  response_code_type retVal;

  FLASHLOG(2,("handle_close: entry\n"));

  if ((open_multi_mode == OPEN_MULTI_MODE_NONE) &&
      (open_mode == OPEN_MODE_NONE))
  {
    transmit_error (NAK_CMD_OUT_SEQ, "Cannot close when not previously opened");
    return 0;
  }


  retVal = do_close();
  if (retVal != ACK)
  {
    transmit_error (retVal, "Close failed");
    return 0;
  }

  open_multi_mode = OPEN_MULTI_MODE_NONE;
  open_mode = OPEN_MODE_NONE;

  reply_buffer[0] = CLOSE_RSP;
  reply_length = 1;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_close: exit\n"));

  return 0;
  }


/*===========================================================================

  This function is used to set the security mode to be used for all
  subsequent operations.

===========================================================================*/

int handle_security_mode (incoming_t buffer)
{
  byte mode = buffer->buffer[1];
  response_code_type retVal;

  FLASHLOG(2,("handle_security_mode: entry\n"));

  retVal = do_security_mode (mode);

  if (retVal != ACK)
  {
    transmit_error (retVal, "Set security mode failed.");
    return 0;
  }

  security_mode_rcvd = TRUE;

  /* Reply to the host as success */
  reply_buffer[0] = SECURITY_MODE_RSP;
  reply_length = 1;

  compute_reply_crc ();
  force_xmit_reply ();

  FLASHLOG(2,("handle_security_mode: exit\n"));

  return 0;
}


/*===========================================================================

  This function is used to receive and validate a partition table used
  in all subsequent programming operations

===========================================================================*/

int handle_parti_tbl (incoming_t buffer)
{
  byte *data = &buffer->buffer[2];
  word len = buffer->length - 2;        /* cmd-type : 1, override : 1 */
  response_code_type retVal;
  byte override = buffer->buffer[1];

  FLASHLOG(2,("handle_parti_tbl: entry\n"));

  /* Check for security mode already sent */
  if (security_mode_rcvd != TRUE)
  {
    transmit_error (NAK_CMD_OUT_SEQ,
                    "No security mode received before partition table");
    FLASHLOG(2,("handle_parti_tbl: exit\n"));
    return 0;
  }
  else
  {
    FLASHLOG(2,("handle_parti_tbl: OK - security mode already received\n"));
  }

  /* Check for payload length not exceeded */
  if (len > 512)
  {
    transmit_error (NAK_INVALID_LEN, "Partition table length exceeded");
    FLASHLOG(2,("handle_parti_tbl: exit\n"));
    return 0;
  }
  else
  {
    FLASHLOG(2,("handle_parti_tbl: OK - length check\n"));
  }

  FLASHLOG(2,("handle_parti_tbl: Calling do_partition_table\n"));
  retVal = do_partition_table (data, override, len);
  FLASHLOG(2,("handle_parti_tbl: Back from do_partition_table with status 0x%x\n", retVal));
  if (retVal != ACK)
  {
    if (partition_status == PARTI_TBL_UNKNOWN_ERROR)
    {
      /* Some fatal error occurred that host does not care to parse */
      partition_table_rcvd = FALSE;
      transmit_error (NAK_FAILED, "Unknown error accepting partition table");
      FLASHLOG(2,("handle_parti_tbl: exit\n"));
      return 0;
    }
    else
    {
      /* Error occurred that host needs to parse and possibly send
       * partition table again.
       */
      partition_table_rcvd = FALSE;
      reply_buffer[0] = PARTITION_TABLE_RSP;
      reply_buffer[1] = partition_status;    /* status from below */
      reply_length = 2;

      FLASHLOG(2,("handle_parti_tbl: Error host needs to parse is 0x%x\n", partition_status));
      compute_reply_crc ();
      force_xmit_reply ();
      FLASHLOG(2,("handle_parti_tbl: exit\n"));
      return 0;
    }
  }

  partition_table_rcvd = TRUE;

  reply_buffer[0] = PARTITION_TABLE_RSP;
  reply_buffer[1] = 0x0;                 /* partition table accepted */
  reply_length = 2;

  compute_reply_crc ();
  force_xmit_reply ();
  FLASHLOG(2,("handle_parti_tbl: exit\n"));

  return 0;
}


/*===========================================================================

  This function is used to open for writing a particular image for multi-image
  boot targets

===========================================================================*/

int handle_open_multi (incoming_t buffer)
{
  byte *data = &buffer->buffer[2];
  byte mode = buffer->buffer[1];
  response_code_type retVal;
  word len = buffer->length - 2;  /* length of optional data */


  FLASHLOG(2,("handle_open_multi: entry\n"));

  /* Check for security mode already sent */
  if (security_mode_rcvd != TRUE)
  {
    transmit_error (NAK_CMD_OUT_SEQ,
                    "No security mode received before open multi");
    FLASHLOG(2,("handle_open_multi: exit\n"));
    return 0;
  }

  /* Check for partition table received */
  if (partition_table_rcvd != TRUE)
  {
    transmit_error (NAK_CMD_OUT_SEQ,
                    "No partition table received before open multi");
    FLASHLOG(2,("handle_open_multi: exit\n"));
    return 0;
  }


  FLASHLOG(2,("handle_open_multi: calling do_open_multi\n"));
  FLASHLOG(2,("handle_open_multi: mode value is 0x%x\n", mode));
  retVal = do_open_multi (data, mode, len);
  FLASHLOG(2,("handle_open_multi: back from do_open_multi\n"));
  if (retVal != ACK)
  {
    /* This error cannot be dealt with by the host */
    if (open_multi_status == OPEN_MULTI_UNKNOWN_ERROR)
    {
      transmit_error (NAK_FAILED, "Open multi failed, unknown error");
      FLASHLOG(2,("handle_open_multi: exit\n"));
      return 0;
    }
    else
    {
      /* These errors can be parsed by the host, treat them differently */
      switch (open_multi_status)
      {
        case OPEN_MULTI_LENGTH_EXCEEDED:
        case OPEN_MULTI_PAYLOAD_NOT_ALLOWED:
        case OPEN_MULTI_PAYLOAD_REQUIRED:
          reply_buffer[0] = OPEN_MULTI_IMAGE_RSP;
          reply_buffer[1] = open_multi_status;
          reply_length = 2;
          break;
        default:
          transmit_error (NAK_FAILED, "Open multi failed, unknown error");
          FLASHLOG(2,("handle_open_multi: exit\n"));
          return 0;
      }
    }
  }
  else
  {
    /* Success, store mode and reply */
    open_multi_mode = (open_multi_mode_type) mode;
    reply_buffer[0] = OPEN_MULTI_IMAGE_RSP;
    reply_buffer[1] = OPEN_MULTI_SUCCESS;
    reply_length = 2;
  }

  compute_reply_crc ();
  force_xmit_reply ();
  FLASHLOG(2,("handle_open_multi: exit\n"));

  return 0;
}



/*===========================================================================

  This function processes a command packet, validates the same and invokes the
  appropriate command.

===========================================================================*/

int (*packet_handler[]) (incoming_t) =
{
  handle_hello,               /*  0x01 -  hello.                    */
  handle_invalid,             /*  0x02 -    hello reply             */
  handle_simple_read,         /*  0x03 -  simple read.              */
  handle_invalid,             /*  0x04 -    simple read reply.      */
  handle_simple_write,        /*  0x05 -  simple write.             */
  handle_invalid,             /*  0x06 -    simple write reply.     */
  handle_stream_write,        /*  0x07 -  stream write.             */
  handle_invalid,             /*  0x08 -    block written.          */
  handle_sync,                /*  0x09 -  sync.                     */
  handle_invalid,             /*  0x0A -    sync reply.             */
  handle_reset,               /*  0x0B -  reset.                    */
  handle_invalid,             /*  0x0C -    reset reply.            */
  handle_invalid,             /*  0x0D -    error msg.              */
  handle_invalid,             /*  0x0E -    log   msg.              */
  handle_invalid,             /*  0x0F -  invalid (was unlock)      */
  handle_invalid,             /*  0x10 -   unlock reply.            */
  handle_power_off,           /*  0x11 -  pwr-off.                  */
  handle_invalid,             /*  0x12 -    pwr-off reply.          */
  handle_open,                /*  0x13 -  open.                     */
  handle_invalid,             /*  0x14 -    open reply.             */
  handle_close,               /*  0x15 -  close.                    */
  handle_invalid,             /*  0x16 -    close reply.            */
  handle_security_mode,       /*  0x17 -  security mode.            */
  handle_invalid,             /*  0x18 -    security mode reply.    */
  handle_parti_tbl,           /*  0x19 -  partition table.          */
  handle_invalid,             /*  0x1A -    partition table reply.  */
  handle_open_multi,          /*  0x1B -  open multi-image.         */
  handle_invalid,             /*  0x1C -    open multi-image reply. */
  handle_erase,               /*  0x1D -  erase.                    */
  handle_invalid,             /*  0x1E -    erase reply.            */
  handle_invalid,             /*  0x1F -  Get ECC state packet.     */
  handle_invalid,             /*  0x20 -    Current ECC state reply.*/
  handle_invalid,             /*  0x21 -  Set ECC packet.           */
  handle_invalid,             /*  0x22 -    Set ECC reply.          */
  handle_partition_info,      /*  0x23 -  Physical partition info.  */
  handle_invalid,             /*  0x24 -    partition info reply.   */
};

const int max_packet_handler = 0x23;

void packet_process (void)
{
   incoming_t this;
   int command, pos, count;
   word crc;
   int cmd_handled = FALSE;
   
   CHECK_FOR_DATA ();
   
   while (waiting != NULL)
   {
      /* Enable flag, only used by USB */
      process_packet_flag = TRUE;
      
      /* Pull it out of the waiting queue. */
      this = waiting;
      waiting = waiting->next;
      if (waiting == NULL)
        waiting_last = NULL;
      this->next = NULL;
      
      /* Push the debug data out */
      /*
      WRITE_HAPPY_LED(0x0100);
      pos = 0;
      while (pos < this->length)
      {
         WRITE_HAPPY_LED(*(this->buffer + pos) & 0x00ff);
         pos++;
      } */
#ifndef FEATURE_EMMCBLD_NO_HDLC_RX


      if (enable_crc_check)
      {        
        /* Need to verify the CRC.  Unfortunately, the time to compute the
           CRC of the buffer when running out of ram is longer than the
           time to fill up the FIFO. */
         crc = CRC_16_L_STEP_SEED;
         pos = 0;
         while (pos < this->length)
         {
            CHECK_FOR_DATA ();
            count = this->length - pos;
            if (count > 128)
               count = 128;
            crc = crc_16_l_step (crc, this->buffer + pos, count); /*lint !e734 */
            pos += count;
         }

#ifdef DROP_PACKETS
         /* Simulate dropping of packets */
         drop_count++;
         if (drop_count%DROP_MOD == 0)
         {
            term_put('p');
            FLASHLOG(1,("drop pkt %d\n", drop_count));
            goto bogus_packet;        /*lint !e801 */
         }
#endif

         if (crc != CRC_16_L_OK)
         {
            xmit_log_message ("ERR: CRC invalid");
            goto bogus_packet;        /*lint !e801 */
         }
         else
         {
            this->length -= 2;
         }
      }
      else /* if (enable_crc_check) */
      {    
         /* No CRC check, Simply reduce the length by 2 bytes */   
         this->length -= 2;
      }  
#endif
      command = this->buffer[0];
      
      if(command > max_packet_handler)
      {
             struct stream_dl_packet_info pkt_packet;
             uint32 local_reply_length;
             int i;

             pkt_packet.buffer = this->buffer;
             pkt_packet.in_pckt_length = this->length;
             pkt_packet.resp_buffer = reply_buffer_temp;
             pkt_packet.out_pckt_length = &local_reply_length;

             for (i = 0; i < EMMCBLD_MAX_NUM_EXT_INIT_FN; i++)
             {
                 local_reply_length = 0;
                 if ((emmcbld_api_pack[i].handle_command) != NULL)
                 {
                     if((emmcbld_api_pack[i].handle_command)(&pkt_packet) == TRUE)
                         cmd_handled = TRUE;
                 }

                 if (local_reply_length != 0)
                 {
                     /* If reply buffer is not zero, need to send reply buffer out */
                     alignmemcpy (reply_buffer, reply_buffer_temp, local_reply_length);
                                             reply_length = local_reply_length;
                                 compute_reply_crc();
                                 force_xmit_reply(); 
                                    }
             }
      }

      /* Process received command - 
       * Skip command process if hello command is not required */
      if (cmd_handled == FALSE)
      {
          if (command >= FIRST_COMMAND && (command <= max_packet_handler) && (!need_hello || command == 1))
          {
              (*packet_handler[command - FIRST_COMMAND]) (this);      /*lint !e522 */
          }
          else
          {
              handle_invalid (this);    /*lint !e534 */
          }
      }

bogus_packet:
      /* Add back to free list. */
      this->next = free_list;
      free_list = this;
   }

   /* Disable flag, processing is complete */
   process_packet_flag = FALSE;
}


/*===========================================================================

  This function continuously polls the UART for input characters, and processes
  them into receive buffers.


===========================================================================*/

void packet_loop (void)
{
  for (;;)
  {
    FREEZE_WATCHDOG();
    CHECK_FOR_DATA ();
    FREEZE_WATCHDOG();
    packet_process ();
  }
}
