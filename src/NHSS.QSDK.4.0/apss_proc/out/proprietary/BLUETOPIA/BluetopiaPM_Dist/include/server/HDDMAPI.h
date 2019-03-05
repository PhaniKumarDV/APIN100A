/*****< hddmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMAPI - Human Interface Device (HID) Device Role Manager API for        */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/28/14  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __HDDMAPIH__
#define __HDDMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HDDMType.h"            /* BTPM HID Manager Type Definitions.        */

#include "HDDMMSG.h"             /* BTPM HID Manager Message Formats.         */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the HID Module is initialized.    */
typedef struct _tagHDDM_Initialization_Data_t
{
   unsigned long     IncomingConnectionFlags;
   unsigned int      DeviceReleaseNumber;
   unsigned long     DeviceFlags;
   unsigned int      ParserVersion;
   unsigned int      DeviceSubclass;
   unsigned int      NumberDescriptors;
   HID_Descriptor_t *DescriptorList;
   char             *ServiceName;
} HDDM_Initialization_Data_t;

#define HDDM_INITIALIZATION_DATA_SIZE                          (sizeof(HDDM_Initialization_Data_t))

   /* The following enumerated type represents the HID Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of HID Manager Changes.                                           */
typedef enum
{
   hetHDDConnectionRequest,
   hetHDDConnected,
   hetHDDConnectionStatus,
   hetHDDDisconnected,
   hetHDDControlEvent,
   hetHDDReportDataReceived,
   hetHDDGetReportRequest,
   hetHDDSetReportRequest,
   hetHDDGetProtocolRequest,
   hetHDDSetProtocolRequest,
   hetHDDGetIdleRequest,
   hetHDDSetIdleRequest
} HDDM_Event_Type_t;

   /* This structure defines the data returned in an                    */
   /* hetHDDConnectionRequest event.  The RemoteDeviceAddress is the    */
   /* Bluetooth Address of the remote HID Host.                         */
typedef struct _tagHDDM_Connection_Request_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDDM_Connection_Request_Event_Data_t;

#define HDDM_CONNECTION_REQUEST_EVENT_DATA_SIZE                (sizeof(HDDM_Connection_Request_Event_Data_t))

   /* This structure defines the data returned in an hetHDDConnected    */
   /* event.  The RemoteDeviceAddress is the Bluetooth Address of the   */
   /* remote HID Host.                                                  */
typedef struct _tagHDDM_Connected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDDM_Connected_Event_Data_t;

#define HDDM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(HDDM_Connected_Event_Data_t))

   /* This structure defines the data returned in an                    */
   /* hetHDDConnectionStatus event.  The RemoteDeviceAddress is the     */
   /* Bluetooth Address of the remote HID Host.  The Status member is   */
   /* the status of the outgoing connection request.                    */
typedef struct _tagHDDM_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionStatus;
} HDDM_Connection_Status_Event_Data_t;

#define HDDM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(HDDM_Connection_Status_Event_Data_t))

   /* This structure defines the data returned in an hetHDDDisconnected */
   /* event.  The RemoteDeviceAddress is the Bluetooth Address of the   */
   /* remote HID Host.                                                  */
typedef struct _tagHDDM_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDDM_Disconnected_Event_Data_t;

#define HDDM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(HDDM_Disconnected_Event_Data_t))

   /* This structure defines the data returned in an hetHDDControlEvent */
   /* event.  The RemoteDeviceAddress is the Bluetooth Address of the   */
   /* remote HID Host.  The ControlOperation is the operation being     */
   /* requested.                                                        */
typedef struct _tagHDDM_Control_Event_Data_t
{
   BD_ADDR_t                RemoteDeviceAddress;
   HDDM_Control_Operation_t ControlOperation;
} HDDM_Control_Event_Data_t;

#define HDDM_CONTROL_EVENT_DATA_SIZE                           (sizeof(HDDM_Control_Event_Data_t))

   /* This structure defines the data returned in an                    */
   /* hetHDDReportDataReceived event.  The RemoteDeviceAddress is the   */
   /* Bluetooth Address of the remote HID Host.  The ReportLength is the*/
   /* length of the report data.  ReportData is a pointer to the report */
   /* data buffer.                                                      */
typedef struct _tagHDDM_Report_Data_Received_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  ReportLength;
   Byte_t       *ReportData;
} HDDM_Report_Data_Received_Event_Data_t;

#define HDDM_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE              (sizeof(HDDM_Report_Data_Received_Event_Data_t))

   /* This structure defines the data returned in an                    */
   /* hetHDDGetReportRequest event.  The RemoteDeviceAddress is the     */
   /* Bluetooth Address of the remote HID Host.  The SizeType parameter */
   /* indicates whether to return the whole report (hgrSizeOfReport)    */
   /* or only the specified amount (hgrUseBufferSize).  The ReportType  */
   /* indicates the type of report being requested.  The ReportID       */
   /* specifed the indentifier of the report being requested.           */
   /* BufferSize is only valid of SizeType is hgrUseBufferSize. If so,  */
   /* it indicates the amount of report data to return.                 */
