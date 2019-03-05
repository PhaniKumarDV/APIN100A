/*****< hrpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMGR - HRP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __HRPMGRH__
#define __HRPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHRPM.h"           /* HRP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HRP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HRP Manager  */
   /* Implementation.                                                   */
int _HRPM_Initialize(void);

   /* The following function is responsible for shutting down the HRP   */
   /* Manager Implementation.  After this function is called the HRP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HRPM_Initialize() function.  */
void _HRPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HRP) */
   /* Manager Service.  This Callback will be dispatched by the HRP     */
   /* Manager when various HRP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HRP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HRPM_Un_Register_Collector_Events() function to          */
   /*          un-register the callback from this module.               */
int _HRPM_Register_Collector_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HRP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HRPM_Register_Collector_Events() function).  This function accepts*/
   /* as input the HRP Manager Event Callback ID (return value from     */
   /* HRPM_Register_Collector_Events() function).                       */
int _HRPM_Un_Register_Collector_Events(void);

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Body Sensor Location Request to a remote sensor.  This      */
   /* function accepts as input the Bluetooth Address of the remote     */
   /* device to request the Body Sensor Location from.  This function   */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _HRPM_Get_Body_Sensor_Location(BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to submit */
   /* a Reset Energy Expended Request to a remote sensor.  This function*/
   /* accepts as input the Callback ID (return value from               */
   /* HRPM_Register_Collector_Events() function) as the first parameter.*/
   /* The second parameter is the Bluetooth Address of the remote device*/
   /* to request the execution of the Reset Energy Expended command.    */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _HRPM_Reset_Energy_Expended(BD_ADDR_t *RemoteSensor);

#endif
