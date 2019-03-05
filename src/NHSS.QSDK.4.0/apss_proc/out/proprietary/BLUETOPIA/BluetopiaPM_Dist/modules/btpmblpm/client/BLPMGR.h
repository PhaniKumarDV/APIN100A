/*****< blpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
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

#include "SS1BTBLPM.h"           /* BLP Framework Prototypes/Constants.       */

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

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Blood Pressure   */
   /* (BLP) Manager Service.  This Callback will be dispatched by the   */
   /* BLP Manager when various BLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
int _BLPM_Register_Collector_Event_Callback(BLPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the BLP Manager Event Callback ID (return value  */
   /* from BLPM_Register_Collector_Event_Callback() function).          */
int _BLPM_Un_Register_Collector_Event_Callback(unsigned int CallbackID);

   /* The following function is provided to allow a mechanism to enable */
   /* indications for Blood Pressure Measurements on a specified Blood  */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Enable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to disable*/
   /* indications for Blood Pressure Measurements on a specified Blood  */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Disable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to enable */
   /* notifications for Intermediate Cuff Pressure on a specified Blood */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Enable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications for Intermediate Cuff Pressure on a specified Blood */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
int _BLPM_Disable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Blood Pressure Feature Request to a remote sensor.  This    */
   /* function accepts as input the Bluetooth Address of the remote     */
   /* device to request the Blood Pressure Feature from.  This function */
   /* returns a positive Transaction ID on success; otherwise, a        */
   /* negative error value is returned.                                 */
int _BLPM_Get_Blood_Pressure_Feature(unsigned int CallbackID, BD_ADDR_t *RemoteSensor);

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _BLPM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID);

#endif