typedef struct _tagHDDM_Get_Report_Request_Event_Data_t
{
   BD_ADDR_t                   RemoteDeviceAddress;
   HDDM_Get_Report_Size_Type_t SizeType;
   HDDM_Report_Type_t          ReportType;
   unsigned int                ReportID;
   unsigned int                BufferSize;
} HDDM_Get_Report_Request_Event_Data_t;

#define HDDM_GET_REPORT_REQUEST_EVENT_DATA_SIZE                (sizeof(HDDM_Get_Report_Request_Event_Data_t))

   /* This structure defines the data returned in an                    */
   /* hetHDDSetReportRequest event.  The RemoteDeviceAddress is the     */
   /* Bluetooth Address of the remote HID Host.  The ReportType         */
   /* indicates the type of reported that is being set.  ReportLength   */
   /* indicates the size of the report data being set.  ReportData is a */
   /* pointer to the report data buffer.                                */
typedef struct _tagHDDM_Set_Report_Request_Event_Data_t
{
   BD_ADDR_t           RemoteDeviceAddress;
   HDDM_Report_Type_t  ReportType;
   unsigned int        ReportLength;
   Byte_t             *ReportData;
} HDDM_Set_Report_Request_Event_Data_t;

#define HDDM_SET_REPORT_REQUEST_EVENT_DATA_SIZE                (sizeof(HDDM_Set_Report_Request_Event_Data_t))

   /* This structure defines the data returned in an hetHDDGetProtocol  */
   /* event.  The RemoteDeviceAddress is the Bluetooth Address of the   */
   /* remote HID Host.                                                  */
typedef struct _tagHDDM_Get_Protocol_Request_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDDM_Get_Protocol_Request_Event_Data_t;

#define HDDM_GET_PROTOCOL_REQUEST_EVENT_DATA_SIZE              (sizeof(HDDM_Get_Protocol_Request_Event_Data_t))

   /* This structure defines the data returned in an hetHDDSetProtocol  */
   /* event.  The RemoteDeviceAddress is the Bluetooth Address of the   */
   /* remote HID Host.  The Protocol is the new protocol to set.        */
typedef struct _tagHDDM_Set_Protocol_Request_Event_Data_t
{
   BD_ADDR_t       RemoteDeviceAddress;
   HDDM_Protocol_t Protocol;
} HDDM_Set_Protocol_Request_Event_Data_t;

#define HDDM_SET_PROTOCOL_REQUEST_EVENT_DATA_SIZE              (sizeof(HDDM_Set_Protocol_Request_Event_Data_t))

   /* This structure defines the data returned in an                    */
   /* hetHDDGetIdleRequest event.  The RemoteDeviceAddress is the       */
   /* Bluetooth Address of the remote HID Host.                         */
typedef struct _tagHDDM_Get_Idle_Request_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HDDM_Get_Idle_Request_Event_Data_t;

#define HDDM_GET_IDLE_REQUEST_EVENT_DATA_SIZE                  (sizeof(HDDM_Get_Idle_Request_Event_Data_t))

   /* This structure defines the data returned in an hetHDD event.  The */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote HID    */
   /* Host.  The IdleRate indicates the new Idle Rate to set.           */
typedef struct _tagHDDM_Set_Idle_Request_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int IdleRate;
} HDDM_Set_Idle_Request_Event_Data_t;

#define HDDM_SET_IDLE_REQUEST_EVENT_DATA_SIZE                  (sizeof(HDDM_Set_Idle_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Human Interface Device (HID) Device Manager Event (and Event Data)*/
   /* of a HID Device Manager Event.                                    */
typedef struct _tagHDDM_Event_Data_t
{
   HDDM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HDDM_Connection_Request_Event_Data_t   ConnectionRequestEventData;
      HDDM_Connected_Event_Data_t            ConnectedEventData;
      HDDM_Connection_Status_Event_Data_t    ConnectionStatusEventData;
      HDDM_Disconnected_Event_Data_t         DisconnectedEventData;
      HDDM_Control_Event_Data_t              ControlEventData;
      HDDM_Report_Data_Received_Event_Data_t ReportDataReceivedEventData;
      HDDM_Get_Report_Request_Event_Data_t   GetReportRequestEventData;
      HDDM_Set_Report_Request_Event_Data_t   SetReportRequestEventData;
      HDDM_Get_Protocol_Request_Event_Data_t GetProtocolRequestEventData;
      HDDM_Set_Protocol_Request_Event_Data_t SetProtocolRequestEventData;
      HDDM_Get_Idle_Request_Event_Data_t     GetIdleRequestEventData;
      HDDM_Set_Idle_Request_Event_Data_t     SetIdleRequestEventData;
   } EventData;
} HDDM_Event_Data_t;

