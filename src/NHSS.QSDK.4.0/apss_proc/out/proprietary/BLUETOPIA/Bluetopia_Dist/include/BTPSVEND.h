/*****< btpsvend.h >***********************************************************/
/*      Copyright 2008 - 2012 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPSVEND - Vendor specific functions/definitions/constants used to define */
/*             a set of vendor specific functions supported by the Bluetopia  */
/*             Protocol Stack.  These functions may be unique to a given      */
/*             hardware platform.                                             */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/09/09  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPSVENDH__
#define __BTPSVENDH__

#include "SS1BTPS.h"

typedef struct _tagVendParams_t
{
   BD_ADDR_t BD_ADDR;
   Word_t    CoExMode;
   Word_t    XcalTrim;
} VendParams_t;

#include "BVENDAPI.h"           /* BTPS Vendor Specific Prototypes/Constants. */

#endif
