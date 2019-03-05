/*****< tipmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TIPMAPI - Time Profile (TIP) Manager API for Stonestreet One Bluetooth    */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __TIPMAPIH__
#define __TIPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "TIPMMSG.h"             /* BTPM TIP Manager Message Formats.         */

   /* The following structure represents the data which is passed       */
   /* to this modules at time of initialization. The SupportedRole      */
   /* field represents which roles (Client and/or Server) this modules  */
   /* supports.                                                         */
typedef struct _tagTIPM_Initialization_Info_t
{
   unsigned long SupportedRoles;
} TIPM_Initialization_Info_t;

#define TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_CLIENT        0x00000001
#define TIPM_INITIALIZATION_INFO_SUPPORTED_ROLES_SERVER        0x00000002

   /* The following enumerated type represents the TIP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of TIP Manager Changes.                                           */
typedef enum
{
   /* TIPM Common Events.                                               */
   aetTIPConnected,
   aetTIPDisconnected,

   /* TIMP Server Events.                                               */
   aetTIPGetReferenceTimeRequest,

   /* TIMP Client Events.                                               */
   aetTIPGetCurrentTimeResponse,
   aetTIPCurrentTimeNotification,
   aetTIPLocalTimeInformationResponse,
   aetTIPTimeAccuracyResponse,
   aetTIPNextDSTChangeResponse,
   aetTIPTimeUpdateStateResponse
} TIPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPConnected event.          */
typedef struct _tagTIPM_Connected_Event_Data_t
{
   unsigned int           CallbackID;
   TIPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           SupportedServicesMask;
} TIPM_Connected_Event_Data_t;

#define TIPM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(TIPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPDisconnected event.       */
typedef struct _tagTIPM_Disconnected_Event_Data_t
{
   unsigned int           CallbackID;
   TIPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} TIPM_Disconnected_Event_Data_t;

#define TIPM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(TIPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPGetReferenceTimeRequest   */
   /* event.                                                            */
typedef struct _tagTIPM_Get_Reference_Time_Request_Data_t
{
   unsigned int ServerCallbackID;
} TIPM_Get_Reference_Time_Request_Data_t;

#define TIPM_GET_REFERENCE_TIME_REQUEST_EVENT_DATA_SIZE        (sizeof(TIPM_Get_Reference_Time_Request_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPGetCurrentTimeResponse    */
   /* event.                                                            */
typedef struct _tagTIPM_Get_Current_Time_Response_Data_t
{
   unsigned int             ClientCallbackID;
   BD_ADDR_t                RemoteDeviceAddress;
   unsigned int             Status;
   TIPM_Current_Time_Data_t CurrentTimeData;
} TIPM_Get_Current_Time_Response_Data_t;

#define TIPM_GET_CURRENT_TIME_RESPONSE_EVENT_DATA_SIZE         (sizeof(TIPM_Get_Current_Time_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPcurrentTimeNotification   */
   /* event.                                                            */
typedef struct _tagTIPM_Current_Time_Notification_Data_t
{
   unsigned int             ClientCallbackID;
   BD_ADDR_t                RemoteDeviceAddress;
   TIPM_Current_Time_Data_t CurrentTimeData;
} TIPM_Current_Time_Notification_Data_t;

#define TIPM_CURRENT_TIME_NOTIFICATION_EVENT_DATA_SIZE         (sizeof(TIPM_Current_Time_Notification_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetTIPLocalTimeInformationResponse event.                         */
typedef struct _tagTIPM_Local_Time_Information_Response_Data_t
{
   unsigned int                       ClientCallbackID;
   BD_ADDR_t                          RemoteDeviceAddress;
   unsigned int                       Status;
   TIPM_Local_Time_Information_Data_t LocalTimeInformation;
} TIPM_Local_Time_Information_Response_Data_t;

#define TIPM_LOCAL_TIME_INFORMATION_RESPONSE_EVENT_DATA_SIZE   (sizeof(TIPM_Local_Time_Information_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPTimeAccuracyResponse      */
   /* event.                                                            */
typedef struct _tagTIPM_Time_Accuracy_Response_Data_t
{
   unsigned int                           ClientCallbackID;
   BD_ADDR_t                              RemoteDeviceAddress;
   unsigned int                           Status;
   TIPM_Reference_Time_Information_Data_t ReferenceTimeInformation;
} TIPM_Time_Accuracy_Response_Data_t;

