/*==================================================================
 *
 * FILE:        msm.h
 *
 * DESCRIPTION: Consolidated header file for all target specific public HWIO macro
 * definitions.

 * This file encapsulates all the MSM hardware specific header files,
 * including those for HWIO register access.  It defines no interfaces
 * itself but simply pulls in all required header files.
 *   
 *
 *        Copyright © 2008-2013 Qualcomm Technologies, Inc.
 *               All Rights Reserved.
 *               QUALCOMM Proprietary
 *==================================================================*/

/*===================================================================
 *
 *                       EDIT HISTORY FOR FILE
 *
 *   This section contains comments describing changes made to the
 *   module. Notice that changes are listed in reverse chronological
 *   order.
 *
 *   $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/deviceprogrammer/src/bsp/msm.h#2 $ $DateTime: 2015/07/22 10:35:57 $ $Author: pwbldsvc $
 *   $DateTime: 2015/07/22 10:35:57 $ $Author: pwbldsvc $
 *
 * YYYY-MM-DD   who     what, where, why
 * ----------   ---     ----------------------------------------------
 * 2014-07-02   sb      Updated to support 8909
 * 2013-06-03   ah      Added legal header
 * 2013-05-31   ab      Initial checkin
 */

#ifndef __MSM_H__
#define __MSM_H__

/*=========================================================================
      Include Files
==========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * Common types.
 */
#ifndef _ARM_ASM_
#include "comdef.h"
#endif

/*
 * Public register definitions
 */
//#include "msmhwioreg.h"

/*
 * Global shadow register definitions.
 */
#ifndef _ARM_ASM_
//#include "msmshadow.h"
#endif

/*
 * HWIO access macros.
 */
#include "msmhwio.h"
#include DEVPROG_HWIO_INCLUDE_H

/*
 * Bit shifting assist macros.
 */
//#include "msmshift.h"


/*
 * For now include private definitions here until drivers move to
 * include msmp.h directly.
 */
#ifdef FEATURE_LIBRARY_ONLY
#include "msmp.h"
#endif


#ifdef __cplusplus
}
#endif    


#endif /* __MSM_H__ */

