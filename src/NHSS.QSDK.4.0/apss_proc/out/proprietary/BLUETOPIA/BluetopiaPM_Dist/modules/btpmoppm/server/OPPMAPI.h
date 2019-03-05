/*****< oppmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPMAPI - Object Push Profile API for Stonestreet One Bluetooth           */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/02/13  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __OPPMAPIH__
#define __OPPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "OPPMMSG.h"             /* Object Push IPC Message Definitions.      */

   /* The following enumerated type represents the Object Push Profile  */
   /* Manager (OPPM) event types that are dispatched by the module.     */
typedef enum
{
   oetIncomingConnectionRequest,
   oetConnected,
   oetDisconnected,
   oetConnectionStatus,
   oetPushObjectRequest,
   oetPushObjectResponse,
   oetPullBusinessCardRequest,
   oetPullBusinessCardResponse
} OPPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetIncomingConnectionRequest event.     */
typedef struct _tagOPPM_Incoming_Connection_Request_Event_Data_t
{
   unsigned int ServerPortID;
   BD_ADDR_t    RemoteDeviceAddress;
} OPPM_Incoming_Connection_Request_Event_Data_t;

#define OPPM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(OPPM_Incoming_Connection_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetConnected event.                     */
typedef struct _tagOPPM_Connected_Event_Data_t
{
   unsigned int ServerPortID;
   BD_ADDR_t    RemoteDeviceAddress;
} OPPM_Connected_Event_Data_t;

#define OPPM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(OPPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetDisconnected event.                  */
typedef struct _tagOPPM_Disconnected_Event_Data_t
{
   unsigned int PortID;
   BD_ADDR_t    RemoteDeviceAddress;
} OPPM_Disconnected_Event_Data_t;

#define OPPM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(OPPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetConnectionStatus event.              */
typedef struct _tagOPPM_Connection_Status_Event_Data_t
{
   unsigned int ClientPortID;
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Status;
} OPPM_Connection_Status_Event_Data_t;

#define OPPM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(OPPM_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetPushObjectRequestEvent event. Note,  */
   /* the ObjectName is a UTF-8 encoded string.                         */
typedef struct _tagOPPM_Push_Object_Request_Event_Data_t
{
   unsigned int        ServerPortID;
   BD_ADDR_t           RemoteDeviceAddress;
   OPPM_Object_Type_t  ObjectType;
   char               *ObjectName;
   unsigned long       ObjectTotalLength;
   Boolean_t           Final;
   unsigned int        DataLength;
   Byte_t             *DataBuffer;
} OPPM_Push_Object_Request_Event_Data_t;

#define OPPM_PUSH_OBJECT_REQUEST_EVENT_DATA_SIZE               (sizeof(OPPM_Push_Object_Request_Event_Data_t))

   /* The following structure is a container structure that holds       */
   /* the information returned in a oetPushObjectResponse event. The    */
   /* ResponseCode is a defined OPPM Response Status Code.              */
typedef struct _tagOPPM_Push_Object_Response_Event_Data_t
{
   unsigned int ClientPortID;
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ResponseCode;
} OPPM_Push_Object_Response_Event_Data_t;

#define OPPM_PUSH_OBJECT_RESPONSE_EVENT_DATA_SIZE              (sizeof(OPPM_Push_Object_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetPullBusinessCardRequest event.       */
typedef struct _tagOPPM_Pull_Business_Card_Request_Event_Data_t
{
   unsigned int ServerPortID;
   BD_ADDR_t    RemoteDeviceAddress;
} OPPM_Pull_Business_Card_Request_Event_Data_t;

#define OPPM_PULL_BUSINESS_CARD_REQUEST_EVENT_DATA_SIZE        (sizeof(OPPM_Pull_Business_Card_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information returned in a oetPullBusinessCardResponse event.  The */
   /* ResponseCode member is an OPPM Response Status Code.              */
typedef struct _tagOPPM_Pull_Business_Card_Response_Event_Data_t
{
   unsigned int   ClientPortID;
   BD_ADDR_t      RemoteDeviceAddress;
   unsigned int   ResponseCode;
   unsigned long  ObjectTotalLength;
   Boolean_t      Final;
   unsigned int   DataLength;
   Byte_t        *DataBuffer;
} OPPM_Pull_Business_Card_Response_Event_Data_t;

#define OPPM_PULL_BUSINESS_CARD_RESPONSE_EVENT_DATA_SIZE       (sizeof(OPPM_Pull_Business_Card_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Object Push Manager Event (and Event Data) of an Object Push      */
   /* Manager Event.                                                    */