#define TIPM_TIME_ACCURACY_RESPONSE_EVENT_DATA_SIZE            (sizeof(TIPM_Time_Accuracy_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPNextDSTChangeResponse     */
   /* event.                                                            */
typedef struct _tagTIPM_Next_DST_Change_Response_Data_t
{
   unsigned int              ClientCallbackID;
   BD_ADDR_t                 RemoteDeviceAddress;
   unsigned int              Status;
   TIPM_Time_With_DST_Data_t TimeWithDST;
} TIPM_Next_DST_Change_Response_Data_t;

#define TIPM_NEXT_DST_CHANGE_RESPONSE_EVENT_DATA_SIZE          (sizeof(TIPM_Next_DST_Change_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetTIPTimeUpdateStateResponse   */
   /* event.                                                            */
typedef struct _tagTIPM_Time_Update_State_Response_Data_t
{
   unsigned int                  ClientCallbackID;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  Status;
   TIPM_Time_Update_State_Data_t TimeUpdateStateData;
} TIPM_Time_Update_State_Response_Data_t;

#define TIPM_TIME_UPDATE_STATE_RESPONSE_EVENT_DATA_SIZE        (sizeof(TIPM_Time_Update_State_Response_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Time Profile (TIP) Manager Event (and Event Data) of a TIP Manager*/
   /* Event.                                                            */
typedef struct _tagTIPM_Event_Data_t
{
   TIPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      TIPM_Connected_Event_Data_t                 ConnectedEventData;
      TIPM_Disconnected_Event_Data_t              DisconnectedEventData;
      TIPM_Get_Reference_Time_Request_Data_t      GetReferenceTimeRequestEventData;
      TIPM_Get_Current_Time_Response_Data_t       GetCurrentTimeResponseEventData;
      TIPM_Current_Time_Notification_Data_t       CurrentTimeNotificationEventData;
      TIPM_Local_Time_Information_Response_Data_t LocalTimeInformationResponseEventData;
      TIPM_Time_Accuracy_Response_Data_t          TimeAccuracyResponseEventData;
      TIPM_Next_DST_Change_Response_Data_t        NextDSTChangeResponseEventData;
      TIPM_Time_Update_State_Response_Data_t      TimeUpdateStateResponseEventData;
   } EventData;
} TIPM_Event_Data_t;

#define TIPM_EVENT_DATA_SIZE                             (sizeof(TIPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the Time*/
   /* Profile (TIP) Manager dispatches an event (and the client has     */
   /* registered for events).  This function passes to the caller the   */
   /* TIP Manager Event and the Callback Parameter that was specified   */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *TIPM_Event_Callback_t)(TIPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager TIP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI TIPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TIPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Server Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Server Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Register_Server_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Register_Server_Event_Callback_t)(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Server_Event_Callback() function).  This function   */
   /* accepts as input the Server Event Callback ID (return value from  */
   /* TIPM_Register_Server_Event_Callback() function).                  */
BTPSAPI_DECLARATION void BTPSAPI TIPM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_TIPM_Un_Register_Server_Event_Callback_t)(unsigned int ServerCallbackID);
#endif

   /* The following function is a utility function that is used to set  */
   /* the current Local Time Information.  This function accepts the    */
   /* Server Callback ID (return value from                             */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Local Time Information to set.  This function returns ZERO if */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI TIPM_Set_Local_Time_Information(unsigned int ServerCallbackID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Set_Local_Time_Information_t)(unsigned int ServerCallbackID, TIPM_Local_Time_Information_Data_t *LocalTimeInformation);
#endif

   /* The following function is a utility function that is used to force*/
   /* an update of the Current Time.  This function accepts the Server  */
   /* Callback ID (return value from                                    */
   /* TIPM_Register_Server_Event_Callback() function) and a bit mask    */
   /* that contains the reason for the Current Time Update.  This       */
   /* function returns ZERO if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Update_Current_Time(unsigned int ServerCallbackID, unsigned long AdjustReasonMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Update_Current_Time_t)(unsigned int ServerCallbackID, unsigned long AdjustReasonMask);
#endif

   /* The following function is a utility function that is used to      */
   /* respond to a request for the Reference Time Information.  This    */
   /* function accepts the Server Callback ID (return value from        */
   /* TIPM_Register_Server_Event_Callback() function) and a pointer to  */
   /* the Reference Time Information to respond to the request with.    */
   /* This function returns ZERO if successful, or a negative return    */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Reference_Time_Response(unsigned int ServerCallbackID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Reference_Time_Response_t)(unsigned int ServerCallbackID, TIPM_Reference_Time_Information_Data_t *ReferenceTimeInformation);
