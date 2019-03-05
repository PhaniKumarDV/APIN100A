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

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Audio Manager Implementation.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Audio Manager Implementation.                                     */
int _AUDM_Initialize(void);

   /* The following function is responsible for shutting down the Audio */
   /* Manager Implementation.  After this function is called the Audio  */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _AUDM_Initialize() function.  */
void _AUDM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Audio Stream or*/
   /* Remote Control connection.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _AUDM_Connection_Request_Response(AUD_Connection_Request_Type_t RequestType, BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect an Audio Stream to a remote device.  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Connect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType, unsigned long StreamFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Audio Stream.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _AUDM_Disconnect_Audio_Stream(BD_ADDR_t RemoteDeviceAddress, AUD_Stream_Type_t StreamType);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Audio   */
   /* sessions of the specified stream type (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer). The next parameter is       */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _AUDM_Query_Audio_Connected_Devices(AUD_Stream_Type_t StreamType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

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

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for incoming Audio*/
   /* Stream and Remote Control connections.  This function returns zero*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _AUDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to encode the Audio Data themselves (as  */
   /*          opposed to having this module encode and send the data). */
   /*          The caller can determine the current configuration of the*/
   /*          stream by calling the                                    */
   /*          _AUDM_Query_Audio_Stream_Configuration() function.       */
   /* * NOTE * The data that is sent *MUST* contain the AVDTP Header    */
   /*          Information (i.e. the first byte of the data *MUST* be a */
   /*          valid AVDTP Header byte).                                */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _AUDM_Send_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int AudioManagerDataEventCallbackID, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame);

   /* The following function is responsible for sending the specified   */
   /* Encoded Audio Data to the remote SNK.  This function accepts as   */
   /* input the Audio Manager Data Handler ID (registered via call to   */
   /* the AUDM_Register_Data_Event_Callback() function), followed by the*/
   /* number of bytes of raw, encoded, audio frame information, followed*/
   /* by the raw, encoded, Audio Data to send, followed by flags which  */
   /* specify the format of the data (currently not used, this parameter*/
   /* is reserved for future additions), followed by the RTP Header     */
   /* Information.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
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
int _AUDM_Send_RTP_Encoded_Audio_Data(BD_ADDR_t RemoteDeviceAddress, unsigned int AudioManagerDataEventCallbackID, unsigned int RawAudioDataFrameLength, unsigned char *RawAudioDataFrame, unsigned long Flags, AUD_RTP_Header_Info_t *RTPHeaderInfo);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control session to a remote device.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _AUDM_Connect_Remote_Control(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to disconnect a currently connected Remote Control        */
   /* session.  This function returns zero if successful, or a negative */
   /* return error code if there was an error.                          */
int _AUDM_Disconnect_Remote_Control(BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Remote  */
   /* Control Target or Controller sessions (specified by the first     */
   /* parameter). This function accepts a the local service type to     */
   /* query, followed by buffer information to receive any currently    */
   /* connected device addresses of the specified connection type. The  */
   /* first parameter specifies the local service type to query the     */
   /* connection information for. The second parameter specifies the    */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer). The next parameter is       */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful. The    */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters). This function returns a non-negative   */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer. This    */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _AUDM_Query_Remote_Control_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

   /* The following function is responsible for sending the specified   */
   /* Remote Control Command to the remote Device.  This function       */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* _AUDM_Register_Remote_Control_Event_Callback() function), followed*/
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Response Timeout (in milliseconds), followed by a */
   /* pointer to the actual Remote Control Message to send.  This       */
   /* function returns a positive, value if successful or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Transaction ID of the Remote Control Event that was  */
   /*          submitted.                                               */
