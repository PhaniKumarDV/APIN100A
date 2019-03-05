/*****< sppmmsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPMMSG - Defined Interprocess Communication Messages for the Serial      */
/*            Port Profile (SPP) Manager for Stonestreet One Bluetopia        */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/13/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SPPMMSGH__
#define __SPPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "SPPMType.h"            /* BTPM Serial Port Manager Type Definitions.*/

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Serial Port     */
   /* Profile (SPP) Manager.                                            */
#define BTPM_MESSAGE_GROUP_SERIAL_PORT_PROFILE_MANAGER         0x00000110

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Serial Port      */
   /* Profile (SPP) Manager.                                            */

   /* Serial Port Profile (SPP) Manager Commands.                       */
#define SPPM_MESSAGE_FUNCTION_REGISTER_SERVER_PORT             0x00001001
#define SPPM_MESSAGE_FUNCTION_OPEN_SERVER_REQUEST_RESPONSE     0x00001002
#define SPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_PORT          0x00001003
#define SPPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD          0x00001004
#define SPPM_MESSAGE_FUNCTION_OPEN_REMOTE_PORT                 0x00001005
#define SPPM_MESSAGE_FUNCTION_CLOSE_PORT                       0x00001006
#define SPPM_MESSAGE_FUNCTION_READ_DATA                        0x00001007
#define SPPM_MESSAGE_FUNCTION_WRITE_DATA                       0x00001008
#define SPPM_MESSAGE_FUNCTION_SEND_LINE_STATUS                 0x00001009
#define SPPM_MESSAGE_FUNCTION_SEND_PORT_STATUS                 0x0000100A
#define SPPM_MESSAGE_FUNCTION_QUERY_SERVER_PRESENT             0x0000100B
#define SPPM_MESSAGE_FUNCTION_FIND_FREE_SERVER_PORT            0x0000100C
#define SPPM_MESSAGE_FUNCTION_CHANGE_BUFFER_SIZE               0x0000100D

#define SPPM_MESSAGE_FUNCTION_CONFIGURE_MFI_SETTINGS           0x00001101
#define SPPM_MESSAGE_FUNCTION_QUERY_CONNECTION_TYPE            0x00001102
#define SPPM_MESSAGE_FUNCTION_OPEN_SESSION_REQUEST_RESPONSE    0x00001103
#define SPPM_MESSAGE_FUNCTION_SEND_SESSION_DATA                0x00001104
#define SPPM_MESSAGE_FUNCTION_SEND_NON_SESSION_DATA            0x00001105
#define SPPM_MESSAGE_FUNCTION_CANCEL_PACKET                    0x00001106
#define SPPM_MESSAGE_FUNCTION_SEND_CONTROL_MESSAGE             0x00001107

   /* Serial Port Profile (SPP) Manager Asynchronous Events.            */
#define SPPM_MESSAGE_FUNCTION_SERVER_PORT_OPEN_REQUEST         0x00010001
#define SPPM_MESSAGE_FUNCTION_SERVER_PORT_OPENED               0x00010002
#define SPPM_MESSAGE_FUNCTION_PORT_CLOSED                      0x00010003
#define SPPM_MESSAGE_FUNCTION_OPEN_REMOTE_PORT_RESULT          0x00010004
#define SPPM_MESSAGE_FUNCTION_LINE_STATUS_CHANGED              0x00010005
#define SPPM_MESSAGE_FUNCTION_PORT_STATUS_CHANGED              0x00010006
#define SPPM_MESSAGE_FUNCTION_DATA_RECEIVED                    0x00010007
#define SPPM_MESSAGE_FUNCTION_TRANSMIT_DATA_BUFFER_EMPTY       0x00010008

#define SPPM_MESSAGE_FUNCTION_IDPS_STATUS                      0x00010101
#define SPPM_MESSAGE_FUNCTION_SESSION_OPEN_REQUEST             0x00010102
#define SPPM_MESSAGE_FUNCTION_SESSION_CLOSED                   0x00010103
#define SPPM_MESSAGE_FUNCTION_SESSION_DATA_RECEIVED            0x00010104
#define SPPM_MESSAGE_FUNCTION_SESSION_DATA_CONFIRMATION        0x00010105
#define SPPM_MESSAGE_FUNCTION_NON_SESSION_DATA_RECEIVED        0x00010106
#define SPPM_MESSAGE_FUNCTION_NON_SESSION_DATA_CONFIRMATION    0x00010107
#define SPPM_MESSAGE_FUNCTION_UNHANDLED_CTRL_MSG_RECEIVED      0x00010108

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Serial Port Profile     */
   /* (SPP) Manager.                                                    */

   /* Serial Port Profile (SPP) Manager Command/Response Message        */
   /* Formats.                                                          */

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Register a Local     */
   /* Server Port (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_REGISTER_SERVER_PORT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Register_Server_Port_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerPort;
   unsigned long         PortFlags;
} SPPM_Register_Server_Port_Request_t;

#define SPPM_REGISTER_SERVER_PORT_REQUEST_SIZE                 (sizeof(SPPM_Register_Server_Port_Request_t))

   /* The following constants are used with the PortFlags member of the */
   /* SPPM_Register_Server_Port_Request_t message to control various    */
   /* options for the registered port.                                  */
