/*****< blpmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMTYPE - Blood Pressure Manager API Type Definitions and Constants      */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BLPMTYPEH__
#define __BLPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "BLPMMSG.h"      /* BTPM Blood Pressure Manager Message Formats.     */

   /* The following enumerated type represents all of the Connection    */
   /* types that are supported by BLPM.                                 */
typedef enum _tagBLPM_Connection_Type_t
{
   bctSensor,
   bctCollector
} BLPM_Connection_Type_t;

#endif

