/*****< hfrmgr.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HFRMGR - Hands Free Manager Implementation for Stonestreet One Bluetooth  */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/28/11  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HFRMGRH__
#define __HFRMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTHFRM.h"           /* HFRE Framework Prototypes/Constants.      */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Hands Free Manager implementation.  This function  */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Hands Free Manager Implementation.                                */
int _HFRM_Initialize(HFRM_Initialization_Data_t *AudioGatewayInitializationInfo, HFRM_Initialization_Data_t *HandsFreeInitializationInfo);

   /* The following function is responsible for shutting down the Hands */
   /* Free Manager implementation.  After this function is called the   */
   /* Hands Free Manager implementation will no longer operate until it */
   /* is initialized again via a call to the _HFRM_Initialize()         */
   /* function.                                                         */
void _HFRM_Cleanup(void);

   /* The following function is responsible for informing the Hands Free*/
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
   /* * NOTE * When this parameter is set to non-zero, this function    */
   /*          will actually initialize the Hands Free Manager with the */
   /*          specified Bluetooth Stack ID.  When this parameter is set*/
   /*          to zero, this function will actually clean up all        */
   /*          resources associated with the prior initialized Bluetooth*/
   /*          Stack.                                                   */
   /* * NOTE * The return value indicates the controller support for    */
   /*          Wide Band Speech.  The return value is only meaningful of*/
   /*          the BluetoothStackID is non-zer.                         */
Boolean_t _HFRM_SetBluetoothStackID(unsigned int BluetoothStackID);
 
   /* The following function is responsible for installing/removing the */
   /* Hands Free or AufioGateway SDP record.                            */
int _HFRM_UpdateSDPRecord(HFRM_Connection_Type_t ConnectionType, Boolean_t Install);

   /* The following function is a utility function that exists to allow */
   /* the caller a mechanism to determine the incoming connection type  */
   /* of the specified incoming connection (based on the Hands Free Port*/
   /* ID).                                                              */
Boolean_t _HFRM_QueryIncomingConnectionType(unsigned int HFREID, HFRM_Connection_Type_t *ConnectionType, unsigned int *ServerPort);

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a Local Server. This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the port has been successfully opened. A   */
   /*          port open indication event will notify of this status.   */
int _HFRM_Connection_Request_Response(unsigned int HFREID, Boolean_t AcceptConnection);

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Hands Free/Audio Gateway device.   */
   /* This function returns a non zero value if successful, or a        */
   /* negative return error code if there was an error.  The return     */
   /* value from this function (if successful) represents the Bluetopia */
   /* Hands Free ID that is used to track this connection.  This        */
   /* function accepts the connection type to make as the first         */
   /* parameter.  This parameter specifies the LOCAL connection type    */
   /* (i.e. if the caller would like to connect the local Hands Free    */
   /* service to a remote Audio Gateway device, the Hands Free          */
   /* connection type would be specified for this parameter).  This     */
   /* function also accepts the connection information for the remote   */
   /* device (address and server port).                                 */
int _HFRM_Connect_Remote_Device(HFRM_Connection_Type_t ConnectionType, BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort);

   /* The following function exists to close an active Hands Free or    */
   /* Audio Gateway connection that was previously opened by any of the */
   /* following mechanisms:                                             */
   /*   - Successful call to HFRM_Connect_Remote_Device() function.     */
   /*   - Incoming open request (Hands Free or Audio Gateway) which was */
   /*     accepted either automatically or by a call to                 */
   /*     HFRM_Connection_Request_Response().                           */
   /* This function accepts as input the type of the local connection   */
   /* which should close its active connection.  This function returns  */
   /* zero if successful, or a negative return value if there was an    */
   /* error.  This function does NOT un-register any Hands Free or Audio*/
   /* Gateway services from the system, it ONLY disconnects any         */
   /* connection that is currently active on the specified service.     */
int _HFRM_Disconnect_Device(unsigned int HFREID);

   /* This function is responsible for disabling echo cancellation and  */
   /* noise reduction on the remote device.  This function may be       */
   /* performed by both the Hands Free and the Audio Gateway connections*/
   /* for which a valid service level connection exists but no audio    */
   /* connection exists.  This function accepts as its input parameter  */
   /* the Hands Free Port ID indicating the connection that is to       */
   /* receive this command.  This function returns zero if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * It is not possible to enable this feature once it has    */
   /*          been disbled because the specification provides no means */
   /*          to re-enable this feature.  This feature will remained   */
   /*          disabled until the current service level connection has  */
   /*          been dropped.                                            */
