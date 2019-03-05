/*===========================================================================

 EMMCBLD_BSP_SCORPION.c

GENERAL DESCRIPTION
 This file contain the platform dependent code that is SCorpion processor
 specific

  
 Copyright (c) 2009 - 2010 Qualcomm Technologies Incorporated.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary

============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/bsp/emmcbld_bsp_krait.c#1 $ 
  $DateTime: 2014/09/09 14:47:07 $ 
  $Author: pwbldsvc $

YYYY-MM-DD   who     what, where, why
----------   ---     ----------------------------------------------------------
2011-06-15   ah      8960 bringup

===========================================================================*/


#include "msm.h"
#include "emmcbld_bsp.h"
#include "sdcc_api.h"

/******************************************************************************
*
* Description:
*    This function Resets the target
*
******************************************************************************/
void emmcbld_bsp_target_reset(void)
{
//  HWIO_OUT(SCSS_WDT0_RST, HWIO_FMSK(SCSS_WDT0_RST, STB));
//  HWIO_OUT(TCSR_SMPSS_WDOG_CFG, 
//  		HWIO_FMSK(TCSR_SMPSS_WDOG_CFG, SCSS_WDT0CPU0BITEEXPIRED));
//  HWIO_OUT(SCSS_WDT0_EN, HWIO_FMSK(SCSS_WDT0_EN, EN));
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
}

/******************************************************************************
*
* Description:
*    Kick the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_kick_wdog(void)
{
   HWIO_OUT(APCS_WDT0_RST, HWIO_FMSK(APCS_WDT0_RST, STB));
   HWIO_OUT(TCSR_SMPSS_WDOG_CFG, 
   HWIO_FMSK(TCSR_SMPSS_WDOG_CFG, SCSS_WDT0CPU0BITEEXPIRED));
   HWIO_OUT(APCS_WDT0_EN, HWIO_FMSK(APCS_WDT0_EN, EN));
}

/******************************************************************************
*
* Description:
*    Disable the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_freeze_wdog(void)
{
   HWIO_OUT(APCS_WDT0_FRZ, HWIO_FMSK(APCS_WDT0_FRZ, FRZ));
   HWIO_OUT(TCSR_SMPSS_WDOG_CFG, 0);
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
   uint32 count;
   HWIO_OUT(APCS_GPT0_CNT, 0);
   HWIO_OUT(APCS_GPT0_EN, 1);
   
   count = us * 0.032768;
   while (HWIO_IN(APCS_GPT0_CNT) < count);
   HWIO_OUT(APCS_GPT0_EN, 0);
}

