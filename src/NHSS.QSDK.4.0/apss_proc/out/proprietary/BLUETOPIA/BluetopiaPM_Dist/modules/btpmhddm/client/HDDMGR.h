/*****< hddmgr.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDDMGR - HID Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/28/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __HDDMGRH__
#define __HDDMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HDD Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HDD Manager  */
   /* Implementation.                                                   */
int _HDDM_Initialize(void);

   /* The following function is responsible for shutting down the HDD   */
   /* Manager Implementation.  After this function is called the HDD    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HDDM_Initialize() function.  */
void _HDDM_Cleanup(void);

   /* from a remote HID Host.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.            */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HDD Connected   */
   /*          event will be dispatched to signify the actual result.   */
int _HDDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);

   /* Connect to a remote HID Host device.  The RemoteDeviceAddress is  */
   /* the Bluetooth Address of the remote HID Host.  The ConnectionFlags*/
   /* specifiy whay security, if any, is required for the connection.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HDDM_Connect_Remote_Host(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags);

   /* Disconnect from a remote HID Host.  The RemoteDeviceAddress       */
   /* is the Bluetooth Address of the remote HID Host.  The             */
   /* SendVirtualCableUnplug parameter indicates whether the device     */
   /* should be disconnected with a Virtual Cable Unplug (TRUE) or      */
   /* simply at the Bluetooth Link (FALSE).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HDDM_Disconnect(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug);

   /* Determine if there are currently any connected HID Hosts.  This   */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently connected HID Hosts.  The first parameter specifies the */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer).  The next parameter is      */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HDDM_Query_Connected_Hosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
int _HDDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

   /* Send the specified HID Report Data to a currently connected       */
   /* remote device.  This function accepts as input the HDD            */
   /* Manager Report Data Handler ID (registered via call to the        */
   /* _HDDM_Register_Data_Event_Callback() function), followed by the   */
   /* remote device address of the remote HID Host to send the report   */
   /* data to, followed by the report data itself.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HDDM_Send_Report_Data(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);

   /* Respond to a GetReportRequest.  The _HDDManagerDataCallback       */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* ReportType indicates the type of report being sent as the         */
   /* response.  The ReportDataLength indicates the size of the report  */
   /* data.  ReportData is a pointer to the report data buffer.  This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Get_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData);

   /* Responsd to a SetReportRequest. The _HDDManagerDataCallback       */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

   /* Respond to a GetProtocolRequest.  The _HDDManagerDataCallback     */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* Protocol indicates the current HID Protocol.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HDDM_Get_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol);

   /* Respond to a SetProtocolResponse.  The _HDDManagerDataCallback    */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

   /* Respond to a GetIdleResponse.  The _HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* IdleRate is the current Idle Rate.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int _HDDM_Get_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate);

   /* Respond to a SetIdleRequest.  The _HDDManagerDataCallback         */
   /* ID is the identifier returned via a successful call to            */
   /* _HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress    */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDDM_Set_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service.  This Callback will be dispatched by*/
   /* the HID Manager when various HID Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HID Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          _HDDM_UnRegisterEventCallback() function to un-register  */
   /*          the callback from this module.                           */
int _HDDM_Register_Event_Callback();

   /* The following function is provided to allow a mechanism           */
   /* to un-register a previously registered HID Manager                */
   /* Event Callback (registered via a successful call to the           */
   /* _HDDM_RegisterEventCallback() function).  This function accepts   */
   /* as input the HID Manager Event Callback ID (return value from     */
   /* _HDDM_RegisterEventCallback() function).                          */
int _HDDM_Un_Register_Event_Callback(unsigned int HDDManagerCallbackID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service to explicitly process HID report     */
   /* data.  This Callback will be dispatched by the HID Manager when   */
   /* various HID Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HID Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HDDM_Send_Report_Data() function to send report data.   */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HDDM_Un_Register_Data_Event_Callback() function to      */
   /*          un-register the callback from this module.               */
int _HDDM_Register_Data_Event_Callback();

   /* The following function is provided to allow a mechanism           */
   /* to un-register a previously registered HID Manager                */
   /* Event Callback (registered via a successful call to the           */
   /* _HDDM_Register_Data_Event_Callback() function).  This function    */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from _HDDM_Register_Data_Event_Callback() function).        */
int _HDDM_Un_Register_Data_Event_Callback(unsigned int HDDManagerDataCallbackID);

#endif
