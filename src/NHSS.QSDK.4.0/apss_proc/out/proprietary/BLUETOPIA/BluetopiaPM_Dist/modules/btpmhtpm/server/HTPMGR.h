/*****< htpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMGR - Health Thermometer Manager Implementation for Stonestreet One    */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HTPMGRH__
#define __HTPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHTS.h"            /* HTS  Framework Prototypes/Constants.      */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HTP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HTP Manager  */
   /* Implementation.                                                   */
int _HTPM_Initialize(void);

   /* The following function is responsible for shutting down the HTP   */
   /* Manager Implementation.  After this function is called the HTP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HTPM_Initialize() function.  */
void _HTPM_Cleanup(void);

   /* The following function is responsible for informing the HTP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HTP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HTPM_SetBluetoothStackID(unsigned int BluetoothStackID);

#endif