#define SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHORIZATION  0x00000001
#define SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_AUTHENTICATION 0x00000002
#define SPPM_REGISTER_SERVER_PORT_FLAGS_REQUIRE_ENCRYPTION     0x00000004
#define SPPM_REGISTER_SERVER_PORT_FLAGS_MFI_ALLOWED            0x40000000
#define SPPM_REGISTER_SERVER_PORT_FLAGS_MFI_REQUIRED           0x80000000

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Register a Local     */
   /* Server Port (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_REGISTER_SERVER_PORT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Register_Server_Port_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          PortHandle;
} SPPM_Register_Server_Port_Response_t;

#define SPPM_REGISTER_SERVER_PORT_RESPONSE_SIZE                (sizeof(SPPM_Register_Server_Port_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Respond to a Local   */
   /* Server Port Connection/Authorization (Request).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_SERVER_REQUEST_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Server_Port_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Boolean_t             Accept;
} SPPM_Open_Server_Port_Request_Response_Request_t;

#define SPPM_OPEN_SERVER_PORT_REQUEST_RESPONSE_REQUEST_SIZE    (sizeof(SPPM_Open_Server_Port_Request_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Respond to a Local   */
   /* Server Port Connection/Authorization (Response).                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_SERVER_REQUEST_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Server_Port_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Open_Server_Port_Request_Response_Response_t;

#define SPPM_OPEN_SERVER_PORT_REQUEST_RESPONSE_RESPONSE_SIZE   (sizeof(SPPM_Open_Server_Port_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Un-Register a Local  */
   /* Server Port (Request).                                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_UN_REGISTER_SERVER_PORT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Un_Register_Server_Port_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
} SPPM_Un_Register_Server_Port_Request_t;

#define SPPM_UN_REGISTER_SERVER_PORT_REQUEST_SIZE              (sizeof(SPPM_Un_Register_Server_Port_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Un-Register a Local  */
   /* Server Port (Response).                                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_SERVER_REQUEST_RESPONSE    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Un_Register_Server_Port_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Un_Register_Server_Port_Response_t;

#define SPPM_UN_REGISTER_SERVER_PORT_RESPONSE_SIZE             (sizeof(SPPM_Un_Register_Server_Port_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Register an SDP      */
   /* Record for an already registered SPP Server Port (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The VariableData member is a variable data member that   */
   /*          consists of the following data:                          */
   /*             - NumberServiceUUIDs*SDP_UUID_ENTRY_SIZE bytes (all of*/
   /*               the Service UUID's to append to the Service UUID    */
   /*               List).                                              */
   /*             - ProtocolListAttributeDataLength bytes (Raw SDP      */
   /*               Attribute Data that represents the protocols to     */
   /*               append to the Protocol Descriptor List).            */
   /*             - ServiceNameLength bytes (UTF-8 encoded which        */
   /*               represents the service name).                       */
typedef struct _tagSPPM_Register_Service_Record_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          NumberServiceClassUUID;
   unsigned int          ProtocolListAttributeDataLength;
   unsigned int          ServiceNameLength;
   unsigned char         VariableData[1];
} SPPM_Register_Service_Record_Request_t ;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Register SDP Request Message given the number of SDP UUID */
   /* Entries present in the Service UUID list (NOT the number of bytes */
   /* occupied by the list) and the number of actual RAW SDP Attribute  */
   /* bytes that are to be added to the Protocol Descriptor List        */
   /* Attribute (raw bytes).  This function accepts as it's input the   */
   /* total number individual SDP UUID Entries that are present starting*/
   /* from the VariableData member of the                               */
   /* SPPM_Register_Service_Record_Request_t structure followed by the  */
   /* total number of raw SDP Attribute Bytes that are to be appended to*/
   /* the Protocol Descriptor List Attribute followed by the number of  */
   /* individual UTF-8 characters (NULL terminated - including the NULL */
   /* terminator) and returns the total number of bytes required to hold*/
   /* the entire message.                                               */
#define SPPM_REGISTER_SERVICE_RECORD_REQUEST_SIZE(_x, _y, _z)  (STRUCTURE_OFFSET(SPPM_Register_Service_Record_Request_t, VariableData) + (((unsigned int)(_x))*(SDP_UUID_ENTRY_SIZE)) + (unsigned int)(_y) + (unsigned int)(_z))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Register an SDP      */
   /* Record for an already registered SPP Server Port (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_REGISTER_SERVICE_RECORD         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Register_Service_Record_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned long         ServiceRecordHandle;
} SPPM_Register_Service_Record_Response_t;

#define SPPM_REGISTER_SERVICE_RECORD_RESPONSE_SIZE             (sizeof(SPPM_Register_Service_Record_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Open a Remote Server */
   /* Port (Request).                                                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_REMOTE_PORT                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Remote_Port_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ServerPort;
   unsigned long         OpenFlags;
} SPPM_Open_Remote_Port_Request_t;

#define SPPM_OPEN_REMOTE_PORT_REQUEST_SIZE                     (sizeof(SPPM_Open_Remote_Port_Request_t))

   /* The following constants are used with the OpenFlags member of the */
   /* SPPM_Open_Remote_Port_Request_t message to control various options*/
   /* to Open the port.                                                 */