#define HDDM_EVENT_DATA_SIZE                                   (sizeof(HDDM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Human Interface Device (HID) Manager dispatches an event (and the */
   /* client has registered for events).  This function passes to the   */
   /* caller the HID Manager Event and the Callback Parameter that was  */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the Event Data ONLY in the context of this    */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  Because of      */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Message will */
   /* not be processed while this function call is outstanding).        */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *HDDM_Event_Callback_t)(HDDM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HDDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HDDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Accept or reject (authorize) an incoming HID connection from a    */
   /* remote HID Host.  This function returns zero if successful, or a  */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HDD Connected   */
   /*          event will be dispatched to signify the actual result.   */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Connection_Request_Response_t)(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept);
#endif

   /* Connect to a remote HID Host device.  The RemoteDeviceAddress is  */
   /* the Bluetooth Address of the remote HID Host.  The ConnectionFlags*/
   /* specifiy whay security, if any, is required for the connection.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHDDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HDDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Connect_Remote_Host(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Connect_Remote_Host_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* Disconnect from a remote HID Host.  The RemoteDeviceAddress       */
   /* is the Bluetooth Address of the remote HID Host.  The             */
   /* SendVirtualCableUnplug parameter indicates whether the device     */
   /* should be disconnected with a Virtual Cable Unplug (TRUE) or      */
   /* simply at the Bluetooth Link (FALSE).  This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Disconnect(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Disconnect_t)(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableUnplug);
#endif

   /* Determine if there are currently any connected HID Hosts.  This   */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently connected HID Hosts.  The first parameter specifies the */
   /* maximum number of BD_ADDR entries that the buffer will support    */
   /* (i.e. can be copied into the buffer).  The next parameter is      */
   /* optional and, if specified, will be populated with the total      */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Query_Connected_Hosts(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Query_Connected_Hosts_t)(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Change_Incoming_Connection_Flags_t)(unsigned long ConnectionFlags);
#endif

   /* Send the specified HID Report Data to a currently connected       */
   /* remote device.  This function accepts as input the HDD            */
   /* Manager Report Data Handler ID (registered via call to the        */
   /* HDDM_Register_Data_Event_Callback() function), followed by the    */
   /* remote device address of the remote HID Host to send the report   */
   /* data to, followed by the report data itself.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Send_Report_Data(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Send_Report_Data_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);
#endif

   /* Respond to a GetReportRequest.  The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* ReportType indicates the type of report being sent as the         */
   /* response.  The ReportDataLength indicates the size of the report  */
   /* data.  ReportData is a pointer to the report data buffer.  This   */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Get_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Get_Report_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Report_Type_t ReportType, unsigned int ReportDataLength, Byte_t *ReportData);
#endif

   /* Responsd to a SetReportRequest. The HDDManagerDataCallback        */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Set_Report_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Set_Report_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
#endif

   /* Respond to a GetProtocolRequest.  The HDDManagerDataCallback      */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* Protocol indicates the current HID Protocol.  This function       */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Get_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Get_Protocol_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, HDDM_Protocol_t Protocol);
#endif

   /* Respond to a SetProtocolResponse.  The HDDManagerDataCallback     */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Set_Protocol_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Set_Protocol_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
#endif

   /* Respond to a GetIdleResponse.  The HDDManagerDataCallback         */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  The        */
   /* IdleRate is the current Idle Rate.  This function returns zero if */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Get_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Get_Idle_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result, unsigned int IdleRate);
#endif

   /* Respond to a SetIdleRequest.  The HDDManagerDataCallback          */
   /* ID is the identifier returned via a successful call to            */
   /* HDDM_Register_Data_Event_Callback().  The RemoteDeviceAddress     */
   /* is the Bluetooth Address of the remote HID Host.  The Result      */
   /* parameter indicates the result status of the request.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Set_Idle_Response(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Set_Idle_Response_t)(unsigned int HDDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HDDM_Result_t Result);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service.  This Callback will be dispatched by*/
   /* the HID Manager when various HID Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HID Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HDDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Register_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Register_Event_Callback_t)(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HDDM_RegisterEventCallback() function).                           */
BTPSAPI_DECLARATION void BTPSAPI HDDM_Un_Register_Event_Callback(unsigned int HDDManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HDDM_Un_Register_Event_Callback_t)(unsigned int HDDManagerCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HID) Manager Service to explicitly process HID report     */
   /* data.  This Callback will be dispatched by the HID Manager when   */
   /* various HID Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HID Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HDDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HDDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HDDM_Register_Data_Event_Callback(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HDDM_Register_Data_Event_Callback_t)(HDDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HDDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HDDM_Register_Data_Event_Callback() function).         */
BTPSAPI_DECLARATION void BTPSAPI HDDM_Un_Register_Data_Event_Callback(unsigned int HDDManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HDDM_Un_Register_Data_Event_Callback_t)(unsigned int HDDManagerDataCallbackID);
#endif

#endif
