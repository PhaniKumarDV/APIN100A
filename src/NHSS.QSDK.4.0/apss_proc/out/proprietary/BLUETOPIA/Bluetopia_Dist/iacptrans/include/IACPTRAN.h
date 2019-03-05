/*****< iacptran.h >***********************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  IACPTRAN - Apple Authentication Coprocessor Transport Type Definitions,   */
/*              Prototypes, and Constants.                                    */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/08/11  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __IACPTRANH__
#define __IACPTRANH__

#include "BTPSKRNL.h"      /* Bluetooth Kernel Prototypes/Constants.          */
#include "IACPType.h"      /* Types for the Authentication Coprocessor.       */

#define IACP_INVALID_PARAMETER               (-1)  /* Denotes an invalid      */
                                                   /* parameter was passed to */
                                                   /* a function.             */
#define IACP_MODULE_NOT_INITIALIZED          (-2)  /* Denotes that a function */
                                                   /* has been called before  */
                                                   /* the initialize function.*/
#define IACP_RESPONSE_TIMEOUT                (-3)  /* Denotes that a response */
                                                   /* was not received from   */
                                                   /* the chip in the allotted*/
                                                   /* amount of time.         */

   /* The following values are used with the Status parameter (return   */
   /* value) of the IACPTR_Read() and IACPTR_Write() functions.         */
   /* * NOTE * These are the ONLY values that can be specified for this */
   /*          return parameter.                                        */
#define IACP_STATUS_SUCCESS                  (0)   /* Denotes a function      */
                                                   /* completed without error.*/
#define IACP_STATUS_READ_FAILURE             (1)   /* Denotes a Register Read */
                                                   /* failure.                */
#define IACP_STATUS_WRITE_FAILURE            (2)   /* Denotes a Register      */
                                                   /* Write failure.          */

   /* The following function is responsible for Initializing the ACP    */
   /* hardware.  The function is passed a pointer to an opaque object.  */
   /* Since the hardware platform is not know, it is intended that the  */
   /* parameter be used to pass hardware specific information to the    */
   /* module that would be needed for proper operation.  Each           */
   /* implementation will define what gets passed to this function.     */
   /* This function returns zero if successful or a negative value if   */
   /* the initialization fails.                                         */
BTPSAPI_DECLARATION int BTPSAPI IACPTR_Initialize(void *Parameters);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IACPTR_Initialize_t)(void *Parameters);
#endif

   /* The following function is responsible for performing any cleanup  */
   /* that may be required by the hardware or module.  This function    */
   /* returns no status.                                                */
BTPSAPI_DECLARATION void BTPSAPI IACPTR_Cleanup(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_IACPTR_Cleanup_t)(void);
#endif

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
BTPSAPI_DECLARATION int BTPSAPI IACPTR_Read(unsigned char Register, unsigned char BytesToRead, unsigned char *ReadBuffer, unsigned char *Status);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IACPTR_Read_t)(unsigned char Register, unsigned char BytesToRead, unsigned char *ReadBuffer, unsigned char *Status);
#endif

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
BTPSAPI_DECLARATION int BTPSAPI IACPTR_Write(unsigned char Register, unsigned char BytesToWrite, unsigned char *WriteBuffer, unsigned char *Status);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_IACPTR_Write_t)(unsigned char Register, unsigned char BytesToWrite, unsigned char *WriteBuffer, unsigned char *Status);
#endif

   /* The following function is responsible for resetting the ACP       */
   /* hardware.  The implementation should not return until the reset is*/
   /* complete.                                                         */
   /* * NOTE * This function only applies to chips version 2.0B and     */
   /*          earlier.  For chip version 2.0C and newer, this function */
   /*          should be implemented as a NO-OP.                        */
BTPSAPI_DECLARATION void BTPSAPI IACPTR_Reset(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_IACPTR_Reset_t)(void);
#endif

#endif
