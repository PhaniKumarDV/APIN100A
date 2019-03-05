/*****< hogmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMGR - HID over GATT Manager Implementation for Stonestreet One         */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HOGMGRH__
#define __HOGMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHIDS.h"           /* HIDS Framework Prototypes/Constants.      */
#include "SS1BTSCP.h"            /* ScPs Framework Prototypes/Constants.      */
#include "SS1BTBAS.h"            /* BAS  Framework Prototypes/Constants.      */
#include "SS1BTDIS.h"            /* DIS  Framework Prototypes/Constants.      */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HOGM_Initialize(void);

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HOGM_Initialize() function.  */
void _HOGM_Cleanup(void);

   /* The following function is responsible for informing the HID       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HID Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HOGM_SetBluetoothStackID(unsigned int BluetoothStackID);

#endif
