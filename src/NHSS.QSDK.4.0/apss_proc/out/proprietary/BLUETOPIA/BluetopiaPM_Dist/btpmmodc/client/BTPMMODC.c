/*****< btpmmodc.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMOD - Installable Module Handler for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/12/10  D. Lange       Initial creation.                               */
/******************************************************************************/

#include "SS1BTPM.h"             /* BTPM API Prototypes and Constants.        */

#include "BTPMMODC.h"            /* BTPM Module Handler List.                 */

#ifdef MODC_SUPPORT_ANCM
#include "SS1BTANCM.h"           /* Apple Notification Center Service Module. */
#endif

#ifdef MODC_SUPPORT_ANPM
#include "SS1BTANPM.h"           /* Alert Notification Manager Module.        */
#endif

#ifdef MODC_SUPPORT_ANTM
#include "SS1BTANTM.h"           /* ANT+ Manager Module.                      */
#endif

#ifdef MODC_SUPPORT_AUDM
#include "SS1BTAUDM.h"           /* Audio Manager Module.                     */
#endif

#ifdef MODC_SUPPORT_BASM
#include "SS1BTBASM.h"           /* Battery Service Manager Module.           */
#endif

#ifdef MODC_SUPPORT_BLPM
#include "SS1BTBLPM.h"           /* Blood Pressure Profile Manager Module.    */
#endif

#ifdef MODC_SUPPORT_CPPM
#include "SS1BTCPPM.h"           /* Cycling Power Profile Manager Module      */
#endif

#ifdef MODC_SUPPORT_CSCM
#include "SS1BTCSCM.h"           /* Cycling Speed Cadence Manager Module.     */
#endif

#ifdef MODC_SUPPORT_FMPM
#include "SS1BTFMPM.h"           /* Find Me Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_FTPM
#include "SS1BTFTPM.h"           /* File Transfer Manager Module.             */
#endif

#ifdef MODC_SUPPORT_GLPM
#include "SS1BTGLPM.h"           /* Glucose Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_HDDM
#include "SS1BTHDDM.h"           /* HID Device Manager Module.                */
#endif

#ifdef MODC_SUPPORT_HDPM
#include "SS1BTHDPM.h"           /* Health Device Manager Module.             */
#endif

#ifdef MODC_SUPPORT_HDSM
#include "SS1BTHDSM.h"           /* Headset Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_HFRM
#include "SS1BTHFRM.h"           /* Hands Free Manager Module.                */
#endif

#ifdef MODC_SUPPORT_HIDM
#include "SS1BTHIDM.h"           /* HID Host Manager Module.                  */
#endif

#ifdef MODC_SUPPORT_HOGM
#include "SS1BTHOGM.h"           /* HID over GATT Manager Module.             */
#endif

#ifdef MODC_SUPPORT_HRPM
#include "SS1BTHRPM.h"           /* Heart Rate Manager Module.                */
#endif

#ifdef MODC_SUPPORT_HTPM
#include "SS1BTHTPM.h"           /* Health Thermometer Manager Module.        */
#endif

#ifdef MODC_SUPPORT_MAPM
#include "SS1BTMAPM.h"           /* Message Access Manager Module.            */
#endif

#ifdef MODC_SUPPORT_OPPM
#include "SS1BTOPPM.h"           /* Object Push Manager Module.               */
#endif

#ifdef MODC_SUPPORT_PANM
#include "SS1BTPANM.h"           /* PAN Manager Module.                       */
#endif

#ifdef MODC_SUPPORT_PASM
#include "SS1BTPASM.h"           /* Phone Alert Status Manager Module.        */
#endif

#ifdef MODC_SUPPORT_PBAM
#include "SS1BTPBAM.h"           /* Phone Book Access Host Manager Module.    */
#endif

#ifdef MODC_SUPPORT_PXPM
#include "SS1BTPXPM.h"           /* Proximity Manager Module.                 */
#endif

#ifdef MODC_SUPPORT_RSCM
#include "SS1BTRSCM.h"           /* Running Speed Cadence Manager Module.     */
#endif

#ifdef MODC_SUPPORT_TDSM
#include "SS1BTTDSM.h"           /* 3D Sync Manager Module.                   */
#endif

