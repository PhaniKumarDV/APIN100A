/*****< basmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BASMGR - BAS Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BASMGRH__
#define __BASMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the BAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager BAS Manager  */
   /* Implementation.                                                   */
int _BASM_Initialize(void);

   /* The following function is responsible for shutting down the BAS   */
   /* Manager Implementation.  After this function is called the BAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _BASM_Initialize() function.  */
void _BASM_Cleanup(void);

   /* The following function is responsible for informing the BAS       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the BAS Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _BASM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to read an Attribute Handle on a remote device.  The      */
   /* function accepts the Bluetooth Address of the remote device and   */
   /* the Attribute Handle of the attribute on the remote device to     */
   /* read.  This function returns a positive non-zero Transaction ID if*/
   /* successful, or a negative return error code if there was an error.*/
int _BASM_Read_Value(BD_ADDR_t *RemoteServer, Word_t Handle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write to an Attribute Handle on a remote device.  The  */
   /* function accepts the Bluetooth Address of the remote device, the  */
   /* Attribute Handle of the attribute on the remote device to write,  */
   /* the length of the data, and a pointer to the data that will be    */
   /* written.  This function returns a positive non-zero Transaction ID*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _BASM_Write_Value(BD_ADDR_t *RemoteServer, Word_t Handle, Word_t DataLength, Byte_t *Data);

   /* The following is a utility function that is used to cancel an     */
   /* outstanding transaction.  The first parameter to this function is */
   /* the TransactionID of the transaction that is being canceled.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _BASM_Cancel_Transaction(unsigned int TransactionID);

#endif