#define SPPM_OPEN_REMOTE_PORT_FLAGS_REQUIRE_AUTHENTICATION     0x00000001
#define SPPM_OPEN_REMOTE_PORT_FLAGS_REQUIRE_ENCRYPTION         0x00000002
#define SPPM_OPEN_REMOTE_PORT_FLAGS_MFI_REQUIRED               0x80000000

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Open a Remote Server */
   /* Port (Response).                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_REMOTE_PORT                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Remote_Port_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          PortHandle;
} SPPM_Open_Remote_Port_Response_t;

#define SPPM_OPEN_REMOTE_PORT_RESPONSE_SIZE                    (sizeof(SPPM_Open_Remote_Port_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Close a currently    */
   /* connected Port - either Local or Remote (Request).                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CLOSE_PORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Close_Port_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
} SPPM_Close_Port_Request_t;

#define SPPM_CLOSE_PORT_REQUEST_SIZE                           (sizeof(SPPM_Close_Port_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to Close a currently    */
   /* connected Port - either Local or Remote (Response).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CLOSE_PORT                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Close_Port_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Close_Port_Response_t;

#define SPPM_CLOSE_PORT_RESPONSE_SIZE                          (sizeof(SPPM_Close_Port_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to read data from a     */
   /* currently connected Port - either Local or Remote (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_READ_DATA                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Read_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          DataLength;
} SPPM_Read_Data_Request_t;

#define SPPM_READ_DATA_REQUEST_SIZE                            (sizeof(SPPM_Read_Data_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to read data from a     */
   /* currently connected Port - either Local or Remote (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_READ_DATA                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Read_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TotalDataLength;
   unsigned int          DataLength;
   unsigned char         VariableData[1];
} SPPM_Read_Data_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Read Data Response Message given the total number of Data */
   /* bytes that are to be returned in the response.  This function     */
   /* accepts as it's input the total number individual Data bytes that */
   /* are present in the response starting from the VariableData member */
   /* of the SPPM_Read_Data_Response_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
   /* * NOTE * The value for this MACRO *MUST* be the same as the       */
   /*          value that is specified in the DataLength member of the  */
   /*          SPPM_Read_Data_Response_t message.                       */
#define SPPM_READ_DATA_RESPONSE_SIZE(_x)                       (STRUCTURE_OFFSET(SPPM_Read_Data_Response_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to write data to a      */
   /* currently connected Port - either Local or Remote (Request).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_WRITE_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Write_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          DataLength;
   unsigned char         VariableData[1];
} SPPM_Write_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Write Data Request Message given the total number of Data */
   /* bytes that are to be written in the request.  This function       */
   /* accepts as it's input the total number individual Data bytes that */
   /* are present in the request starting from the VariableData member  */
   /* of the SPPM_Write_Data_Request_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the DataLength member of the        */
   /*          SPPM_Write_Data_Request_t message.                       */
#define SPPM_WRITE_DATA_REQUEST_SIZE(_x)                       (STRUCTURE_OFFSET(SPPM_Write_Data_Request_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to write data to a      */
   /* currently connected Port - either Local or Remote (Response).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_WRITE_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Write_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          DataLengthWritten;
} SPPM_Write_Data_Response_t;

#define SPPM_WRITE_DATA_RESPONSE_SIZE                          (sizeof(SPPM_Write_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send a Line Status   */
   /* over a currently connected Port - either Local or Remote          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_LINE_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Line_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned long         LineStatusMask;
} SPPM_Send_Line_Status_Request_t;

#define SPPM_SEND_LINE_STATUS_REQUEST_SIZE                     (sizeof(SPPM_Send_Line_Status_Request_t))

   /* The following constants are used with the LineStatusMask members  */
   /* of the SPPM_Send_Line_Status_Request_t and                        */
   /* SPPM_Line_Status_Changed_Message_t messages to specify the Line   */
   /* Status Mask.                                                      */
#define SPPM_LINE_STATUS_MASK_NO_ERROR_VALUE                   (SPP_LINE_STATUS_NO_ERROR_VALUE)
#define SPPM_LINE_STATUS_MASK_OVERRUN_ERROR_MASK               (SPP_LINE_STATUS_OVERRUN_ERROR_BIT_MASK)
#define SPPM_LINE_STATUS_MASK_PARITY_ERROR_MASK                (SPP_LINE_STATUS_PARITY_ERROR_BIT_MASK)
#define SPPM_LINE_STATUS_MASK_FRAMING_ERROR_MASK               (SPP_LINE_STATUS_FRAMING_ERROR_BIT_MASK)

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send a Line Status   */
   /* over a currently connected Port - either Local or Remote          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_LINE_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Line_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Send_Line_Status_Response_t;

#define SPPM_SEND_LINE_STATUS_RESPONSE_SIZE                    (sizeof(SPPM_Send_Line_Status_Response_t))

   /* The following structure is a container structure that holds the   */
   /* information pertaining to a Port Status Event.                    */
typedef struct _tagSPPM_Port_Status_t
{
   unsigned long PortStatusMask;
   Boolean_t     BreakSignal;
   unsigned int  BreakTimeout;
} SPPM_Port_Status_t;

#define SPPM_PORT_STATUS_SIZE                                  (sizeof(SPPM_Port_Status_t))

   /* The following constants are used with the PortStatusMask member of*/
   /* the SPPM_Port_Status_t to specify the Port Status Mask.           */
#define SPPM_PORT_STATUS_MASK_NO_ERROR_VALUE                   (SPP_PORT_STATUS_CLEAR_VALUE)
#define SPPM_PORT_STATUS_MASK_RTS_CTS_MASK                     (SPP_PORT_STATUS_RTS_CTS_BIT)
#define SPPM_PORT_STATUS_MASK_DTR_DSR_MASK                     (SPP_PORT_STATUS_DTR_DSR_BIT)
#define SPPM_PORT_STATUS_MASK_RING_INDICATOR_MASK              (SPP_PORT_STATUS_RING_INDICATOR_BIT)
#define SPPM_PORT_STATUS_MASK_CARRIER_DETECT_MASK              (SPP_PORT_STATUS_CARRIER_DETECT_BIT)

   /* The following constants are used with the BreakTimeout member of  */
   /* the of the SPPM_Port_Status_t to specify the Break Timeout when a */
   /* Break Signal is sent/received.                                    */
