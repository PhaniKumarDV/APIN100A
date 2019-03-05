/*===========================================================================

 eMMC-Boot Loader Download Tool - 

GENERAL DESCRIPTION
 This is the file that contain main_c function where the program enters.  
  
 Copyright (c) 2009 - 2010, 2015 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary

============================================================================*/

/*===========================================================================
 *
 *                           EDIT HISTORY FOR FILE
 *
 *  This section contains comments describing changes made to the module.
 *  Notice that changes are listed in reverse chronological order.
 *
 *  $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_main.c#2 $ 
 *  $DateTime: 2015/12/29 22:44:40 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------------------
 * 2015-12-30   gs      Enable eMMC clocks on USB download
 * 2011-06-15   ah      Added tiny hack for DALSYS_BusyWait for 8960 scons meta build
 * 2011-02-14   rh      Removed "inc" in include path since now handled by scons
 * 2010-10-29   rh      Adding support for loading external modules
 * 2010-06-23   rh      Utilize the new debug bookmark macros
 * 2010-04-21   vj      Featurize usage of UART for 7x30
 * 2010-01-12   vj      Added Support for 7x27 target
 * 2009-10-01   rh      Initial Creation
 *==========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/ 
#include <string.h>

#include "emmcbld.h"
#include "sdcc_api.h"
#include "emmcbld_debug.h"
#include "emmcbld_memctl.h"
#include "emmcbld_msm.h"
#include "emmcbld_bsp.h"
#include "emmcbld_packet_api.h"

/* If this value has to be changed, please also change in emmcbld_start.s */
#define SVC_Stack_Size   0x4000
/*===========================================================================

                           DEFINITIONS

===========================================================================*/

/* Main stack used by jsdc_start.s */
//dword svc_stack[(SVC_Stack_Size/4)+4]; 

/* Pointer to UART/USB function dispatch table */
DISPPTR disptbl;

/* Extern Definitions */
/* Dispatch tables for transports.*/
extern DISP uart_dispatch_table;
extern DISP usb_dispatch_table;

/* Function Prototypes */
void init_interface_to_host (void);
void empty_init_function(struct emmcbld_init_info *p_info);

/* Table contain inti functions */
void (*emmcbld_init_fn_en1)(struct emmcbld_init_info *p_info) = empty_init_function;
void (*emmcbld_init_fn_en2)(struct emmcbld_init_info *p_info) = empty_init_function;
void (*emmcbld_init_fn_en3)(struct emmcbld_init_info *p_info) = empty_init_function;
void (*emmcbld_init_fn_en4)(struct emmcbld_init_info *p_info) = empty_init_function;

struct emmcbld_init_info emmcbld_api_pack[EMMCBLD_MAX_NUM_EXT_INIT_FN];

/* Define default subsystem handler function */
#ifndef EMMCBLD_INIT_FN_REG_ENTRY1
#define EMMCBLD_INIT_FN_REG_ENTRY1 empty_init_function
#endif

#ifndef EMMCBLD_INIT_FN_REG_ENTRY2
#define EMMCBLD_INIT_FN_REG_ENTRY2 empty_init_function
#endif

#ifndef EMMCBLD_INIT_FN_REG_ENTRY3
#define EMMCBLD_INIT_FN_REG_ENTRY3 empty_init_function
#endif

#ifndef EMMCBLD_INIT_FN_REG_ENTRY4
#define EMMCBLD_INIT_FN_REG_ENTRY4 empty_init_function
#endif

/*===========================================================================

                      FUNCTION PROTOTYPES

===========================================================================*/

/*===========================================================================

DESCRIPTION
  This function calls the appropriate routine based on what TRACE32 told us.

DEPENDENCIES
   Assumes jtag_flash_param (global variable) has valid parameters.

RETURN VALUE
  None.

SIDE EFFECTS

===========================================================================*/
void main_c(void)
{

   emmcbld_bsp_hw_init();
   /* Setup the profiling interface during startup */
   BPROFILE_WRITE_SYNC ();

   /* initializes the USB interface */
   init_interface_to_host ();

   /* Initialize the packet module.  */
   packet_init ();               

   /* Call any sub units if they are registered */
   emmcbld_init_fn_en1 = EMMCBLD_INIT_FN_REG_ENTRY1;
   emmcbld_init_fn_en2 = EMMCBLD_INIT_FN_REG_ENTRY2; 
   emmcbld_init_fn_en3 = EMMCBLD_INIT_FN_REG_ENTRY3;
   emmcbld_init_fn_en4 = EMMCBLD_INIT_FN_REG_ENTRY4;

   (emmcbld_init_fn_en1)(&emmcbld_api_pack[0]);
   (emmcbld_init_fn_en2)(&emmcbld_api_pack[1]);
   (emmcbld_init_fn_en3)(&emmcbld_api_pack[2]);
   (emmcbld_init_fn_en4)(&emmcbld_api_pack[3]);

   /*  Stuff a packet and handle commands - never returns */
   packet_loop ();

   /* It should not get to this point */
   while(1)
   {
      //emmcbld_clk_busy_wait(10000);
   }
} /* main_c */


/*===========================================================================

  This function does the required hardware initialization and identifies the
  interface used by the host to download the software into the MSM. It uses
  the functions provided by the UART/USB modules to do all the
  necessary checks on the UART/USB status registers.

  The dispatch table pointer for the proper transport interface (UART/USB)
  is set up.
===========================================================================*/

static void init_interface_to_host (void)
{
  boolean usb_running  = FALSE;
#ifndef FEATURE_EMMCBLD_DISABLE_UART
  boolean uart_running = FALSE;
#endif /* FEATURE_EMMCBLD_DISABLE_UART */

  /*
   * Call USB init function which may return TRUE or FALSE.  If USB is
   * running, it is because DLOAD used it to download this binary to the
   * target, just use it, do not even call any UART functions.
   */

  usb_running = (usb_dispatch_table.init) ();
  if (usb_running == TRUE)
  {
    /* Wait here until USB becomes active with characters */
    
    while ((usb_dispatch_table.active) () == FALSE)
    {
       FREEZE_WATCHDOG();
    }
    disptbl = &usb_dispatch_table;
    return;
  }

#ifndef FEATURE_EMMCBLD_DISABLE_UART
  uart_running = (uart_dispatch_table.init) ();
  if (uart_running == TRUE)
  {
    /* Wait here until UART becomes active with characters */
    while ((uart_dispatch_table.active) () == FALSE)
    {
       FREEZE_WATCHDOG();
    }
    disptbl = &uart_dispatch_table;
    return;
  }
#endif /* FEATURE_EMMCBLD_DISABLE_UART */
  return;
}

/* Stub code required to compile USB driver in HOSTDL environment */
uint32 rex_int_lock ( void )
{
   return 0;
} 

uint32 rex_int_free ( void )
{
   return 0;
} 
/*
void clk_pause (int microseconds)
{
  emmcbld_clk_busy_wait (100 * microseconds);
}
*/
void boot_hw_kick_watch_dog()
{
   ;
}

void empty_init_function(struct emmcbld_init_info *p_info)
{
   p_info->handle_command = NULL;
   p_info->handle_hello_command = NULL;
}

// With approval from RH of SDCC team :)
void DALSYS_BusyWait(uint32 pause_time_us)
{
	int i;
        for (i=0;i<1000;i++);
}

