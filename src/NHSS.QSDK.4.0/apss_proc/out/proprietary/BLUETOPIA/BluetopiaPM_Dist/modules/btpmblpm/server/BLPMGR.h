/*****< blpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMGR - BLP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BLPMGRH__
#define __BLPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the BLP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager BLP Manager  */
   /* Implementation.                                                   */
int _BLPM_Initialize(void);

   /* The following function is responsible for shutting down the BLP   */
   /* Manager Implementation.  After this function is called the BLP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _BLPM_Initialize() function.  */
void _BLPM_Cleanup(void);

   /* The following function is responsible for informing the BLP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the BLP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _BLPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to read an Attribute Handle on a remote device.  The      */
   /* function accepts the Bluetooth Address of the remote device and   */
   /* the Attribute Handle of the attribute on the remote device to     */
   /* read.  This function returns a positive non-zero Transaction ID if*/
   /* successful, or a negative return error code if there was an error.*/
int _BLPM_Read_Value(BD_ADDR_t *RemoteServer, Word_t Handle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write to an Attribute Handle on a remote device.  The  */
   /* function accepts the Bluetooth Address of the remote device, the  */
   /* Attribute Handle of the attribute on the remote device to write,  */
   /* the length of the data, and a pointer to the data that will be    */
   /* written.  This function returns a positive non-zero Transaction ID*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _BLPM_Write_Value(BD_ADDR_t *RemoteServer, Word_t Handle, Word_t DataLength, Byte_t *Data);

   /* The following is a utility function that is used to cancel an     */
   /* outstanding transaction.  The first parameter to this function is */
   /* the TransactionID of the transaction that is being canceled.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _BLPM_Cancel_Transaction(unsigned int TransactionID);

#endif