#define SPPM_PORT_STATUS_BREAK_TIMEOUT_MINIMUM                 (SPP_BREAK_SIGNAL_MINIMUM)
#define SPPM_PORT_STATUS_BREAK_TIMEOUT_MAXIMUM                 (SPP_BREAK_SIGNAL_MAXIMUM)

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send a Port Status   */
   /* over a currently connected Port - either Local or Remote          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_PORT_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Port_Status_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   SPPM_Port_Status_t    PortStatus;
} SPPM_Send_Port_Status_Request_t;

#define SPPM_SEND_PORT_STATUS_REQUEST_SIZE                     (sizeof(SPPM_Send_Port_Status_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send a Port Status   */
   /* over a currently connected Port - either Local or Remote          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_PORT_STATUS                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Port_Status_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Send_Port_Status_Response_t;

#define SPPM_SEND_PORT_STATUS_RESPONSE_SIZE                    (sizeof(SPPM_Send_Port_Status_Response_t))

   /* The following structure represents the Message definition for     */
   /* a Serial Port Profile (SPP) Manager Message to query whether a    */
   /* particular port is allocated as a server (Request).               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_QUERY_SERVER_PRESENT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Query_Server_Present_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          ServerPort;
} SPPM_Query_Server_Present_Request_t;

#define SPPM_QUERY_SERVER_PRESENT_REQUEST_SIZE                 (sizeof(SPPM_Query_Server_Present_Request_t))

   /* The following structure represents the Message definition for     */
   /* a Serial Port Profile (SPP) Manager Message to query whether a    */
   /* particular port is allocated as a server (Response).              */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_QUERY_SERVER_PRESENT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Query_Server_Present_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   Boolean_t             Present;
} SPPM_Query_Server_Present_Response_t;

#define SPPM_QUERY_SERVER_PRESENT_RESPONSE_SIZE                (sizeof(SPPM_Query_Server_Present_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to locate an available  */
   /* port which does not currently host an SPP Server (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_FIND_FREE_SERVER_PORT           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Find_Free_Server_Port_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} SPPM_Find_Free_Server_Port_Request_t;

#define SPPM_FIND_FREE_SERVER_PORT_REQUEST_SIZE                (sizeof(SPPM_Find_Free_Server_Port_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to locate an available  */
   /* port which does not currently host an SPP Server (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_FIND_FREE_SERVER_PORT           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Find_Free_Server_Port_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ServerPort;
} SPPM_Find_Free_Server_Port_Response_t;

#define SPPM_FIND_FREE_SERVER_PORT_RESPONSE_SIZE               (sizeof(SPPM_Find_Free_Server_Port_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to change the Buffer    */
   /* Size for a specifed SPPM Port (Request).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CHANGE_BUFFER_SIZE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Change_Buffer_Size_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          ReceiveBufferSize;
   unsigned int          TransmitBufferSize;
} SPPM_Change_Buffer_Size_Request_t;

#define SPPM_CHANGE_BUFFER_SIZE_REQUEST_SIZE                   (sizeof(SPPM_Change_Buffer_Size_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to change the Buffer    */
   /* Size for a specifed SPPM Port (Response).                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CHANGE_BUFFER_SIZE              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Change_Buffer_Size_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Change_Buffer_Size_Response_t;

#define SPPM_CHANGE_BUFFER_SIZE_RESPONSE_SIZE                  (sizeof(SPPM_Change_Buffer_Size_Response_t))

   /* The following structure is used with the                          */
   /* SPPM_Configure_MFi_Settings_Request_t request message.  This      */
   /* structure represents the format of each individual FID token that */
   /* is part of of the VariableData member of the                      */
   /* SPPM_Configure_MFi_Settings_Request_t request.                    */
typedef struct _tagSPPM_MFi_Configuration_FID_Token_t
{
   Byte_t FIDType;
   Byte_t FIDSubType;
   Byte_t FIDDataLength;
   Byte_t VariableFIDData[1];
} SPPM_MFi_Configuration_FID_Token_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an individual Serial Port Profile (SPP)*/
   /* Manager Configuration FID token given the total number of FID Data*/
   /* Length bytes that are part of the FID token Data.  This function  */
   /* accepts as it's input the total number individual FID token Data  */
   /* bytes that are present in the request starting from the           */
   /* VariableFIDData member of the SPPM_MFi_Configuration_Token_t      */
   /* structure and returns the total number of bytes required to hold  */
   /* the individual MFi Configuration FID token.                       */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the FIDDataLength member of the     */
   /*          SPPM_MFi_Configuration_FID_Token_t token.                */
