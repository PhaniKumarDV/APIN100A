/*****< sapapi.h >*************************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SAPAPI -  Stonestreet One Bluetooth Stack SIM Access Profile API Type     */
/*            Definitions, Constants, and Prototypes.                         */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/06/03  R. Sledge      Initial creation.                               */
/******************************************************************************/
#ifndef __SAPAPIH__
#define __SAPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTSAP_ERROR_INVALID_PARAMETER                             (-1000)
#define BTSAP_ERROR_NOT_INITIALIZED                               (-1001)
#define BTSAP_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTSAP_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTSAP_ERROR_INVALID_OPERATION                             (-1005)

   /* SDP Service Classes for the SIM Access Profile.                   */

   /* The following MACRO is a utility MACRO that assigns the SIM Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (SIM_ACCESS_PROFILE_UUID_16) to the specified UUID_16_t variable. */
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the SIM_ACCESS_PROFILE_UUID_16 Constant value. */
#define SDP_ASSIGN_SIM_ACCESS_PROFILE_UUID_16(_x)                       ASSIGN_SDP_UUID_16((_x), 0x11, 0x2D)

   /* The following MACRO is a utility MACRO that assigns the SIM Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (SIM_ACCESS_PROFILE_UUID_32) to the specified UUID_32_t variable. */
   /* This MACRO accepts one parameter which is the UUID_32_t variable  */
   /* that is to receive the SIM_ACCESS_PROFILE_UUID_32 Constant value. */
#define SDP_ASSIGN_SIM_ACCESS_PROFILE_UUID_32(_x)                       ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x2D)

   /* The following MACRO is a utility MACRO that assigns the SIM Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (SIM_ACCESS_PROFILE_UUID_128) to the specified UUID_128_t         */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* SIM_ACCESS_PROFILE_UUID_128 Constant value.                       */
#define SDP_ASSIGN_SIM_ACCESS_PROFILE_UUID_128(_x)                      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x2D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* SIM Access Profile Servers.                                       */
#define SAP_PROFILE_VERSION                                             (0x0101)

   /* The following BIT definitions are used to denote the possible SIM */
   /* Access Profile Server Modes that can be applied to a SIM Access   */
   /* Profile Client Connection.  These BIT definitions are used with   */
   /* the SAP_Set_Server_Mode() and SAP_Get_Server_Mode() mode          */
   /* functions.                                                        */
#define SAP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION                     (0x00000000)
#define SAP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION                        (0x00000001)

   /* The following BIT MASK is used to mask the Server Mode Accept     */
   /* Connection settings from other (undefined) Server Mode bits.      */
#define SAP_SERVER_MODE_CONNECTION_MASK                                 (0x00000001)

   /* The following constants represent the Open Status Values that are */
   /* possible for the OpenStatus member of the Open Indication event.  */
#define SAP_OPEN_STATUS_SUCCESS                                         (0x00)
#define SAP_OPEN_STATUS_CONNECTION_TIMEOUT                              (0x01)
#define SAP_OPEN_STATUS_CONNECTION_REFUSED                              (0x02)
#define SAP_OPEN_STATUS_UNKNOWN_ERROR                                   (0x04)

   /* The following constants represent the Connection Status Values    */
   /* that are possible for the ConnectionStatus member of the Connect  */
   /* Response event.  These values may also be used with the           */
   /* ConnectionStatus parameter of the SAP_Connect_Response() function.*/
#define SAP_CONNECT_STATUS_SUCCESS                                      (0x00)
#define SAP_CONNECT_STATUS_ERROR_SERVER_UNABLE_TO_ESTABLISH_CONNECTION  (0x01)
#define SAP_CONNECT_STATUS_ERROR_UNSUPPORTED_MAXIMUM_MESSAGE_SIZE       (0x02)
#define SAP_CONNECT_STATUS_ERROR_MAXIMUM_MESSAGE_SIZE_TO_SMALL          (0x03)
#define SAP_CONNECT_STATUS_UNKNOWN                                      (0xFF)

   /* The following constants represent the Disconnection Type Values   */
   /* that are possible for the DisconnectionType member of the         */
   /* Disconnection Indication event.  These values may also be used    */
   /* with the DisconnectionType parameter of the                       */
   /* SAP_Disconnect_Indication() function.                             */
#define SAP_DISCONNECT_TYPE_GRACEFUL                                    (0x00)
#define SAP_DISCONNECT_TYPE_IMMEDIATE                                   (0x01)
#define SAP_DISCONNECT_TYPE_UNKNOWN                                     (0xFF)

   /* The following constants represent the Result Code Values that are */
   /* possible for the ResultCode member of the Transfer APDU Response, */
   /* Transfer ATR Response, Power SIM Off Response, Power SIM On       */
   /* Response, Reset SIM Response, and Transfer Card Reader Status     */
   /* Response events.  These values may also be used as the ResultCode */
   /* parameter of the *_Response() functions that have this parameter. */
   /* * NOTE *  Not all Result Codes are possible with some events.     */
   /* * NOTE *  Not all Result Codes are possible with some functions.  */
#define SAP_RESULT_CODE_SUCCESS                                         (0x00)
#define SAP_RESULT_CODE_ERROR_NO_REASON                                 (0x01)
#define SAP_RESULT_CODE_ERROR_CARD_NOT_ACCESSIBLE                       (0x02)
#define SAP_RESULT_CODE_ERROR_CARD_ALREADY_OFF                          (0x03)
#define SAP_RESULT_CODE_ERROR_CARD_REMOVED                              (0x04)
#define SAP_RESULT_CODE_ERROR_CARD_ALREADY_ON                           (0x05)
#define SAP_RESULT_CODE_ERROR_DATA_NOT_AVAILABLE                        (0x06)
#define SAP_RESULT_CODE_ERROR_NOT_SUPPORTED                             (0x07)
#define SAP_RESULT_CODE_UNKNOWN                                         (0xFF)

   /* The following constants represent the Card Reader Status Bit Mask */
   /* Values that maybe used with the CardReaderStatus member of the    */
   /* Transfer Card Reader Status Response event.  These values may also*/
   /* be used with the CardReaderStatus parameter of the                */
   /* SAP_Transfer_Card_Reader_Status_Response() function.              */
