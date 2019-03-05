/*****< ss1btps.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SS1BTPM - Stonestreet One Bluetopia Platform Manager Type Definitions,    */
/*            Prototypes, and Constants.                                      */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/07/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SS1BTPMH__
#define __SS1BTPMH__

#include "SS1BTPS.h"             /* Bluetooth Stack API Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */
#include "BTPMERR.h"             /* BTPM Error Prototypes/Constants.          */
#include "BTPMVER.h"             /* BTPM Version Constants.                   */

#include "DBGAPI.h"              /* BTPM Debug Module Prototypes/Constants.   */
#include "TMRAPI.h"              /* BTPM Timer Prototypes/Constants.          */
#include "MSGAPI.h"              /* BTPM Message Module Prototypes/Constants. */
#include "DEVMAPI.h"             /* BTPM Device Manager Prototypes/Constants. */
#include "SETAPI.h"              /* BTPM Settings/Configuration Prototypes.   */
#include "BTPMMAPI.h"            /* Main BTPM Handler Prototypes/Constants.   */
#include "SPPMAPI.h"             /* BTPM SPP Manager Prototypes/Constants.    */
#include "SCOMAPI.h"             /* BTPM SCO Manager Prototypes/Constants.    */

#if BTPM_CONFIGURATION_GENERIC_ATTRIBUTE_MANAGER_SUPPORTED

   #include "GATMAPI.h"          /* BTPM GATT Manager Prototypes/Constants.   */

#endif

#endif