#define SPPM_MFI_CONFIGURATION_FID_TOKEN_SIZE(_x)              (STRUCTURE_OFFSET(SPPM_MFi_Configuration_FID_Token_t, VariableFIDData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to configure the MFi    */
   /* settings that are to be supported by the local Serial Port        */
   /* Manager (Request).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CONFIGURE_MFI_SETTINGS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Configure_MFi_Settings_Request_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  MaximumReceivePacketSize;
   unsigned int                  DataPacketTimeout;
   Byte_t                        NumberSupportedLingos;
   Byte_t                        SupportedLingoList[SPPM_MFI_MAXIMUM_SUPPORTED_LINGOS];
   SPPM_MFi_Accessory_Info_t     AccessoryInfo;
   Byte_t                        NumberSupportedProtocols;
   SPPM_MFi_Protocol_String_t    SupportedProtocolList[SPPM_MFI_MAXIMUM_SUPPORTED_PROTOCOLS];
   Byte_t                        BundleSeedIDString[SPPM_MFI_MAXIMUM_SUPPORTED_BUNDLE_SEED_ID_LENGTH];
   SPPM_MFi_Language_ID_String_t CurrentLanguage;
   Byte_t                        NumberSupportedLanguages;
   SPPM_MFi_Language_ID_String_t SupportedLanguagesList[SPPM_MFI_MAXIMUM_SUPPORTED_LANGUAGES];
   Word_t                        NumberControlMessagesSent;
   Word_t                        NumberControlMessagesReceived;
   Byte_t                        NumberFIDTokens;
   unsigned int                  FIDTokenDataLength;
   unsigned char                 VariableData[1];
} SPPM_Configure_MFi_Settings_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Configure MFi Settings Request Message given the total    */
   /* number of Control Message IDs and FID Token data bytes that are to*/
   /* be written in the request.  This function accepts as it's input,  */
   /* respectively, the total number of Control Message IDs supported   */
   /* for Sending, the total number of Control Message IDs supported    */
   /* for Receiving, and the total number of individual Data bytes that */
   /* represent the FID Tokens.  This function returns the total number */
   /* of bytes required to hold the entire message.                     */
   /* * NOTE * The third parameter for this MACRO *MUST* be the same as */
   /*          the value that is specified in the FIDTokenDataLength    */
   /*          member of the SPPM_Configure_MFi_Settings_Request_t      */
   /*          message.                                                 */
   /* * NOTE * The FID Token data consists of packed FID Data.          */
   /*          Each element of the data will consist of:                */
   /*             - FID Type        - 1 byte                            */
   /*             - FID Sub Type    - 1 byte                            */
   /*             - FID Data Length - 1 byte                            */
   /*             - FID Data        - FID Data length byte(s)           */
   /*          Each element will be of type                             */
   /*          SPPM_MFi_Configuration_FID_Token_t.  This structure is   */
   /*          variable in length based on the number of FID token data */
   /*          bytes.                                                   */
   /* * NOTE * The variable length fields shall be packed in the        */
   /*          VariableData array in the order: Control Messages Sent   */
   /*          (Word_t[]), Control Messages Received (Word_t[]), FID    */
   /*          Tokens (SPPM_MFi_Configuration_FID_Token_t *). Each      */
   /*          list shall begin on a Word_t (16-bit) boundary, but the  */
   /*          elements of the FID Tokens list may be Byte_t (8-bit)    */
   /*          aligned because the SPPM_MFi_Configuration_FID_Token_t   */
   /*          structure is composed of only Byte_t members and each FID*/
   /*          Tokens list element may be of variable length.           */
#define SPPM_CONFIGURE_MFI_SETTINGS_REQUEST_SIZE(_x, _y, _z)   (STRUCTURE_OFFSET(SPPM_Configure_MFi_Settings_Request_t, VariableData) + (((unsigned int)(_x))*(WORD_SIZE)) + (((unsigned int)(_y))*(WORD_SIZE)) + (unsigned int)(_z))

   /* This structure is a subset of the above structure.  This is the   */
   /* original Authentication Information structure that was supported  */
   /* by the Device Manager.  See comments above for a description of   */
   /* the individual fields in this message.                            */
   /* * NOTE * Clients can look at the Message size to determine what   */
   /*          structure that has been returned from the server.        */
typedef struct _tagSPPM_Configure_MFi_Settings_Request_Legacy_1_t
{
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  MaximumReceivePacketSize;
   unsigned int                  DataPacketTimeout;
   Byte_t                        NumberSupportedLingos;
   Byte_t                        SupportedLingoList[SPPM_MFI_MAXIMUM_SUPPORTED_LINGOS];
   SPPM_MFi_Accessory_Info_t     AccessoryInfo;
   Byte_t                        NumberSupportedProtocols;
   SPPM_MFi_Protocol_String_t    SupportedProtocolList[SPPM_MFI_MAXIMUM_SUPPORTED_PROTOCOLS];
   Byte_t                        BundleSeedIDString[SPPM_MFI_MAXIMUM_SUPPORTED_BUNDLE_SEED_ID_LENGTH];
   SPPM_MFi_Language_ID_String_t CurrentLanguage;
   Byte_t                        NumberSupportedLanguages;
   SPPM_MFi_Language_ID_String_t SupportedLanguagesList[SPPM_MFI_MAXIMUM_SUPPORTED_LANGUAGES];
   Byte_t                        NumberFIDTokens;
   unsigned int                  FIDTokenDataLength;
   unsigned char                 VariableData[1];
} SPPM_Configure_MFi_Settings_Request_Legacy_1_t;

   /* The following MACRO is provided to allow the programmer a         */
   /* very simple means of quickly determining the total number of      */
   /* Bytes that will be required to hold a an entire Serial Port       */
   /* Profile (SPP) Manager Configure MFi Settings Request Message      */
   /* given the total number of FID Token data bytes that are to        */
   /* be written in the request.  This function accepts as it's         */
   /* input the total number individual Data bytes that are present     */
   /* in the request starting from the VariableData member of the       */
   /* SPPM_Configure_MFi_Settings_Request_Legacy_1_t structure and      */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the FIDTokeDataLength member of the */
   /*          SPPM_Configure_MFi_Settings_Request_Legacy_1_t message.  */
   /* * NOTE * The FID Token data consists of packed FID Data.          */
   /*          Each element of the data will consist of:                */
   /*             - FID Type        - 1 byte                            */
   /*             - FID Sub Type    - 1 byte                            */
   /*             - FID Data Length - 1 byte                            */
   /*             - FID Data        - FID Data length byte(s)           */
   /*          Each element will be of type                             */
   /*          SPPM_MFi_Configuration_FID_Token_t.  This structure is   */
   /*          variable in length based on the number of FID token data */
   /*          bytes.                                                   */