int _HFRM_Disable_Remote_Echo_Cancellation_Noise_Reduction(unsigned int HFREID);

   /* When called by a Hands Free device, this function is responsible  */
   /* for requesting activation or deactivation of the voice recognition*/
   /* which resides on the remote Audio Gateway.  When called by an     */
   /* Audio Gateway, this function is responsible for informing the     */
   /* remote Hands Free device of the current activation state of the   */
   /* local voice recognition function.  This function may only be      */
   /* called by local devices that were opened with support for voice   */
   /* recognition.  This function accepts as its input parameters the   */
   /* connection ID indicating the local connection which will process  */
   /* the command and a BOOLEAN flag specifying the type of request or  */
   /* notification to send.  When active the voice recognition function */
   /* on the Audio Gateway is turned on, when inactive the voice        */
   /* recognition function on the Audio Gateway is turned off.  This    */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HFRM_Set_Remote_Voice_Recognition_Activation(unsigned int HFREID, Boolean_t VoiceRecognitionActive);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices speaker gain.  This function may    */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current speaker   */
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the speaker gain*/
   /* of the remote Hands Free device.  This function accepts as its    */
   /* input parameters the connection ID indicating the local           */
   /* connection which will process the command and the speaker gain to */
   /* be sent to the remote device.  The speaker gain Parameter *MUST*  */
   /* be between the values:                                            */
   /*                                                                   */
   /*    HFRE_SPEAKER_GAIN_MINIMUM                                      */
   /*    HFRE_SPEAKER_GAIN_MAXIMUM                                      */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Set_Remote_Speaker_Gain(unsigned int HFREID, unsigned int SpeakerGain);

   /* This function is responsible for allowing synchronization and     */
   /* setting of the remote devices microphone gain.  This function may */
   /* only be performed if a valid service level connection exists.     */
   /* When called by a Hands Free device this function is provided as a */
   /* means to inform the remote Audio Gateway of the current microphone*/
   /* gain value.  When called by an Audio Gateway this function        */
   /* provides a means for the Audio Gateway to control the microphone  */
   /* gain of the remote Hands Free device.  This function accepts as   */
   /* its input parameters the connection ID indicating the local       */
   /* connection which will process the command and the microphone gain */
   /* to be sent to the remote device.  The microphone gain Parameter   */
   /* *MUST* be between the values:                                     */
   /*                                                                   */
   /*    HFRE_MICROPHONE_GAIN_MINIMUM                                   */
   /*    HFRE_MICROPHONE_GAIN_MAXIMUM                                   */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Set_Remote_Microphone_Gain(unsigned int HFREID, unsigned int MicrophoneGain);

   /* Send a codec ID. The audio gateway uses this function to send the */
   /* preferred codec. The hands free device uses the function to       */ 
   /* confirm the audio gateways choice. HFREID is the hands free ID    */
   /* and CodecID identifies the codec.                                 */
int _HFRM_Send_Select_Codec(unsigned int HFREID, unsigned char CodecID);

   /* The following function is responsible for querying the remote     */
   /* control indicator status.  This function may only be performed by */
   /* a local Hands Free unit with a valid service level connection to a*/
   /* connected remote Audio Gateway.  The results to this query will be*/
   /* returned as part of the control indicator status confirmation     */
   /* event (hetHFRControlIndicatorStatus).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Query_Remote_Control_Indicator_Status(unsigned int HFREID);

   /* This function is responsible for enabling or disabling the        */
   /* indicator event notification on a remote Audio Gateway.  This     */
   /* function may only be performed by Hands Free devices that have a  */
   /* valid service level connection to a connected remote Audio        */
   /* Gateway.  When enabled, the remote Audio Gateway device will send */
   /* unsolicited responses to update the local device of the current   */
   /* control indicator values.  This function accepts as its input     */
   /* parameter a BOOLEAN flag used to enable or disable event          */
   /* notification.  This function returns zero if successful or a      */
   /* negative return error code if there was an error.                 */