#define SAP_CARD_READER_STATUS_CARD_READER_IDENTITY_BIT_MASK            (0x07)
#define SAP_CARD_READER_STATUS_READER_REMOVABLE_BIT_MASK                (0x08)
#define SAP_CARD_READER_STATUS_READER_PRESENT_BIT_MASK                  (0x10)
#define SAP_CARD_READER_STATUS_READER_ID_1_SIZE_BIT_MASK                (0x20)
#define SAP_CARD_READER_STATUS_CARD_PRESENT_BIT_MASK                    (0x40)
#define SAP_CARD_READER_STATUS_CARD_POWERED_BIT_MASK                    (0x80)

   /* The following constants represent the Status Change Values that   */
   /* are possible for the StatusChange member of the Status Indication */
   /* event.  These values may also be used as the StatusChange         */
   /* parameter of the SAP_Status_Indication() function.                */
#define SAP_STATUS_CHANGE_UNKNOWN_ERROR                                 (0x00)
#define SAP_STATUS_CHANGE_CARD_RESET                                    (0x01)
#define SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE                           (0x02)
#define SAP_STATUS_CHANGE_CARD_REMOVED                                  (0x03)
#define SAP_STATUS_CHANGE_CARD_INSERTED                                 (0x04)
#define SAP_STATUS_CHANGE_CARD_RECOVERED                                (0x05)
#define SAP_STATUS_CHANGE_UNKNOWN                                       (0xFF)

   /* The following constants represent the Protocol Transport          */
   /* identifiers that are possible.                                    */
#define SAP_TRANSPORT_PROTOCOL_T0                                       (0x00)
#define SAP_TRANSPORT_PROTOCOL_T1                                       (0x01)

   /* SIM Access Profile Event API Types.                               */
typedef enum
{
   etSAP_Open_Indication,
   etSAP_Open_Confirmation,
   etSAP_Connect_Request,
   etSAP_Connect_Response,
   etSAP_Close_Indication,
   etSAP_Disconnect_Indication,
   etSAP_Disconnect_Request,
   etSAP_Disconnect_Response,
   etSAP_Transfer_APDU_Request,
   etSAP_Transfer_APDU_Response,
   etSAP_Transfer_ATR_Request,
   etSAP_Transfer_ATR_Response,
   etSAP_Power_SIM_Off_Request,
   etSAP_Power_SIM_Off_Response,
   etSAP_Power_SIM_On_Request,
   etSAP_Power_SIM_On_Response,
   etSAP_Reset_SIM_Request,
   etSAP_Reset_SIM_Response,
   etSAP_Status_Indication,
   etSAP_Transfer_Card_Reader_Status_Request,
   etSAP_Transfer_Card_Reader_Status_Response,
   etSAP_Error_Response,
   etSAP_Open_Request_Indication,
   etSAP_Transport_Protocol_Request,
   etSAP_Transport_Protocol_Response
} SAP_Event_Type_t;

   /* The following Event is dispatched when a Remote Client Requests a */
   /* Connection to a Local Server.  The SAPID member specifies the     */
   /* Identifier of the Connection to the Local Server.  The BD_ADDR    */
   /* member specifies the address of the Remote Client requesting the  */
   /* connection to the Local Server.                                   */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            SAP_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagSAP_Open_Request_Indication_Data_t
{
   unsigned int SAPID;
   BD_ADDR_t    BD_ADDR;
} SAP_Open_Request_Indication_Data_t;

#define SAP_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(SAP_Open_Request_Indication_Data_t))

   /* The following Event is dispatched when a SIM Access Client makes a*/
   /* Connection to a Registered SIM Access Server.  The SAPID member   */
   /* specifies the identifier of the Local SIM Access Server that is   */
   /* being connected with.  The BD_ADDR member specifies the address of*/
   /* the SIM Access Client that is being connected to the Local SIM    */
   /* Access Server.                                                    */
typedef struct _tagSAP_Open_Indication_Data_t
{
   unsigned int SAPID;
   BD_ADDR_t    BD_ADDR;
} SAP_Open_Indication_Data_t;

#define SAP_OPEN_INDICATION_DATA_SIZE                   (sizeof(SAP_Open_Indication_Data_t))

   /* The following Event is dispatched to the Local SIM Access Client  */
   /* to indicate the success or failure of a previously submitted      */
   /* Connection Attempt.  The SAPID member specifies the Identifier of */
   /* the Local SIM Access Client that has requested the Connection.    */
   /* The OpenStatus member specifies the status of the Connection      */
   /* Attempt.  Valid values are:                                       */
   /*    - SAP_OPEN_STATUS_SUCCESS                                      */
   /*    - SAP_OPEN_STATUS_CONNECTION_TIMEOUT                           */
   /*    - SAP_OPEN_STATUS_CONNECTION_REFUSED                           */
   /*    - SAP_OPEN_STATUS_UNKNOWN_ERROR                                */
typedef struct _tagSAP_Open_Confirmation_Data_t
{
   unsigned int SAPID;
   unsigned int OpenStatus;
} SAP_Open_Confirmation_Data_t;

