/*****< pxpmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMTYPE - Proximity Manager API Type Definitions and Constants for       */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMTYPEH__
#define __PXPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "PXPMMSG.h"      /* BTPM Proximity Manager Message Formats.          */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Proximity connection.  A Monitor value indicates the local*/
   /* device is acting as a Proximity Monitor.  A Report value indicates*/
   /* the local device is acting as a Proximity Reporter.               */
typedef enum
{
   pctPXPMonitor,
   pctPXPReporter
} PXPM_Connection_Type_t;

   /* The following enumerated type contains all of the valid alert     */
   /* levels that may be set using this profile.                        */
typedef enum
{
   alNoAlert,
   alMildAlert,
   alHighAlert
} PXPM_Alert_Level_t;

#endif

