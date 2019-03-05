/*****< fmpmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMTYPE - Find Me Profile Manager API Type Definitions and Constants for */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FMPMTYPEH__
#define __FMPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "FMPMMSG.h"      /* BTPM Find Me Profile Manager Message Formats.    */

   /* The following enumerated type is used to indicate the specific    */
   /* type of Find Me connection.                                       */
typedef enum
{
   fctTarget,
   fctLocator
} FMPM_Connection_Type_t;

   /* The following enumerated type contains all of the valid alert     */
   /* levels that may be set using this profile.                        */
typedef enum
{
   falNoAlert,
   falMildAlert,
   falHighAlert
} FMPM_Alert_Level_t;

#endif