#ifdef MODC_SUPPORT_TIPM
#include "SS1BTTIPM.h"           /* Time Manager Module.                      */
#endif

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

   /* Main Module list that contains all configured modules.            */
   /* * NOTE * HID Device support is left out as both Host and Device   */
   /*          require the same psm.                                    */
static MOD_ModuleHandlerEntry_t ModuleHandlerList[] =
{
#ifdef MODC_SUPPORT_ANCM
   { ANCM_InitializationHandlerFunction, NULL, ANCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_ANPM
   { ANPM_InitializationHandlerFunction, NULL, ANPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_ANTM
   { ANTM_InitializationHandlerFunction, NULL, ANTM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_AUDM
   { AUDM_InitializationHandlerFunction, NULL, AUDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_BASM
   { BASM_InitializationHandlerFunction, NULL, NULL                              },
#endif
#ifdef MODC_SUPPORT_BLPM
   { BLPM_InitializationHandlerFunction, NULL, NULL                              },
#endif
#ifdef MODC_SUPPORT_CPPM
   { CPPM_InitializationHandlerFunction, NULL, CPPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_CSCM
   { CSCM_InitializationHandlerFunction, NULL, CSCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_FMPM
   { FMPM_InitializationHandlerFunction, NULL, FMPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_FTPM
   { FTPM_InitializationHandlerFunction, NULL, FTPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_GLPM
   { GLPM_InitializationHandlerFunction, NULL, GLPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDDM
//   { HDDM_InitializationHandlerFunction, NULL, HDDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDPM
   { HDPM_InitializationHandlerFunction, NULL, HDPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HDSM
   { HDSM_InitializationHandlerFunction, NULL, HDSM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HFRM
   { HFRM_InitializationHandlerFunction, NULL, HFRM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HIDM
   { HIDM_InitializationHandlerFunction, NULL, HIDM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HOGM
   { HOGM_InitializationHandlerFunction, NULL, HOGM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_HRPM
   { HRPM_InitializationHandlerFunction, NULL, NULL                              },
#endif
#ifdef MODC_SUPPORT_HTPM
   { HTPM_InitializationHandlerFunction, NULL, HTPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_MAPM
   { MAPM_InitializationHandlerFunction, NULL, MAPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_OPPM
   { OPPM_InitializationHandlerFunction, NULL, OPPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PANM
   { PANM_InitializationHandlerFunction, NULL, PANM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PASM
   { PASM_InitializationHandlerFunction, NULL, PASM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PBAM
   { PBAM_InitializationHandlerFunction, NULL, PBAM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_PXPM
   { PXPM_InitializationHandlerFunction, NULL, PXPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_RSCM
   { RSCM_InitializationHandlerFunction, NULL, RSCM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_TIPM
   { TIPM_InitializationHandlerFunction, NULL, TIPM_DeviceManagerHandlerFunction },
#endif
#ifdef MODC_SUPPORT_TDSM
   { TDSM_InitializationHandlerFunction, NULL, TDSM_DeviceManagerHandlerFunction },
#endif
   { NULL,                               NULL, NULL                              }
} ;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Module Handler Service.  This function */
   /* returns a pointer to a Module Handler Entry List (or NULL if there*/
   /* were no Modules installed).  The returned list will simply be an  */
   /* array of Module Handler Entries, with the last entry in the list  */
   /* signified by an entry that has NULL present for all Module Handler*/
   /* Functions.                                                        */
MOD_ModuleHandlerEntry_t *BTPSAPI MOD_GetModuleList(void)
{
   MOD_ModuleHandlerEntry_t *ret_val;

   DebugPrint((BTPM_DEBUG_ZONE_MODULE_MANAGER | BTPM_DEBUG_LEVEL_FUNCTION), ("Enter\n"));

   ret_val = ModuleHandlerList;

   DebugPrint((BTPM_DEBUG_ZONE_MODULE_MANAGER | BTPM_DEBUG_LEVEL_FUNCTION), ("Exit: %p\n", ret_val));

   /* Simply return the Module Handler List.                            */
   return(ret_val);
}
