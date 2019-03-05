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
/*   12/01/12  D. Lange       Initial creation.                               */
/*   12/04/12  T. Cook        Finished Implementation.                        */
/******************************************************************************/
#ifndef __PASMGRH__
#define __PASMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPASM.h"           /* PASS Framework Prototypes/Constants.      */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the PAS Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager PAS Manager  */
   /* Implementation.                                                   */
int _PASM_Initialize(PASS_Ringer_Setting_t DefaultRingerSetting, PASS_Alert_Status_t *DefaultAlertStatus);

   /* The following function is responsible for shutting down the PAS   */
   /* Manager Implementation.  After this function is called the PAS    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _PASM_Initialize() function.  */
void _PASM_Cleanup(void);

   /* The following function is responsible for informing the PAS       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the PAS Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _PASM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the PASS Service registered by this   */
   /* module.                                                           */
unsigned int _PASM_Query_Number_Attributes(void);

   /* The following function is responsible for querying the Connection */
   /* ID of a specified connection.  The first parameter is the BD_ADDR */
   /* of the connection to query the Connection ID.  The second         */
   /* parameter is a pointer to return the Connection ID if this        */
   /* function is successful.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID);

   /* The following function is responsible for setting the Alert Status*/
   /* value.  The only parameter is the Alert Status to set as the      */
   /* current Alert Status.  This function returns a zero if successful */
   /* or a negative return error code if an error occurs.               */
int _PASM_Set_Alert_Status(PASS_Alert_Status_t *AlertStatus);

   /* The following function is responsible for setting the Ringer      */
   /* Setting value.  The only parameter is the Ringer Setting to set as*/
   /* the current Ringer Setting.  This function returns a zero if      */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Set_Ringer_Setting(PASS_Ringer_Setting_t RingerSetting);

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request.  The first parameter to this        */
   /* function is the Transaction ID of the request.  The final         */
   /* parameter contains the Client Configuration to send to the remote */
   /* device.  This function returns a zero if successful or a negative */
   /* return error code if an error occurs.                             */
int _PASM_Read_Client_Configuration_Response(unsigned int TransactionID, Boolean_t NotificationsEnabled);

   /* The following function is responsible for sending a notification  */
   /* of a specified characteristic to a specified remote device.  The  */
   /* first parameter to this function is the ConnectionID of the remote*/
   /* device to send the notification to.  The final parameter specifies*/
   /* the characteristic to notify.  This function returns a zero if    */
   /* successful or a negative return error code if an error occurs.    */
int _PASM_Send_Notification(unsigned int ConnectionID, PASS_Characteristic_Type_t CharacteristicType);

#endif