int _HFRM_Enable_Remote_Indicator_Event_Notification(unsigned int HFREID, Boolean_t EnableEventNotification);

   /* This function is responsible for querying the call holding and    */
   /* multi-party services which are supported by the remote Audio      */
   /* Gateway.  This function is used by Hands Free connections which   */
   /* support three way calling and call waiting to determine the       */
   /* features supported by the remote Audio Gateway.  This function can*/
   /* only be used if a valid service level connection to a connected   */
   /* remote Audio Gateway exists.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support(unsigned int HFREID);

   /* This function is responsible for allowing the control of multiple */
   /* concurrent calls and provides a means for holding calls, releasing*/
   /* calls, switching between two calls and adding a call to a         */
   /* multi-party conference.  This function may only be performed by   */
   /* Hands Free units that support call waiting and multi-party        */
   /* services as well as have a valid service level connection to a    */
   /* connected remote Audio Gateway.  The selection which is made      */
   /* should be one that is supported by the remote Audio Gateway       */
   /* (queried via a call to the                                        */
   /* HFRM_Query_Remote_Call_Holding_Multiparty_Service_Support()       */
   /* function).  This function accepts as its input parameter the      */
   /* selection of how to handle the currently waiting call.  If the    */
   /* selected handling type requires an index it should be provided in */
   /* the last parameter.  Otherwise the final paramter is ignored.     */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Send_Call_Holding_Multiparty_Selection(unsigned int HFREID, HFRE_Call_Hold_Multiparty_Handling_Type_t CallHoldMultipartyHandling, unsigned int Index);

   /* This function is responsible for enabling or disabling call       */
   /* waiting notification on a remote Audio Gateway.  By default the   */
   /* call waiting notification is enabled in the network but disabled  */
   /* for notification via the service level connection (between Hands  */
   /* Free and Audio Gateway).  This function may only be performed by a*/
   /* Hands Free unit for which a valid service level connection to a   */
   /* connected remote Audio Gateway exists.  This function may only be */
   /* used to enable call waiting notifications if the local Hands Free */
   /* service supports call waiting and multi-party services.  This     */
   /* function accepts as its input parameter a BOOLEAN flag specifying */
   /* if this is a call to enable or disable this functionality.  This  */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HFRM_Enable_Remote_Call_Waiting_Notification(unsigned int HFREID, Boolean_t EnableNotification);

   /* This function is responsible for enabling or disabling call line  */
   /* identification notification on a remote Audio Gateway.  By        */
   /* default, the call line identification notification via the service*/
   /* level connection is disabled.  This function may only be performed*/
   /* by Hands Free units for which a valid service level connection to */
   /* a connected remote Audio Gateway exists.  This function may only  */
   /* be used to enable call line notifications if the local Hands Free */
   /* unit supports call line identification.  This function accepts as */
   /* its input parameters a BOOLEAN flag specifying if this is a call  */
   /* to enable or disable this functionality.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Enable_Remote_Call_Line_Identification_Notification(unsigned int HFREID, Boolean_t EnableNotification);

   /* This function is responsible for dialing a phone number on a      */
   /* remote Audio Gateway.  This function may only be performed by     */
   /* Hands Free units for which a valid service level connection to a  */
   /* remote Audio Gateway exists.  This function accepts as its input  */
   /* parameter the phone number to dial on the remote Audio Gateway.   */
   /* This parameter should be a pointer to a NULL terminated string and*/
   /* its length *MUST* be between the values of:                       */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Dial_Phone_Number(unsigned int HFREID, char *PhoneNumber);

   /* This function is responsible for dialing a phone number from a    */
   /* memory location (index) found on the remote Audio Gateway.  This  */
   /* function may only be performed by Hands Free devices for which a  */
   /* valid service level connection to a connected remote Audio Gateway*/
   /* exists.  This function accepts as its input parameter the memory  */
   /* location (index) for which the phone number to dial already exists*/
   /* on the remote Audio Gateway.  This function returns zero if       */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Dial_Phone_Number_From_Memory(unsigned int HFREID, unsigned int MemoryLocation);

   /* This function is responsible for re-dialing the last number dialed*/
   /* on a remote Audio Gateway.  This function may only be performed by*/
   /* Hands Free devices for which a valid service level connection to a*/
   /* connected remote Audio Gateway exists.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Redial_Last_Phone_Number(unsigned int HFREID);

   /* This function is responsible for sending the command to a remote  */
   /* Audi Gateway to answer an incoming call.  This function may only  */
   /* be performed by Hands Free devices for which a valid service level*/
   /* connection to a connected remote Audio Gateway exists.  This      */
   /* function return zero if successful or a negative return error code*/
   /* if there was an error.                                            */
