#ifndef __EMMCBLD_MSM_H
#define __EMMCBLD_MSM_H
/*===========================================================================

  MSM specific configuration

DESCRIPTION
  This file contains definitions specific to the Target
  MSM.

 Copyright (c) 2009 - 2010 Qualcomm Technologies Incorporated.
 All Rights Reserved.
 Qualcomm Confidential and Proprietary
===========================================================================*/


/*===========================================================================
 *
 *                     EDIT HISTORY FOR FILE
 *
 * This section contains comments describing changes made to the module.
 * Notice that changes are listed in reverse chronological order.
 *
 *
 * $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_msm.h#1 $ 
 * $DateTime: 2014/09/09 14:47:07 $ 
 * $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2010-06-16   rh      Move the watchdog related function to BSP structure
 * 2010-01-12   vj      Added Support for 7x27 target
 * 2008-06-18   vtw     Initial version, ported from mjnand.
 *
 *===========================================================================*/
 
#include "msm.h"

/* Map the watchdot to the BSP routine */
#define KICK_WATCHDOG          emmcbld_bsp_kick_wdog
#define FREEZE_WATCHDOG        emmcbld_bsp_freeze_wdog


#endif /* __EMMCBLD_MSM_H */