#define SAP_OPEN_CONFIRMATION_DATA_SIZE                 (sizeof(SAP_Open_Confirmation_Data_t))

   /* The following event is dispatched when the remote SIM Access      */
   /* Server or Client disconnects from the local SIM Access Server or  */
   /* Client.  The SAPID member specifies the Identifier for the Local  */
   /* SIM Access Server or Client Connection being Disconnected.  This  */
   /* Event is NOT Dispatched in response to the Local SIM Access Server*/
   /* or Client requesting the disconnection.  This Event is dispatched */
   /* only when the remote device terminates the connection (and/or     */
   /* Bluetooth Link).                                                  */
typedef struct _tagSAP_Close_Indication_Data_t
{
   unsigned int SAPID;
} SAP_Close_Indication_Data_t;

#define SAP_CLOSE_INDICATION_DATA_SIZE                  (sizeof(SAP_Close_Indication_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a CONNECT_REQ message from the Remote SIM Access Client. */
   /* The SAPID member specifies the Identifier of the Local SIM Access */
   /* Server receiving this message.  The MaxMessageSize member         */
   /* specifies the Maximum Message Size requested by the Remote SIM    */
   /* Access Client for this connection.                                */
typedef struct _tagSAP_Connect_Request_Data_t
{
   unsigned int SAPID;
   Word_t       MaxMessageSize;
} SAP_Connect_Request_Data_t;

#define SAP_CONNECT_REQUEST_DATA_SIZE                   (sizeof(SAP_Connect_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a CONNECT_RESP message from the Remote SIM Access Server.*/
   /* The SAPID member specifies the Identifier of the Local SIM Access */
   /* Server receiving this message.  The ConnectionStatus member       */
   /* specifies the status of the pending connection.  The              */
   /* MaxMessageSize member specifies the Remote SIM Access Servers     */
   /* requested or accepted Maximum Message Size.                       */
typedef struct _tagSAP_Connect_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ConnectionStatus;
   Word_t       MaxMessageSize;
} SAP_Connect_Response_Data_t;

#define SAP_CONNECT_RESPONSE_DATA_SIZE                  (sizeof(SAP_Connect_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a DISCONNECT_IND message from the Remote SIM Access      */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The DisconnectionType  */
   /* member specifies how the Remote SIM Access Server wishes to       */
   /* disconnect the SIM Access Profile Connection.                     */
typedef struct _tagSAP_Disconnect_Indication_Data_t
{
   unsigned int SAPID;
   Byte_t       DisconnectionType;
} SAP_Disconnect_Indication_Data_t;

#define SAP_DISCONNECT_INDICATION_DATA_SIZE             (sizeof(SAP_Disconnect_Indication_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a DISCONNECT_REQ message from the Remote SIM Access      */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.                         */
typedef struct _tagSAP_Disconnect_Request_Data_t
{
   unsigned int SAPID;
} SAP_Disconnect_Request_Data_t;

#define SAP_DISCONNECT_REQUEST_DATA_SIZE                (sizeof(SAP_Disconnect_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a DISCONNECT_RESP message from the Remote SIM Access     */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.                         */
typedef struct _tagSAP_Disconnect_Response_Data_t
{
   unsigned int SAPID;
} SAP_Disconnect_Response_Data_t;

#define SAP_DISCONNECT_RESPONSE_DATA_SIZE               (sizeof(SAP_Disconnect_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a TRANSFER_APDU_REQ message from the Remote SIM Access   */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.  The CommandAPDULength  */
   /* member specifies the Length (in bytes) of the buffer pointed to by*/
   /* the CommandAPDU member.  The CommandAPDU member is the actual     */
   /* Command APDU that is associated with this message.                */
typedef struct _tagSAP_Transfer_APDU_Request_Data_t
{
   unsigned int  SAPID;
   Word_t        CommandAPDULength;
   Byte_t       *CommandAPDU;
} SAP_Transfer_APDU_Request_Data_t;

#define SAP_TRANSFER_APDU_REQUEST_DATA_SIZE             (sizeof(SAP_Transfer_APDU_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a TRANSFER_APDU_RESP message from the Remote SIM Access  */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ResultCode member  */
   /* specifies the status of the original request.  The                */
   /* ResponseAPDULength member specifies the Length (in bytes) of the  */
   /* buffer pointed to by the ResponseAPDU member.  The ResponseAPDU   */
   /* member is the actual Response APDU that is associated with this   */
   /* message.  Note that if the Result Code that is supplied is any    */
   /* value other than Success, the Response APDU members should be     */
   /* ignored.                                                          */
typedef struct _tagSAP_Transfer_APDU_Response_Data_t
{
   unsigned int  SAPID;
   Byte_t        ResultCode;
   Word_t        ResponseAPDULength;
   Byte_t       *ResponseAPDU;
} SAP_Transfer_APDU_Response_Data_t;

#define SAP_TRANSFER_APDU_RESPONSE_DATA_SIZE            (sizeof(SAP_Transfer_APDU_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a TRANSFER_ATR_REQ message from the Remote SIM Access    */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.                         */
typedef struct _tagSAP_Transfer_ATR_Request_Data_t
{
   unsigned int SAPID;
} SAP_Transfer_ATR_Request_Data_t;

#define SAP_TRANSFER_ATR_REQUEST_DATA_SIZE              (sizeof(SAP_Transfer_ATR_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a TRANSFER_ATR_RESP message from the Remote SIM Access   */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ResultCode member  */
   /* specifies the status of the original request.  The ATR Length     */
   /* member specifies the Length (in bytes) of buffer pointed to by the*/
   /* ATR member.  The ATR member is the actual ATR that is associated  */
   /* with this message.  Note that if the Result Code that is supplied */
   /* is any value other than Success, the ATR members should be        */
   /* ignored.                                                          */
