/*****< hidmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HIDMAPI - Human Interface Device (HID) Manager API for Stonestreet One    */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __HIDMAPIH__
#define __HIDMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HIDMType.h"            /* BTPM HID Manager Type Definitions.        */

#include "HIDMMSG.h"             /* BTPM HID Manager Message Formats.         */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the HID Module is initialized.    */
typedef struct _tagHIDM_Initialization_Data_t
{
   unsigned long IncomingConnectionFlags;
} HIDM_Initialization_Data_t;

#define HIDM_INITIALIZATION_DATA_SIZE                          (sizeof(HIDM_Initialization_Data_t))

   /* The following enumerated type represents the HID Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of HID Manager Changes.                                           */
typedef enum
{
   hetHIDDeviceConnectionRequest,
   hetHIDDeviceConnected,
   hetHIDDeviceConnectionStatus,
   hetHIDDeviceDisconnected,
   hetHIDBootKeyboardKeyPress,
   hetHIDBootKeyboardKeyRepeat,
   hetHIDBootMouseEvent,
   hetHIDReportDataReceived,
   hetHIDGetReportResponse,
   hetHIDSetReportResponse,
   hetHIDGetProtocolResponse,
   hetHIDSetProtocolResponse,
   hetHIDGetIdleResponse,
   hetHIDSetIdleResponse,
} HIDM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDDeviceConnectionRequest   */
   /* event.                                                            */
typedef struct _tagHIDM_HID_Device_Connection_Request_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HIDM_HID_Device_Connection_Request_Event_Data_t;

#define HIDM_HID_DEVICE_CONNECTION_REQUEST_EVENT_DATA_SIZE     (sizeof(HIDM_HID_Device_Connection_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDDeviceConnected event.    */
typedef struct _tagHIDM_HID_Device_Connected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HIDM_HID_Device_Connected_Event_Data_t;

#define HIDM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE              (sizeof(HIDM_HID_Device_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDDeviceConnectionStatus    */
   /* event.                                                            */
typedef struct _tagHID_HID_Device_Connection_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ConnectionStatus;
} HIDM_HID_Device_Connection_Status_Event_Data_t;

#define HIDM_HID_DEVICE_CONNECTION_STATUS_EVENT_DATA_SIZE      (sizeof(HIDM_HID_Device_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDDeviceDisconnected event. */
typedef struct _tagHIDM_HID_Device_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HIDM_HID_Device_Disconnected_Event_Data_t;

#define HIDM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE           (sizeof(HIDM_HID_Device_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDBootKeyboardKeyPress      */
   /* event.                                                            */
typedef struct _tagHIDM_HID_Boot_Keyboard_Key_Press_Event_Data_t
{
   unsigned int HIDManagerDataCallbackID;
   BD_ADDR_t    RemoteDeviceAddress;
   Boolean_t    KeyDown;
   Byte_t       KeyModifiers;
   Byte_t       Key;
} HIDM_HID_Boot_Keyboard_Key_Press_Event_Data_t;

#define HIDM_HID_BOOT_KEYBOARD_KEY_PRESS_EVENT_DATA_SIZE       (sizeof(HIDM_HID_Boot_Keyboard_Key_Press_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDBootKeyboardKeyRepeat     */
   /* event.                                                            */
typedef struct _tagHIDM_HID_Boot_Keyboard_Key_Repeat_Event_Data_t
{
   unsigned int HIDManagerDataCallbackID;
   BD_ADDR_t    RemoteDeviceAddress;
   Byte_t       KeyModifiers;
   Byte_t       Key;
} HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Data_t;

#define HIDM_HID_BOOT_KEYBOARD_KEY_REPEAT_EVENT_DATA_SIZE      (sizeof(HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDBootMouseEvent event.     */
typedef struct _tagHIDM_HID_Boot_Mouse_Event_Event_Data_t
{
   unsigned int HIDManagerDataCallbackID;
   BD_ADDR_t    RemoteDeviceAddress;
   SByte_t      CX;
   SByte_t      CY;
   Byte_t       ButtonState;
   SByte_t      CZ;
} HIDM_HID_Boot_Mouse_Event_Event_Data_t;

