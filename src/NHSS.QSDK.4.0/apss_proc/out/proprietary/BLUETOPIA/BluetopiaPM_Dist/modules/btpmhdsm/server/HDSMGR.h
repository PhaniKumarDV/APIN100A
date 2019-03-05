/*****< hdsmgr.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDSMGR - Headset Manager Implementation for Stonestreet One Bluetooth     */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/17/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HDSMGRH__
#define __HDSMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHDSM.h"           /* HDSET Framework Prototypes/Constants.     */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Headset Manager implementation.  This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Headset Manager Implementation.                                   */
int _HDSM_Initialize(HDSM_Initialization_Data_t *AudioGatewayInitializationInfo, HDSM_Initialization_Data_t *HeadsetInitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Headset Manager implementation.  After this function is called the*/
   /* Headset Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _HDSM_Initialize() function.  */
void _HDSM_Cleanup(void);

   /* The following function is responsible for informing the Headset   */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Headset Manager with the    */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
void _HDSM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for installing/removing the */
   /* Headset or AufioGateway SDP record.                               */
int _HDSM_UpdateSDPRecord(HDSM_Connection_Type_t ConnectionType, Boolean_t Install);

   /* The following function is a utility function that exists to allow */
   /* the caller a mechanism to determine the incoming connection type  */
   /* of the specified incoming connection (based on the Headset Port   */
   /* ID).                                                              */
Boolean_t _HDSM_QueryIncomingConnectionType(unsigned int HDSETID, HDSM_Connection_Type_t *ConnectionType, unsigned int *ServerPort);

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. A   */
   /*          port open indication event will notify of this status.   */
int _HDSM_Connection_Request_Response(unsigned int HDSETID, Boolean_t AcceptConnection);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns a non zero value if successful, or a negative    */
   /* return error code if there was an error.  The return value from   */
   /* this function (if successful) represents the Bluetopia Headset ID */
   /* that is used to track this connection.  This function accepts the */
   /* connection type to make as the first parameter.  This parameter   */
   /* specifies the LOCAL connection type (i.e.  if the caller would    */
   /* like to connect the local Headset service to a remote Audio       */
   /* Gateway device, the Headset connection type would be specified for*/
   /* this parameter).  This function also accepts the connection       */
   /* information for the remote device (address and server port).      */
int _HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort);

   /* The following function exists to close an active Headset or Audio */
   /* Gateway connection that was previously opened by any of the       */
   /* following mechanisms: - Successful call to                        */
   /* HDSM_Connect_Remote_Device() function.  - Incoming open request   */
   /* (Headset or Audio Gateway) which was accepted either automatically*/
   /* or by a call to HDSM_Connection_Request_Response().  This function*/
   /* accepts as input the type of the local connection which should    */
   /* close its active connection.  This function returns zero if       */
   /* successful, or a negative return value if there was an error.     */
   /* This function does NOT un-register any Headset or Audio Gateway   */
   /* services from the system, it ONLY disconnects any connection that */
   /* is currently active on the specified service.                     */
int _HDSM_Disconnect_Device(unsigned int HDSETID);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection ID indicating the local connection which will process  */
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /* HDSET_SPEAKER_GAIN_MINIMUM HDSET_SPEAKER_GAIN_MAXIMUM This        */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HDSM_Set_Remote_Speaker_Gain(unsigned int HDSETID, unsigned int SpeakerGain);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection ID indicating the local connection which will process  */
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HDSM_Set_Remote_Microphone_Gain(unsigned int HDSETID, unsigned int MicrophoneGain);

   /* This function is responsible for sending a button press to a      */
   /* remote Audi Gateway.  This function return zero if successful or a*/
   /* negative return error code if there was an error.                 */
int _HDSM_Send_Button_Press(unsigned int HDSETID);

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _HDSM_Ring_Indication(unsigned int HDSETID);

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset devices.  This function      */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HDSM_Setup_Audio_Connection(unsigned int HDSETID, Boolean_t InBandRinging);

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
int _HDSM_Release_Audio_Connection(unsigned int HDSETID);

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset ID, followed by the length (in Bytes)*/
   /* of the audio data to send, and a pointer to the audio data to send*/
   /* to the remote dntity.  This function returns zero if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data via*/
   /*          a hardware codec.                                        */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _HDSM_Send_Audio_Data(unsigned int HDSETID, unsigned int AudioDataLength, unsigned char *AudioData);

#endif