int _HFRM_Answer_Incoming_Call(unsigned int HFREID);

   /* This function is responsible for transmitting DTMF codes to a     */
   /* remote Audio Gateway to be sent as a DTMF code over an on-going   */
   /* call.  This function may only be performed by Hands Free devices  */
   /* for which a valid service level connection to a connected remote  */
   /* Audio Gateway exists and an on-going call exists.  This function  */
   /* accepts as input the DTMF code to be transmitted.  This Code must */
   /* be one of the characters:                                         */
   /*                                                                   */
   /*   0-9, *, #, or A-D.                                              */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
int _HFRM_Transmit_DTMF_Code(unsigned int HFREID, char DTMFCode);

   /* This function is responsible for retrieving a phone number to     */
   /* associate with a unique voice tag to be stored in memory by the   */
   /* local Hands Free device.  This function may only be performed by a*/
   /* Hands Free device for which a valid service level connection to a */
   /* connected remote Audio Gateway exists.  The Hands Free unit must  */
   /* also support voice recognition to be able to use this function.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * When this function is called no other function may be    */
   /*          called until a voice tag response is received from the   */
   /*          remote Audio Gateway.                                    */
int _HFRM_Voice_Tag_Request(unsigned int HFREID);

   /* This function is responsible for sending a hang-up command to a   */
   /* remote Audio Gateway.  This function may be used to reject an     */
   /* incoming call or to terminate an on-going call.  This function may*/
   /* only be performed by Hands Free devices for which a valid service */
   /* level connection exists.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int _HFRM_Hang_Up_Call(unsigned int HFREID);

   /* The following function is responsible for querying the current    */
   /* call list of the remote Audio Gateway device.  This function may  */
   /* only be performed by a Hands Free device with a valid service     */
   /* level connection to a connected Audio Gateway.  This function     */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HFRM_Query_Remote_Current_Calls_List(unsigned int HFREID);

   /* The following function is responsible for setting the network     */
   /* operator format to long alphanumeric.  This function may only be  */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected Audio Gateway.  This function returns   */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Set_Network_Operator_Selection_Format(unsigned int HFREID);

   /* The following function is responsible for reading the network     */
   /* operator.  This function may only be performed by a Hands Free    */
   /* device with a valid service level connection.  This function      */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * The network operator format must be set before querying  */
   /*          the current network operator.                            */
int _HFRM_Query_Remote_Network_Operator_Selection(unsigned int HFREID);

   /* The following function is responsible for enabling or disabling   */
   /* extended error results reporting.  This function may only be      */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* accepts as its input parameter a BOOLEAN flag indicating whether  */
   /* the reporting should be enabled (TRUE) or disabled (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HFRM_Enable_Remote_Extended_Error_Result(unsigned int HFREID, Boolean_t EnableExtendedErrorResults);

   /* The following function is responsible for retrieving the          */
   /* subscriber number information.  This function may only be         */
   /* performed by a Hands Free device with a valid service level       */
   /* connection to a connected remote Audio Gateway.  This function    */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HFRM_Query_Subscriber_Number_Information(unsigned int HFREID);

   /* The following function is responsible for retrieving the current  */
   /* response and hold status.  This function may only be performed by */
   /* a Hands Free device with a valid service level connection to a    */
   /* connected Audio Gateway.  This function returns zero if successful*/
   /* or a negative return error code if there was an error.            */
int _HFRM_Query_Response_Hold_Status(unsigned int HFREID);

   /* The following function is responsible for setting the state of an */
   /* incoming call.  This function may only be performed by a Hands    */
   /* Free unit with a valid service level connection to a remote Audio */
   /* Gateway.  This function accepts as its input parameter the call   */
   /* state to set as part of this message.  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Set_Incoming_Call_State(unsigned int HFREID, HFRE_Call_State_t CallState);

   /* The following function is responsible for sending an arbitrary    */
   /* command to the remote Audio Gateway (i.e.  non Bluetooth Hands    */
   /* Free Profile command).  This function may only be performed by a  */
   /* Hands Free with a valid service level connection.  This function  */
   /* accepts as its input parameter a NULL terminated ASCII string that*/
   /* represents the arbitrary command to send.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * The Command string passed to this function *MUST* begin  */
   /*          with AT and *MUST* end with the a carriage return ('\r') */
   /*          if this is the first portion of an arbitrary command     */
   /*          that will span multiple writes.  Subsequent calls (until */
   /*          the actual status reponse is received) can begin with    */
   /*          any character, however, they must end with a carriage    */
   /*          return ('\r').                                           */
