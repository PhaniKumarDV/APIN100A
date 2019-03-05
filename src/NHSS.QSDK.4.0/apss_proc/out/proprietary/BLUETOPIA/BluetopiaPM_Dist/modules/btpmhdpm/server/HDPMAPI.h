/*****< hdpmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HDPMAPI - Health Device Profile Manager API for Stonestreet One Bluetooth */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Greg Hensley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/13  G. Hensley     Initial creation.                               */
/******************************************************************************/
#ifndef __HDPMAPIH__
#define __HDPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HDPMMSG.h"             /* BTPM HDP Manager Message Formats.         */

#include "MCAPType.h"            /* MCAP Prototypes/Constants.                */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Health Device Profile Module  */
   /* is initialized.                                                   */
typedef struct _tagHDPM_Initialization_Info_t
{
   char *ServiceName;
   char *ProviderName;
} HDPM_Initialization_Info_t;

#define HDPM_INITIALIZATION_INFO_SIZE                          (sizeof(HDPM_Initialization_Info_t))

   /* The following enumerated type represents the Health Device Profile*/
   /* Manager Event Types that are dispatched by this module.           */
typedef enum
{
   hetHDPConnectionStatus,
   hetHDPDisconnected,
   hetHDPIncomingDataConnectionRequest,
   hetHDPDataConnected,
   hetHDPDataDisconnected,
   hetHDPDataConnectionStatus,
   hetHDPDataReceived,
} HDPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPConnectionStatus event.   */
typedef struct _tagHDPM_Connection_Status_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   DWord_t   Instance;
   int       Status;
} HDPM_Connection_Status_Event_Data_t;

#define HDPM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(HDPM_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPDisconnected event.       */
typedef struct _tagHDPM_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   DWord_t   Instance;
} HDPM_Disconnected_Event_Data_t;

#define HDPM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(HDPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* hetHDPIncomingDataConnectionRequest event.                        */
typedef struct _tagHDPM_Incoming_Data_Connection_Request_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   unsigned int       EndpointID;
   HDP_Channel_Mode_t ChannelMode;
   unsigned int       DataLinkID;
} HDPM_Incoming_Data_Connection_Request_Event_Data_t;

#define HDPM_INCOMING_DATA_CONNECTION_REQUEST_EVENT_DATA_SIZE  (sizeof(HDPM_Incoming_Data_Connection_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPDataConnected event.      */
typedef struct _tagHDPM_Data_Connected_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int EndpointID;
   unsigned int DataLinkID;
} HDPM_Data_Connected_Event_Data_t;

#define HDPM_DATA_CONNECTED_EVENT_DATA_SIZE                    (sizeof(HDPM_Data_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPDataDisconnected event.   */
typedef struct _tagHDPM_Data_Disconnected_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int DataLinkID;
   unsigned int Reason;
} HDPM_Data_Disconnected_Event_Data_t;

#define HDPM_DATA_DISCONNECTED_EVENT_DATA_SIZE                 (sizeof(HDPM_Data_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPDataConnectionStatus      */
   /* event.                                                            */
typedef struct _tagHDPM_Data_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Instance;
   unsigned int EndpointID;
   unsigned int DataLinkID;
   int          Status;
} HDPM_Data_Connection_Status_Event_Data_t;

#define HDPM_DATA_CONNECTION_STATUS_EVENT_DATA_SIZE            (sizeof(HDPM_Data_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHDPDataReceived event.       */
typedef struct _tagHDPM_Data_Received_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int DataLinkID;
   Word_t       DataLength;
   Byte_t      *Data;
} HDPM_Data_Received_Event_Data_t;

#define HDPM_DATA_RECEIVED_EVENT_DATA_SIZE                     (sizeof(HDPM_Data_Received_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Health Device Profile Manager Event (and Event Data) of a Health  */
   /* Device Profile Manager Event.                                     */
typedef struct _tagHDPM_Event_Data_t
{
   HDPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HDPM_Connection_Status_Event_Data_t                ConnectionStatusEventData;
      HDPM_Disconnected_Event_Data_t                     DisconnectedEventData;
      HDPM_Incoming_Data_Connection_Request_Event_Data_t IncomingDataConnectionRequestEventData;
      HDPM_Data_Connected_Event_Data_t                   DataConnectedEventData;
      HDPM_Data_Disconnected_Event_Data_t                DataDisconnectedEventData;
      HDPM_Data_Connection_Status_Event_Data_t           DataConnectionStatusEventData;
      HDPM_Data_Received_Event_Data_t                    DataReceivedEventData;
   } EventData;
} HDPM_Event_Data_t;

