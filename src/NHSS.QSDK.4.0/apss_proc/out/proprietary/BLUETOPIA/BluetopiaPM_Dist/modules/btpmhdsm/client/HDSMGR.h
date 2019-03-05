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
int _HDSM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Headset Manager implementation.  After this function is called the*/
   /* Headset Manager implementation will no longer operate until it is */
   /* initialized again via a call to the _HDSM_Initialize() function.  */
void _HDSM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened.  An */
   /*          hetHDSConnected event will notify of this status.        */
int _HDSM_Connection_Request_Response(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Headset/Audio Gateway device.  This*/
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.  This function accepts the connection */
   /* type to make as the first parameter.  This parameter specifies the*/
   /* LOCAL connection type (i.e.  if the caller would like to connect  */
   /* the local Headset service to a remote Audio Gateway device, the   */
   /* Headset connection type would be specified for this parameter).   */
   /* This function also accepts the connection information for the     */
   /* remote device (address and server port).  This function accepts   */
   /* the connection flags to apply to control how the connection is    */
   /* made regarding encryption and/or authentication.                  */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e.  NULL) then the */
   /*          connection status will be returned asynchronously in the */
   /*          Headset Manager Connection Status Event (if specified).  */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDSConnectionStatus event will be dispatched to denote*/
   /*          the status of the connection.  This is the ONLY way to   */
   /*          receive this event, as an event callack registered with  */
   /*          the HDSM_Register_Event_Callback() will NOT receive      */
   /*          connection status events.                                */
