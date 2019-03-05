/*****< htpmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMGR - HTP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
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

#include "SS1BTHTPM.h"           /* HTP Framework Prototypes/Constants.       */

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

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HTP) */
   /* Manager Service.  This Callback will be dispatched by the HTP     */
   /* Manager when various HTP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HTP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HTPM_Un_Register_Collector_Events() function to          */
   /*          un-register the callback from this module.               */
int _HTPM_Register_Collector_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HTP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HTPM_Register_Collector_Events() function).  This function accepts*/
   /* as input the HTP Manager Event Callback ID (return value from     */
   /* HTPM_Register_Collector_Events() function).                       */
int _HTPM_Un_Register_Collector_Events(unsigned int HTPCollectorEventHandlerID);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Temperature Type procedure to a remote HTP   */
   /* Sensor.  This function accepts as input the HTP Collector Event   */
   /* Handler ID (return value from _HTPM_Register_Collector_Events()   */
   /* function), and the BD_ADDR of the remote HTP Sensor.  This        */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
int _HTPM_Get_Temperature_Type_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Event Handler ID (return value from                               */
   /* _HTPM_Register_Collector_Events() function), and the BD_ADDR of   */
   /* the remote HTP Sensor.  This function returns the positive,       */
   /* non-zero, Transaction ID of the request on success or a negative  */
   /* error code.                                                       */
int _HTPM_Get_Measurement_Interval_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Set Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Event Handler ID (return value from                               */
   /* _HTPM_Register_Collector_Events() function), the BD_ADDR of the   */
   /* remote HTP Sensor, and the Measurement Interval to attempt to set.*/
   /* This function returns the positive, non-zero, Transaction ID of   */
   /* the request on success or a negative error code.                  */
int _HTPM_Set_Measurement_Interval_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval);

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval Valid Range procedure to*/
   /* a remote HTP Sensor.  This function accepts as input the HTP      */
   /* Collector Event Handler ID (return value from                     */
   /* _HTPM_Register_Collector_Events() function), and the BD_ADDR of   */
   /* the remote HTP Sensor.  This function returns the positive,       */
   /* non-zero, Transaction ID of the request on success or a negative  */
   /* error code.                                                       */
int _HTPM_Get_Measurement_Interval_Valid_Range_Request(unsigned int HTPCollectorEventHandlerID, BD_ADDR_t RemoteDeviceAddress);

#endif