#define SPPM_CONFIGURE_MFI_SETTINGS_REQUEST_LEGACY_1_SIZE(_x)  (STRUCTURE_OFFSET(SPPM_Configure_MFi_Settings_Request_Legacy_1_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to configure the MFi    */
   /* settings that are to be supported by the local Serial Port        */
   /* Manager (Response).                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CONFIGURE_MFI_SETTINGS          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Configure_MFi_Settings_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Configure_MFi_Settings_Response_t;

#define SPPM_CONFIGURE_MFI_SETTINGS_RESPONSE_SIZE              (sizeof(SPPM_Configure_MFi_Settings_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to query the current    */
   /* connection type of a currently connected Port - either Local or   */
   /* Remote (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_QUERY_CONNECTION_TYPE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Query_Connection_Type_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
} SPPM_Query_Connection_Type_Request_t;

#define SPPM_QUERY_CONNECTION_TYPE_REQUEST_SIZE                (sizeof(SPPM_Query_Connection_Type_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to query the current    */
   /* connection type of a currently connected Port - either Local or   */
   /* Remote (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_QUERY_CONNECTION_TYPE           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Query_Connection_Type_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   int                    Status;
   SPPM_Connection_Type_t ConnectionType;
} SPPM_Query_Connection_Type_Response_t;

#define SPPM_QUERY_CONNECTION_TYPE_RESPONSE_SIZE               (sizeof(SPPM_Query_Connection_Type_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to respond to an        */
   /* incoming open session request on a currently connected remote port*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_SESSION_REQUEST_RESPONSE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Session_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                SessionID;
   Boolean_t             Accept;
} SPPM_Open_Session_Request_Response_Request_t;

#define SPPM_OPEN_SESSION_REQUEST_RESPONSE_REQUEST_SIZE        (sizeof(SPPM_Open_Session_Request_Response_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to respond to an        */
   /* incoming open session request on a currently connected remote port*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_SESSION_REQUEST_RESPONSE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Session_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} SPPM_Open_Session_Request_Response_Response_t;

#define SPPM_OPEN_SESSION_REQUEST_RESPONSE_RESPONSE_SIZE       (sizeof(SPPM_Open_Session_Request_Response_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send MFi session data*/
   /* to a currently connected Port - either Local or Remote (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_SESSION_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Session_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                SessionID;
   Word_t                SessionDataLength;
   unsigned char         VariableData[1];
} SPPM_Send_Session_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Send MFi Session Data Request Message given the total     */
   /* number of Session Data bytes that are to be written in the        */
   /* request.  This function accepts as it's input the total number    */
   /* individual Data bytes that are present in the request starting    */
   /* from the VariableData member of the                               */
   /* SPPM_Send_Session_Data_Request_t structure and returns the total  */
   /* number of bytes required to hold the entire message.              */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the SessionDataLength member of the */
   /*          SPPM_Send_Session_Data_Request_t message.                */
#define SPPM_SEND_SESSION_DATA_REQUEST_SIZE(_x)                (STRUCTURE_OFFSET(SPPM_Send_Session_Data_Request_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send MFi session data*/
   /* to a currently connected Port - either Local or Remote (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_SESSION_DATA               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Session_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          PacketID;
} SPPM_Send_Session_Data_Response_t;

#define SPPM_SEND_SESSION_DATA_RESPONSE_SIZE                   (sizeof(SPPM_Send_Session_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send non MFi session */
   /* data to a currently connected Port - either Local or Remote       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_NON_SESSION_DATA           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Non_Session_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Byte_t                Lingo;
   Byte_t                CommandID;
   Word_t                TransactionID;
   Word_t                DataLength;
   unsigned char         VariableData[1];
} SPPM_Send_Non_Session_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Send Non MFi Session Data Request Message given the total */
   /* number of Non Session Data bytes that are to be written in the    */
   /* request.  This function accepts as it's input the total number    */
   /* individual Data bytes that are present in the request starting    */
   /* from the VariableData member of the                               */
   /* SPPM_Send_Non_Session_Data_Request_t structure and returns the    */
   /* total number of bytes required to hold the entire message.        */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the DataLength member of the        */
   /*          SPPM_Send_Non_Session_Data_Request_t message.            */
#define SPPM_SEND_NON_SESSION_DATA_REQUEST_SIZE(_x)            (STRUCTURE_OFFSET(SPPM_Send_Non_Session_Data_Request_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send non MFi session */
   /* data to a currently connected Port - either Local or Remote       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_NON_SESSION_DATA           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Non_Session_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          PacketID;
} SPPM_Send_Non_Session_Data_Response_t;

#define SPPM_SEND_NON_SESSION_DATA_RESPONSE_SIZE               (sizeof(SPPM_Send_Non_Session_Data_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to cancel a currently   */
   /* queued MFi packet of a currently connected Port - either Local or */
   /* Remote (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CANCEL_PACKET                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Cancel_Packet_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          PacketID;
} SPPM_Cancel_Packet_Request_t;

#define SPPM_CANCEL_PACKET_REQUEST_SIZE                        (sizeof(SPPM_Cancel_Packet_Request_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to cancel a currently   */
   /* queued MFi packet of a currently connected Port - either Local or */
   /* Remote (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_CANCEL_PACKET                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Cancel_Packet_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   int                    Status;
} SPPM_Cancel_Packet_Response_t;

#define SPPM_CANCEL_PACKET_RESPONSE_SIZE                       (sizeof(SPPM_Cancel_Packet_Response_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send an MFi Control  */
   /* Message to a currently connected Port - either Local or Remote    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_CONTROL_MESSAGE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Control_Message_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                ControlMessageID;
   Word_t                DataLength;
   unsigned char         VariableData[1];
} SPPM_Send_Control_Message_Request_t;

   /* The following MACRO is provided to allow the programmer a         */
   /* very simple means of quickly determining the total number of      */
   /* Bytes that will be required to hold a an entire Serial Port       */
   /* Profile (SPP) Manager Send Control Message Request Message        */
   /* given the total number of Message Payload bytes that are to be    */
   /* written in the request.  This function accepts as it's input      */
   /* the total number individual Payload bytes that are present        */
   /* in the request starting from the VariableData member of the       */
   /* SPPM_Send_Control_Message_Request_t structure and returns the     */
   /* total number of bytes required to hold the entire message.        */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the DataLength member of the        */
   /*          SPPM_Send_Control_Message_Request_t message.             */