#endif

   /* TIPM Client Functions.                                            */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Client callback function with the Time (TIP)*/
   /* Manager Service.  This Callback will be dispatched by the TIP     */
   /* Manager when various TIP Manager Client Events occur.  This       */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a TIP Manager Client Event needs to be*/
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TIPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Register_Client_Event_Callback(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Register_Client_Event_Callback_t)(TIPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered TIP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* TIPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the Client Event Callback ID (return value from  */
   /* TIPM_Register_Client_Event_Callback() function).                  */
BTPSAPI_DECLARATION void BTPSAPI TIPM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_TIPM_Un_Register_Client_Event_Callback_t)(unsigned int ClientCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the current time from a remote TIP server.  The    */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetGetCurrentTimeResponse  */
   /*          event.                                                   */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Get_Current_Time(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Get_Current_Time_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to enable current time notifications from a remote TIP    */
   /* Server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.  This*/
   /* function returns zero if successful and a negative return error   */
   /* code if there is an error.                                        */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Enable_Time_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Enable_Time_Notifications_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Enable);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the local time information from a remote TIP       */
   /* server.  The first parameter is the CallbackID returned from      */
   /* a successful call to TIPM_Register_Client_Events.  The second     */
   /* parameter is the Bluetooth Address of the remote TIP Server.      */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The                  */
   /*          result of the request will be returned in a              */
   /*          aetLocalTimeInformationResponse event.                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Get_Local_Time_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Get_Local_Time_Information_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time accuracy and the reference time           */
   /* information of a remote TIP Server.  The first parameter          */
   /* is the CallbackID returned from a successful call to              */
   /* TIPM_Register_Client_Events.  The second parameter is the         */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeAccuracyResponse    */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Get_Time_Accuracy(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Get_Time_Accuracy_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the next Daylight Savings Time change information  */
   /* from a remote TIP Server.  The first parameter is the CallbackID  */
   /* returned from a successful call to TIPM_Register_Client_Events.   */
   /* The second parameter is the Bluetooth Address of the remote TIP   */
   /* Server.  This function returns a positive number representing the */
   /* Transaction ID of this request if successful and a negative return*/
   /* error code if there was an error.                                 */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetNextDSTChangeResponse   */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Get_Next_DST_Change_Information(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Get_Next_DST_Change_Information_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to get the time update state of a remote TIP Server.  The */
   /* first parameter is the CallbackID returned from a successful call */
   /* to TIPM_Register_Client_Events.  The second parameter is the      */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* a positive number representing the Transaction ID of this request */
   /* if successful and a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successul return from this function does not mean the  */
   /*          request was successfully completed. The result of the    */
   /*          request will be returned in a aetTimeUpdateStateResponse */
   /*          event.                                                   */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Get_Reference_Time_Update_State(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Get_Reference_Time_Update_State_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to request a reference time update on a remote TIP Server */
   /* The first parameter is the CallbackID returned from a successful  */
   /* call to TIPM_Register_Client_Events.  The second parameter is the */
   /* Bluetooth Address of the remote TIP Server.  This function returns*/
   /* zero if successful and a negative return error code if there is an*/
   /* error.                                                            */
   /* * NOTE * Not all TIP Servers support the capability. The caller   */
   /*          should check the SupportedServicesMask returned          */
   /*          by either an aetTIPConnected event or a call to          */
   /*          TIPM_Quert_Connected_Devices() to determine if this is   */
   /*          supported.                                               */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Request_Reference_Time_Update(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Request_Reference_Time_Update_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of BD_ADDR entries that the buffer   */
   /* will support (i.e. can be copied into the buffer).  The next      */
   /* parameter is optional and, if specified, will be populated        */
   /* with the total number of connected devices if the function is     */
   /* successful.  The final parameter can be used to retrieve the total*/
   /* number of connected devices (regardless of the size of the list   */
   /* specified by the first two parameters).  This function returns    */
   /* a non-negative value if successful which represents the number    */
   /* of connected devices that were copied into the specified input    */
   /* buffer.  This function returns a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI TIPM_Query_Connected_Devices(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TIPM_Query_Connected_Devices_t)(TIPM_Connection_Type_t ConnectionType, unsigned int MaximumRemoteDeviceListEntries, TIPM_Remote_Device_t *RemoteDeviceAddressList, unsigned int *TotalNumberConnectedDevices);
#endif

#endif