int _HFRM_Send_Arbitrary_Command(unsigned int HFREID, char *ArbitraryCommand);

   /* Send the list of supported codecs to a remote audio gateway.      */
   /* HFREID is the hands free ID. NumberSupportedCodecs is the number  */
   /* of codec ID in the list. AvailableCodecList is the list.          */
int _HFRM_Send_Available_Codec_List(unsigned int HFREID, unsigned int NumberSupportedCodecs, unsigned char *AvailableCodecList);

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway with a valid service level connection to a       */
   /* connected remote Hands Free device.  This function accepts as its */
   /* input parameters the number of indicators and list of name/value  */
   /* pairs for the indicators to be updated.  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Update_Current_Control_Indicator_Status(unsigned int HFREID, unsigned int NumberUpdateIndicators, HFRE_Indicator_Update_t *UpdateIndicators);

   /* The following function is responsible for updating the current    */
   /* control indicator status.  This function may only be performed by */
   /* an Audio Gateway.  The function will initially set the specified  */
   /* indicator, then, if a valid service level connection exists and   */
   /* event reporting is activated (via the set remote event indicator  */
   /* event notification function by the remote device) an event        */
   /* notification will be sent to the remote device.  This function    */
   /* accepts as its input parameters the name of the indicator to be   */
   /* updated and the new indicator value.  This function returns zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _HFRM_Update_Current_Control_Indicator_Status_By_Name(unsigned int HFREID, char *IndicatorName, unsigned int IndicatorValue);

   /* This function is responsible for sending a call waiting           */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call waiting       */
   /* notification enabled and have a valid service level connection to */
   /* a connected remote Hands Free device.  This function accepts as   */
   /* its input parameter the phone number of the incoming call, if a   */
   /* number is available.  This parameter should be a pointer to a NULL*/
   /* terminated ASCII string (if specified) and must have a length less*/
   /* than:                                                             */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * It is valid to either pass a NULL for the PhoneNumber    */
   /*          parameter or a blank string to specify that there is no  */
   /*          phone number present.                                    */
int _HFRM_Send_Call_Waiting_Notification(unsigned int HFREID, char *PhoneNumber);

   /* This function is responsible for sending call line identification */
   /* notifications to a remote Hands Free device.  This function may   */
   /* only be performed by Audio Gateways which have call line          */
   /* identification notification enabled and have a valid service level*/
   /* connection to a connected remote Hands Free device.  This function*/
   /* accepts as its input parameters the phone number of the incoming  */
   /* call.  This parameter should be a pointer to a NULL terminated    */
   /* string and its length *MUST* be between the values of:            */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* This function return zero if successful or a negative return error*/
   /* code if there was an error.                                       */
int _HFRM_Send_Call_Line_Identification_Notification(unsigned int HFREID, char *PhoneNumber);

   /* This function is responsible for sending a ring indication to a   */
   /* remote Hands Free unit.  This function may only be performed by   */
   /* Audio Gateways for which a valid service level connection to a    */
   /* connected remote Hands Free device exists.  This function returns */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