#define HDPM_EVENT_DATA_SIZE                                   (sizeof(HDPM_Event_Data_t))

   /* The following declared type represents the Prototype Function     */
   /* for an Event Callback. This function will be called whenever the  */
   /* Health Device Profile Manager dispatches an event (and the client */
   /* has registered for events). This function passes to the caller the*/
   /* Health Device Profile Manager Event and the Callback Parameter    */
   /* that was specified when this Callback was installed. The caller   */
   /* is free to use the contents of the Event Data ONLY in the context */
   /* of this callback. If the caller requires the Data for a longer    */
   /* period of time, then the callback function MUST copy the data     */
   /* into another Data Buffer. This function is guaranteed NOT to be   */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e. this function DOES NOT have be reentrant). Because */
   /* of this, the processing in this function should be as efficient   */
   /* as possible. It should also be noted that this function is called */
   /* in the Thread Context of a Thread that the User does NOT own.     */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Message will */
   /* not be processed while this function call is outstanding).        */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *HDPM_Event_Callback_t)(HDPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HDP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
BTPSAPI_DECLARATION void BTPSAPI HDPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
BTPSAPI_DECLARATION void BTPSAPI HDPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register an Endpoint on the Local HDP Server. The first*/
   /* parameter defines the Data Type that will be supported by this    */
   /* endpoint. The second parameter specifies whether the Endpoint     */
   /* will be a data source or sink. The third parameter is optional    */
   /* and can be used to specify a short, human-readable description of */
   /* the Endpoint. The final parameters specify the Event Callback and */
   /* Callback parameter (to receive events related to the registered   */
   /* endpoint). This function returns a positive, non-zero, value if   */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Endpoint ID that*/
   /*          can be used with various functions in this module to     */
   /*          refer to this endpoint.                                  */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Register_Endpoint(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description, HDPM_Event_Callback_t EventCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Register_Endpoint_t)(Word_t DataType, HDP_Device_Role_t LocalRole, char *Description, HDPM_Event_Callback_t EventCallback, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Endpoint. This     */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Un_Register_Endpoint(unsigned int EndpointID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Un_Register_Endpoint_t)(unsigned int EndpointID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to establish a data connection to a*/
   /* local endpoint. The first parameter is the DataLinkID associated  */
   /* with the connection request. The second parameter is one of       */
   /* the MCAP_RESPONSE_CODE_* constants which indicates either that    */
   /* the request should be accepted (MCAP_RESPONSE_CODE_SUCCESS) or    */
   /* provides a reason for rejecting the request. If the request is to */
   /* be accepted, and the request is for a local Data Source, the final*/
   /* parameter indicates whether the connection shall use the Reliable */
   /* or Streaming communication mode. This function returns zero if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Data Connected  */
   /*          event will be dispatched to signify the actual result.   */
   /* * NOTE * If the connection is accepted, and the connection request*/
   /*          is for a local Data Sink, then ChannelMode must be set to*/
   /*          the Mode indicated in the request.  If the connection is */
   /*          accepted for a local Data Source, ChannelMode must be set*/
   /*          to either cmReliable or cmStreaming. If the connection   */
   /*          request is rejected, ChannelMode is ignored.             */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Data_Connection_Request_Response(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Data_Connection_Request_Response_t)(unsigned int DataLinkID, Byte_t ResponseCode, HDP_Channel_Mode_t ChannelMode);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the available HDP Instances on a remote    */
   /* device. The first parameter specifies the Address of the Remote   */
   /* Device to query. The second parameter specifies the maximum       */
   /* number of Instances that the buffer will support (i.e. can be     */
   /* copied into the buffer). The next parameter is optional and,      */
   /* if specified, will be populated with up to the total number of    */
   /* Instances advertised by the remote device, if the function is     */
   /* successful. The final parameter is optional and can be used to    */
   /* retrieve the total number of available Instances (regardless of   */
   /* the size of the list specified by the first two parameters).      */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Instances that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Instance  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Query_Remote_Device_Instances(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Query_Remote_Device_Instances_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int MaximumInstanceListEntries, DWord_t *InstanceList, unsigned int *TotalNumberInstances);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the available Endpoints published for a specific */
   /* HDP Instances on a remote device. The first parameter specifies   */
   /* the Address of the Remote Device to query. The second parameter   */
   /* specifies Instance on the Remote Device. The third parameter      */
   /* specifies the maximum number of Endpoints that the buffer will    */
   /* support (i.e. can be copied into the buffer). The next parameter  */
   /* is optional and, if specified, will be populated with up to the   */
   /* total number of Endpoints published by the remote device, if the  */
   /* function is successful. The final parameter is optional and can   */
   /* be used to retrieve the total number of Endpoints (regardless     */
   /* of the size of the list specified by the first two parameters).   */
   /* This function returns a non-negative value if successful which    */
   /* represents the number of Endpoints that were copied into the      */
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum Endpoint  */
   /*          List Entries, in which case the final parameter *MUST* be*/
   /*          specified.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Query_Remote_Device_Instance_Endpoints(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Query_Remote_Device_Instance_Endpoints_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, unsigned int MaximumEndpointListEntries, HDPM_Endpoint_Info_t *EndpointInfoList, unsigned int *TotalNumberEndpoints);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the description of a known Endpoint published in */
   /* a specific HDP Instance by a remote device. The first parameter   */
   /* specifies the Address of the Remote Device to query. The second   */
   /* parameter specifies Instance on the Remote Device. The third      */
   /* parameter identifies the Endpoint to query. The fourth and fifth  */
   /* parameters specific the size of the buffer and the buffer to hold */
   /* the description string, respectively. The final parameter is      */
   /* optional and, if specified, will be set to the total size of the  */
   /* description string for the given Endpoint, if the function is     */
   /* successful (regardless of the size of the list specified by the   */
   /* first two parameters). This function returns a non-negative value */
   /* if successful which represents the number of bytes copied into the*/
   /* specified buffer. This function returns a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * It is possible to specify zero for the Maximum           */
   /*          Description Length, in which case the final parameter    */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Query_Remote_Device_Endpoint_Description(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Query_Remote_Device_Endpoint_Description_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Endpoint_Info_t *EndpointInfo, unsigned int MaximumDescriptionLength, char *DescriptionBuffer, unsigned int *TotalDescriptionLength);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to establish a connection to a specific HDP Instance on a */
   /* Remote Device. The first parameter specifies the Remote Device to */
   /* connect to. The second parameter specifies the HDP Instance on the*/
   /* remote device. The next two parameters specify the (optional)     */
   /* Event Callback and Callback parameter (to receive events related  */
   /* to the connection attempt). This function returns zero if         */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Connect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an existing connection to a specific HDP Instance*/
   /* on a Remote Device. The first parameter specifies the Remote      */
   /* Device. The second parameter specifies the HDP Instance on the    */
   /* remote device from which to disconnect. This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Disconnect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Disconnect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to establish an HDP connection to an Endpoint of    */
   /* a specific HDP Instance on a Remote Device. The first parameter   */
   /* specifies the Remote Device to connect to. The second parameter   */
   /* specifies the HDP Instance on the remote device. The third        */
   /* parameter specifies the Endpoint of that Instance to which the    */
   /* connection will be attempted. The fourth parameter specifies      */
   /* the type of connection that will be established. The next two     */
   /* parameters specify the Event Callback and Callback parameter (to  */
   /* receive events related to the connection). This function returns a*/
   /* positive value if successful, or a negative return value if there */
   /* was an error.                                                     */
   /* * NOTE * A successful return value represents the Data Link ID    */
   /*          shall be used with various functions and by various      */
   /*          events in this module to reference this data connection. */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Connect_Remote_Device_Endpoint(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Connect_Remote_Device_Endpoint_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t Instance, Byte_t EndpointID, HDP_Channel_Mode_t ChannelMode, HDPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to disconnect an established HDP data connection.   */
   /* This function accepts the Data Link ID of the data connection     */
   /* to disconnect. This function returns zero if successful, or a     */
   /* negative return value if there was an error.                      */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Disconnect_Remote_Device_Endpoint(unsigned int DataLinkID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Disconnect_Remote_Device_Endpoint_t)(unsigned int DataLinkID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to send data over an established HDP data connection. The */
   /* first parameter is the Data Link ID which represents the data     */
   /* connection to use. The final parameters specify the data (and     */
   /* amount) to be sent. This function returns zero if successful, or a*/
   /* negative return error code if there was an error.                 */
   /* * NOTE * This function will either send all of the data or none of*/
   /*          the data.                                                */
BTPSAPI_DECLARATION int BTPSAPI HDPM_Write_Data(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDPM_Write_Data_t)(unsigned int DataLinkID, unsigned int DataLength, Byte_t *DataBuffer);
#endif

#endif
