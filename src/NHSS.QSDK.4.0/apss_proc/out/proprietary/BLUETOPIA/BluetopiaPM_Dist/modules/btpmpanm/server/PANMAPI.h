/*****< panmapi.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PANMAPI - Personal Area Network (PAN) Manager API for Stonestreet One     */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author: Matt Seabold                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/28/11  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __PANMAPI__
#define __PANMAPI__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "PANMMSG.h"             /* BTPM PAN Manager Message Formats.         */

   /* The following structure is the structure that is used to hold     */
   /* naming information for SDP Records.                               */
typedef struct _tagPANM_Naming_Data_t
{
   char *ServiceName;
   char *ServiceDescription;
} PANM_Naming_Data_t;

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the PAN Module is initialized.    */
typedef struct _tagPANM_Initialization_Info_t
{
   unsigned long       ServiceTypeFlags;
   unsigned int        VirtualNetworkDriverIndex;
   PANM_Naming_Data_t *NamingData_User;
   PANM_Naming_Data_t *NamingData_AccessPoint;
   PANM_Naming_Data_t *NamingData_GroupAdHoc;
   unsigned int        NumberNetworkPacketTypes;
   Word_t             *NetworkPacketTypeList;
   Word_t              SecurityDescription;
   Word_t              NetworkAccessType;
   DWord_t             MaxNetAccessRate;
   unsigned long       IncomingConnectionFlags;
} PANM_Initialization_Info_t;

#define PANM_INITIALIZATION_INFO_SIZE                          (sizeof(PANM_Initialization_Info_t))

   /* The following structure is used with the                          */
   /* PANM_Query_Current_Configuration() function as a container to hold*/
   /* the currently configured configuration.  See the                  */
   /* PANM_Query_Current_Configuration() function for more information. */
typedef struct _tagPANM_Current_Configuration_t
{
   unsigned long ServiceTypeFlags;
   unsigned long IncomingConnectionFlags;
} PANM_Current_Configuration_t;

#define PANM_CURRENT_CONFIGURATION_SIZE                        (sizeof(PANM_Current_Configuration_t))

   /* The following enumerated type represents the PAN Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of PAN Manager Changed.                                           */
typedef enum
{
   petPANMIncomingConnectionRequest,
   petPANMConnected,
   petPANMDisconnected,
   petPANMConnectionStatus
} PANM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petPANMIncomingConnectionRequest*/
   /* event.                                                            */
typedef struct _tagPANM_Incoming_Connection_Request_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} PANM_Incoming_Connection_Request_Event_Data_t;

#define PANM_INCOMING_CONNECTION_REQUEST_EVENT_DATA_SIZE       (sizeof(PANM_Incoming_Connection_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petPANMConnected event.         */
typedef struct _tagPANM_Connected_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   PAN_Service_Type_t ServiceType;
} PANM_Connected_Event_Data_t;

#define PANM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(PANM_Connected_Event_Data_t))

   /* The following event is dispatched when a remote device disconnects*/
   /* from the local device).  The RemoteDeviceAddress member specifies */
   /* the Bluetooth device address of the device that disconnected from */
   /* the profile.                                                      */
typedef struct _tagPANM_Disconnected_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   PAN_Service_Type_t ServiceType;
} PANM_Disconnected_Event_Data_t;

#define PANM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(PANM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a petPANMConnectionStatus event.  */
typedef struct _tagPANM_Connection_Status_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   PAN_Service_Type_t ServiceType;
   unsigned int       Status;
} PANM_Connection_Status_Event_Data_t;

#define PANM_CONNECTION_STATUS_EVENT_DATA_SIZE                 (sizeof(PANM_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Personal Area Network (PAN) Manager Event and Event Data.         */
typedef struct _tagPANM_EventData_t
{
   PANM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      PANM_Incoming_Connection_Request_Event_Data_t IncomingConnectionReqeustEventData;
      PANM_Connected_Event_Data_t                   ConnectedEventData;
      PANM_Disconnected_Event_Data_t                DisconnectedEventData;
      PANM_Connection_Status_Event_Data_t           ConnectionStatusEventData;
   } EventData;
} PANM_Event_Data_t;