typedef struct _tagSAP_Transfer_ATR_Response_Data_t
{
   unsigned int  SAPID;
   Byte_t        ResultCode;
   Word_t        ATRLength;
   Byte_t       *ATR;
} SAP_Transfer_ATR_Response_Data_t;

#define SAP_TRANSFER_ATR_RESPONSE_DATA_SIZE             (sizeof(SAP_Transfer_ATR_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a POWER_SIM_OFF_REQ message from the Remote SIM Access   */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.                         */
typedef struct _tagSAP_Power_SIM_Off_Request_Data_t
{
   unsigned int SAPID;
} SAP_Power_SIM_Off_Request_Data_t;

#define SAP_POWER_SIM_OFF_REQUEST_DATA_SIZE             (sizeof(SAP_Power_SIM_Off_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a POWER_SIM_OFF_RESP message from the Remote SIM Access  */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ResultCode member  */
   /* specifies the status of the original request.                     */
typedef struct _tagSAP_Power_SIM_Off_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ResultCode;
} SAP_Power_SIM_Off_Response_Data_t;

#define SAP_POWER_SIM_OFF_RESPONSE_DATA_SIZE            (sizeof(SAP_Power_SIM_Off_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a POWER_SIM_ON_REQ message from the Remote SIM Access    */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.                         */
typedef struct _tagSAP_Power_SIM_On_Request_Data_t
{
   unsigned int SAPID;
} SAP_Power_SIM_On_Request_Data_t;

#define SAP_POWER_SIM_ON_REQUEST_DATA_SIZE              (sizeof(SAP_Power_SIM_On_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a POWER_SIM_ON_RESP message from the Remote SIM Access   */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ResultCode member  */
   /* specifies the status of the original request.                     */
typedef struct _tagSAP_Power_SIM_On_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ResultCode;
} SAP_Power_SIM_On_Response_Data_t;

#define SAP_POWER_SIM_ON_RESPONSE_DATA_SIZE             (sizeof(SAP_Power_SIM_On_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a RESET_SIM_REQ message from the Remote SIM Access       */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.                         */
typedef struct _tagSAP_Reset_SIM_Request_Data_t
{
   unsigned int SAPID;
} SAP_Reset_SIM_Request_Data_t;

#define SAP_RESET_SIM_REQUEST_DATA_SIZE                 (sizeof(SAP_Reset_SIM_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a RESET_SIM_RESP message from the Remote SIM Access      */
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ResultCode member  */
   /* specifies the status of the original request.                     */
typedef struct _tagSAP_Reset_SIM_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ResultCode;
} SAP_Reset_SIM_Response_Data_t;

#define SAP_RESET_SIM_RESPONSE_DATA_SIZE                (sizeof(SAP_Reset_SIM_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a STATUS_IND message from the Remote SIM Access Server.  */
   /* The SAPID member specifies the Identifier of the Local SIM Access */
   /* Client receiving this message.  The StatusChange member specifies */
   /* the Current Status of the Remote SIM Access Server.               */
typedef struct _tagSAP_Status_Indication_Data_t
{
   unsigned int SAPID;
   Byte_t       StatusChange;
} SAP_Status_Indication_Data_t;

#define SAP_STATUS_INDICATION_DATA_SIZE                 (sizeof(SAP_Status_Indication_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a TRANSFER_CARD_READER_STATUS_REQ message from the Remote*/
   /* SIM Access Client.  The SAPID member specifies the Identifier of  */
   /* the Local SIM Access Server receiving this message.               */
typedef struct _tagSAP_Transfer_Card_Reader_Status_Request_Data_t
{
   unsigned int SAPID;
} SAP_Transfer_Card_Reader_Status_Request_Data_t;

#define SAP_TRANSFER_CARD_READER_STATUS_REQUEST_DATA_SIZE (sizeof(SAP_Transfer_Card_Reader_Status_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a TRANSFER_CARD_READER_STATUS_RESP message from the      */
   /* Remote SIM Access Server.  The SAPID member specifies the         */
   /* Identifier of the Local SIM Access Client receiving this message. */
   /* The ResultCode member specifies the status of the original        */
   /* request.  The CardReaderStatus member specifies the Status of the */
   /* Card Reader.  Note that if the Result Code that is supplied is any*/
   /* value other than Success, the Card Reader Status member should be */
   /* ignored.                                                          */
typedef struct _tagSAP_Transfer_Card_Reader_Status_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ResultCode;
   Byte_t       CardReaderStatus;
} SAP_Transfer_Card_Reader_Status_Response_Data_t;

#define SAP_TRANSFER_CARD_READER_STATUS_RESPONSE_DATA_SIZE (sizeof(SAP_Transfer_Card_Reader_Status_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives an ERROR_RESP message from the Remote SIM Access Server. */
   /* The SAPID member specifies the Identifier of the Local SIM Access */
   /* Client receiving this message.                                    */
typedef struct _tagSAP_Error_Response_Data_t
{
   unsigned int SAPID;
} SAP_Error_Response_Data_t;

#define SAP_ERROR_RESPONSE_DATA_SIZE                    (sizeof(SAP_Error_Response_Data_t))

   /* The following event is dispatched when the Local SIM Access Server*/
   /* receives a SET_TRANSPORT_PROTOCOL_REQ message from the Remote SAP */
   /* Client.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Server receiving this message.  The ProtocolID denotes */
   /* the desired protocol (either SAP_TRANSPORT_PROTOCOL_T0 or         */
   /* SAP_TRANSPORT_PROTOCOL_T1.)                                       */
typedef struct _tagSAP_Transport_Protocol_Request_Data_t
{
   unsigned int SAPID;
   Byte_t       ProtocolID;
} SAP_Transport_Protocol_Request_Data_t;