#define SPPM_SEND_CONTROL_MESSAGE_REQUEST_SIZE(_x)             (STRUCTURE_OFFSET(SPPM_Send_Control_Message_Request_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message to send an MFi Control  */
   /* Message to a currently connected Port - either Local or Remote    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SEND_CONTROL_MESSAGE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Send_Control_Message_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          PacketID;
} SPPM_Send_Control_Message_Response_t;

#define SPPM_SEND_CONTROL_MESSAGE_RESPONSE_SIZE                (sizeof(SPPM_Send_Control_Message_Response_t))

   /* Serial Port Profile (SPP) Manager Asynchronous Message Formats.   */

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that a local server port requires authorization for an incoming   */
   /* connection (asynchronously).                                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SERVER_PORT_OPEN_REQUEST        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Server_Port_Open_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   BD_ADDR_t             RemoteDeviceAddress;
} SPPM_Server_Port_Open_Request_Message_t;

#define SPPM_SERVER_PORT_OPEN_REQUEST_MESSAGE_SIZE             (sizeof(SPPM_Server_Port_Open_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that a local server port now has a connection (asynchronously).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SERVER_PORT_OPENED              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Server_Port_Opened_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           PortHandle;
   BD_ADDR_t              RemoteDeviceAddress;
   SPPM_Connection_Type_t ConnectionType;
} SPPM_Server_Port_Opened_Message_t;

#define SPPM_SERVER_PORT_OPENED_MESSAGE_SIZE                   (sizeof(SPPM_Server_Port_Opened_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that a port (either server or remote) no longer has an active     */
   /* connection - asynchronously).                                     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_PORT_CLOSED                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Port_Closed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
} SPPM_Port_Closed_Message_t;

#define SPPM_PORT_CLOSED_MESSAGE_SIZE                          (sizeof(SPPM_Port_Closed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that of the status of a prior opened remote port (whether the     */
   /* connection was successful - asynchronously).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_OPEN_REMOTE_PORT_RESULT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Open_Remote_Port_Result_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           PortHandle;
   int                    Status;
   SPPM_Connection_Type_t ConnectionType;
} SPPM_Open_Remote_Port_Result_Message_t;

#define SPPM_OPEN_REMOTE_PORT_RESULT_MESSAGE_SIZE              (sizeof(SPPM_Open_Remote_Port_Result_Message_t))

   /* The following constants are used with the PortResult member of the*/
   /* SPPM_Open_Remote_Port_Result_Message_t message to describe the    */
   /* actual Port Connection Result Status.                             */
#define SPPM_OPEN_REMOTE_PORT_STATUS_SUCCESS                   0x00000000
#define SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_TIMEOUT           0x00000001
#define SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_REFUSED           0x00000002
#define SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_SECURITY          0x00000003
#define SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_DEVICE_POWER_OFF  0x00000004
#define SPPM_OPEN_REMOTE_PORT_STATUS_FAILURE_UNKNOWN           0x00000005

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the Line Status of an active connection has changed          */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_LINE_STATUS_CHANGED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Line_Status_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned long         LineStatusMask;
} SPPM_Line_Status_Changed_Message_t;

#define SPPM_LINE_STATUS_CHANGED_MESSAGE_SIZE                  (sizeof(SPPM_Line_Status_Changed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the Port Status of an active connection has changed          */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_PORT_STATUS_CHANGED             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Port_Status_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   SPPM_Port_Status_t    PortStatus;
} SPPM_Port_Status_Changed_Message_t;

#define SPPM_PORT_STATUS_CHANGED_MESSAGE_SIZE                  (sizeof(SPPM_Port_Status_Changed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that Data has been received on an active connection               */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_DATA_RECEIVED                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          DataLength;
} SPPM_Data_Received_Message_t;

#define SPPM_DATA_RECEIVED_MESSAGE_SIZE                        (sizeof(SPPM_Data_Received_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the Transmit Buffer is now empty on an active connection     */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_TRANSMIT_DATA_BUFFER_EMPTY      */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * This Message will ONLY be dispatched if the client       */
   /*          attempted to write data, it was successful, and not all  */
   /*          of the data was able to be sent.                         */
typedef struct _tagSPPM_Transmit_Data_Buffer_Empty_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
} SPPM_Transmit_Data_Buffer_Empty_Message_t;

#define SPPM_TRANSMIT_DATA_BUFFER_EMPTY_MESSAGE_SIZE           (sizeof(SPPM_Transmit_Data_Buffer_Empty_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* of an IDPS Status event - asynchronously.                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_IDPS_STATUS                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_IDPS_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   SPPM_IDPS_State_t     IDPSState;
   unsigned int          Status;
} SPPM_IDPS_Status_Message_t;

#define SPPM_IDPS_STATUS_MESSAGE_SIZE                          (sizeof(SPPM_IDPS_Status_Message_t))

   /* The following constants represent the defined IDPS Status values  */
   /* that are valid.                                                   */
#define SPPM_IDPS_STATUS_SUCCESS                                        0
#define SPPM_IDPS_STATUS_STATUS_ERROR_RETRYING                          1
#define SPPM_IDPS_STATUS_STATUS_TIMEOUT_HALTING                         2
#define SPPM_IDPS_STATUS_STATUS_GENERAL_FAILURE                         3
#define SPPM_IDPS_STATUS_STATUS_PROCESS_FAILURE                         4
#define SPPM_IDPS_STATUS_STATUS_PROCESS_TIMEOUT_RETRYING                5

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the remote device would like to open an MFi session          */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SESSION_OPEN_REQUEST            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Session_Open_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          MaximimTransmitPacket;
   unsigned int          MaximumReceivePacket;
   Word_t                SessionID;
   Byte_t                ProtocolIndex;
} SPPM_Session_Open_Request_Message_t;