int _HFRM_Ring_Indication(unsigned int HFREID);

   /* This function is responsible for enabling or disabling in-band    */
   /* ring tone capabilities for a connected Hands Free device.  This   */
   /* function may only be performed by Audio Gateways for which a valid*/
   /* service kevel connection exists.  This function may only be used  */
   /* to enable in-band ring tone capabilities if the local Audio       */
   /* Gateway supports this feature.  This function accepts as its input*/
   /* parameter a BOOLEAN flag specifying if this is a call to Enable or*/
   /* Disable this functionality.  This function returns zero if        */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Enable_Remote_In_Band_Ring_Tone_Setting(unsigned int HFREID, Boolean_t EnableInBandRing);

   /* This function is responsible for responding to a request that was */
   /* received for a phone number to be associated with a unique voice  */
   /* tag by a remote Hands Free device.  This function may only be     */
   /* performed by Audio Gateways that have received a voice tag request*/
   /* Indication.  This function accepts as its input parameter the     */
   /* phone number to be associated with the voice tag.  If the request */
   /* is accepted, the phone number Parameter string length *MUST* be   */
   /* between the values:                                               */
   /*                                                                   */
   /*    HFRE_PHONE_NUMBER_LENGTH_MINIMUM                               */
   /*    HFRE_PHONE_NUMBER_LENGTH_MAXIMUM                               */
   /*                                                                   */
   /* If the caller wishes to reject the request, the phone number      */
   /* parameter should be set to NULL to indicate this.  This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
int _HFRM_Voice_Tag_Response(unsigned int HFREID, char *PhoneNumber);

   /* The following function is responsible for sending the current     */
   /* calls list entries to a remote Hands Free device.  This function  */
   /* may only be performed by Audio Gateways that have received a      */
   /* request to query the remote current calls list.  This function    */
   /* accepts as its input parameters the list of current call entries  */
   /* to be sent and length of the list.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Send_Current_Calls_List(unsigned int HFREID, unsigned int NumberListEntries, HFRE_Current_Call_List_Entry_t *CurrentCallListEntry);

   /* The following function is responsible for sending the network     */
   /* operator.  This function may only be performed by Audio Gateways  */
   /* that have received a request to query the remote network operator */
   /* selection.  This function accepts as input the current network    */
   /* mode and the current network operator.  The network operator      */
   /* should be expressed as a NULL terminated ASCII string (if         */
   /* specified) and must have a length less than:                      */
   /*                                                                   */
   /*    HFRE_NETWORK_OPERATOR_LENGTH_MAXIMUM                           */
   /*                                                                   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * It is valid to either pass a NULL for the NetworkOperator*/
   /*          parameter or a blank string to specify that there is no  */
   /*          network operator present.                                */
int _HFRM_Send_Network_Operator_Selection(unsigned int HFREID, unsigned int NetworkMode, char *NetworkOperator);

   /* The following function is responsible for sending extended error  */
   /* results.  This function may only be performed by an Audio Gateway */
   /* with a valid service level connection.  This function accepts as  */
   /* its input parameter the result code to send as part of the error  */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Extended_Error_Result(unsigned int HFREID, unsigned int ResultCode);

   /* The following function is responsible for sending subscriber      */
   /* number information.  This function may only be performed by an    */
   /* Audio Gateway that has received a request to query the subscriber */
   /* number information.  This function accepts as its input parameters*/
   /* the number of subscribers followed by a list of subscriber        */
   /* numbers.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Subscriber_Number_Information(unsigned int HFREID, unsigned int NumberListEntries, HFRM_Subscriber_Number_Information_t *SubscriberNumberList);

   /* The following function is responsible for sending information     */
   /* about the incoming call state.  This function may only be         */
   /* performed by an Audio Gateway that has a valid service level      */
   /* connection to a remote Hands Free device.  This function accepts  */
   /* as its input parameter the call state to set as part of this      */
   /* message.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
int _HFRM_Send_Incoming_Call_State(unsigned int HFREID, HFRE_Call_State_t CallState);

   /* The following function is responsible for sending a terminating   */
   /* response code from an Audio Gateway to a remote Hands Free device.*/
   /* This function may only be performed by an Audio Gateway that has a*/
   /* valid service level connection to a remote Hands Free device.     */
   /* This function can be called in any context where a normal Audio   */
   /* Gateway response function is called if the intention is to        */
   /* generate an error in response to the request.  It also must be    */
   /* called after certain requests that previously automatically       */
   /* generated an OK response.  In general, either this function or an */
   /* explicit response must be called after each request to the Audio  */
   /* Gateway.  This function accepts as its input parameters the type  */
   /* of result to return in the terminating response and, if the result*/
   /* type indicates an extended error code value, the error code.  This*/
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
int _HFRM_Send_Terminating_Response(unsigned int HFREID, HFRE_Extended_Result_t ResultType, unsigned int ResultValue);

   /* The following function is responsible for enabling the processing */
   /* of arbitrary commands from a remote Hands Free device.  Once this */
   /* function is called the hetHFRArbitraryCommandIndication event will*/
   /* be dispatched when an arbitrary command is received (i.e. a non   */
   /* Hands Free profile command).  If this function is not called, the */
   /* Audio Gateway will silently respond to any arbitrary commands with*/
   /* an error response ("ERROR").  If support is enabled, then the     */
   /* caller is responsible for responding TO ALL arbitrary command     */
   /* indications (hetHFRArbitraryCommandIndication).  If the arbitrary */
   /* command is not supported, then the caller should simply respond   */
   /* with:                                                             */
   /*                                                                   */
   /*   HFRM_Send_Terminating_Response()                                */
   /*                                                                   */
   /* specifying the erError response. This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * Once arbitrary command processing is enabled for an      */
   /*          Audio Gateway it cannot be disabled.                     */
   /* * NOTE * The default value is disabled (i.e. the                  */
   /*          hetHFRArbitraryCommandIndication will NEVER be dispatched*/
   /*          and the Audio Gateway will always respond with an error  */
   /*          response ("ERROR") when an arbitrary command is received.*/
   /* * NOTE * If support is enabled, the caller is guaranteed that a   */
   /*          hetHFRArbitraryCommandIndication will NOT be dispatched  */
   /*          before a service level indication is present. If an      */
   /*          arbitrary command is received, it will be responded with */
   /*          silently with an error response ("ERROR").               */
   /* * NOTE * This function is not applicable to Hands Free devices,   */
   /*          as Hands Free devices will always receive the            */
   /*          hetHFRArbitraryResponseIndication.  No action is required*/
   /*          and the event can simply be ignored.                     */
