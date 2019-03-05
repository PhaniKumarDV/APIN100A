/*****< hdpmmsg.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPMMSG - Defined Interprocess Communication Messages for the Health      */
/*            Device Profile (HDP) Manager for Stonestreet One Bluetopia      */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HDPMMSGH__
#define __HDPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHDP.h"            /* HDP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HDPMType.h"            /* BTPM HDP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Health Device   */
   /* Profile (HDP) Manager.                                            */
#define BTPM_MESSAGE_GROUP_HEALTH_DEVICE_MANAGER                0x00001008

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Health Device    */
   /* Profile (HDP) Manager.                                            */

   /* Health Device Profile (HDP) Manager Commands.                     */
#define HDPM_MESSAGE_FUNCTION_REGISTER_ENDPOINT                 0x00001001
#define HDPM_MESSAGE_FUNCTION_UN_REGISTER_ENDPOINT              0x00001002
#define HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_REQUEST_RESPONSE  0x00001003
#define HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_INSTANCES     0x00001004
#define HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ENDPOINTS     0x00001005
#define HDPM_MESSAGE_FUNCTION_QUERY_ENDPOINT_DESCRIPTION        0x00001006
#define HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE             0x00001007
#define HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE          0x00001008
#define HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE_ENDPOINT    0x00001009
#define HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE_ENDPOINT 0x0000100A
#define HDPM_MESSAGE_FUNCTION_WRITE_DATA                        0x0000100B

   /* Health Device Profile (HDP) Manager Asynchronous Events.          */
#define HDPM_MESSAGE_FUNCTION_CONNECTION_STATUS                 0x00010001
#define HDPM_MESSAGE_FUNCTION_DISCONNECTED                      0x00010002
#define HDPM_MESSAGE_FUNCTION_INCOMING_DATA_CONNECTION_REQUEST  0x00010003
#define HDPM_MESSAGE_FUNCTION_DATA_CONNECTED                    0x00010004
#define HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED                 0x00010005
#define HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS            0x00010006
#define HDPM_MESSAGE_FUNCTION_DATA_RECEIVED                     0x00010007

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Health Device Profile   */
   /* (HDP) Manager.                                                    */

   /* Health Device Profile (HDP) Manager Command/Response Message      */
   /* Formats.                                                          */

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to register a local       */
   /* endpoint. (Request)                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_REGISTER_ENDPOINT               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Register_Endpoint_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   Word_t                DataType;
   HDP_Device_Role_t     LocalRole;
   unsigned int          DescriptionLength;
   char                  Description[1];
} HDPM_Register_Endpoint_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire Register Endpoint message given*/
   /* the length of the endpoint description. This function accepts as  */
   /* it's input the Description Length and returns the total number of */
   /* bytes required to hold the entire message.                        */
#define HDPM_REGISTER_ENDPOINT_REQUEST_SIZE(_x)                (STRUCTURE_OFFSET(HDPM_Register_Endpoint_Request_t, Description) + ((unsigned int)(_x)))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to respond to a request to  */
   /* register a local endpoint. (Response)                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_REGISTER_ENDPOINT               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Register_Endpoint_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Register_Endpoint_Response_t;

#define HDPM_REGISTER_ENDPOINT_RESPONSE_SIZE                   (sizeof(HDPM_Register_Endpoint_Response_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to un-register a local    */
   /* endpoint. (Request)                                               */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_UN_REGISTER_ENDPOINT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Un_Register_Endpoint_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          EndpointID;
} HDPM_Un_Register_Endpoint_Request_t;

