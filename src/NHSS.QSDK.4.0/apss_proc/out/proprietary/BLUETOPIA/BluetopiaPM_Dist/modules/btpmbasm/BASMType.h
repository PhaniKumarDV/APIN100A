/*****< hrpmtype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BASMTYPE - Battery Service Manager API Type Definitions and Constants     */
/*             for Stonestreet One Bluetooth Protocol Stack Platform Manager. */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BASMTYPEH__
#define __BASMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "BASMMSG.h"      /* BTPM Battery Service Manager Message Formats.    */

   /* The following enumerated type represents all of the Connection    */
   /* types that are supported by BASM.                                 */
typedef enum _tagBASM_Connection_Type_t
{
   bctServer,
   bctClient
} BASM_Connection_Type_t;

#endif

