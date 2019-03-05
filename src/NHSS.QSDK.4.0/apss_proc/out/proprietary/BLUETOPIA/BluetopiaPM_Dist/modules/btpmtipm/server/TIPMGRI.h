/*****< tipmgri.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGRI - TIP Manager Implementation for Stonestreet One Bluetooth        */
/*            Protocol Stack Platform Manager (Platform specific portion).    */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/20/13  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __TIPMGRIH__
#define __TIPMGRIH__

#include "SS1BTCTS.h"            /* Bluetopia CTS Prototypes/Constants.       */
#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to to     */
   /* fetch the current time of the current platform (implementation    */
   /* specific) and formats it in the format required for the Current   */
   /* Time Service.  This function returns TRUE if successful, or FALSE */
   /* if there was an error.                                            */
Boolean_t _TIPM_Get_Current_Platform_Time(CTS_Current_Time_Data_t *CurrentTime, unsigned long AdjustMask);

#endif
