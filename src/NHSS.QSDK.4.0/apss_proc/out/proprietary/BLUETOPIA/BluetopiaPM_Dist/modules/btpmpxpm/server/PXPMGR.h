/*****< pxpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMGR - PXP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMGRH__
#define __PXPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PXP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PXP Manager  */
   /* Implementation.                                                   */
int _PXPM_Initialize(void);

   /* The following function is responsible for shutting down the PXP   */
   /* Manager Implementation.  After this function is called the PXP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PXPM_Initialize() function.  */
void _PXPM_Cleanup(void);

   /* The following function is responsible for informing the PXP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PXP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PXPM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the Connection ID for a specified LE connection. */
   /* The function accepts the BD_ADDR of the connection to query the   */
   /* Connection ID for and a pointer to store the Connection ID for.   */
   /* This functions returns ZERO on success or a negative error code.  */
int _PXPM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to read a specified Attribute on the specified remote     */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to read the value from, and the Attribute Handle of the attribute */
   /* on the remote device to read.  This function returns a positive   */
   /* non-zero value if successful, or a negative return error code if  */
   /* there was an error.                                               */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _PXPM_Read_Value(unsigned int ConnectionID, Word_t AttributeHandle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write a specified Attribute on the specified remote    */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to write the value to, the Attribute Handle of the attribute on   */
   /* the remote device to write, the length of the data and a pointer  */
   /* to the data to write.  This function returns a positive non-zero  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _PXPM_Write_Value(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);

   /* The following function is provided to allow a mechanism for local */
   /* modules to perform a write without response to a specified        */
   /* Attribute on the specified remote device.  The first parameter to */
   /* this function is the ConnectionID of the device to write to.  The */
   /* second parameter specifies the handle of the attribute that is to */
   /* be written.  The final two parameters specify the length and a    */
   /* pointer to the data that is to be written.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * No event is generated by this function.                  */
int _PXPM_Write_Value_Without_Response(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);

   /* The following function is a utility function that is used to      */
   /* cancel an outstanding transaction.  The first parameter to this   */
   /* function is the TransactionID of the transaction that is being    */
   /* canceled.  On success this function will return ZERO or a negative*/
   /* error code on failure.                                            */
int _PXPM_Cancel_Transaction(unsigned int TransactionID);

   /* The following function is a utility function that is used to get  */
   /* the RSSI for the specified BD_ADDR (of connected LE Device).      */
int _PXPM_Get_Link_RSSI(BD_ADDR_t BD_ADDR, int *RSSI);

#endif
