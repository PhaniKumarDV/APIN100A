/*****< hrpmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMTYPE - Heart Rate Manager API Type Definitions and Constants          */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __HRPMTYPEH__
#define __HRPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "HRPMMSG.h"      /* BTPM Heart Rate Manager Message Formats.         */

   /* The following enumerated type represents all of the Connection    */
   /* types that are supported by HRPM.                                 */
typedef enum _tagHRPM_Connection_Type_t
{
   hctSensor,
   hctCollector
} HRPM_Connection_Type_t;

   /* The following enumerated type represents all of the Category types*/
   /* that are supported by HRPM.                                       */
typedef enum _tagHRPM_Body_Sensor_Location_t
{
   bslOther,
   bslChest,
   bslWrist,
   bslFinger,
   bslHand,
   bslEarLobe,
   bslFoot
} HRPM_Body_Sensor_Location_t;

#endif