#define HIDM_HID_BOOT_MOUSE_EVENT_EVENT_DATA_SIZE              (sizeof(HIDM_HID_Boot_Mouse_Event_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDReportDataReceived event. */
typedef struct _tagHIDM_HID_Report_Data_Received_Event_Data_t
{
   unsigned int  HIDManagerDataCallbackID;
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  ReportLength;
   Byte_t       *ReportData;
} HIDM_HID_Report_Data_Received_Event_Data_t;

#define HIDM_HID_REPORT_DATA_RECEIVED_EVENT_DATA_SIZE          (sizeof(HIDM_HID_Report_Data_Received_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDGetReportResponse event.  */
typedef struct _tagHIDM_HID_Get_Report_Response_Data_t
{
   unsigned int        HIDManagerDataCallbackID;
   BD_ADDR_t           RemoteDeviceAddress;
   HIDM_Result_t       Status;
   HIDM_Report_Type_t  ReportType;
   Word_t              ReportLength;
   Byte_t             *ReportData;
} HIDM_HID_Get_Report_Response_Data_t;

#define HIDM_HID_GET_REPORT_RESPONSE_DATA_SIZE                 (sizeof(HIDM_HID_Get_Report_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDSetReportResponse event.  */
typedef struct _tagHIDM_HID_Set_Report_Response_Data_t
{
   unsigned int  HIDManagerDataCallbackID;
   BD_ADDR_t     RemoteDeviceAddress;
   HIDM_Result_t Status;
} HIDM_HID_Set_Report_Response_Data_t;

#define HIDM_HID_SET_REPORT_RESPONSE_DATA_SIZE                 (sizeof(HIDM_HID_Set_Report_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDGetProtocolResponse event.*/
typedef struct _tagHIDM_HID_Get_Protocol_Response_Data_t
{
   unsigned int    HIDManagerDataCallbackID;
   BD_ADDR_t       RemoteDeviceAddress;
   HIDM_Result_t   Status;
   HIDM_Protocol_t Protocol;
} HIDM_HID_Get_Protocol_Response_Data_t;

#define HIDM_HID_GET_PROTOCOL_RESPONSE_DATA_SIZE               (sizeof(HIDM_HID_Get_Protocol_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDSetProtocolResponse event.*/
typedef struct _tagHIDM_HID_Set_Protocol_Response_Data_t
{
   unsigned int  HIDManagerDataCallbackID;
   BD_ADDR_t     RemoteDeviceAddress;
   HIDM_Result_t Status;
} HIDM_HID_Set_Protocol_Response_Data_t;

#define HIDM_HID_SET_PROTOCOL_RESPONSE_DATA_SIZE               (sizeof(HIDM_HID_Set_Protocol_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDGetIdleResponse event.    */
typedef struct _tagHIDM_HID_Get_Idle_Response_Data_t
{
   unsigned int  HIDManagerDataCallbackID;
   BD_ADDR_t     RemoteDeviceAddress;
   HIDM_Result_t Status;
   Byte_t        IdleRate;
} HIDM_HID_Get_Idle_Response_Data_t;

#define HIDM_HID_GET_IDLE_RESPONSE_DATA_SIZE                   (sizeof(HIDM_HID_Get_Idle_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHIDSetIdleResponse event.    */
typedef struct _tagHIDM_HID_Set_Idle_Response_Data_t
{
   unsigned int  HIDManagerDataCallbackID;
   BD_ADDR_t     RemoteDeviceAddress;
   HIDM_Result_t Status;
} HIDM_HID_Set_Idle_Response_Data_t;

#define HIDM_HID_SET_IDLE_RESPONSE_DATA_SIZE                   (sizeof(HIDM_HID_Set_Idle_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Human Interface Device (HID) Manager Event (and Event Data) of a  */
   /* HID Manager Event.                                                */
