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

#include "SS1BTBASM.h"           /* BAS Framework Prototypes/Constants.       */

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

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Battery Service  */
   /* (BAS) Manager.  This Callback will be dispatched by the BAS       */
   /* Manager when various BAS Manager Events occur.  This function     */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BASM_Un_Register_Client_Events() function to un-register */
   /*          the callback from this module.                           */
int _BASM_Register_Client_Event_Callback(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BAS Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BASM_Register_Client_Events() function).  This function accepts as*/
   /* input the BAS Manager Event Callback ID (return value from        */
   /* BASM_Register_Client_Events() function).                          */
int _BASM_Un_Register_Client_Event_Callback(unsigned int CallbackID);

   /* The following function is provided to allow a mechanism to Enable */
   /* Notifications for a specific Battery Service instance on a remote */
   /* device.  This function accepts as input the Callback ID (obtained */
   /* from _BASM_Register_Client_Event_Callback() function) as the first*/
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device.  The third parameter is the Instance*/
   /* ID of the Battery server on the remote device.  This function     */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _BASM_Enable_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);

   /* The following function is provided to allow a mechanism to Disable*/
   /* Notifications for a specific Battery Service instance on a remote */
   /* device.  This function accepts as input the Callback ID (obtained */
   /* from _BASM_Register_Client_Event_Callback() function) as the first*/
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device.  The third parameter is the Instance*/
   /* ID of the Battery server on the remote device.  This function     */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
int _BASM_Disable_Notifications(unsigned int CallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Level Request to a remote server.  This function    */
   /* accepts as input the Callback ID (obtained from                   */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device to request the Battery Level from.   */
   /* The third parameter is the Instance ID of the Battery server on   */
   /* the remote device.  This function returns a positive Transaction  */
   /* ID on success; otherwise, a negative error value is returned.     */
int _BASM_Get_Battery_Level(unsigned int ClientCallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Identification Request to a remote server.  This    */
   /* function accepts as input the Callback ID (obtained from          */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is a pointer to the Bluetooth    */
   /* Address of the remote device to request the Battery Identification*/
   /* from.  The third parameter is the Instance ID of the Battery      */
   /* server on the remote device.  This function returns a positive    */
   /* Transaction ID on success; otherwise, a negative error value is   */
   /* returned.                                                         */
int _BASM_Get_Battery_Identification(unsigned int ClientCallbackID, BD_ADDR_t *RemoteServer, unsigned int InstanceID);

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (obtained from                   */
   /* _BASM_Register_Client_Event_Callback() function) as the first     */
   /* parameter.  The second parameter is the Transaction ID of the     */
   /* outstanding transaction.  This function returns zero on success;  */
   /* otherwise, a negative error value is returned.                    */
int _BASM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID);

#endif
