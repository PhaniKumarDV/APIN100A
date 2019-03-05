/*****< iacptran.c >***********************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  IACPTRAN - Stonestreet One Apple Authentication Coprocessor Transport     */
/*             Layer.                                                         */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/13/11  T. Thomas      Initial creation.                               */
/******************************************************************************/
#include "BTPSKRNL.h"            /* Bluetooth Kernel Prototypes/Constants.    */
#include "IACPTRAN.h"            /* ACP Transport Prototypes/Constants.       */

   /* The following function is responsible for Initializing the ACP    */
   /* hardware.  The function is passed a pointer to an opaque object.  */
   /* Since the hardware platform is not know, it is intended that the  */
   /* parameter be used to pass hardware specific information to the    */
   /* module that would be needed for proper operation.  Each           */
   /* implementation will define what gets passed to this function.     */
   /* This function returns zero if successful or a negative value if   */
   /* the initialization fails.                                         */
int BTPSAPI IACPTR_Initialize(void *Parameters)
{
//xxx
   return(0);
}

   /* The following function is responsible for performing any cleanup  */
   /* that may be required by the hardware or module.  This function    */
   /* returns no status.                                                */
void BTPSAPI IACPTR_Cleanup(void)
{
//xxx
}

   /* The following function is responsible for Reading Data from a     */
   /* device via the ACP interface.  The first parameter to this        */
   /* function is the Register/Address that is to be read.  The second  */
   /* parameter indicates the number of bytes that are to be read from  */
   /* the device.  The third parameter is a pointer to a buffer where   */
   /* the data read is placed.  The size of the Read Buffer must be     */
   /* large enough to hold the number of bytes being read.  The last    */
   /* parameter is a pointer to a variable that will receive status     */
   /* information about the result of the read operation.  This function*/
   /* should return a non-negative return value if successful (and      */
   /* return IACP_STATUS_SUCCESS in the status parameter).  This        */
   /* function should return a negative return error code if there is an*/
   /* error with the parameters and/or module initialization.  Finally, */
   /* this function can also return success (non-negative return value) */
   /* as well as specifying a non-successful Status value (to denote    */
   /* that there was an actual error during the write process, but the  */
   /* module is initialized and configured correctly).                  */
   /* * NOTE * If this function returns a non-negative return value then*/
   /*          the caller will ALSO examine the value that was placed in*/
   /*          the Status parameter to determine the actual status of   */
   /*          the operation (success or failure).  This means that a   */
   /*          successful return value will be a non-negative return    */
   /*          value and the status member will contain the value:      */
   /*          IACP_STATUS_SUCCESS                                      */
int BTPSAPI IACPTR_Read(unsigned char Register, unsigned char BytesToRead, unsigned char *ReadBuffer, unsigned char *Status)
{
   int ret_val;

   /* Verify that the parameters passed in appear valid.                */
   if((BytesToRead) && (ReadBuffer) && (Status))
   {
//xxx
   }
   else
      ret_val = IACP_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for Writing Data to a device*/
   /* via the ACP interface.  The first parameter to this function is   */
   /* the Register/Address where the write operation is to start.  The  */
   /* second parameter indicates the number of bytes that are to be     */
   /* written to the device.  The third parameter is a pointer to a     */
   /* buffer where the data to be written is placed.  The last parameter*/
   /* is a pointer to a variable that will receive status information   */
   /* about the result of the write operation.  This function should    */
   /* return a non-negative return value if successful (and return      */
   /* IACP_STATUS_SUCCESS in the status parameter).  This function      */
   /* should return a negative return error code if there is an error   */
   /* with the parameters and/or module initialization.  Finally, this  */
   /* function can also return success (non-negative return value) as   */
   /* well as specifying a non-successful Status value (to denote that  */
   /* there was an actual error during the write process, but the module*/
   /* is initialized and configured correctly).                         */
   /* * NOTE * If this function returns a non-negative return value then*/
   /*          the caller will ALSO examine the value that was placed in*/
   /*          the Status parameter to determine the actual status of   */
   /*          the operation (success or failure).  This means that a   */
   /*          successful return value will be a non-negative return    */
   /*          value and the status member will contain the value:      */
   /*          IACP_STATUS_SUCCESS                                      */
int BTPSAPI IACPTR_Write(unsigned char Register, unsigned char BytesToWrite, unsigned char *WriteBuffer, unsigned char *Status)
{
   int ret_val;

   /* Verify that the parameters passed in appear valid.                */
   if((BytesToWrite) && (WriteBuffer) && (Status))
   {
//xxx
   }
   else
      ret_val = IACP_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for resetting the ACP       */
   /* hardware.  The implementation should not return until the reset is*/
   /* complete.                                                         */
   /* * NOTE * This function only applies to chips version 2.0B and     */
   /*          earlier.  For chip version 2.0C and newer, this function */
   /*          should be implemented as a NO-OP.                        */
void BTPSAPI IACPTR_Reset(void)
{
//xxx
}

