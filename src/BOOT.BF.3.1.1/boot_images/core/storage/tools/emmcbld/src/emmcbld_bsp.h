#ifndef __EMMCBLD_BSP_H
#define __EMMCBLD_BSP_H
/******************************************************************************
 * emmcbld_bsp.h
 *
 * This file provides SDCC abstraction for dependent drivers for Non-OS(BOOT).
 *
 * Copyright (c) 2009 - 2010
 * Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 * Qualcomm Confidential and Proprietary
 *
 *****************************************************************************/
/*=============================================================================
 *
 *                       EDIT HISTORY FOR MODULE
 *
 * $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_bsp.h#1 $
 * $DateTime: 2014/09/09 14:47:07 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2010-06-23   rh      Clean up and adding WDOG related furnction to BSP
 * 2010-01-12   vj      Added Support for 7x27 target
 * 2009-05-05   rh      Initial release
 *=============================================================================*/


/******************************************************************************
*
* Description:
*    Disable the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_freeze_wdog(void);

/******************************************************************************
*
* Description:
*    Kick the watchdog timer
*      
******************************************************************************/
void emmcbld_bsp_kick_wdog(void);

/******************************************************************************
*
* Description:
*    This function Resets the target
*
******************************************************************************/
void emmcbld_bsp_target_reset(void);

/******************************************************************************
*
* Description:
*    This function performs the HW Init for eHOSTDL
*    The HW init includes enabling SDRAM, Clocks etc...
*      
******************************************************************************/
void emmcbld_bsp_hw_init(void);

/******************************************************************************
*
* Description:
*    This function provides a busy wait for the caller.
*
* Arguments:
*    us              [IN] : time in micro seconds
*      
******************************************************************************/
void emmcbld_clk_busy_wait(uint32 us);


#endif /* __EMMCBLD_BSP_H */



