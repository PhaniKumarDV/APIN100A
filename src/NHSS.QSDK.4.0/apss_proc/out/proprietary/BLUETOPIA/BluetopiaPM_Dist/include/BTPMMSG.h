/*****< btpmmsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMMSG - Interprocess Communication Abstraction layer Message Handling   */
/*            for Stonestreet One Bluetooth Protocol Stack Platform Manager.  */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/04/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMMSGH__
#define __BTPMMSGH__

#include "MSGAPI.h"              /* BTPM Message API Prototypes and Constants.*/

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Message Handling Service.  This        */
   /* function accepts as input, a pointer to an initialization         */
   /* container structure that contains various platform specific       */
   /* initialization information for Message Handler.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Message Handling Service.                                         */
int MSG_Initialize(void *InitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Message Handling Service.  After this  */
   /* function is called the Bluetooth Platform Manager Message         */
   /* Handlinger service will no longer operate until it is initialized */
   /* again via a call to the MSG_Initialize() function.                */
void MSG_Cleanup(void);

#endif
