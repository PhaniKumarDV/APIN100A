/*****< tipmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMGR - TIP Manager Implementation for Stonestreet One Bluetooth         */
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

#include "SS1BTCTS.h"            /* Bluetopia CTS Prototypes/Constants.       */
#include "SS1BTRTU.h"            /* Bluetopia RTUS Prototypes/Constants.      */
#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the TIP Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager TIP Manager  */
   /* Implementation.                                                   */
int _TIPM_Initialize(unsigned int SupportedTIPRoles);

   /* The following function is responsible for shutting down the TIP   */
   /* Manager Implementation.  After this function is called the TIP    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _TIPM_Initialize() function.  */
void _TIPM_Cleanup(void);

   /* The following function is responsible for informing the TIP       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * The last parameter to this function can be used to       */
   /*          specify the region in the GATT database that the ANS     */
   /*          Service will reside.                                     */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the TIP Manager with the        */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _TIPM_SetBluetoothStackID(unsigned int BluetoothStackID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* The following function is responsible for querying the number of  */
   /* attributes that are used by the CTS Service registered by this    */
   /* module.                                                           */
unsigned int _TIPM_Query_Number_Attributes(void);

   /* The following function is provided to allow a mechanism to to     */
   /* fetch the current time of the current platform (and formats it    */
   /* into the format expected by the Current Time Service).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _TIPM_Get_Current_Time(CTS_Current_Time_Data_t *CurrentTime, unsigned long AdjustMask);

   /* The following function is responsible for querying the Connection */
   /* ID of a specified connection.  The first parameter is the BD_ADDR */
   /* of the connection to query the Connection ID.  The second         */
   /* parameter is a pointer to return the Connection ID if this        */
   /* function is successful.  This function returns a zero if          */
   /* successful or a negative return error code if an error occurs.    */
int _TIPM_Query_Connection_ID(BD_ADDR_t BD_ADDR, unsigned int *ConnectionID);

   /* The following function is responsible for responding to a Read    */
   /* Current Time Request that was received earlier.  This function    */
   /* accepts as input the Transaction ID of the request, an optional   */
   /* Error Code to respond with an error, and a optional pointer to a  */
   /* structure containing the current time.  This function returns zero*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If ErrorCode is equal to ZERO then Current_Time MUST     */
   /*          point to a structure containing the current time to      */
   /*          respond to the request.                                  */
   /* * NOTE * If ErrorCode is NON-ZERO then an error response will be  */
   /*          sent to the request.                                     */
int _TIPM_CTS_Current_Time_Read_Request_Response(unsigned int TransactionID, Byte_t ErrorCode, CTS_Current_Time_Data_t *Current_Time);

   /* The following function is responsible for setting the Local Time  */
   /* Information stored in the device.  This function accepts as input */
   /* a pointer to the Local Time Information to set.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_CTS_Set_Local_Time_Information(CTS_Local_Time_Information_Data_t *Local_Time);

   /* The following function is responsible for responding to a Read    */
   /* Reference Time Information Request that was received earlier.     */
   /* This function accepts as input the Transaction ID of the request, */
   /* an optional Error Code to respond with an error, and a optional   */
   /* pointer to a structure containing the Reference Time Information. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If ErrorCode is equal to ZERO Reference_Time Current_Time*/
   /*          MUST point to a structure containing the Reference Time  */
   /*          Information to respond to the request.                   */
   /* * NOTE * If ErrorCode is NON-ZERO then an error response will be  */
   /*          sent to the request.                                     */
int _TIPM_CTS_Reference_Time_Information_Read_Request_Response(unsigned int TransactionID, Byte_t ErrorCode, CTS_Reference_Time_Information_Data_t *Reference_Time);

   /* The following function is responsible for responding to a Read    */
   /* Client Configuration Request that was received earlier.  This     */
   /* function accepts as input the Transaction ID of the request and   */
   /* the Client Configuration to respond with.  This function returns  */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _TIPM_CTS_Read_Client_Configuration_Response(unsigned int TransactionID, Word_t ClientConfiguration);

   /* The following function is responsible for sending a Current Time  */
   /* Notification to a specified connection.  This function accepts as */
   /* input the Connection ID of the connection to the device to notify */
   /* and the Current Time that is to be notified.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_CTS_Notify_Current_Time(unsigned int ConnectionID, CTS_Current_Time_Data_t *Current_Time);

   /* The following function is responsible for sending a get current   */
   /* time request.  This function accepts as input the Connection ID   */
   /* of the connection to the device and the handle of the current     */
   /* time characteristic.  This function returns a positive value      */
   /* representing the transaction ID of the request, or a negative     */
   /* return error code if there was an error.                          */
int _TIPM_Get_Current_Time_Request(unsigned int ConnectionID, Word_t CurrentTimeHandle);

   /* The following function is responsible for enabling or disabling   */
   /* time notifications.  This function accepts as input the Connection*/
   /* ID of the connection to the device and the handle of the current  */
   /* time client configuration characteristic decriptor.  This function*/
   /* returns a positive value representing the transaction ID of the   */
   /* request, or a negative return error code if there was an error.   */
int _TIPM_Enable_Time_Notifications(unsigned int ConnectionID, Word_t CurrentTimeCCCD, Boolean_t Enable);

   /* The following function is responsible for sending a get local     */
   /* time information request.  This function accepts as input the     */
   /* Connection ID of the connection to the device and the handle of   */
   /* the local time information characteristic.  This function returns */
   /* a positive value representing the transaction ID of the request,  */
   /* or a negative return error code if there was an error.            */
int _TIPM_Get_Local_Time_Information(unsigned int ConnectionID, Word_t LocalTimeInformationHandle);

   /* The following function is responsible for sending a get time      */
   /* accuracy request.  This function accepts as input the Connection  */
   /* ID of the connection to the device and the handle of the reference*/
   /* time information characteristic.  This function returns a positive*/
   /* value representing the transaction ID of the request, or a        */
   /* negative return error code if there was an error.                 */
int _TIPM_Get_Time_Accuracy(unsigned int ConnectionID, Word_t ReferenceTimeInformationHandle);

   /* The following function is responsible for sending a get next DST  */
   /* change request.  This function accepts as input the Connection    */
   /* ID of the connection to the device and the handle of the time     */
   /* with DST characteristic.  This function returns a positive value  */
   /* representing the transaction ID of the request, or a negative     */
   /* return error code if there was an error.                          */
int _TIPM_Get_Next_DST_Change_Information(unsigned int ConnectionID, Word_t TimeWithDSTHandle);

   /* The following function is responsible for getting the reference   */
   /* time update state.  This function accepts as input the Connection */
   /* ID of the connection to the device and the handle of the time     */
   /* update state characteristic.  This function returns a positive    */
   /* value representing the transaction ID of the request, or a        */
   /* negative return error code if there was an error.                 */
int _TIPM_Get_Reference_Time_Update_State(unsigned int ConnectionID, Word_t TimeUpdateStateHandle);

   /* The following function is responsible for requesting a reference  */
   /* time update.  This function accepts as input the Connection ID of */
   /* the connection to the device and the handle of the time update    */
   /* control point characteristic.  This function returns a positive   */
   /* value representing number of bytes written, or a negative return  */
   /* error code if there was an error.                                 */
int _TIPM_Request_Reference_Time_Update(unsigned int ConnectionID, Word_t TimeUpdateControlPointHandle);

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Current Time      */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Current_Time_Data        */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Current_Time(unsigned int ValueLength, Byte_t *Value, CTS_Current_Time_Data_t *CTSCurrentTime);

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Local Time        */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Local_Time_Data structure*/
   /* in which to store the decoded information. This function returns  */
   /* zero if successful and a negative return error code if there was  */
   /* an error.                                                         */
int _TIPM_Decode_Local_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Local_Time_Information_Data_t *CTSLocalTime);

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Reference Time    */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the CTS_Reference_Time_Data      */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Reference_Time_Information(unsigned int ValueLength, Byte_t *Value, CTS_Reference_Time_Information_Data_t *CTSReferenceTime);

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Time With DST     */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the NDCS_Time_With_DST_Data      */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Time_With_DST(unsigned int ValueLength, Byte_t *Value, NDCS_Time_With_Dst_Data_t *NDCSTimeWithDST);

   /* The following functions is responsible for decoding binary data   */
   /* returned from a remote time server into a valid Time Update State */
   /* structure. The first parameter is the length of the data returned */
   /* from the server. The second parameter is a pointer to the data    */
   /* returned. The third parameter is the RTUS_Time_Update_State_Data  */
   /* structure in which to store the decoded information. This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
int _TIPM_Decode_Time_Update_State(unsigned int ValueLength, Byte_t *Value, RTUS_Time_Update_State_Data_t *RTUSTimeUpdateState);

#endif
