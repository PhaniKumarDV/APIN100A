/*****< glpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMGR - Glucose Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GLPMGRH__
#define __GLPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTGLS.h"            /* GLS Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the GLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager GLP Manager  */
   /* Implementation.                                                   */
int _GLPM_Initialize(void);

   /* The following function is responsible for shutting down the GLP   */
   /* Manager Implementation.  After this function is called the GLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _GLPM_Initialize() function.  */
void _GLPM_Cleanup(void);

   /* The following function is responsible for informing the GLP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the GLP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _GLPM_SetBluetoothStackID(unsigned int BluetoothStackID);

#endif
