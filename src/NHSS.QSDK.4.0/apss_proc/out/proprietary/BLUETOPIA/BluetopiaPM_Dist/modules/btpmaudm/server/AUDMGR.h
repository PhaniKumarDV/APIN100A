/*****< audmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDMGR - Audio Manager Implementation for Stonestreet One Bluetooth       */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __AUDMGRH__
#define __AUDMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTAUD.h"            /* Audio Framework Prototypes/Constants.     */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Audio Manager Implementation.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Audio Manager Implementation.                                     */
int _AUDM_Initialize(AUDM_Initialization_Data_t *InitializationInfo);

   /* The following function is responsible for shutting down the Audio */
   /* Manager Implementation.  After this function is called the Audio  */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _AUDM_Initialize() function.  */
void _AUDM_Cleanup(void);

   /* The following function is responsible for informing the Audio     */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Audio Manager with the      */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _AUDM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream state of the     */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream State of the Audio Stream (if*/
   /* this function is successful).                                     */
int _AUDM_Query_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t *StreamState);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream format of the    */
   /* specified Audio Stream.  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.  The final */
   /* parameter will hold the Audio Stream Format of the Audio Stream   */
   /* (if this function is successful).                                 */
int _AUDM_Query_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);

   /* The following function is provided to allow a mechanism for local */
   /* modules to start/suspend the specified Audio Stream.  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Change_Audio_Stream_State(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_State_t StreamState);

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the current format used by the specified Audio  */
   /* Stream.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Change_Audio_Stream_Format(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Format_t *StreamFormat);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if the current Audio Stream configuration of */
   /* the specified Audio Stream.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* The final parameter will hold the Audio Stream Configuration of   */
   /* the Audio Stream (if this function is successful).                */
int _AUDM_Query_Audio_Stream_Configuration(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, AUD_Stream_Configuration_t *StreamConfiguration);

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the number of bytes of raw, encoded, audio frame            */
   /* information, followed by the raw, encoded, Audio Data to send.    */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          _AUDM_Query_Audiop_Stream_Configuration() function.      */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _AUDM_Send_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame);

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the number of bytes of raw, encoded, audio frame            */
   /* information, followed by the raw, encoded, Audio Data to send,    */
   /* followed by flags which specify the format of the data (currently */
   /* not used, this parameter is reserved for future additions),       */
   /* followed by the RTP Header Information.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          AUDM_Query_Audio_Stream_Configuration() function.        */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
   /* * NOTE * This is a low level function and allows the user to      */
   /*          specify the RTP Header Information for the outgoing data */
   /*          packet.  To use the default values for the RTP Header    */
   /*          Information use AUDM_Send_Encoded_Audio_Data() instead.  */
int _AUDM_Send_RTP_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* session. This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Device Address of the Device to send the     */
   /* command to, followed by the Response Timeout (in milliseconds),   */
   /* followed by a pointer to the actual Remote Control Command Data to*/
   /* send.  This function returns a positive, value if successful or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
int _AUDM_Send_Remote_Control_Command(BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData);

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Device Address of the Device to send the     */
   /* command to, followed by the Transaction ID of the Remote Control  */
   /* Event, followed by a pointer to the actual Remote Control Message */
   /* to send.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Send_Remote_Control_Response(BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* session. This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress);

#endif