#define HDPM_UN_REGISTER_ENDPOINT_REQUEST_SIZE                 (sizeof(HDPM_Un_Register_Endpoint_Request_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to respond to a request to  */
   /* un-register a local endpoint. (Response)                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_UN_REGISTER_ENDPOINT            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Un_Register_Endpoint_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Un_Register_Endpoint_Response_t;

#define HDPM_UN_REGISTER_ENDPOINT_RESPONSE_SIZE                (sizeof(HDPM_Un_Register_Endpoint_Response_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to respond to an incoming   */
   /* data connection request for a local endpoint. (Request)           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_REQUEST_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Data_Connection_Request_Response_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataLinkID;
   Byte_t                ResponseCode;
   HDP_Channel_Mode_t    ChannelMode;
} HDPM_Data_Connection_Request_Response_Request_t;

#define HDPM_DATA_CONNECTION_REQUEST_RESPONSE_REQUEST_SIZE     (sizeof(HDPM_Data_Connection_Request_Response_Request_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to respond to an incoming   */
   /* data connection request for a local endpoint. (Response)          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_REQUEST_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Data_Connection_Request_Response_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Data_Connection_Request_Response_Response_t;

#define HDPM_DATA_CONNECTION_REQUEST_RESPONSE_RESPONSE_SIZE    (sizeof(HDPM_Data_Connection_Request_Response_Response_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the instances    */
   /* advertised by a remote device. (Request)                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_INSTANCES    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Remote_Device_Instances_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HDPM_Query_Remote_Device_Instances_Request_t;

#define HDPM_QUERY_REMOTE_DEVICE_INSTANCES_REQUEST_SIZE        (sizeof(HDPM_Query_Remote_Device_Instances_Request_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the instances    */
   /* advertised by a remote device. (Response)                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_INSTANCES    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Remote_Device_Instances_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberInstances;
   DWord_t               InstanceList[1];
} HDPM_Query_Remote_Device_Instances_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes     */
   /* that will be required to hold an entire query instances response  */
   /* message given the number of instances. This MACRO accepts as it's */
   /* input the total number of instances (NOT bytes) that are present  */
   /* in the InstanceList returns the total number of bytes required to */
   /* hold the entire message.                                          */
#define HDPM_QUERY_REMOTE_DEVICE_INSTANCES_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(HDPM_Query_Remote_Device_Instances_Response_t, InstanceList) + (unsigned int)((sizeof(DWord_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the endpoints    */
   /* supported by an instance on a remote device. (Request)            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ENDPOINTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Remote_Device_Endpoints_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
} HDPM_Query_Remote_Device_Endpoints_Request_t;

#define HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_REQUEST_SIZE        (sizeof(HDPM_Query_Remote_Device_Endpoints_Request_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the endpoints    */
   /* supported by and instance on a remote device. (Response)          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_REMOTE_DEVICE_ENDPOINTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Remote_Device_Endpoints_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          NumberEndpoints;
   HDPM_Endpoint_Info_t  EndpointList[1];
} HDPM_Query_Remote_Device_Endpoints_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes     */
   /* that will be required to hold an entire query endpoints response  */
   /* message given the number of endpoints. This MACRO accepts as it's */
   /* input the total number of endpoints (NOT bytes) that are present  */
   /* in the InstanceList returns the total number of bytes required to */
   /* hold the entire message.                                          */
#define HDPM_QUERY_REMOTE_DEVICE_ENDPOINTS_RESPONSE_SIZE(_x)   (STRUCTURE_OFFSET(HDPM_Query_Remote_Device_Endpoints_Response_t, EndpointList) + (unsigned int)((sizeof(HDPM_Endpoint_Info_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the description  */
   /* of an endpoint in an instance on a remote device. (Request)       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_ENDPOINT_DESCRIPTION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Endpoint_Description_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
   HDPM_Endpoint_Info_t  EndpointInfo;
} HDPM_Query_Endpoint_Description_Request_t;

#define HDPM_QUERY_ENDPOINT_DESCRIPTION_REQUEST_SIZE           (sizeof(HDPM_Query_Endpoint_Description_Request_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to query the description  */
   /* of an endpoint in an instance on a remote device. (Response)      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_QUERY_ENDPOINT_DESCRIPTION       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Query_Endpoint_Description_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          DescriptionLength;
   char                  DescriptionBuffer[1];
} HDPM_Query_Endpoint_Description_Response_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire query endpoint description     */
   /* response message given the number of bytes in the description     */
   /* string. This MACRO accepts as it's input the total number of bytes*/
   /* that are present in the DescriptionBuffer and returns the total   */
   /* number of bytes required to hold the entire message.              */
