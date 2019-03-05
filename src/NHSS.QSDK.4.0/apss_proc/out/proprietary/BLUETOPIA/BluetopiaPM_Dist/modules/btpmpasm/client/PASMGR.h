/*****< pasmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMGR - Phone Alert Status (PAS) Manager Implementation for Stonestreet  */
/*           One Bluetooth Protocol Stack Platform Manager.                   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PASMGRH__
#define __PASMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPASM.h"           /* PAS Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAS Manager  */
   /* Implementation.                                                   */
int _PASM_Initialize(void);

   /* The following function is responsible for shutting down the PAS   */
   /* Manager Implementation.  After this function is called the PAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PASM_Initialize() function.  */
void _PASM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Phone Alert Status Server callback function */
   /* with the Phone Alert Status (PAS) Manager Service.  Events will be*/
   /* dispatched by the PAS Manager when various PAS Manager Server     */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _PASM_Un_Register_Server_Events() function to un-register*/
   /*          the event callback from this module.                     */
   /* * NOTE * Only 1 Server Event Callback can be registered in the    */
   /*          system at a time.                                        */
int _PASM_Register_Server_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Phone Alert Status (PAS)      */
   /* Manager Server Event Callback (registered via a successful call to*/
   /* the PASM_Register_Server_Events() function).  This function       */
   /* accepts as input the PAS Manager Event Callback ID (return value  */
   /* from PASM_Register_Server_Events() function).  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _PASM_Un_Register_Server_Events(unsigned int ServerCallbackID);

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS).  This function is  */
   /* responsible for updating the Alert Status internally, as well as  */
   /* dispatching any Alert Notifications that have been registered by  */
   /* Phone Alert Status (PAS) clients.  This function accepts as it's  */
   /* parameter the Server callback ID that was returned from a         */
   /* successful call to PASM_Register_Server_Event_Callback() followed */
   /* by the Alert Status value to set.  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
int _PASM_Set_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) alert       */
   /* status.  This function accepts as it's parameter the Server       */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured alert status upon successful   */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the AlertStatus    */
   /*          buffer will contain the currently configured alert       */
   /*          status.  If this function returns an error then the      */
   /*          contents of the AlertStatus buffer will be undefined.    */
int _PASM_Query_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS) ringer setting as   */
   /* well as dispatching any Alert Notifications that have been        */
   /* registered by PAS clients.  This function accepts as it's         */
   /* parameter the PAS Server callback ID that was returned from a     */
   /* successful call to PASM_Register_Server_Event_Callback() and the  */
   /* ringer value to configure.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _PASM_Set_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting);

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) ringer      */
   /* setting.  This function accepts as it's parameter the Server      */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured ringer setting upon successful */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the RingerSetting  */
   /*          buffer will contain the currently configured ringer      */
   /*          setting.  If this function returns an error then the     */
   /*          contents of the RingerSetting buffer will be undefined.  */
int _PASM_Query_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting);

#endif