int _AUDM_Send_Remote_Control_Command(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned long ResponseTimeout, AUD_Remote_Control_Command_Data_t *CommandData);

   /* The following function is responsible for sending the specified   */
   /* Remote Control Response to the remote Device.  This function      */
   /* accepts as input the Audio Manager Remote Control Handler ID      */
   /* (registered via call to the                                       */
   /* _AUDM_Register_Remote_Control_Event_Callback() function), followed*/
   /* by the Device Address of the Device to send the command to,       */
   /* followed by the Transaction ID of the Remote Control Event,       */
   /* followed by a pointer to the actual Remote Control Response       */
   /* Message to send.  This function returns zero if successful or a   */
   /* negative return error code if there was an error.                 */
int _AUDM_Send_Remote_Control_Response(unsigned int AudioManagerRemoteControlEventCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int TransactionID, AUD_Remote_Control_Response_Data_t *ResponseData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Audio Manager    */
   /* Service.  This Callback will be dispatched by the Audio Manager   */
   /* when various Audio Manager Events occur.  This function returns a */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the        */
   /*          Stream Event Handler ID.  This value can be passed to the*/
   /*          _AUDM_Un_Register_Stream_Events() function to Un-Register*/
   /*          the Event Handler.                                       */
int _AUDM_Register_Stream_Events(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Event Handler   */
   /* (registered via a successful call to the                          */
   /* _AUDM_Register_Stream_Events() function).  This function accepts  */
   /* input the Stream Event Handler ID (return value from              */
   /* _AUDM_Register_Stream_Events() function).                         */
int _AUDM_Un_Register_Stream_Events(unsigned int StreamEventsHandlerID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Audio     */
   /* Manager Service to explicitly process Data (either Source or      */
   /* Sink).  This Callback will be dispatched by the Audio Manager when*/
   /* various Audio Manager Events occur.  This function accepts Audio  */
   /* Stream Type to register.  This function returns a positive        */
   /* (non-zero) value if successful, or a negative return error code if*/
   /* there was an error.                                               */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _AUDM_Send_Encoded_Audio_Data() function to send data    */
   /*          for the Audio Source).                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Audio Stream Type.                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _AUDM_Un_Register_Data_Events() function to              */
   /*          un-register the callback from this module.               */
int _AUDM_Register_Stream_Data_Events(AUD_Stream_Type_t StreamType);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Data Event      */
   /* Callback (registered via a successful call to the                 */
   /* _AUDM_Register_Stream_Data_Events() function).  This function     */
   /* accepts as input the Audio Manager Event Callback ID (return value*/
   /* from _AUDM_Register_Stream_Data_Events() function).               */
int _AUDM_Un_Register_Stream_Data_Events(unsigned int AudioManagerDataCallbackID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback with the Audio Manager      */
   /* Service to explicitly process Remote Control Data (either         */
   /* Controller or Target).  This function accepts the Service Type    */
   /* (Target or Controller) of the events that are to be registered    */
   /* for.  This function returns a positive (non-zero) value if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _AUDM_Send_Remote_Control_Command() or                   */
   /*          _AUDM_Send_Remote_Control_Response() functions to send   */
   /*          Remote Control Events.                                   */
   /* * NOTE * There can only be a single Data Event Handler registered */
   /*          for each Service Type.                                   */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _AUDM_Un_Register_Remote_Control_Event_Callback()        */
   /*          function to un-register the callback from this module.   */
int _AUDM_Register_Remote_Control_Event_Callback(unsigned int ServiceType);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Audio Manager Remote Control  */
   /* Event Callback (registered via a successful call to the           */
   /* _AUDM_Register_Remote_Control_Event_Callback() function).  This   */
   /* function accepts as input the Audio Manager Remote Control Event  */
   /* Callback ID (return value from                                    */
   /* _AUDM_Register_Remote_Control_Event_Callback() function).         */
int _AUDM_Un_Register_Remote_Control_Event_Callback(unsigned int AudioManagerRemoteControlCallbackID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect a Remote Control Browsing session to a remote  */
   /* device.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _AUDM_Connect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect a currently connected Remote Control  */
   /* Browsing session.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
int _AUDM_Disconnect_Remote_Control_Browsing(BD_ADDR_t RemoteDeviceAddress);

#endif
