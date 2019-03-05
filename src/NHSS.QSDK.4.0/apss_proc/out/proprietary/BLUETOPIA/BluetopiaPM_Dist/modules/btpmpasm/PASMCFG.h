/*****< pasmcfg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMCFG - Stonestreet One Phone Alert Status (PAS) Manager configuration  */
/*            directives/constants.  The information contained in this file   */
/*            controls various compile time parameters that are needed to     */
/*            build Bluetopia Phone Alert Status (PAS) Manager for a          */
/*            specific platform.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __PASMCFGH__
#define __PASMCFGH__

#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

   /* Phone Alert Status Manager Configuration.                         */
#define PASM_CONFIGURATION_DEFAULT_RINGER_STATE                (FALSE)
#define PASM_CONFIGURATION_DEFAULT_VIBRATE_STATE               (FALSE)
#define PASM_CONFIGURATION_DEFAULT_DISPLAY_STATE               (TRUE)
#define PASM_CONFIGURATION_DEFAULT_RINGER_SETTING              (prsNormal)

#endif
