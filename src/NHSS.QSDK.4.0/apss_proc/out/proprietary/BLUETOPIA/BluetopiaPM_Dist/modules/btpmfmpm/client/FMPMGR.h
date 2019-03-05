/*****< fmpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FMPMGR - FMP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/27/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __FMPMGRH__
#define __FMPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTFMPM.h"           /* FMP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the FMP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager FMP Manager  */
   /* Implementation.                                                   */
int _FMPM_Initialize(void);

   /* The following function is responsible for shutting down the FMP   */
   /* Manager Implementation.  After this function is called the FMP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _FMPM_Initialize() function.  */
void _FMPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Alert            */
   /* Notification (FMP) Manager Service.  This Callback will be        */
   /* dispatched by the FMP Manager when various FMP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a FMP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _FMPM_Un_Register_Target_Events() function to un-register*/
   /*          the callback from this module.                           */
int _FMPM_Register_Target_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered FMP Target Manager Event      */
   /* Callback (registered via a successful call to the                 */
   /* _FMPM_Register_Target_Events() function).  This function accepts  */
   /* as input the FMP Manager Event Callback ID (return value from     */
   /* _FMPM_Register_Target_Events() function).                         */
int _FMPM_Un_Register_Target_Events(unsigned int FMPTargetEventHandlerID);

#endif