typedef struct _tagHIDM_Event_Data_t
{
   HIDM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HIDM_HID_Device_Connection_Request_Event_Data_t DeviceConnectionRequestData;
      HIDM_HID_Device_Connected_Event_Data_t          DeviceConnectedEventData;
      HIDM_HID_Device_Connection_Status_Event_Data_t  DeviceConnectionStatusEventData;
      HIDM_HID_Device_Disconnected_Event_Data_t       DeviceDisconnectedEventData;
      HIDM_HID_Boot_Keyboard_Key_Press_Event_Data_t   BootKeyboardKeyPressEventData;
      HIDM_HID_Boot_Keyboard_Key_Repeat_Event_Data_t  BootKeyboardKeyRepeatEventData;
      HIDM_HID_Boot_Mouse_Event_Event_Data_t          BootMouseEventEventData;
      HIDM_HID_Report_Data_Received_Event_Data_t      ReportDataReceivedEventData;
      HIDM_HID_Get_Report_Response_Data_t             GetReportResponseEventData;
      HIDM_HID_Set_Report_Response_Data_t             SetReportResponseEventData;
      HIDM_HID_Get_Protocol_Response_Data_t           GetProtocolResponseEventData;
      HIDM_HID_Set_Protocol_Response_Data_t           SetProtocolResponseEventData;
      HIDM_HID_Get_Idle_Response_Data_t               GetIdleResponseEventData;
      HIDM_HID_Set_Idle_Response_Data_t               SetIdleResponseEventData;
   } EventData;
} HIDM_Event_Data_t;

#define HIDM_EVENT_DATA_SIZE                                   (sizeof(HIDM_Event_Data_t))

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
typedef void (BTPSAPI *HIDM_Event_Callback_t)(HIDM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HID Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HIDM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HIDM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming HID connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A HID Connected   */
   /*          event will be dispatched to signify the actual result.   */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Connection_Request_Response_t)(BD_ADDR_t RemoteDeviceAddress, Boolean_t Accept, unsigned long ConnectionFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to connect to a remote HID device.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          HID Connection Status Event (if specified).              */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          hetHIDConnectionStatus event will be dispatched  to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the HIDM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Connect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long ConnectionFlags, HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* local device to disconnect a currently connected remote device    */
   /* (connected to the local device's HID Host).  This function accepts*/
   /* as input the Remote Device Address of the Remote HID Device to    */
   /* disconnect from the local HID Host followed by a BOOLEAN value    */
   /* that specifies whether or not the device is be disconnected via a */
   /* Virtual Cable Disconnection (TRUE), or merely disconnected at the */
   /* Bluetooth link (FALSE).  This function returns zero if successful,*/
   /* or a negative return error code if there was an error.            */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Disconnect_Device(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Disconnect_Device_t)(BD_ADDR_t RemoteDeviceAddress, Boolean_t SendVirtualCableDisconnect);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected HID     */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected HID devices.  The first parameter */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated with   */
   /* the total number of connected devices if the function is          */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns a  */
   /* non-negative value if successful which represents the number of   */
   /* connected devices that were copied into the specified input       */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Query_Connected_Devices_t)(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to change the incoming Connection Flags for HID Manager   */
   /* Connections.  This function returns zero if successful, or a      */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Change_Incoming_Connection_Flags_t)(unsigned long ConnectionFlags);
#endif

   /* The following function is used to set the HID Host Keyboard Key   */
   /* Repeat behavior.  Some Host operating systems nativity support Key*/
   /* Repeat automatically, but for those Host operating systems that do*/
   /* not - this function will instruct the HID Manager to simulate Key */
   /* Repeat behavior (with the specified parameters).  This function   */
   /* accepts the initial amount to delay (in milliseconds) before      */
   /* starting the repeat functionality.  The final parameter specifies */
   /* the rate of repeat (in milliseconds).  This function returns zero */
   /* if successful or a negative value if there was an error.          */
   /* * NOTE * Specifying zero for the Repeat Delay (first parameter)   */
   /*          will disable HID Manager Key Repeat processing.  This    */
   /*          means that only Key Up/Down events will be dispatched    */
   /*          and No Key Repeat events will be dispatched.             */
   /* * NOTE * The Key Repeat parameters can only be changed when there */
   /*          are no actively connected HID devices.                   */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Set_Keyboard_Repeat_Rate(unsigned int RepeatDelay, unsigned int RepeatRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Set_Keyboard_Repeat_Rate_t)(unsigned int RepeatDelay, unsigned int RepeatRate);