typedef struct _tagOPPM_Event_Data_t
{
   OPPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      OPPM_Incoming_Connection_Request_Event_Data_t IncomingConnectionRequestEventData;
      OPPM_Connected_Event_Data_t                   ConnectedEventData;
      OPPM_Disconnected_Event_Data_t                DisconnectedEventData;
      OPPM_Connection_Status_Event_Data_t           ConnectionStatusEventData;
      OPPM_Push_Object_Request_Event_Data_t         PushObjectRequestEventData;
      OPPM_Push_Object_Response_Event_Data_t        PushObjectResponseEventData;
      OPPM_Pull_Business_Card_Request_Event_Data_t  PullBusinessCardRequestEventData;
      OPPM_Pull_Business_Card_Response_Event_Data_t PullBusinessCardResponseEventData;
   } EventData;
} OPPM_Event_Data_t;

#define OPPM_EVENT_DATA_SIZE                                   (sizeof(OPPM_Event_Data_t))

   /* The following declared type represents the prototype function for */
   /* an event and data callback. This function will be called whenever */
   /* the Object Push Manager dispatches an event. This function        */
   /* passes to the caller the Message Access Manager event and the     */
   /* callback parameter that was specified when this callback was      */
   /* installed. The caller is free to use the contents of the EventData*/
   /* ONLY in the context of this callback. If the caller requires the  */
   /* data for a longer period of time, then the callback function      */
   /* MUST copy the data into another data buffer. This function is     */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e. this function DOES NOT have    */
   /* be reentrant). Because of this, the processing in this function   */
   /* should be as efficient as possible. It should also be noted that  */
   /* this function is called in the thread context of a thread that the*/
   /* user does NOT own. Therefore, processing in this function should  */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another message will not be processed while this function call is */
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by receiving other Events.  A deadlock */
   /*          WILL occur because NO event callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *OPPM_Event_Callback_t)(OPPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning   */
   /* up the Bluetopia Platform Manager Opbject Push Profile (OPP)      */
   /* Manager (OPPM) module.  This function should be registered with   */
   /* the Bluetopia Platform Manager module handler and will be called  */
   /* when the Platform Manager is initialized (or shut down).          */
void BTPSAPI OPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI OPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming OPP connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A OPP Connected   */
   /*          event will be dispatched to signify the actual result.   */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Connection_Request_Response(unsigned int ServerPortID, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Connection_Request_Response_t)(unsigned int ServerPortID, Boolean_t Accept);