#define SAP_TRANSPORT_PROTOCOL_REQUEST_DATA_SIZE              (sizeof(SAP_Transport_Protocol_Request_Data_t))

   /* The following event is dispatched when the Local SIM Access Client*/
   /* receives a SET_TRANSPORT_PROTOCOL_RESP message from the Remote SAP*/
   /* Server.  The SAPID member specifies the Identifier of the Local   */
   /* SIM Access Client receiving this message.  The ProtocolID denotes */
   /* the desired protocol (either SAP_TRANSPORT_PROTOCOL_T0 or         */
   /* SAP_TRANSPORT_PROTOCOL_T1.)                                       */
typedef struct _tagSAP_Transport_Protocol_Response_Data_t
{
   unsigned int SAPID;
   Byte_t       ResultCode;
} SAP_Transport_Protocol_Response_Data_t;

#define SAP_TRANSPORT_PROTOCOL_RESPONSE_DATA_SIZE              (sizeof(SAP_Transport_Protocol_Response_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all SIM Event Data.                                       */
typedef struct _tagSAP_Event_Data_t
{
   SAP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      SAP_Open_Request_Indication_Data_t              *SAP_Open_Request_Indication_Data;
      SAP_Open_Indication_Data_t                      *SAP_Open_Indication_Data;
      SAP_Open_Confirmation_Data_t                    *SAP_Open_Confirmation_Data;
      SAP_Close_Indication_Data_t                     *SAP_Close_Indication_Data;
      SAP_Connect_Request_Data_t                      *SAP_Connect_Request_Data;
      SAP_Connect_Response_Data_t                     *SAP_Connect_Response_Data;
      SAP_Disconnect_Indication_Data_t                *SAP_Disconnect_Indication_Data;
      SAP_Disconnect_Request_Data_t                   *SAP_Disconnect_Request_Data;
      SAP_Disconnect_Response_Data_t                  *SAP_Disconnect_Response_Data;
      SAP_Transfer_APDU_Request_Data_t                *SAP_Transfer_APDU_Request_Data;
      SAP_Transfer_APDU_Response_Data_t               *SAP_Transfer_APDU_Response_Data;
      SAP_Transfer_ATR_Request_Data_t                 *SAP_Transfer_ATR_Request_Data;
      SAP_Transfer_ATR_Response_Data_t                *SAP_Transfer_ATR_Response_Data;
      SAP_Power_SIM_Off_Request_Data_t                *SAP_Power_SIM_Off_Request_Data;
      SAP_Power_SIM_Off_Response_Data_t               *SAP_Power_SIM_Off_Response_Data;
      SAP_Power_SIM_On_Request_Data_t                 *SAP_Power_SIM_On_Request_Data;
      SAP_Power_SIM_On_Response_Data_t                *SAP_Power_SIM_On_Response_Data;
      SAP_Reset_SIM_Request_Data_t                    *SAP_Reset_SIM_Request_Data;
      SAP_Reset_SIM_Response_Data_t                   *SAP_Reset_SIM_Response_Data;
      SAP_Status_Indication_Data_t                    *SAP_Status_Indication_Data;
      SAP_Transfer_Card_Reader_Status_Request_Data_t  *SAP_Transfer_Card_Reader_Status_Request_Data;
      SAP_Transfer_Card_Reader_Status_Response_Data_t *SAP_Transfer_Card_Reader_Status_Response_Data;
      SAP_Error_Response_Data_t                       *SAP_Error_Response_Data;
      SAP_Transport_Protocol_Request_Data_t           *SAP_Transport_Protocol_Request_Data;
      SAP_Transport_Protocol_Response_Data_t          *SAP_Transport_Protocol_Response_Data;
   } Event_Data;
} SAP_Event_Data_t;

#define SAP_EVENT_DATA_SIZE                             (sizeof(SAP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a SAP Event Callback.  This function will be called whenever a    */
   /* SIM Access Profile Event occurs that is associated with the       */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the SAP Event Data that occurred, and the */
   /* SAP Event Callback Parameter that was specified when this Callback*/
   /* was installed.  The caller is free to use the contents of the SAP */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another SAP Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving SAP Events.         */
   /*            A Deadlock WILL occur because NO SAP Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *SAP_Event_Callback_t)(unsigned int BluetoothStackID, SAP_Event_Data_t *SAP_Event_Data, unsigned long CallbackParameter);

   /* The following function is provided to allow a mechanism to Open a */
   /* SIM Access Server on the specified Bluetooth SPP Serial Port.     */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that this Server Port is to be           */
   /* established.  The second parameter is the value of the Server Port*/
   /* to establish.  This number *MUST* be between                      */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.              */
   /* The final two parameters to this function are the SIM Event       */
   /* Callback function and the Callback Parameter that is passed to the*/
   /* Event Callback when SAP Events occur.  This function returns a    */
   /* non-zero, positive, number on success or a negative return value  */
   /* if there was an error.  A successful return value will be a SAP ID*/
   /* that can used to reference the Opened SIM Access Server in ALL    */
   /* other SIM Access Server functions in this module.  Once a SIM     */
   /* Access Server is opened, it can only be Un-Registered via a call  */
   /* to SAP_Close_Server() function (passing the return value from this*/
   /* function as the SAP ID).                                          */
