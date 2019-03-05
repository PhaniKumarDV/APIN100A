/*===========================================================================

 EMMCBLD_BSP.c

GENERAL DESCRIPTION
 This file contain the platform dependent code that is shared across
 all platform.

  
 Copyright (c) 2009 - 2010 Qualcomm Technologies Incorporated.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary

============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/bsp/emmcbld_bsp.c#1 $ 
  $DateTime: 2014/09/09 14:47:07 $ 
  $Author: pwbldsvc $

YYYY-MM-DD   who     what, where, why
----------   ---     ----------------------------------------------------------
2010-06-23   rh      Move watchdog related function into BSP
2010-01-12   vj      Added Support for 7x27 target
2009-10-01   rh      Initial Creation

===========================================================================*/


#include "msm.h"
#include "emmcbld_bsp.h"
#include "sdcc_api.h"
#include "emmcbld_msm.h"

#define BUS_SPEED_IN_MHZ      20


/******************************************************************************
*
* Description:
*    Disable the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_freeze_wdog(void)
{
   HWIO_OUT(WDOG_FREEZE, HWIO_FMSK(WDOG_FREEZE,WDOG_FREEZE));
}

/******************************************************************************
*
* Description:
*    Kick the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_kick_wdog(void)
{
   HWIO_OUT(WDOG_RESET, HWIO_FMSK(WDOG_RESET,WATCH_DOG));
}

/******************************************************************************
*
* Description:
*    This function Resets the target
*
******************************************************************************/
void emmcbld_bsp_target_reset(void)
{
   emmcbld_bsp_kick_wdog();

   for (;;)
   {
     /* let the watch dog timer expire here... */
   }
}

/******************************************************************************
*
* Description:
*    This function performs the HW Init for eHOSTDL
*    The HW init includes enabling SDRAM, Clocks etc...
*      
******************************************************************************/
void emmcbld_bsp_hw_init(void)
{
   /* FREEZE WatchDog till handle_reset is invoked when host sends RESET req */
   emmcbld_bsp_freeze_wdog();
}

/******************************************************************************
*
* Description:
*    This function provides a busy wait for the caller.
*
* Arguments:
*    us              [IN] : time in micro seconds
*      
******************************************************************************/
void emmcbld_clk_busy_wait(uint32 us)
{
  uint32 wait_remaining_cycle =0;/* how many cycles we have to wait*/
  uint32 pause_cycle=0;/* max is 65535. unit is clock cycle*/

  /* How many cycles we should stall? */
  wait_remaining_cycle = BUS_SPEED_IN_MHZ * us;

  /* Each penalty of write is 13 cycles */

  while (wait_remaining_cycle >= 13)
  {
    /* Max of PAUSE TIME stall time is 65535 cycles */
    pause_cycle = MIN(65535, (wait_remaining_cycle-13));
    HWIO_OUT (MSSAHB_PAUSE_TIMER, pause_cycle);
    wait_remaining_cycle -= (pause_cycle + 13);
  }
}


