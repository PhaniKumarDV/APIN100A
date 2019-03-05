/*****< anpmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGR - ANP Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __TIPMGRH__
#define __TIPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTTIPM.h"           /* ANP Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the ANP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager ANP Manager  */
   /* Implementation.                                                   */
int _TIPM_Initialize(void);

   /* The following function is responsible for shutting down the ANP   */
   /* Manager Implementation.  After this function is called the ANP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _TIPM_Initialize() function.  */
void _TIPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Server Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Server Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _TIPM_Un_Register_Server_Events() function to un-register*/
   /*          the callback from this module.                           */
int _TIPM_Register_Server_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* _TIPM_Register_Server_Events() function).  This function accepts  */
   /* the Server Event Handler ID that was returned via a successful    */
   /* call to _TIPM_Register_Server_Events().  This function returns a  */
   /* zero on success or a negative return error code if there was an   */
   /* error.                                                            */
int _TIPM_Un_Register_Server_Events(unsigned int ServerEventHandlerID);

   /* The following function is a utility function that is used to set  */
   /* the current Local Time Information.  This function accepts the    */
   /* Server Event Handler ID (return value from                        */
   /* _TIPM_Register_Server_Events() function) and a pointer to the     */
   /* Local Time Information to set.  This function returns ZERO if     */
   /* successful, or a negative return error code if there was an error.*/
int _TIPM_Set_Local_Time_Information(unsigned int ServerEventHandlerID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation);

   /* The following function is a utility function that is used to force*/
   /* an update of the Current Time.  This function accepts the Server  */
   /* Event Handler ID (return value from _TIPM_Register_Server_Events()*/
   /* function) and a bit mask that contains the reason for the Current */
   /* Time Update.  This function returns ZERO if successful, or a      */
   /* negative return error code if there was an error.                 */
int _TIPM_Update_Current_Time(unsigned int ServerEventHandlerID, unsigned long AdjustReasonMask);

   /* The following function is a utility function that is used to      */
   /* respond to a request for the Reference Time Information.  This    */
   /* function accepts the Server Event Handler ID (return value from   */
   /* _TIPM_Register_Server_Events() function) and a pointer to the     */
   /* Reference Time Information to respond to the request with.  This  */
   /* function returns ZERO if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _TIPM_Reference_Time_Response(unsigned int ServerEventHandlerID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Client callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Client Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Client Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
int _TIPM_Register_Client_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the Client Event Callback ID (return value from  */
   /* TIPM_Register_Client_Event_Callback() function).                  */
int _TIPM_Un_Register_Client_Events(unsigned int ClientCallbackID);

int _TIPM_Get_Current_Time(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

int _TIPM_Enable_Time_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable);

int _TIPM_Get_Local_Time_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

int _TIPM_Get_Time_Accuracy(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

int _TIPM_Get_Next_DST_Change_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

int _TIPM_Get_Reference_Time_Update_State(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

int _TIPM_Request_Reference_Time_Update(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated        */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns    */
   /* a non-negative value if successful which represents the number    */
   /* of connected devices that were copied into the specified input    */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _TIPM_Query_Connected_Devices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices);

#endif
