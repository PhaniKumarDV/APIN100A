/*****< anpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMGR - ANP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/18/11  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __ANPMGRH__
#define __ANPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTANPM.h"           /* ANP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANP Manager  */
   /* Implementation.                                                   */
int _ANPM_Initialize(void);

   /* The following function is responsible for shutting down the ANP   */
   /* Manager Implementation.  After this function is called the ANP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _ANPM_Initialize() function.  */
void _ANPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of New Alerts for a specific category and*/
   /* the text of the last alert for the specified category.  This      */
   /* function accepts as the Category ID of the specific category, the */
   /* number of new alerts for the specified category and a text string */
   /* that describes the last alert for the specified category (if any).*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
   /* * NOTE * If there is not a last alert text available, then specify*/
   /*          NULL.                                                    */
int _ANPM_Set_New_Alert(ANPM_Category_Identification_t CategoryID, unsigned int NewAlertCount, char *LastAlertText);

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of Un-Read Alerts for a specific         */
   /* category.  This function accepts as the Category ID of the        */
   /* specific category, and the number of un-read alerts for the       */
   /* specified category.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
int _ANPM_Set_Un_Read_Alert(ANPM_Category_Identification_t CategoryID, unsigned int UnReadAlertCount);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Alert            */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a ANP Manager Event needs to*/
   /* be dispatched.  This function returns a zero on success or a      */
   /* negative return error code if there was an error.                 */
int _ANPM_Register_ANP_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* ANPM_Register_Event_Callback() function).                         */
int _ANPM_Un_Register_ANP_Events(void);

   /* ANPM Client functions.                                            */

   /* This functions submits a request to a remote ANP Server to get    */
   /* the supported categories.  The ClientCallbackID parameter should  */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * An aetANPSupportedNewAlertCategoriesResult event will be */
   /*          dispatched when this request completes.                  */
int _ANPM_Get_Supported_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type);

   /* This functions will enable or disable notifications from a        */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
int _ANPM_Enable_Disable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Notification_Type_t Type, Boolean_t Enable);

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
int _ANPM_Enable_Disable_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type, Boolean_t Enable);

   /* This functions submits a request to a remote ANP Server to request*/
   /* an immediate Notification.  The ClientCallbackID parameter should */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns a positive value representing the  */
   /* Transaction ID if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
int _ANPM_Request_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID, ANPM_Notification_Type_t Type);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Client     */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int _ANPM_Register_Client_Event_Callback(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Client_Event_Callback() function).             */
int _ANPM_Un_Register_Client_Event_Callback(unsigned int ANPManagerCallbackID);

#endif