#define PANM_EVENT_DATA_SIZE                                   (sizeof(PANM_Event_Data_t))

   /* The following declared type represents the Prototype Function     */
   /* for an Event Callback. This function will be called whenever the  */
   /* Personal Area Network (PAN) Manager dispatches an event (and the  */
   /* client has registered for events). This function passes to the    */
   /* caller the PAN Manager Event and the Callback Parameter that      */
   /* was specified when this Callback was installed. The caller is     */
   /* free to use the contents of the Event Data ONLY in the context    */
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
   /* * NOTE * This function MUST NOT block and wait for events that    */
   /*          can only be satisfied by Receiving other Events.  A      */
   /*          deadlock WILL occur because NO Event Callbacks will      */
   /*          be issued while this function is currently outstanding.  */
typedef void (BTPSAPI *PANM_Event_Callback_t)(PANM_Event_Data_t *EventData, void *CallbackParameter);

   /* Personal Area Networking (PAN) Module Installation/Support        */
   /* Functions.                                                        */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Personal Area Networking (PAN)     */
   /* Manager module.  This function should be registered with the      */
   /* Bluetopia Platform Manager module handler and will be called when */
   /* the Platform Manager is initialized (or shut down).               */
void BTPSAPI PANM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PANM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Personal Area Networking (PAN) Connection Management Functions.   */

   /* The following function is provided to allow a mechanism for local */
   /* module to respond to a request to connect to a local PAN server.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * This function returning success does not necessarily     */
   /*          indicate that the connection has been successfully       */
   /*          opened. A petPANMConnected event will notifiy of this    */
   /*          status.                                                  */
BTPSAPI_DECLARATION int BTPSAPI PANM_Connection_Request_Response(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Connection_Request_Response_t)(BD_ADDR_t RemoteDeviceAddress, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to request to open a remote PANM server connection.  This  */
   /* function returns zero if successful and a negative value if there */
   /* was an error.                                                     */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          PANM Connection Status Event (if specified).             */
   /* * NOTE * The two callback parameters are optional, and if         */
   /*          specified, will be the Callback that the                 */
   /*          petPANMConnectionStatus event will be dispatched to      */
   /*          denote the status of the connection.  This is the ONLY   */
   /*          way to receive this event, as an event callack           */
   /*          registered with the PANM_Register_Event_Callback() will  */
   /*          NOT receive connection status events.                    */
BTPSAPI_DECLARATION int BTPSAPI PANM_Connect_Remote_Device(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags, PANM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Connect_Remote_Device_t)(BD_ADDR_t RemoteDeviceAddress, PAN_Service_Type_t LocalServiceType, PAN_Service_Type_t RemoteServiceType, unsigned long ConnectionFlags, PANM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to close a previously opened connection.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function does not unregister a PAN server.  It only */
   /*          disconnects any currently active connection.             */
BTPSAPI_DECLARATION int BTPSAPI PANM_Close_Connection(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Close_Connection_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Personal*/
   /* Area Networking devices.  This function accepts the buffer        */
   /* information to receive any currently connected devices.  The first*/
   /* parameter specifies the maximum number of BD_ADDR entries that the*/
   /* buffer will support (i.e. can be copied into the buffer).  The    */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of connected devices if the function is     */
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
BTPSAPI_DECLARATION int BTPSAPI PANM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Query_Connected_Devices_t)(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current configuration for the Personal Area  */
   /* Networking (PAN) Manager.  This function returns zero if          */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI PANM_Query_Current_Configuration(PANM_Current_Configuration_t *CurrentConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Query_Current_Configuration_t)(PANM_Current_Configuration_t *CurrentConfiguration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to change the Incoming Connection Flags. This function     */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PANM_Change_Incoming_Connection_Flags(unsigned long ConnectionFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Change_Incoming_Connection_Flags_t)(unsigned long ConnectionFlags);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Personal Area    */
   /* Network (PAN) Manager Service. This Callback will be dispatched   */
   /* by the PAN Manager when various PAN Manager Events occur. This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a PAN Manager Event needs to be       */
   /* dispatched. This function returns a positive (non-zero) value if  */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          PANM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI PANM_Register_Event_Callback(PANM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PANM_Register_Event_Callback_t)(PANM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PAN Manager Event Callback.   */
   /* This function accepts as input the PAN Manager Event Callback ID  */
   /* (return value from PANM_Register_Event_Callback() function).      */
BTPSAPI_DECLARATION void BTPSAPI PANM_Un_Register_Event_Callback(unsigned int PANManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_PANM_Un_Register_Event_Callback_t)(unsigned int PANManagerCallbackID);
#endif

#endif
