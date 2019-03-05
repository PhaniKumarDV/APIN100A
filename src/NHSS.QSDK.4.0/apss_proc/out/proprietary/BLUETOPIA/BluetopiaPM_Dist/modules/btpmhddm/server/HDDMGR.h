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

#include "SS1BTHID.h"            /* HID Framework Prototypes/Constants.       */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the HID Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager HID Manager  */
   /* Implementation.                                                   */
int _HDDM_Initialize(HDDM_Initialization_Data_t *InitializationData);

   /* The following function is responsible for shutting down the HID   */
   /* Manager Implementation.  After this function is called the HID    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _HDDM_Initialize() function.  */
void _HDDM_Cleanup(void);

   /* The following function is responsible for informing the HID       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the HID Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDDM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local HID Server.  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Stack associated with the HID Server that is responding */
   /* to the request.  The second parameter to this function is the HID */
   /* ID of the HID Server for which an open request was received.  The */
   /* final parameter to this function specifies whether to accept the  */
   /* pending connection request (or to reject the request).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
int _HDDM_Open_Request_Response(unsigned int HIDID, Boolean_t AcceptConnection);

   /* The following function is responsible for opening a connection    */
   /* to a Remote HID Host on the Specified Bluetooth Device.  This     */
   /* function accepts as its first parameter the Bluetooth Stack ID    */
   /* of the Bluetooth Stack which is to open the HID Connection.  The  */
   /* second parameter specifies the Board Address (NON NULL) of the    */
   /* Remote Bluetooth Device to connect with.  The third parameter to  */
   /* this function is the HID Configuration Specification to be used in*/
   /* the negotiation of the L2CAP Channels associated with this Host   */
   /* Client.  The final two parameters specify the HID Event Callback  */
   /* function and Callback Parameter, respectively, of the HID Event   */
   /* Callback that is to process any further events associated with    */
   /* this Host Client.  This function returns a non-zero, positive,    */
   /* value if successful, or a negative return error code if this      */
   /* function is unsuccessful.  If this function is successful,        */
   /* the return value will represent the HID ID that can be passed     */
   /* to all other functions that require it.  Once a Connection is     */
   /* opened to a Remote Host it can only be closed via a call to the   */
   /* _HDDM_Close_Connection() function (passing in the return value    */
   /* from a successful call to this function as the HID ID input       */
   /* parameter).                                                       */
int _HDDM_Connect_Remote_Host(BD_ADDR_t BD_ADDR);

   /* The following function is responsible for closing a HID           */
   /* connection established through a connection made to a Registered  */
   /* Server or a connection that was made by calling either the        */
   /* _HDDM_Open_Remote_Device() or HID_Open_Remote_Host() functions.   */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the HID ID specified by the Second  */
   /* Parameter is valid for.  This function returns zero if successful,*/
   /* or a negative return error code if an error occurred.  Note that  */
   /* if this function is called with the HID ID of a Local Server, the */
   /* Server will remain registered but the connection associated with  */
   /* the specified HID ID will be closed. SendVirtualCableDisconnect   */
   /* specifies whether to send the virtual cable disconnect control    */
   /* command before disconnecting.                                     */
int _HDDM_Close_Connection(unsigned int HIDID, Boolean_t SendVirtualCableDisconnect);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_REPORT transaction.  This function */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Statuses are used to  */
   /* respond, a HANDSHAKE response that has a Result Code parameter of */
   /* the specified Error Condition is sent.  If the ResultType         */
   /* specified is rtData, the GET_REPORT transaction is responded to   */
   /* with a DATA Response that has the Report (specified by the final  */
   /* parameter) as its Payload.  The fourth parameter is the type of   */
   /* report being sent.  Note that rtOther is an Invalid Report Type   */
   /* for use with this function.  The final two parameters are the     */
   /* Length of the Report Payload to send and a pointer to the Report  */
   /* Payload that will be sent.  This function returns a zero if       */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Get_Report_Response(unsigned int HIDID, HID_Result_Type_t ResultType, HID_Report_Type_Type_t ReportType, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_REPORT transaction.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified, this function responds to the SET_REPORT request with a*/
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Set_Report_Response(unsigned int HIDID, HID_Result_Type_t ResultType);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtSuccessful Result Type is Invalid for use with   */
   /* this function.  If the rtNotReady through rtErrFatal Result Types */
   /* are specified, this function will respond to the GET_PROTOCOL     */
   /* request with a HANDSHAKE response that has a Result Code parameter*/
   /* of the specified Error Condition.  If the ResultType specified is */
   /* rtData, the GET_PROTOCOL transaction is responded to with a DATA  */
   /* Response that has the Protocol type specified as the final        */
   /* parameter as its Payload.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Get_Protocol_Response(unsigned int HIDID, HID_Result_Type_t ResultType, HID_Protocol_Type_t Protocol);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_PROTOCOL transaction.  This        */
   /* function accepts the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which is to send the response and the HID ID for which the        */
   /* Connection has been established.  The third parameter to this     */
   /* function is the Result Type that is to be associated with this    */
   /* response.  The rtData Result Type is Invalid for use with this    */
   /* function.  If the rtSuccessful through rtErrFatal Result Types are*/
   /* specified then this function will respond to the SET_PROTOCOL     */
   /* Transaction with a HANDSHAKE response that has a Result Code      */
   /* parameter that matches the specified Result Type.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _HDDM_Set_Protocol_Response(unsigned int HIDID, HID_Result_Type_t ResultType);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding GET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The            */
   /* rtSuccessful Result Type is Invalid for use with this function.   */
   /* If the rtNotReady through rtErrFatal Result Types are specified,  */
   /* then this function will respond to the GET_IDLE Transaction with a*/
   /* HANDSHAKE response that has a Result Code parameter of the        */
   /* specified Error Condition.  If the ResultType specified is rtData */
   /* the GET_IDLE transaction is responded to with a DATA Response that*/
   /* has the Idle Rate specified as the final parameter as its Payload.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _HDDM_Get_Idle_Response(unsigned int HIDID, HID_Result_Type_t ResultType, Byte_t IdleRate);

   /* The following function is responsible for Sending the appropriate */
   /* Response to an Outstanding SET_IDLE transaction.  This function   */
   /* accepts the Bluetooth Stack ID of the Bluetooth Stack which is to */
   /* send the response and the HID ID for which the Connection has been*/
   /* established.  The third parameter to this function is the Result  */
   /* Type that is to be associated with this response.  The rtData     */
   /* Result Type is Invalid for use with this function.  If the        */
   /* rtSuccessful through rtErrFatal Result Types are specified, then  */
   /* this function will respond to the SET_IDLE Transaction with a     */
   /* HANDSHAKE response that has a Result Code parameter that matches  */
   /* the specified Result Type.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDDM_Set_Idle_Response(unsigned int HIDID, HID_Result_Type_t ResultType);

   /* The following function is responsible for sending Reports over    */
   /* the Interrupt Channel.  This function accepts the Bluetooth Stack */
   /* ID of the Bluetooth Stack which is to send the Report Data and    */
   /* the HID ID for which the Connection has been established.  The    */
   /* third parameter is the type of report being sent.  The final two  */
   /* parameters are the Length of the Report Payload to send and a     */
   /* pointer to the Report Payload that will be sent.  This function   */
   /* returns a zero if successful, or a negative return error code if  */
   /* there was an error.                                               */
int _HDDM_Data_Write(unsigned int HIDID, Word_t ReportPayloadSize, Byte_t *ReportDataPayload);

#endif
