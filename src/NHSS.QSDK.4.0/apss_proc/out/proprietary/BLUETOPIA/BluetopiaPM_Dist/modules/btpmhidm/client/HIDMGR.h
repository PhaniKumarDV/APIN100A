/*****< hidmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMGR - HID Manager Implementation for Stonestreet One Bluetooth         */
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
#ifndef __HIDMGRH__
#define __HIDMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHIDH.h"           /* HID Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HIDM_Initialize(void);

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HIDM_Initialize() function.  */
void _HIDM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming HID connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HID Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _HIDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected HID     */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected HID devices.  The first parameter */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated with   */
   /* the total number of connected devices if the function is          */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HIDM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int _HIDM_Change_Incoming_Connection_Flags(unsigned int ConnectionFlags);

   /* The following function is used to set the HID Host Keyboard Key   */
   /* Repeat behavior.  Some Host operating systems nativity support Key*/
   /* Repeat automatically, but for those Host operating systems that do*/
   /* not - this function will instruct the HID Manager to simulate Key */
   /* Repeat behavior (with the specified parameters).  This function   */
   /* accepts the initial amount to delay (in milliseconds) before      */
   /* starting the repeat functionality.  The final parameter specifies */
   /* the rate of repeat (in milliseconds).  This function returns zero */
   /* if successful or a negative value if there was an error.          */
   /* * NOTE * Specifying zero for the Repeat Delay (first parameter)   */
   /*          will disable HID Manager Key Repeat processing.  This    */
   /*          means that only Key Up/Down events will be dispatched    */
   /*          and No Key Repeat events will be dispatched.             */
   /* * NOTE * The Key Repeat parameters can only be changed when there */
   /*          are no actively connected HID devices.                   */
int _HIDM_Set_Keyboard_Repeat_Rate(unsigned int RepeatDelay, unsigned int RepeatRate);

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the HID Manager Data Handler ID         */
   /* (registered via call to the HIDM_Register_Data_Event_Callback()   */
   /* function), followed by the remote device address of the remote HID*/
   /* device to send the report data to, followed by the report data    */
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _HIDM_Send_Report_Data(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report requested.  The    */
   /* fourth parameter is the Report ID determined by the Device's SDP  */
   /* record.  Passing HIDM_INVALID_REPORT_ID as the value for this     */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID);

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input     */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report being sent.  The   */
   /* final two parameters to this function are the Length of the Report*/
   /* Data to send and a pointer to the Report Data that will be sent.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData);

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the protocol to be set.  This     */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol);

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Get_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);


   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the Idle Rate to be set.  The Idle*/
   /* Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms */
   /* with a range from 4ms to 1.020s).  This function returns a zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
int _HIDM_Send_Set_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HID Manager      */
   /* Service.  This Callback will be dispatched by the HID Manager when*/
   /* various HID Manager Events occur.  This function returns a        */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the HID    */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HIDM_Un_Register_HID_Events() function to Un-Register   */
   /*          the Event Handler.                                       */
int _HIDM_Register_HID_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Handler     */
   /* (registered via a successful call to the                          */
   /* _HIDM_Register_HID_Events() function).  This function accepts     */
   /* input the HID Event Handler ID (return value from                 */
   /* _HIDM_Register_HID_Events() function).                            */
int _HIDM_Un_Register_HID_Events(unsigned int HIDEventsHandlerID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the HID       */
   /* Manager Service to explicitly process HID Data.  This Callback    */
   /* will be dispatched by the HID Manager when various HID Manager    */
   /* Events occur.  This function returns a positive (non-zero) value  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HIDM_Send_Report_Data() function to send data).         */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          in the system.                                           */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HIDM_Un_Register_HID_Data_Events() function to          */
   /*          un-register the callback from this module.               */
int _HIDM_Register_HID_Data_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Data Event        */
   /* Callback (registered via a successful call to the                 */
   /* _HIDM_Register_HID_Data_Events() function).  This function accepts*/
   /* as input the HID Manager Event Callback ID (return value from     */
   /* _HIDM_Register_HID_Data_Events() function).                       */
int _HIDM_Un_Register_HID_Data_Events(unsigned int HIDManagerDataCallbackID);

#endif