#define SPPM_SESSION_OPEN_REQUEST_MESSAGE_SIZE                 (sizeof(SPPM_Session_Open_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the remote device has closed an active MFi session           */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SESSION_CLOSED                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Session_Closed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                SessionID;
} SPPM_Session_Closed_Message_t;

#define SPPM_SESSION_CLOSED_MESSAGE_SIZE                       (sizeof(SPPM_Session_Closed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that session data has been received from the the remote device    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_SESSION_DATA_RECEIVED           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Session_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                SessionID;
   Word_t                SessionDataLength;
   unsigned char         VariableData[1];
} SPPM_Session_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Session Data Received Asynchronous Message given the total*/
   /* number of session data bytes that were received.  This function   */
   /* accepts as it's input the total number individual Data bytes that */
   /* are present in the received data starting from the VariableData   */
   /* member of the SPPM_Session_Data_Received_Message_t structure and  */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the SessionDataLength member of the */
   /*          SPPM_Session_Data_Received_Message_t message.            */
#define SPPM_SESSION_DATA_RECEIVED_MESSAGE_SIZE(_x)            (STRUCTURE_OFFSET(SPPM_Session_Data_Received_Message_t, VariableData) + (unsigned int)(_x))

   /* The following constants represent the defined status values for   */
   /* the Data Confirmation events (both the Session Data and Non       */
   /* Session Data events - SPPM_Session_Data_Confirmation_Message_t and*/
   /* SPPM_Non_Session_Data_Confirmation_Message_t events).             */
#define SPPM_DATA_CONFIRMATION_STATUS_PACKET_SENT                       0
#define SPPM_DATA_CONFIRMATION_STATUS_PACKET_ACKNOWLEDGED               1
#define SPPM_DATA_CONFIRMATION_STATUS_PACKET_FAILED                     2
#define SPPM_DATA_CONFIRMATION_STATUS_PACKET_CANCELED                   3

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the local client of the confirmation result of previously    */
   /* sent session data on an active MFi session (asynchronously).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*                SPPM_MESSAGE_FUNCTION_SESSION_DATA_CONFIRMATION    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Session_Data_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                SessionID;
   unsigned int          PacketID;
   unsigned int          Status;
} SPPM_Session_Data_Confirmation_Message_t;

#define SPPM_SESSION_DATA_CONFIRMATION_MESSAGE_SIZE            (sizeof(SPPM_Session_Data_Confirmation_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that non session data has been received from the the remote device*/
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_NON_SESSION_DATA_RECEIVED       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Non_Session_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Byte_t                Lingo;
   Byte_t                CommandID;
   Word_t                DataLength;
   unsigned char         VariableData[1];
} SPPM_Non_Session_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Serial Port Profile (SPP)    */
   /* Manager Non Session Data Received Asynchronous Message given the  */
   /* total number of non session data bytes that were received.  This  */
   /* function accepts as it's input the total number individual Data   */
   /* bytes that are present in the received data starting from the     */
   /* VariableData member of the                                        */
   /* SPPM_Non_Session_Data_Received_Message_t structure and returns the*/
   /* total number of bytes required to hold the entire message.        */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the DataLength member of the        */
   /*          SPPM_Non_Session_Data_Received_Message_t message.        */
#define SPPM_NON_SESSION_DATA_RECEIVED_MESSAGE_SIZE(_x)        (STRUCTURE_OFFSET(SPPM_Non_Session_Data_Received_Message_t, VariableData) + (unsigned int)(_x))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that the local client of the confirmation result of previously    */
   /* sent non session data on an active MFi session (asynchronously).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*                SPPM_MESSAGE_FUNCTION_NON_SESSION_DATA_CONFIRMATION*/
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Non_Session_Data_Confirmation_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   unsigned int          PacketID;
   Word_t                TransactionID;
   unsigned int          Status;
} SPPM_Non_Session_Data_Confirmation_Message_t;

#define SPPM_NON_SESSION_DATA_CONFIRMATION_MESSAGE_SIZE        (sizeof(SPPM_Non_Session_Data_Confirmation_Message_t))

   /* The following structure represents the Message definition for a   */
   /* Serial Port Profile (SPP) Manager Message that informs the client */
   /* that a Control Message has been received from the the remote      */
   /* device (asynchronously).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             SPPM_MESSAGE_FUNCTION_UNHANDLED_CTRL_MSG_RECEIVED     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagSPPM_Unhandled_Control_Message_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          PortHandle;
   Word_t                ControlMessageID;
   Word_t                DataLength;
   unsigned char         VariableData[1];
} SPPM_Unhandled_Control_Message_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a         */
   /* very simple means of quickly determining the total number         */
   /* of Bytes that will be required to hold a an entire Serial         */
   /* Port Profile (SPP) Manager Unhandled Control Message Received     */
   /* Asynchronous Message given the total number of Message Payload    */
   /* bytes that were received.  This function accepts as it's input    */
   /* the total number individual Payload bytes that are present in     */
   /* the received data starting from the VariableData member of the    */
   /* SPPM_Unhandled_Control_Message_Received_Message_t structure and   */
   /* returns the total number of bytes required to hold the entire     */
   /* message.                                                          */
   /* * NOTE * The value for this MACRO *MUST* be the same as the value */
   /*          that is specified in the DataLength member of the        */
   /*          SPPM_Unhandled_Control_Message_Received_Message_t        */
   /*          message.                                                 */
#define SPPM_UNHANDLED_CONTROL_MESSAGE_RECEIVED_MESSAGE_SIZE(_x)  (STRUCTURE_OFFSET(SPPM_Unhandled_Control_Message_Received_Message_t, VariableData) + (unsigned int)(_x))

#endif
