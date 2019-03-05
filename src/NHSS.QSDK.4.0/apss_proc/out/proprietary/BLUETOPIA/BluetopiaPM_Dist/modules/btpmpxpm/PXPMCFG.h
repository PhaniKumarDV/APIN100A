/*****< pxpmcfg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMCFG - Stonestreet One Proximity Manager configuration                 */
/*            directives/constants.  The information contained in this file   */
/*            controls various compile time parameters that are needed to     */
/*            build Bluetopia Proximity Manager for a specific platform.      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMCFGH__
#define __PXPMCFGH__

#include "BTPSKRNL.h"           /* BTPS Kernel Prototypes/Constants.          */

   /* Proximity Module Configuration.                                   */
#define PXPM_CONFIGURATION_MINIMUM_REFRESH_TIME                  1000
#define PXPM_CONFIGURATION_MAXIMUM_REFRESH_TIME                  30000
#define PXPM_CONFIGURATION_DEFAULT_REFRESH_TIME                  3000
#define PXPM_CONFIGURATION_DEFAULT_PATH_LOSS_THRESHOLD           80
#define PXPM_CONFIGURATION_DEFAULT_ALERT_LEVEL                   alMildAlert

#endif
