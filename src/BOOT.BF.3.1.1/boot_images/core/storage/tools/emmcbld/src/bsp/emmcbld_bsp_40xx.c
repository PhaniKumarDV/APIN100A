/*===========================================================================

 EMMCBLD_BSP_SCORPION.c

GENERAL DESCRIPTION
 This file contain the platform dependent code that is SCorpion processor
 specific

  
 Copyright (c) 2009 - 2010, 2015 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary

============================================================================*/

/*===========================================================================

                           EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/bsp/emmcbld_bsp_40xx.c#3 $ 
  $DateTime: 2015/12/29 22:44:40 $ 
  $Author: pwbldsvc $

YYYY-MM-DD   who     what, where, why
----------   ---     ----------------------------------------------------------
2015-12-30   gs      Enable eMMC clocks on usb download
2015-01-08   nm      ipq40xx specific changes
2012-12-12   sy      Update Pmic header file
2011-06-15   ah      8960 bringup

===========================================================================*/

#include "HALhwio.h"

#include "msm.h"
#include "emmcbld_bsp.h"
#include "sdcc_api.h"
#include "SpmiBus.h"
#include "SpmiCfg.h"
#include "pm_pon.h"

extern void qhsusb_dci_write_scratch(uint32 core_id,uint8 offset,uint32 data);

/******************************************************************************
*
* Description:
*    This function Resets the target
*
******************************************************************************/
void emmcbld_bsp_target_reset(void)
{
  SpmiCfg_Init(TRUE);
  SpmiBus_Init();

  /* clear debug software entry register before pulling ps_hold low. */
  HWIO_OUT( TCSR_RESET_DEBUG_SW_ENTRY, 0x0 );

  /* pull down pmic PS_HOLD to force a reset*/
  HWIO_OUTF( GCNT_PSHOLD, GCNT_PSHOLD_PSHOLD, 0x0 );

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
  HWIO_OUTF(GCC_SDCC1_AHB_CBCR, CLK_ENABLE, 1);
  HWIO_OUTF(GCC_SDCC1_APPS_CBCR, CLK_ENABLE, 1);
}

/******************************************************************************
*
* Description:
*    Kick the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_kick_wdog(void)
{ 
   //HWIO_OUT(APCS_WDOG_CTL, 1);
   //HWIO_OUT(APCS_WDOG_BARK_TIME, 0xFFFFF);
   //HWIO_OUT(APCS_WDOG_RESET, 1);
   
}

/******************************************************************************
*
* Description:
*    Disable the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_freeze_wdog(void)
{
   //HWIO_OUT(APCS_WDOG_CTL, 0);
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
/*
void emmcbld_clk_busy_wait(uint32 us)
{
   uint32 count;
   HWIO_OUT(APCS_GPT0_CNT, 0);
   HWIO_OUT(APCS_GPT0_EN, 1);
   
   count = us * 0.032768;
   while (HWIO_IN(APCS_GPT0_CNT) < count);
   HWIO_OUT(APCS_GPT0_EN, 0);
}
*/

/*
#define HWIO_OUT(hwiosym, val)                                   __msmhwio_out(hwiosym, val)
#define __msmhwio_out(hwiosym, val)                             HWIO_##hwiosym##_OUT(val)

#define HWIO_FMSK(hwio_regsym, hwio_fldsym)              __msmhwio_fmsk(hwio_regsym, hwio_fldsym)
#define __msmhwio_fmsk(hwiosym, hwiofldsym)                     HWIO_##hwiosym##_##hwiofldsym##_BMSK

//////HWIO_OUT(APCS_WDT0_RST, HWIO_FMSK(APCS_WDT0_RST, STB));
////HWIO_APCS_WDT0_RST_OUT(HWIO_APCS_WDT0_RST_STB_BMSK)
//HWIO_APCS_WDOG_RESET_OUT(HWIO_APCS_WDOG_RESET_RESET_BMSK)
HWIO_OUT(APCS_WDOG_RESET, HWIO_FMSK(APCS_WDOG_RESET, RESET))
---
//////HWIO_OUT(TCSR_SMPSS_WDOG_CFG, HWIO_FMSK(TCSR_SMPSS_WDOG_CFG, SCSS_WDT0CPU0BITEEXPIRED));
//HWIO_TCSR_SMPSS_WDOG_CFG_OUT(HWIO_TCSR_SMPSS_WDOG_CFG_SCSS_WDT0CPU0BITEEXPIRED_BMSK)

---
//////HWIO_OUT(APCS_WDT0_EN, HWIO_FMSK(APCS_WDT0_EN, EN));
////HWIO_APCS_WDT0_EN_OUT(HWIO_APCS_WDT0_EN_EN_BMSK)
//HWIO_APCS_WDOG_CTL_OUT(HWIO_APCS_WDOG_CTL_ENABLE_BMSK)
HWIO_OUT(APCS_WDOG_CTL, HWIO_FMSK(APCS_WDOG_CTL, ENABLE))
*/