int _HDSM_Connect_Remote_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags);

   /* The following function exists to close an active Headset or Audio */
   /* Gateway connection that was previously opened by any of the       */
   /* following mechanisms:                                             */
   /*   - Successful call to HDSM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Headset or Audio Gateway) which was    */
   /*     accepted either automatically or by a call to                 */
   /*     HDSM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Headset or Audio   */
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
int _HDSM_Disconnect_Device(HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Headset */
   /* or Audio Gateway Devices (specified by the first parameter).  This*/
   /* function accepts a the local service type to query, followed by   */
   /* buffer information to receive any currently connected device      */
   /* addresses of the specified connection type.  The first parameter  */
   /* specifies the local service type to query the connection          */
   /* information for.  The second parameter specifies the maximum      */
   /* number of BD_ADDR entries that the buffer will support (i.e.  can */
   /* be copied into the buffer).  The next parameter is optional and,  */
   /* if specified, will be populated with the total number of connected*/
   /* devices if the function is successful.  The final parameter can be*/
   /* used to retrieve the total number of connected devices (regardless*/
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer.  This function returns a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int _HDSM_Query_Connected_Devices(HDSM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for Headset or Audio   */
   /* Gateway connections.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * On input the TotalNumberAdditionalIndicators member of   */
   /*          the structure should be set to the total number of       */
   /*          additional indicator entry structures that the           */
   /*          AdditionalIndicatorList member points to.  On return from*/
   /*          this function this structure member holds the total      */
   /*          number of additional indicator entries that are supported*/
   /*          by the connection.  The NumberAdditionalIndicators member*/
   /*          will hold (on return) the number of indicator entries    */
   /*          that are actually present in the list.                   */
   /* * NOTE * It is possible to not query the additional indicators by */
   /*          passing zero for the TotalNumberAdditionalIndicators     */
   /*          member.  This member will still hold the total of        */
   /*          supported indicators on return of this function.         */
int _HDSM_Query_Current_Configuration(HDSM_Connection_Type_t ConnectionType, HDSM_Current_Configuration_t *CurrentConfiguration);

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming connection flags for Headset and   */
   /* Audio Gateway connections.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
int _HDSM_Change_Incoming_Connection_Flags(HDSM_Connection_Type_t ConnectionType, unsigned long ConnectionFlags);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  When called by a     */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current speaker gain value.  When     */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the speaker gain of the remote Headset   */
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the speaker gain to be sent to the remote device. */
   /* The speaker gain Parameter *MUST* be between the values:          */
   /*                                                                   */
   /*    HDSET_SPEAKER_GAIN_MINIMUM                                     */
   /*    HDSET_SPEAKER_GAIN_MAXIMUM                                     */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Set_Remote_Speaker_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int SpeakerGain);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  When called by a  */
   /* Headset device this function is provided as a means to inform the */
   /* remote Audio Gateway of the current microphone gain value.  When  */
   /* called by an Audio Gateway this function provides a means for the */
   /* Audio Gateway to control the microphone gain of the remote Headset*/
   /* device.  This function accepts as its input parameters the        */
   /* connection type indicating the local connection which will process*/
   /* the command and the microphone gain to be sent to the remote      */
   /* device.  The microphone gain Parameter *MUST* be between the      */
   /* values:                                                           */
   /*                                                                   */
   /*    HDSET_MICROPHONE_GAIN_MINIMUM                                  */
   /*    HDSET_MICROPHONE_GAIN_MAXIMUM                                  */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Set_Remote_Microphone_Gain(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int MicrophoneGain);

   /* This function is responsible for sending a button press to a      */
   /* remote Audio Gateway to.  This function return zero if successful */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Send_Button_Press(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

   /* This function is responsible for sending a ring indication to a   */
   /* remote Headset unit.  This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Ring_Indication(unsigned int HeadsetManagerEventCallbackID, BD_ADDR_t RemoteDeviceAddress);

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Headset device.  This function       */
   /* accepts as its input parameter the connection type indicating     */
   /* which connection will process the command.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
   /* * NOTE * The InBandRinging parameter specifies if the purpose of  */
   /*          setting up the Audio Connection is for In-Band Ring      */
   /*          (TRUE) or not (FALSE).                                   */
int _HDSM_Setup_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Boolean_t InBandRinging);

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HDSM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Headset */
   /* device.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * The HeadsetManagerEventCallbackID parameter *MUST* be the*/
   /*          callback ID that was registered via a successful call to */
   /*          the HDSM_Register_Event_Callback() function (specifying  */
   /*          it as a control callback for the specified service type).*/
   /*          There can only be a single control callback registered   */
   /*          for each service type in the system.                     */
int _HDSM_Release_Audio_Connection(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress);

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Headset Manager Data Handler ID (registered  */
   /* via call to the _HDSM_Register_Data_Events() function), followed  */
   /* by the the connection type indicating which connection will       */
   /* transmit the audio data, the length (in Bytes) of the audio data  */
   /* to send, and a pointer to the audio data to send to the remote    */
   /* entity.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
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
int _HDSM_Send_Audio_Data(unsigned int HeadsetManagerDataEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int AudioDataLength, unsigned char *AudioData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the HDSET Manager    */
   /* Service.  This Callback will be dispatched by the HDSET Manager   */
   /* when various HDSET Manager events occur.  This function accepts as*/
   /* its parameters the ConnectionType of the type of connection to    */
   /* register for events, and a boolean that specifies if this is the  */
   /* control callback (there can only be one control callback in the   */
   /* system).  This function returns a non-zero value if successful or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * The return value from this function specifies the HDSET  */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _HDSM_Un_Register_Events() function to un-Register the   */
   /*          event handler.                                           */
int _HDSM_Register_Events(HDSM_Connection_Type_t ConnectionType, Boolean_t ControlHandler);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager event callback*/
   /* (registered via a successful call to the _HDSM_Register_Events()  */
   /* function.  This function accepts as input the Headset Manager     */
   /* event callback ID (return value from the _HDSM_Register_Events()  */
   /* function).                                                        */
int _HDSM_Un_Register_Events(unsigned int HeadsetManagerEventCallbackID);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register an event callback function with the Headset   */
   /* Profile Manager service to explicitly process SCO audio data.     */
   /* This callback will be dispatched by the Headset Manager when      */
   /* various Headset Manager events occur.  This function accepts the  */
   /* connection type which indicates the connection type the data      */
   /* registration callback to register for.  This function returns a   */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          _HDSM_Send_Audio_Data() function to send SCO audio data. */
   /* * NOTE * There can only be a single data event handler registered */
   /*          for each type of Headset Manager connection type.        */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          _HDSM_Un_Register_Data_Events() function to un-register  */
   /*          the callback from this module.                           */
int _HDSM_Register_Data_Events(HDSM_Connection_Type_t ConnectionType);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Headset Manager data event    */
   /* callback (registered via a successful call to the                 */
   /* _HDSM_Register_Data_Events() function.  This function accepts as  */
   /* input the Headset Manager event callback ID (return value from the*/
   /* _HDSM_Register_Data_Events() function).                           */
int _HDSM_Un_Register_Data_Events(unsigned int HeadsetManagerDataCallbackID);

   /* The following function is provided to allow a mechanism to query  */
   /* the low level SCO Handle for an active SCO Connection. The        */
   /* first parameter is the Callback ID that is returned from a        */
   /* successful call to HDSM_Register_Event_Callback().  The second    */
   /* parameter is the local connection type of the SCO connection.  The*/
   /* third parameter is the address of the remote device of the SCO    */
   /* connection.  The fourth parameter is a pointer to the location to */
   /* store the SCO Handle. This function returns zero if successful or */
   /* a negative return error code if there was an error.               */
int _HDSM_Query_SCO_Connection_Handle(unsigned int HeadsetManagerEventCallbackID, HDSM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, Word_t *SCOHandle);

#endif