int _HFRM_Enable_Arbitrary_Command_Processing(unsigned int HFREID);

   /* The following function is responsible for sending an arbitrary    */
   /* response to the remote Hands Free device (i.e. non Bluetooth      */
   /* Hands Free Profile response) - either solicited or non-solicited. */
   /* This function may only be performed by an Audio Gateway with a    */
   /* valid service level connection. This function accepts as its      */
   /* input parameter a NULL terminated ASCII string that represents    */
   /* the arbitrary response to send. This function returns zero if     */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The Response string passed to this function *MUST* begin */
   /*          with a carriage return/line feed ("\r\n").               */
int _HFRM_Send_Arbitrary_Response(unsigned int HFREID, char *ArbitraryResponse);

   /* This function is responsible for setting up an audio connection   */
   /* between the local and remote device.  This function may be used by*/
   /* either an Audio Gateway or a Hands Free device for which a valid  */
   /* service level connection Exists.  This function accepts as its    */
   /* input parameter the connection type indicating which connection   */
   /* will process the command.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
int _HFRM_Setup_Audio_Connection(unsigned int HFREID);

   /* This function is responsible for releasing an audio connection    */
   /* which was previously established by the remote device or by a     */
   /* successful call to the HFRM_Setup_Audio_Connection() function.    */
   /* This function may be used by either an Audio Gateway or a Hands   */
   /* Free device.  This function returns zero if successful or a       */
   /* negative return error code if there was an error.                 */
int _HFRM_Release_Audio_Connection(unsigned int HFREID);

   /* The following function is provides a mechanism for sending SCO    */
   /* audio data to a remote device.  This function can only be called  */
   /* once an audio connection has been established.  This function     */
   /* accepts as input the Hands Free ID, followed by the length (in    */
   /* Bytes) of the audio data to send, and a pointer to the audio data */
   /* to send to the remote dntity.  This function returns zero if      */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * This function is only applicable for Bluetooth devices   */
   /*          that are configured to support packetized SCO audio.     */
   /*          This function will have no effect on Bluetooth devices   */
   /*          that are configured to process SCO audio via hardare     */
   /*          codec.                                                   */
   /* * NOTE * This is a low level function that exists for applications*/
   /*          that would like to process the audio data themselves (as */
   /*          opposed to having the hardware process the audio data    */
   /*          via a hardware codec.                                    */
   /* * NOTE * The data that is sent *MUST* be formatted in the correct */
   /*          SCO format that is expected by the device.               */
   /* * NOTE * This function assumes the specified data is being sent at*/
   /*          real time pacing, and the data is queued to be sent      */
   /*          immediately.                                             */
int _HFRM_Send_Audio_Data(unsigned int HFREID, unsigned int AudioDataLength, unsigned char *AudioData);

#endif
