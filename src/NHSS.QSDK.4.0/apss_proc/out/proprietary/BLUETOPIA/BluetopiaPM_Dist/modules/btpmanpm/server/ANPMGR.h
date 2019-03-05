/*****< anpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMGR - ANP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANPMGRH__
#define __ANPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANP Manager  */
   /* Implementation.                                                   */
int _ANPM_Initialize(Word_t SupportedNewAlertCategories, Word_t SupportedUnReadAlertCategories);

   /* The following function is responsible for shutting down the ANP   */
   /* Manager Implementation.  After this function is called the ANP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANPM_Initialize() function.  */
void _ANPM_Cleanup(void);

   /* The following function is responsible for informing the ANP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the ANP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _ANPM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the ANS Service registered by this    */
   /* module.                                                           */
unsigned int _ANPM_Query_Number_Attributes(void);

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request that was received earlier.  This     */
   /* function accepts as input the Transaction ID of the request and a */
   /* Boolean that indicates if notifications are enabled for this if   */
   /* TRUE.  This function returns zero if successful, or a negative    */
   /* return error code if there was an error.                          */
int _ANS_Read_Client_Configuration_Response(unsigned int TransactionID, Boolean_t NotificationsEnabled);

   /* The following function is responsible for sending a New Alert     */
   /* Notification to the specified connection.  This function accepts  */
   /* as input the ConnectionID of the remote client to send the        */
   /* notification to and the new alert data to notify.  This functions */
   /* returns ZERO if success or a negative error code.                 */
int _ANS_New_Alert_Notification(unsigned int ConnectionID, ANS_New_Alert_Data_t *NewAlertData);

   /* The following function is responsible for sending a Un-Read Alert */
   /* Notification to the specified connection.  This function accepts  */
   /* as input the ConnectionID of the remote client to send the        */
   /* notification to and the un-read alert data to notify.  This       */
   /* functions returns ZERO if success or a negative error code.       */
int _ANS_UnRead_Alert_Notification(unsigned int ConnectionID, ANS_Un_Read_Alert_Data_t *UnReadAlert);

#endif
