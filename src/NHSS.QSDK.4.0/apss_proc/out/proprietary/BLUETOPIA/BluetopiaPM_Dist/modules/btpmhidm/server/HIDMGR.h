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

   /* The following function is responsible for informing the HID       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HID Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HIDM_SetBluetoothStackID(unsigned int BluetoothStackID);

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
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHIDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HIDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
int _HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect);

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
   /* function accepts as input the remote device address of the remote */
   /* HID device to send the report data to, followed by the report data*/
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _HIDM_Send_Report_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as       */
   /* input the remote device address of the remote HID device to       */
   /* send the report data to, the type of report requested, and        */
   /* the Report ID determined by the Device's SDP record.  Passing     */
   /* HIDM_INVALID_REPORT_ID as the value for this parameter will       */
   /* indicate that this parameter is not used and will exclude the     */
   /* appropriate byte from the transaction payload.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HIDM_Send_Get_Report_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID);

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input the */
   /* remote device address of the remote HID device to send the report */
   /* data to, the type of report being sent, the Length of the Report  */
   /* Data to send, and a pointer to the Report Data that will be sent. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _HIDM_Send_Set_Report_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData);

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to.  This function returns a zero if successful,  */
   /* or a negative return error code if there was an error.            */
int _HIDM_Send_Get_Protocol_Request(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to and the protocol to be set.  This function     */
   /* returns a zero if successful, or a negative return error code if  */
   /* there was an error.                                               */
int _HIDM_Send_Set_Protocol_Request(BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol);

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the the remote device address of the remote HID device      */
   /* to send the report data to.  This function returns a zero if      */
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Send_Get_Idle_Request(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the remote device address of the remote HID device to send  */
   /* the report data to and the Idle Rate to be set.  The Idle Rate    */
   /* LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms with */
   /* a range from 4ms to 1.020s).  This function returns a zero if     */
   /* successful, or a negative return error code if there was an error.*/
int _HIDM_Send_Set_Idle_Request(BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate);

#endif