#define HDPM_QUERY_ENDPOINT_DESCRIPTION_RESPONSE_SIZE(_x)      (STRUCTURE_OFFSET(HDPM_Query_Endpoint_Description_Response_t, DescriptionBuffer) + (unsigned int)(_x))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to connect to an instance on*/
   /* a remote device. (Request)                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Connect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
} HDPM_Connect_Remote_Device_Request_t;

#define HDPM_CONNECT_REMOTE_DEVICE_REQUEST_SIZE                (sizeof(HDPM_Connect_Remote_Device_Request_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to connect to an instance on*/
   /* a remote device. (Response)                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Connect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Connect_Remote_Device_Response_t;

#define HDPM_CONNECT_REMOTE_DEVICE_RESPONSE_SIZE               (sizeof(HDPM_Connect_Remote_Device_Response_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to disconnect from an     */
   /* instance on a remote device. (Request)                            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Disconnect_Remote_Device_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
} HDPM_Disconnect_Remote_Device_Request_t;

#define HDPM_DISCONNECT_REMOTE_DEVICE_REQUEST_SIZE             (sizeof(HDPM_Disconnect_Remote_Device_Request_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to disconnect from an     */
   /* instance on a remote device. (Response)                           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Disconnect_Remote_Device_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Disconnect_Remote_Device_Response_t;

#define HDPM_DISCONNECT_REMOTE_DEVICE_RESPONSE_SIZE            (sizeof(HDPM_Disconnect_Remote_Device_Response_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to connect to an endpoint of*/
   /* an instance on a remote device. (Request)                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE_ENDPOINT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Connect_Remote_Device_Endpoint_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
   Byte_t                EndpointID;
   HDP_Channel_Mode_t    ChannelMode;
} HDPM_Connect_Remote_Device_Endpoint_Request_t;

#define HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE       (sizeof(HDPM_Connect_Remote_Device_Endpoint_Request_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message to connect to an endpoint of*/
   /* an instance on a remote device. (Response)                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_CONNECT_REMOTE_DEVICE_ENDPOINT   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Connect_Remote_Device_Endpoint_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Connect_Remote_Device_Endpoint_Response_t;

#define HDPM_CONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE      (sizeof(HDPM_Connect_Remote_Device_Endpoint_Response_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to disconnect from an     */
   /* endpoint of an instance on a remote device. (Request)             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE_ENDPOINT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Disconnect_Remote_Device_Endpoint_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataLinkID;
} HDPM_Disconnect_Remote_Device_Endpoint_Request_t;

#define HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_REQUEST_SIZE    (sizeof(HDPM_Disconnect_Remote_Device_Endpoint_Request_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to disconnect from an     */
   /* endpoint of an instance on a remote device. (Response)            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           HDPM_MESSAGE_FUNCTION_DISCONNECT_REMOTE_DEVICE_ENDPOINT */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Disconnect_Remote_Device_Endpoint_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Disconnect_Remote_Device_Endpoint_Response_t;

#define HDPM_DISCONNECT_REMOTE_DEVICE_ENDPOINT_RESPONSE_SIZE   (sizeof(HDPM_Disconnect_Remote_Device_Endpoint_Response_t))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to send data to to a      */
   /* connected endpoint of an instance on a remote device. (Request)   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_WRITE_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Write_Data_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          DataLinkID;
   unsigned int          DataLength;
   Byte_t                DataBuffer[1]; 
} HDPM_Write_Data_Request_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire write data request message     */
   /* given the amount of data to be writte. This MACRO accepts as it's */
   /* input the total number of bytes being written and returns the     */
   /* total number of bytes required to hold the entire message.        */
#define HDPM_WRITE_DATA_REQUEST_SIZE(_x)                       (STRUCTURE_OFFSET(HDPM_Write_Data_Request_t, DataBuffer) + (unsigned int)((sizeof(Byte_t)*((unsigned int)(_x)))))

   /* The following structure represents the message definition for     */
   /* a Health Device Profile Manager message to send data to to a      */
   /* connected endpoint of an instance on a remote device. (Response)  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_WRITE_DATA                      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Write_Data_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HDPM_Write_Data_Response_t;

#define HDPM_WRITE_DATA_RESPONSE_SIZE                     (sizeof(HDPM_Write_Data_Response_t))


   /* Health Device Profile Manager Asynchronous Message Formats.       */

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client     */
   /* that a connection to a remote HDP device has been established.    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_CONNECTION_STATUS               */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
   int                   Status;
} HDPM_Connection_Status_Message_t;

#define HDPM_CONNECTION_STATUS_MESSAGE_SIZE                    (sizeof(HDPM_Connection_Status_Message_t))

   /* The following constants are used with the Status member of the    */
   /* HDPM_Connection_Status_Message_t and                              */
   /* HDPM_Data_Connection_Status_Message_t messages to describe the    */
   /* actual connection result status.                                  */