#endif

   /* The following function is responsible for sending the specified   */
   /* HID Report Data to a currently connected remote device.  This     */
   /* function accepts as input the HID Manager Report Data Handler ID  */
   /* (registered via call to the HIDM_Register_Data_Event_Callback()   */
   /* function), followed by the remote device address of the remote HID*/
   /* device to send the report data to, followed by the report data    */
   /* itself.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Report_Data(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Report_Data_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ReportDataLength, Byte_t *ReportData);
#endif

   /* The following function is responsible for Sending a GET_REPORT    */
   /* transaction to the remote device.  This function accepts as input */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report requested.  The    */
   /* fourth parameter is the Report ID determined by the Device's SDP  */
   /* record.  Passing HIDM_INVALID_REPORT_ID as the value for this     */
   /* parameter will indicate that this parameter is not used and will  */
   /* exclude the appropriate byte from the transaction payload.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Get_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Get_Report_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Byte_t ReportID);
#endif

   /* The following function is responsible for Sending a SET_REPORT    */
   /* request to the remote device.  This function accepts as input     */
   /* the HID Manager Report Data Handler ID (registered via call to    */
   /* the HIDM_Register_Data_Event_Callback() function) and the remote  */
   /* device address of the remote HID device to send the report data   */
   /* to.  The third parameter is the type of report being sent.  The   */
   /* final two parameters to this function are the Length of the Report*/
   /* Data to send and a pointer to the Report Data that will be sent.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Report Confirmation event */
   /*          indicates that a Response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Set_Report_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Set_Report_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Report_Type_t ReportType, Word_t ReportDataLength, Byte_t *ReportData);
#endif

   /* The following function is responsible for Sending a GET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Get_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Get_Protocol_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for Sending a SET_PROTOCOL  */
   /* transaction to the remote HID device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the protocol to be set.  This     */
   /* function returns a zero if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel Request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Protocol Confirmation     */
   /*          event indicates that a response has been received and the*/
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Set_Protocol_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Set_Protocol_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HIDM_Protocol_t Protocol);
#endif

   /* The following function is responsible for Sending a GET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via      */
   /* call to the HIDM_Register_Data_Event_Callback() function) and     */
   /* the remote device address of the remote HID device to send the    */
   /* report data to.  This function returns a zero if successful, or a */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Get Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Get_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Get_Idle_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is responsible for Sending a SET_IDLE      */
   /* transaction to the remote HID Device.  This function accepts as   */
   /* input the HID Manager Report Data Handler ID (registered via call */
   /* to the HIDM_Register_Data_Event_Callback() function) and the      */
   /* remote device address of the remote HID device to send the report */
   /* data to.  The last parameter is the Idle Rate to be set.  The Idle*/
   /* Rate LSB is weighted to 4ms (i.e. the Idle Rate resolution is 4ms */
   /* with a range from 4ms to 1.020s).  This function returns a zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Control Channel transfers have two phases, a Request by  */
   /*          the host and a Response by the device.  Only ONE host    */
   /*          control channel request shall be outstanding at a time.  */
   /*          Reception of a HID Manager Set Idle Confirmation event   */
   /*          indicates that a response has been received and the      */
   /*          Control Channel is now free for further Transactions.    */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Send_Set_Idle_Request(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Send_Set_Idle_Request_t)(unsigned int HIDManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Byte_t IdleRate);
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
   /*          HIDM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Register_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Register_Event_Callback_t)(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_RegisterEventCallback() function).  This function accepts as */
   /* input the HID Manager Event Callback ID (return value from        */
   /* HIDM_RegisterEventCallback() function).                           */
BTPSAPI_DECLARATION void BTPSAPI HIDM_Un_Register_Event_Callback(unsigned int HIDManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HIDM_Un_Register_Event_Callback_t)(unsigned int HIDManagerCallbackID);
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
   /*          HIDM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HIDM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HIDM_Register_Data_Event_Callback(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HIDM_Register_Data_Event_Callback_t)(HIDM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HID Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HIDM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HID Manager Data Event Callback ID (return   */
   /* value from HIDM_Register_Data_Event_Callback() function).         */
BTPSAPI_DECLARATION void BTPSAPI HIDM_Un_Register_Data_Event_Callback(unsigned int HIDManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HIDM_Un_Register_Data_Event_Callback_t)(unsigned int HIDManagerDataCallbackID);
#endif

#endif