BTPSAPI_DECLARATION int BTPSAPI SAP_Open_Server(unsigned int BluetoothStackID, unsigned int ServerPort, SAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Open_Server_t)(unsigned int BluetoothStackID, unsigned int ServerPort, SAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is provided as a mechanism to Un-Register a*/
   /* SIM Access Server (which was registered by a successful call to   */
   /* the SAP_Open_Server() function).  This function accepts as input  */
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* SAP Server ID is valid with (specified by the second parameter).  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.  Note that this function does NOT    */
   /* delete any SDP Service Record Handles.                            */
BTPSAPI_DECLARATION int BTPSAPI SAP_Close_Server(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is responsible for responding to requests  */
   /* to connect to a Server.  This function accepts as input the       */
   /* Bluetooth Stack ID of the Local Bluetooth Protocol Stack, the SAP */
   /* ID (which *MUST* have been obtained by calling the                */
   /* SAP_Open_Server() function), and as the final parameter whether to*/
   /* accept the pending connection request.  This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
BTPSAPI_DECLARATION int BTPSAPI SAP_Open_Request_Response(unsigned int BluetoothStackID, unsigned int SAPID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a mechanism to add a  */
   /* Generic SIM Access Server Service Record to the SDP Database.     */
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the SAP Server ID is valid with     */
   /* (specified by the second parameter).  The third parameter         */
   /* specifies the Service Name to associate with the SDP Record.  The */
   /* final parameter is a pointer to a DWord_t which receives the SDP  */
   /* Service Record Handle if this function successfully creates an SDP*/
   /* Service Record.  If this function returns zero, then the          */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the SAP ID that */
   /*          was returned from the SAP_Open_Server() function.  This  */
   /*          function should NEVER be used with a SAP ID returned from*/
   /*          the SAP_Open_Remote_Server() function.                   */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          SAP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI SAP_Register_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int SAPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Register_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int SAPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the SIM*/
   /* Access Server SDP Service Record (specified by the third          */
   /* parameter) from the SDP Database.  This MACRO simply maps to the  */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on, the SIM Server ID (returned from a      */
   /* successful call to the SAP_Open_Server() function), and the SDP   */
   /* Service Record Handle.  The SDP Service Record Handle was returned*/
   /* via a successful call to the SAP_Register_Server_SDP_Record()     */
   /* function.  See the SAP_Register_Server_SDP_Record() function for  */
   /* more information.  This MACRO returns the result of the           */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define SAP_Un_Register_SDP_Record(__BluetoothStackID, __SAPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is provided to allow a mechanism to Open a */
   /* connection to a Remote SIM Access Server on the specified Server  */
   /* Port.  This function accepts as input the Bluetooth Stack ID of   */
   /* the Bluetooth Protocol Stack that the SIM Access Client will be   */
   /* associated with.  The second parameter specifies the BD_ADDR of   */
   /* the Remote SIM Access Server to connect with.  The third parameter*/
   /* specifies the Remote Server Port to connect with.  This number    */
   /* *MUST* be between SPP_PORT_NUMBER_MINIMUM and                     */
   /* SPP_PORT_NUMBER_MAXIMUM.  The final two parameters to this        */
   /* function are the SAP Event Callback function and the Callback     */
   /* Parameter that is passed to the Event Callback when SAP Events    */
   /* occur.  This function returns a non-zero, positive, number on     */
   /* success or a negative return value if there was an error.  A      */
   /* successful return value will be a SAP ID that can used to         */
   /* reference the Opened SIM Access Client in ALL other SIM Access    */
   /* Client functions in this module.  Once a connection to a Remote   */
   /* SIM Access Server is opened, it can only be closed via a call to  */
   /* the SAP_Close_Connection() function (passing the return value from*/
   /* this function as the SAP ID).                                     */
BTPSAPI_DECLARATION int BTPSAPI SAP_Open_Remote_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, SAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Open_Remote_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, SAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* terminate a possible connection to a Remote SIM Access Server or  */
   /* Client.  If this function is called by a Server, the Connection to*/
   /* the Client will be terminated, but the Server will remain         */
   /* registered.  This function accepts as input the Bluetooth Protocol*/
   /* Stack ID of the Bluetooth Protocol Stack which handles the Server */
   /* or Client and the SAP ID that was returned from the               */
   /* SAP_Open_Server() or the SAP_Open_Remote_Server() function.  This */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.  This function does NOT Un-Register a Server  */
   /* Port from the system it ONLY disconnects the connection that is   */
   /* currently active on the Server Port.  The SAP_Close_Server()      */
   /* function can be used to Un-Register the Server.                   */
BTPSAPI_DECLARATION int BTPSAPI SAP_Close_Connection(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to send a Connect Request message to  */
   /* the Remote SIM Access Server.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Client ID is valid with (specified by the second parameter).  The */
   /* final parameter specifies the requested Maximum Message Size to   */
   /* negotiate for this connection.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI SAP_Connect_Request(unsigned int BluetoothStackID, unsigned int SAPID, Word_t MaxMessageSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Connect_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID, Word_t MaxMessageSize);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to send a Connect Response message to */
   /* the Remote SIM Access Client.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Server ID is valid with (specified by the second parameter).  The */
   /* third parameter specifies the Connection Status associated with   */
   /* this response.  Valid values are:                                 */
   /*    -SAP_CONNECT_STATUS_SUCCESS                                    */
   /*    -SAP_CONNECT_STATUS_ERROR_SERVER_UNABLE_TO_ESTABLISH_CONNECTION*/
   /*    -SAP_CONNECT_STATUS_ERROR_UNSUPPORTED_MAXIMUM_MESSAGE_SIZE     */
   /*    -SAP_CONNECT_STATUS_ERROR_MAXIMUM_MESSAGE_SIZE_TO_SMALL        */
   /* The final parameter specifies a proposed value for the Maximum    */
   /* Message Size to negotiate for this connection.  Note that if the  */
   /* Connection Status is Success, the final parameter to this function*/
   /* is ignored.  This function returns zero if successful, or a       */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Connect_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ConnectionStatus, Word_t MaxMessageSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Connect_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ConnectionStatus, Word_t MaxMessageSize);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to request or inform the Remote SIM   */
   /* Access Client of the release of the SIM Access Profile connection.*/
   /* This function accepts as input the Bluetooth Stack ID of the      */
   /* Bluetooth Protocol Stack that the SAP Server ID is valid with     */
   /* (specified by the second parameter).  The final parameter to this */
   /* function specifies the Disconnection Type to be associated with   */
   /* this message.  Valid values are:                                  */
   /*    - SAP_DISCONNECT_TYPE_GRACEFUL                                 */
   /*    - SAP_DISCONNECT_TYPE_IMMEDIATE                                */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Disconnect_Indication(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t DisconnectionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Disconnect_Indication_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t DisconnectionType);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to send a Disconnect Request message  */
   /* to the Remote SIM Access Server.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* SAP Client ID is valid with (specified by the second parameter).  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Disconnect_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Disconnect_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to send a Disconnect Response message */
   /* to the Remote SIM Access Client.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* SAP Client ID is valid with (specified by the second parameter).  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Disconnect_Response(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Disconnect_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to Transfer a Command APDU to the     */
   /* Remote SIM Access Server.  This function accepts as input the     */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Client ID is valid with (specified by the second parameter).  The */
   /* final two parameters to this function are the Length of the buffer*/
   /* containing the Command APDU to send and a pointer to the buffer   */
   /* containing the Command APDU that will be sent.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_APDU_Request(unsigned int BluetoothStackID, unsigned int SAPID, Word_t CommandAPDULength, Byte_t *CommandAPDU);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_APDU_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID, Word_t CommandAPDULength, Byte_t *CommandAPDU);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to the Remote SIM Access   */
   /* Clients Command APDU with a Response APDU.  This function accepts */
   /* as input the Bluetooth Stack ID of the Bluetooth Protocol Stack   */
   /* that the SAP Server ID is valid with (specified by the second     */
   /* parameter).  The third parameter specifies the Result Code        */
   /* associated with this response.  Valid values are:                 */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_CARD_NOT_ACCESSIBLE                    */
   /*    - SAP_RESULT_CODE_ERROR_CARD_ALREADY_OFF                       */
   /*    - SAP_RESULT_CODE_ERROR_CARD_REMOVED                           */
   /* The final two parameters to this function are the Length of the   */
   /* buffer containing the Response APDU to send and a pointer to the  */
   /* buffer containing the Response APDU that will be sent.  Note that */
   /* if the Result Code that is supplied is any value other then       */
   /* Success, the final parameters to this function are ignored.  This */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_APDU_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Word_t ResponseAPDULength, Byte_t *ResponseAPDU);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_APDU_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Word_t ResponseAPDULength, Byte_t *ResponseAPDU);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to Transfer the ATR from the SIM.  This function accepts as*/
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the SAP Client ID is valid with (specified by the second          */
   /* parameter).  This function returns zero if successful, or a       */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_ATR_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_ATR_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a Request to Transfer   */
   /* the ATR from the SIM to the Remote SIM Access Client.  This       */
   /* function accepts as input the Bluetooth Stack ID of the Bluetooth */
   /* Protocol Stack that the SAP Server ID is valid with (specified by */
   /* the second parameter).  The third parameter specifies the Result  */
   /* Code associated with this response.  Valid values are:            */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_CARD_ALREADY_OFF                       */
   /*    - SAP_RESULT_CODE_ERROR_CARD_REMOVED                           */
   /*    - SAP_RESULT_CODE_ERROR_DATA_NOT_AVAILABLE                     */
   /* The final two parameters to this function are the Length of the   */
   /* buffer containing the Answer To Reset (ATR) to send and a pointer */
   /* to the buffer containing the Answer To Reset (ATR) that will be   */
   /* sent.  Note that if the Result Code that is supplied is any value */
   /* other than Success, the final parameters to this function are     */
   /* ignored.  This function returns zero if successful, or a negative */
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_ATR_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Word_t ATRLength, Byte_t *ATR);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_ATR_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Word_t ATRLength, Byte_t *ATR);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to Power the SIM Off.  This function accepts as input the  */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Client ID is valid with (specified by the second parameter).  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI SAP_Power_SIM_Off_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Power_SIM_Off_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a Power SIM Off Request */
   /* from the Remote SIM Access Client.  This function accepts as input*/
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* SAP Server ID is valid with (specified by the second parameter).  */
   /* The final parameter specifies the Result Code associated with this*/
   /* response.  Valid values are:                                      */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_CARD_ALREADY_OFF                       */
   /*    - SAP_RESULT_CODE_ERROR_CARD_REMOVED                           */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Power_SIM_Off_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Power_SIM_Off_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to Power the SIM On.  This function accepts as input the   */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Client ID is valid with (specified by the second parameter).  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI SAP_Power_SIM_On_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Power_SIM_On_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a Power SIM On Request  */
   /* from the Remote SIM Access Client.  This function accepts as input*/
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* SAP Server ID is valid with (specified by the second parameter).  */
   /* The final parameter specifies the Result Code associated with this*/
   /* response. Valid values are:                                       */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_CARD_NOT_ACCESSIBLE                    */
   /*    - SAP_RESULT_CODE_ERROR_CARD_REMOVED                           */
   /*    - SAP_RESULT_CODE_ERROR_CARD_ALREADY_ON                        */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Power_SIM_On_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Power_SIM_On_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to Reset the SIM.  This function accepts as input the      */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Client ID is valid with (specified by the second parameter).  This*/
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI SAP_Reset_SIM_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Reset_SIM_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a Reset SIM Request from*/
   /* the Remote SIM Access Client.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the SAP   */
   /* Server ID is valid with (specified by the second parameter).  The */
   /* final parameter specifies the Result Code associated with this    */
   /* response. Valid values are:                                       */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_CARD_NOT_ACCESSIBLE                    */
   /*    - SAP_RESULT_CODE_ERROR_CARD_ALREADY_OFF                       */
   /*    - SAP_RESULT_CODE_ERROR_CARD_REMOVED                           */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Reset_SIM_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Reset_SIM_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to inform the Remote SIM Access Client*/
   /* of the Status or Status Changes between the SIM Access Server and */
   /* the SIM.  This function accepts as input the Bluetooth Stack ID of*/
   /* the Bluetooth Protocol Stack that the SAP Server ID is valid with */
   /* (specified by the second parameter).  The final parameter         */
   /* specifies the Status Change associated with this message.  Valid  */
   /* values during connection setup are:                               */
   /*    - SAP_STATUS_CHANGE_UNKNOWN_ERROR                              */
   /*    - SAP_STATUS_CHANGE_CARD_RESET                                 */
   /*    - SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE                        */
   /*    - SAP_STATUS_CHANGE_CARD_REMOVED                               */
   /* Valid values during an ongoing connection are:                    */
   /*    - SAP_STATUS_CHANGE_UNKNOWN_ERROR                              */
   /*    - SAP_STATUS_CHANGE_CARD_REMOVED                               */
   /*    - SAP_STATUS_CHANGE_CARD_INSERTED                              */
   /*    - SAP_STATUS_CHANGE_CARD_NOT_ACCESSIBLE                        */
   /*    - SAP_STATUS_CHANGE_CARD_RECOVERED                             */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Status_Indication(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t StatusChange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Status_Indication_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t StatusChange);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to Transfer the Card Reader Status.  This function accepts */
   /* as input the Bluetooth Stack ID of the Bluetooth Protocol Stack   */
   /* that the SAP Client ID is valid with (specified by the second     */
   /* parameter).  This function returns zero if successful, or a       */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_Card_Reader_Status_Request(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_Card_Reader_Status_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a request to Transfer   */
   /* the Card Reader Status to the Remote SIM Access Client.  This     */
   /* function accepts as input the Bluetooth Stack ID of the Bluetooth */
   /* Protocol Stack that the SAP Server ID is valid with (specified by */
   /* the second parameter).  The third parameter specifies the Result  */
   /* Code associated with this response.  Valid values are:            */
   /*    - SAP_RESULT_CODE_SUCCESS                                      */
   /*    - SAP_RESULT_CODE_ERROR_NO_REASON                              */
   /*    - SAP_RESULT_CODE_ERROR_DATA_NOT_AVAILABLE                     */
   /* The final parameter to this function is the Card Reader Status.   */
   /* Note that if the Result Code that is supplied is any value other  */
   /* than Success, the final parameter to this function is ignored.    */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transfer_Card_Reader_Status_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Byte_t CardReaderStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transfer_Card_Reader_Status_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode, Byte_t CardReaderStatus);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a improperly formated   */
   /* request message from the Remote SIM Access Client.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack that the SAP Server ID is valid with (specified by the      */
   /* second parameter).  This function returns zero if successful, or a*/
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Error_Response(unsigned int BluetoothStackID, unsigned int SAPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Error_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Client to request the Remote SIM Access      */
   /* Server to change transport protocol.  This function accepts       */
   /* as input the Bluetooth Stack ID of the Bluetooth Protocol Stack   */
   /* that the SAP Client ID is valid with (specified by the second     */
   /* parameter), and a ProtocolID corresponding to protocol T=0 or T=1.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transport_Protocol_Request(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ProtocolID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transport_Protocol_Request_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ProtocolID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* specified SIM Access Server to respond to a request to change the */
   /* transport protocol from the remote SIM Access Client.  This       */
   /* function accepts the Bluetooth Stack ID of the stack corresponding*/
   /* to the SAP Server, and a ResultCode value indicating the response */
   /* to send to the client.  This function returns zero if successful, */
   /* or a negative return value if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI SAP_Transport_Protocol_Response(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Transport_Protocol_Response_t)(unsigned int BluetoothStackID, unsigned int SAPID, Byte_t ResultCode);
#endif

   /* The following function is responsible for retrieving the current  */
   /* SAP Server Server Mode for a specified SAP server.  This function */
   /* accepts as its first parameter the Bluetooth Stack ID of the      */
   /* Bluetooth Stack on which the server exists.  The second parameter */
   /* is the SAP ID that specifies the individual Server (this *MUST* be*/
   /* a value that was obtained by calling the SAP_Open_Server()        */
   /* function).  The final parameter to this function is a pointer to a*/
   /* Server Mode variable which will receive the current Server Mode.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if an error occurred.                                  */
   /* ** NOTE ** The Default Server Mode is                             */
   /*               SAP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION.        */
   /* ** NOTE ** This function is used for SAP Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI SAP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int SAPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int SAPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for setting the SAP Server  */
   /* Mode for the specifed SAP server.  This function accepts as its   */
   /* first parameter the Bluetooth Stack ID of the Bluetooth Stack on  */
   /* which the server exists.  The second parameter is the SAP ID that */
   /* specifies the individual Server (this *MUST* be a value that was  */
   /* obtained by calling the SAP_Open_Server() function).  The final   */
   /* parameter to this function is the new Server Mode to set the      */
   /* Server to use.  This function returns zero if successful, or a    */
   /* negative return error code if an error occurred.                  */
   /* ** NOTE ** The Default Server Mode is                             */
   /*               SAP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION.        */
   /* ** NOTE ** This function is used for SAP Servers which use        */
   /*            Bluetooth Security Mode 2.                             */
BTPSAPI_DECLARATION int BTPSAPI SAP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int SAPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SAP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int SAPID, unsigned long ServerModeMask);
#endif

#endif