#define HDPM_CONNECTION_STATUS_SUCCESS                          0x00000000
#define HDPM_CONNECTION_STATUS_FAILURE_TIMEOUT                  0x00000001
#define HDPM_CONNECTION_STATUS_FAILURE_REFUSED                  0x00000002
#define HDPM_CONNECTION_STATUS_FAILURE_CONNECTION_TERMINATED    0x00000003
#define HDPM_CONNECTION_STATUS_FAILURE_CONFIGURATION            0x00000004
#define HDPM_CONNECTION_STATUS_FAILURE_INVALID_INSTANCE         0x00000005
#define HDPM_CONNECTION_STATUS_FAILURE_DEVICE_POWER_OFF         0x00000006
#define HDPM_CONNECTION_STATUS_FAILURE_ABORTED                  0x00000007
#define HDPM_CONNECTION_STATUS_FAILURE_UNKNOWN                  0x00000008

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client     */
   /* that a connection to a remote HDP device has been disconnected.   */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HDPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               Instance;
} HDPM_Disconnected_Message_t;

#define HDPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(HDPM_Disconnected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client     */
   /* that a connection request for a local endpoint has been received. */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_INCOMING_DATA_CONNECTION_REQUEST */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Incoming_Data_Connection_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          EndpointID;
   HDP_Channel_Mode_t    ChannelMode;
   unsigned int          DataLinkID;
} HDPM_Incoming_Data_Connection_Message_t;

#define HDPM_INCOMING_DATA_CONNECTION_MESSAGE_SIZE             (sizeof(HDPM_Incoming_Data_Connection_Message_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client     */
   /* that a data channel to a local endpoint has been established.     */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_CONNECTED                   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Data_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          EndpointID;
   unsigned int          DataLinkID;
} HDPM_Data_Connected_Message_t;

#define HDPM_DATA_CONNECTED_MESSAGE_SIZE                       (sizeof(HDPM_Data_Connected_Message_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client     */
   /* that a data channel to a local endpoint has been disconnected.    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_DISCONNECTED                */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Data_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          DataLinkID;
   unsigned int          Reason;
} HDPM_Data_Disconnected_Message_t;

#define HDPM_DATA_DISCONNECTED_MESSAGE_SIZE                    (sizeof(HDPM_Data_Disconnected_Message_t))

   /* The following constants are used with the Status member of the    */
   /* HDPM_Connection_Status_Message_t message to describe the actual   */
   /* connection result status.                                         */
#define HDPM_DATA_DISCONNECT_REASON_NORMAL_DISCONNECT          0x00000000
#define HDPM_DATA_DISCONNECT_REASON_ABORTED                    0x00000001

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client of  */
   /* the status of a pending connection attempt to an endpoint of an   */
   /* instance of a remote device. (asynchronously).                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_CONNECTION_STATUS           */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The value of the Status member of this structure is      */
   /*          specified by the HDPM_CONNECTION_STATUS_* constants.     */
typedef struct _tagHDPM_Data_Connection_Status_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          Instance;
   unsigned int          EndpointID;
   unsigned int          DataLinkID;
   int                   Status;
} HDPM_Data_Connection_Status_Message_t;

#define HDPM_DATA_CONNECTION_STATUS_MESSAGE_SIZE               (sizeof(HDPM_Data_Connection_Status_Message_t))

   /* The following structure represents the message definition for a   */
   /* Health Device Profile Manager message that informs the client that*/
   /* data was received from a connected endpoint. (asynchronously).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*            HDPM_MESSAGE_FUNCTION_DATA_RECEIVED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHDPM_Data_Received_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          DataLinkID;
   Word_t                DataLength;
   Byte_t                Data[1];
} HDPM_Data_Received_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of bytes that*/
   /* will be required to hold an entire Data Connection Status message */
   /* given the amount of data received. This function accepts as it's  */
   /* input the size of the Data field, in bytes, and returns the total */
   /* number of bytes required to hold the entire message.              */
#define HDPM_DATA_RECEIVED_MESSAGE_SIZE(_x)                    (STRUCTURE_OFFSET(HDPM_Data_Received_Message_t, Data) + ((unsigned int)(_x)))

#endif