#endif

   /* The following function is provided to allows a mechanism for local*/
   /* modules to register an Object Push Server. This first parameter   */
   /* is the RFCOMM server port. If this parameter is zero, the server  */
   /* will be opened on an available port. The SupportedObjectTypes     */
   /* parameter is a bitmask representing the types of objects supported*/
   /* by this server. The IncomingConnectionFlags parameter is a        */
   /* bitmask which indicates whether incoming connections should       */
   /* be authorized, autenticated, or encrypted. The ServiceName        */
   /* parameter is a null-terminate string represting the name of the   */
   /* service to be placed in the Service Record. The EventCallback     */
   /* is the function which will receive events related to this         */
   /* server. The CallbackParameter will be included in each call to    */
   /* the CallbackFunction. This function returns a positive integer    */
   /* representing the ServerPortID of the created server if successful */
   /* and a negative error code if there was an error.                  */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Register_Server(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Register_Server_t)(unsigned int ServerPort, unsigned long SupportedObjectTypes, unsigned long IncomingConnectionFlags, char *ServiceName, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allows a mechanism for      */
   /* local modules to register an Object Push Server registered by a   */
   /* successful call to OPPM_Register_Server(). This function accepts  */
   /* as a parameter the ServerPortID returned from a successful call to*/
   /* OPPM_Register_Server(). This function returns zero if successful  */
   /* and a negative error code if there was an error.                  */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Un_Register_Server(unsigned int ServerPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Un_Register_Server_t)(unsigned int ServerPortID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the details of OPP services offered by a remote  */
   /* Object Push Server device. This function accepts the remote device*/
   /* address of the device whose SDP records will be parsed and the    */
   /* buffer which will hold the parsed service details. This function  */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * This function operates on the locally cached copy of the */
   /*          remote device's Service Records and will return an error */
   /*          if the cache is empty. For information on updating the   */
   /*          local cache, see DEVM_QueryRemoteDeviceServices().       */
   /* * NOTE * When this function is successful, the provided buffer    */
   /*          will be populated with the parsed service                */
   /*          details. This buffer MUST be passed to                   */
   /*          OPPM_Free_Object_Push_Service_Info() in order to release */
   /*          any resources that were allocated during the query       */
   /*          process.                                                 */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Parse_Remote_Object_Push_Services(BD_ADDR_t RemoteDeviceAddress, OPPM_Parsed_Service_Info_t *ServiceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Parse_Remote_Object_Push_Services_t)(BD_ADDR_t RemoteDeviceAddress, OPPM_Parsed_Service_Info_t *ServiceInfo);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to free all resources that were allocated to        */
   /* query the service details of a Object Push Server device. See     */
   /* the OPPM_Query_Remote_Object_Push_Services() function for more    */
   /* information.                                                      */
BTPSAPI_DECLARATION void BTPSAPI OPPM_Free_Parsed_Object_Push_Service_Info(OPPM_Parsed_Service_Info_t *ServiceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_OPPM_Free_Parsed_Object_Push_Service_Info_t)(OPPM_Parsed_Service_Info_t *ServiceInfo);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote Object Push Server device.  The    */
   /* RemoteDeviceAddress and RemoteServerPort parameter specify the    */
   /* connection information for the remote server.  The ConnectionFlags*/
   /* parameter specifies whether authentication or encryption should   */
   /* be used to create this connection.  The CallbackFunction is the   */
   /* function that will be registered to receive events for this       */
   /* connection.  The CallbackParameter is a parameter which will be   */
   /* included in the status callback.  This function returns a positive*/
   /* value representing the ClientPortID of this connection if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e.  the connection is completed).  */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Message Access Manager Event Callback supplied.          */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Connect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, unsigned int RemoteServerPort, unsigned long ConnectionFlags, OPPM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function exists to close an active Object Push      */
   /* connection that was previously opened by a successful call to     */
   /* OPPM_Connect_Remote_Device() function or by a oetConnected        */
   /* event. This function accpets the either the ClientPortID or       */
   /* ServerPortID of the connection as a parameter. This function      */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Disconnect(unsigned int PortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Disconnect_t)(unsigned int PortID);
#endif

   /* The following function is responsible for aborting ANY currently  */
   /* outstanding OPPM profile client request.  This function accepts as*/
   /* input the ClientPortID of the device specifying which connection  */
   /* is to have the Abort issued.  This function returns zero if       */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Abort(unsigned int ClientPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Abort_t)(unsigned int ClientPortID);
#endif

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter is */
   /* the ClientPortID of the remote device connection. The ObjectType  */
   /* parameter specifies the type of object being pushed. The Object   */
   /* Name parameter is a UTF-8 encoded string representing the name    */
   /* of the object to push. The DataLength and DataBuffer specify the  */
   /* length and contents of the object. This function returns zero if  */
   /* successful and a negative error code if there is an error.        */
   /* * NOTE * The Object Name is a pointer to a NULL Terminated UTF-8  */
   /*          String.                                                  */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Push_Object_Request(unsigned int ClientPortID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Push_Object_Request_t)(unsigned int ClientPortID, OPPM_Object_Type_t ObjectType, char *ObjectName, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);
#endif

   /* The following function is responsible for sending an Object       */
   /* Push Response to the remote Client.  The first parameter is the   */
   /* ServerPortID of the local Object Push server. The ResponseCode    */
   /* parameter is the OPPM Response Status code associated with this   */
   /* response. The function returns zero if successful and a negative  */
   /* error code if there is an error.                                  */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Push_Object_Response(unsigned int ServerPortID, unsigned int ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Push_Object_Response_t)(unsigned int ServerPortID, unsigned int ResponseCode);
#endif

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The Client        */
   /* parameter is the ClientPortID of the remote Object Push server    */
   /* connection. This function returns zero if successful and a        */
   /* negative error code if there was an error.                        */
   /* * NOTE * There can only be one outstanding OPPM request active at */
   /*          any one time.  Because of this, another OPPM request     */
   /*          cannot be issued until either the current request is     */
   /*          aborted (by calling the OPPM_Abort() function) or        */
   /*          the current request is completed (this is signified      */
   /*          by receiving a confirmation event in the OPPM event      */
   /*          callback).                                               */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Pull_Business_Card_Request(unsigned int ClientPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Pull_Business_Card_Request_t)(unsigned int ClientPortID);
#endif

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter is the   */
   /* ServerPortID of the remote Object Push client. The ResponseCode   */
   /* parameter is the OPPM Response Status Code associated with the    */
   /* response. The DataLength and DataBuffer parameters contain the    */
   /* business card data to be sent. This function returns zero if      */
   /* successful and a negative return error code if there is an error. */
   /* * NOTE * If the Final parameter is TRUE, then an End-of-Body      */
   /*          header will be placed in the final packet.  If FALSE,    */
   /*          then this function must be called again with the Final   */
   /*          flag TRUE in order for the response to be completed.     */
BTPSAPI_DECLARATION int BTPSAPI OPPM_Pull_Business_Card_Response(unsigned int ServerPortID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPPM_Pull_Business_Card_Response_t)(unsigned int ServerPortID, unsigned int ResponseCode, unsigned long ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, Boolean_t Final);
#endif

#endif
