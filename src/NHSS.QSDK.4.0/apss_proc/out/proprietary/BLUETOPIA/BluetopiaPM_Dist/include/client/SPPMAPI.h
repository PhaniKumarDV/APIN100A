/*****< sppmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPMAPI - Local Serial Port Profile Manager API for Stonestreet One       */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/16/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SPPMAPIH__
#define __SPPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SPPMMSG.h"             /* BTPM SPP Manager Message Formats.         */

   /* The following structure is used with the                          */
   /* SPPM_Register_Service_Port_Service_Record() function.  This       */
   /* structure (when specified) contains additional SDP Service        */
   /* Information that will be added to the SDP Serial Port Profile     */
   /* Service Record Entry.  The first member of this strucuture        */
   /* specifies the Number of Service Class UUID's that are present in  */
   /* the SDPUUIDEntries Array.  This member must be at least one, and  */
   /* the SDPUUIDEntries member must point to an array of SDP UUID      */
   /* Entries that contains (at least) as many entries specified by the */
   /* NumberServiceClassUUID member.  The ProtocolList member is an SDP */
   /* Data Element Sequence that contains a list of Protocol Information*/
   /* that will be added to the generic SDP Service Record.             */
typedef struct _tagSPPM_Service_Record_Information_t
{
   unsigned int        NumberServiceClassUUID;
   SDP_UUID_Entry_t   *SDPUUIDEntries;
   SDP_Data_Element_t *ProtocolList;
   char               *ServiceName;
} SPPM_Service_Record_Information_t;

#define SPPM_SERVICE_RECORD_INFORMATION_SIZE                   (sizeof(SPPM_Service_Record_Information_t))

   /* The following enumerated type represents the Serial Port Profile  */
   /* Manager Event Types that are dispatched by this module.           */
typedef enum
{
   setServerPortOpenRequest,
   setServerPortOpen,
   setPortClose,
   setRemotePortOpenStatus,
   setLineStatusChanged,
   setPortStatusChanged,
   setDataReceived,
   setTransmitBufferEmpty,
   setIDPSStatus,
   setSessionOpenRequest,
   setSessionClose,
   setSessionDataReceived,
   setSessionDataConfirmation,
   setNonSessionDataReceived,
   setNonSessionDataConfirmation,
   setUnhandledControlMessageReceived
} SPPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setServerPortOpenRequest event. */
typedef struct _tagSPPM_Server_Port_Open_Request_Event_Data_t
{
   unsigned int PortHandle;
   BD_ADDR_t    RemoteDeviceAddress;
} SPPM_Server_Port_Open_Request_Event_Data_t;

#define SPPM_SERVER_PORT_OPEN_REQUEST_EVENT_DATA_SIZE          (sizeof(SPPM_Server_Port_Open_Request_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setServerPortOpen event.        */
typedef struct _tagSPPM_Server_Port_Open_Event_Data_t
{
   unsigned int           PortHandle;
   BD_ADDR_t              RemoteDeviceAddress;
   SPPM_Connection_Type_t ConnectionType;
} SPPM_Server_Port_Open_Event_Data_t;

#define SPPM_SERVER_PORT_OPEN_EVENT_DATA_SIZE                  (sizeof(SPPM_Server_Port_Open_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setPortClose event.             */
typedef struct _tagSPPM_Port_Close_Event_Data_t
{
   unsigned int PortHandle;
} SPPM_Port_Close_Event_Data_t;

#define SPPM_PORT_CLOSE_EVENT_DATA_SIZE                        (sizeof(SPPM_Port_Close_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setRemotePortOpenStatus event.  */
typedef struct _tagSPPM_Remote_Port_Open_Status_Event_Data_t
{
   unsigned int           PortHandle;
   int                    Status;
   SPPM_Connection_Type_t ConnectionType;
} SPPM_Remote_Port_Open_Status_Event_Data_t;

#define SPPM_REMOTE_PORT_OPEN_STATUS_EVENT_DATA_SIZE           (sizeof(SPPM_Remote_Port_Open_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setLineStatusChanged event.     */
typedef struct _tagSPPM_Line_Status_Changed_Event_Data_t
{
   unsigned int  PortHandle;
   unsigned long LineStatusMask;
} SPPM_Line_Status_Changed_Event_Data_t;

#define SPPM_LINE_STATUS_CHANGED_EVENT_DATA_SIZE               (sizeof(SPPM_Line_Status_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setPortStatusChanged event.     */
typedef struct _tagSPPM_Port_Status_Changed_Event_Data_t
{
   unsigned int       PortHandle;
   SPPM_Port_Status_t PortStatus;
} SPPM_Port_Status_Changed_Event_Data_t;

#define SPPM_PORT_STATUS_CHANGED_EVENT_DATA_SIZE               (sizeof(SPPM_Port_Status_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setDataReceived event.          */
typedef struct _tagSPPM_Data_Received_Event_Data_t
{
   unsigned int PortHandle;
   unsigned int DataLength;
} SPPM_Data_Received_Event_Data_t;

#define SPPM_DATA_RECEIVED_EVENT_DATA_SIZE                     (sizeof(SPPM_Data_Received_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a setTransmitBufferEmpty event.   */
typedef struct _tagSPPM_Transmit_Buffer_Empty_Event_Data_t
{
   unsigned int PortHandle;
} SPPM_Transmit_Buffer_Empty_Event_Data_t;

#define SPPM_TRANSMIT_BUFFER_EMPTY_EVENT_DATA_SIZE             (sizeof(SPPM_Transmit_Buffer_Empty_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setIDPSStatus event (MFi event  */
   /* only).                                                            */
typedef struct _tagSPPM_IDPS_Status_Event_Data_t
{
   unsigned int      PortHandle;
   SPPM_IDPS_State_t IDPSState;
   unsigned int      Status;
} SPPM_IDPS_Status_Event_Data_t;

#define SPPM_IDPS_STATUS_EVENT_DATA_SIZE                       (sizeof(SPPM_IDPS_Status_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setSessionOpenRequest event (MFi*/
   /* event only).                                                      */
typedef struct _tagSPPM_Session_Open_Request_Event_Data_t
{
   unsigned int PortHandle;
   unsigned int MaximimTransmitPacket;
   unsigned int MaximumReceivePacket;
   Word_t       SessionID;
   Byte_t       ProtocolIndex;
} SPPM_Session_Open_Request_Event_Data_t;

#define SPPM_SESSION_OPEN_REQUEST_EVENT_DATA_SIZE              (sizeof(SPPM_Session_Open_Request_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setSessionClose event (MFi event*/
   /* only).                                                            */
typedef struct _tagSPPM_Session_Close_Event_Data_t
{
   unsigned int PortHandle;
   Word_t       SessionID;
} SPPM_Session_Close_Event_Data_t;

#define SPPM_SESSION_CLOSE_EVENT_DATA_SIZE                     (sizeof(SPPM_Session_Close_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setSessionDataReceived event    */
   /* (MFi event only).                                                 */
typedef struct _tagSPPM_Session_Data_Received_Event_Data_t
{
   unsigned int   PortHandle;
   Word_t         SessionID;
   Word_t         SessionDataLength;
   unsigned char *SessionDataBuffer;
} SPPM_Session_Data_Received_Event_Data_t;

#define SPPM_SESSION_DATA_RECEIVED_EVENT_DATA_SIZE             (sizeof(SPPM_Session_Data_Received_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setSessionDataConfirmation event*/
   /* (MFi event only).                                                 */
typedef struct _tagSPPM_Session_Data_Confirmation_Event_Data_t
{
   unsigned int PortHandle;
   Word_t       SessionID;
   unsigned int PacketID;
   unsigned int Status;
} SPPM_Session_Data_Confirmation_Event_Data_t;

#define SPPM_SESSION_DATA_CONFIRMATION_EVENT_DATA_SIZE         (sizeof(SPPM_Session_Data_Confirmation_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setNonSessionDataReceived event */
   /* (MFi event only).                                                 */
typedef struct _tagSPPM_Non_Session_Data_Received_Event_Data_t
{
   unsigned int   PortHandle;
   Byte_t         Lingo;
   Byte_t         CommandID;
   Word_t         DataLength;
   unsigned char *DataBuffer;
} SPPM_Non_Session_Data_Received_Event_Data_t;

#define SPPM_NON_SESSION_DATA_RECEIVED_EVENT_DATA_SIZE         (sizeof(SPPM_Non_Session_Data_Received_Event_Data_t))

   /* The following structure is the container structure that holds the */
   /* information that is returned in a setNonSessionDataConfirmation   */
   /* event (MFi event only).                                           */
typedef struct _tagSPPM_Non_Session_Data_Confirmation_Event_Data_t
{
   unsigned int PortHandle;
   unsigned int PacketID;
   Word_t       TransactionID;
   unsigned int Status;
} SPPM_Non_Session_Data_Confirmation_Event_Data_t;

#define SPPM_NON_SESSION_DATA_CONFIRMATION_EVENT_DATA_SIZE     (sizeof(SPPM_Non_Session_Data_Confirmation_Event_Data_t))

   /* The following structure is the container structure                */
   /* that holds the information that is returned in a                  */
   /* setUnhandledControlMessageReceived event (MFi event only).        */
typedef struct _tagSPPM_Unhandled_Control_Message_Received_Event_Data_t
{
   unsigned int   PortHandle;
   Word_t         ControlMessageID;
   Word_t         DataLength;
   unsigned char *DataBuffer;
} SPPM_Unhandled_Control_Message_Received_Event_Data_t;

#define SPPM_UNHANDLED_CONTROL_MESSAGE_RECEIVED_EVENT_DATA_SIZE   (sizeof(SPPM_Unhandled_Control_Message_Received_Event_Data_t));

   /* The following structure is a container structure that holds the   */
   /* Serial Port Profile Manager Event (and Event Data) of a Serial    */
   /* Port Profile Manager Event.                                       */
typedef struct _tagSPPM_Event_Data_t
{
   SPPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      SPPM_Server_Port_Open_Request_Event_Data_t           ServerPortOpenRequestEventData;
      SPPM_Server_Port_Open_Event_Data_t                   ServerPortOpenEventData;
      SPPM_Port_Close_Event_Data_t                         PortCloseEventData;
      SPPM_Remote_Port_Open_Status_Event_Data_t            RemotePortOpenStatusEventData;
      SPPM_Line_Status_Changed_Event_Data_t                LineStatusChangedEventData;
      SPPM_Port_Status_Changed_Event_Data_t                PortStatusChangedEventData;
      SPPM_Data_Received_Event_Data_t                      DataReceivedEventData;
      SPPM_Transmit_Buffer_Empty_Event_Data_t              TransmitBufferEmptyEventData;
      SPPM_IDPS_Status_Event_Data_t                        IDPSStatusEventData;
      SPPM_Session_Open_Request_Event_Data_t               SessionOpenRequestEventData;
      SPPM_Session_Close_Event_Data_t                      SessionCloseEventData;
      SPPM_Session_Data_Received_Event_Data_t              SessionDataReceivedEventData;
      SPPM_Session_Data_Confirmation_Event_Data_t          SessionDataConfirmationEventData;
      SPPM_Non_Session_Data_Received_Event_Data_t          NonSessionDataReceivedEventData;
      SPPM_Non_Session_Data_Confirmation_Event_Data_t      NonSessionDataConfirmationEventData;
      SPPM_Unhandled_Control_Message_Received_Event_Data_t UnhandledControlMessageReceivedEventData;
   } EventData;
} SPPM_Event_Data_t;

#define SPPM_EVENT_DATA_SIZE                                   (sizeof(SPPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Serial Port Profile Manager dispatches an event (and the client   */
   /* has registered for events).  This function passes to the caller   */
   /* the Serial Port Profile Manager Event and the Callback Parameter  */
   /* that was specified when this Callback was installed.  The caller  */
   /* is free to use the contents of the Event Data ONLY in the context */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e. this function DOES NOT have be reentrant).         */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* Message will not be processed while this function call is         */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *SPPM_Event_Callback_t)(SPPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a Local Serial Port Profile Manager Serial    */
   /* Port Server.  This function accepts the RFCOMM/SPP Port to use for*/
   /* the Server, followed by the the Port Flags, followed by the Event */
   /* Callback and Callback parameter (to receive events related to the */
   /* registered server).  This function returns a positive, non-zero,  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * A successful return value represents the Server Port     */
   /*          Handle that can be used with various functions in this   */
   /*          module to specify this local server port.                */
BTPSAPI_DECLARATION int BTPSAPI SPPM_RegisterServerPort(unsigned int ServerPort, unsigned long PortFlags, SPPM_Event_Callback_t EventCallback, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_RegisterServerPort_t)(unsigned int ServerPort, unsigned long PortFlags, SPPM_Event_Callback_t EventCallback, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Server Port    */
   /* connection.  This function returns zero if successful, or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Server Port     */
   /*          Opened event will be dispatched to signify the actual    */
   /*          result.                                                  */
BTPSAPI_DECLARATION int BTPSAPI SPPM_OpenServerPortRequestResponse(unsigned int PortHandle, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_OpenServerPortRequestResponse_t)(unsigned int PortHandle, Boolean_t Accept);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Server Port.  This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI SPPM_UnRegisterServerPort(unsigned int PortHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_UnRegisterServerPort_t)(unsigned int PortHandle);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Records for a previously       */
   /* registered Server Port.  This function returns a positive,        */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.                */
BTPSAPI_DECLARATION long BTPSAPI SPPM_RegisterServerPortServiceRecord(unsigned int PortHandle, SPPM_Service_Record_Information_t *ServiceRecordInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef long (BTPSAPI *PFN_SPPM_RegisterServerPortServiceRecord_t)(unsigned int PortHandle, SPPM_Service_Record_Information_t *ServiceRecordInformation);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to open remote Serial Port Profile connection.  This      */
   /* function returns a positive, non-zero, value if successful, or a  */
   /* negative return error code if there was an error.  If this        */
   /* function is successful, the value that is returned represents the */
   /* Serial Port Profile Manager Port Handle of the connection.        */
   /* * NOTE * The final parameter to this function, if specified,      */
   /*          instructs this function to block until the connection    */
   /*          status is received (i.e. the connection is completed).   */
   /*          If this parameter is not specified (i.e. NULL) then the  */
   /*          connection status will be returned asynchronously in the */
   /*          Open Remote Port Result event.                           */
BTPSAPI_DECLARATION int BTPSAPI SPPM_OpenRemotePort(BD_ADDR_t RemoteDevice, unsigned int ServerPort, unsigned long OpenFlags, SPPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_OpenRemotePort_t)(BD_ADDR_t RemoteDevice, unsigned int ServerPort, unsigned long OpenFlags, SPPM_Event_Callback_t EventCallback, void *CallbackParameter, unsigned int *ConnectionStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an active connection for the specified Serial    */
   /* Port (either local server or remote).  The final parameter        */
   /* specifies the timeout (can be zero) to wait for all queued data to*/
   /* be sent before the port is forced is closed.  This function       */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function will NOT Un-Register a local server port,  */
   /*          it will just disconnect any currently connected remote   */
   /*          device.                                                  */
   /* * NOTE * This function CANNOT be called with a Timeout if it is   */
   /*          issued within the SPPM Event Callback.  A deadlock will  */
   /*          occur.                                                   */
BTPSAPI_DECLARATION int BTPSAPI SPPM_ClosePort(unsigned int PortHandle, unsigned int CloseTimeout);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_ClosePort_t)(unsigned int PortHandle, unsigned int CloseTimeout);
#endif

   /* The following constants are used with the CloseTimeout parameter  */
   /* of the SPPM_ClosePort() function to denote special handling that  */
   /* is to be applied to the Timeout.  These values specify do not wait*/
   /* at all and return immediately, and wait indefinitely              */
   /* (respectively).                                                   */
#define SPPM_CLOSE_DATA_FLUSH_TIMEOUT_IMMEDIATE                0x00000000
#define SPPM_CLOSE_DATA_FLUSH_TIMEOUT_INFINITE                 0xFFFFFFFF

   /* The following function is provided to allow a mechanism for local */
   /* modules to read data from an active connection for the specified  */
   /* Serial Port (either local server or remote).  This function       */
   /* returns the number of bytes read or present in the buffer if      */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * This function CANNOT be called with a Timeout if it is   */
   /*          issued within the SPPM Event Callback.  A deadlock will  */
   /*          occur.                                                   */
   /* * NOTE * If the final parameters are specified as 0, 0, NULL      */
   /*          (respectively) then this instructs the Serial Port       */
   /*          Profile Manager to return how many data bytes are        */
   /*          currently in the Serial Port buffer.                     */
   /* * NOTE * This function does NOT block until the specified buffer  */
   /*          is full.  It will return as soon as there is at least a  */
   /*          single byte of data is received (or until the specified  */
   /*          timeout is reached).                                     */
   /* * NOTE * This function will return zero if a timout occurs and    */
   /*          no data has been received.                               */
BTPSAPI_DECLARATION int BTPSAPI SPPM_ReadData(unsigned int PortHandle, unsigned int ReadTimeout, unsigned int DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_ReadData_t)(unsigned int PortHandle, unsigned int ReadTimeout, unsigned int DataLength, unsigned char *DataBuffer);
#endif

   /* The following constants are used with the ReadTimeout parameter of*/
   /* the SPPM_ReadData() function to denote special handling that is   */
   /* to be applied to the Timeout.  These values specify do not wait at*/
   /* all and return immediately, and wait indefinitely (respectively). */
#define SPPM_READ_DATA_READ_TIMEOUT_IMMEDIATE                  0x00000000
#define SPPM_READ_DATA_READ_TIMEOUT_INFINITE                   0xFFFFFFFF

   /* The following function is provided to allow a mechanism for local */
   /* modules to write data to an active connection for the specified   */
   /* Serial Port (either local server or remote).  This function       */
   /* returns the number of bytes written if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * This function does NOT block until the specified buffer  */
   /*          is completely sent (unless an infinite timeout is        */
   /*          specified).                                              */
   /* * NOTE * This function will return the amount of data that was    */
   /*          sent when a timeout occurs.                              */
BTPSAPI_DECLARATION int BTPSAPI SPPM_WriteData(unsigned int PortHandle, unsigned int WriteTimeout, unsigned int DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_WriteData_t)(unsigned int PortHandle, unsigned int WriteTimeout, unsigned int DataLength, unsigned char *DataBuffer);
#endif

   /* The following constants are used with the WriteTimeout parameter  */
   /* of the SPPM_WriteData() function to denote special handling that  */
   /* is to be applied to the Timeout.  These values specify do not wait*/
   /* at all and return immediately, and wait indefinitely              */
   /* (respectively).                                                   */
#define SPPM_WRITE_DATA_WRITE_TIMEOUT_IMMEDIATE                0x00000000
#define SPPM_WRITE_DATA_WRITE_TIMEOUT_INFINITE                 0xFFFFFFFF

   /* The following function is provided to allow a mechanism for local */
   /* modules to send a Line Status notification to the specified Serial*/
   /* Port (either local server or remote).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI SPPM_SendLineStatus(unsigned int PortHandle, unsigned long LineStatusMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_SendLineStatus_t)(unsigned int PortHandle, unsigned long LineStatusMask);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to send a Port Status notification to the specified Serial*/
   /* Port (either local server or remote).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI SPPM_SendPortStatus(unsigned int PortHandle, SPPM_Port_Status_t *PortStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_SendPortStatus_t)(unsigned int PortHandle, SPPM_Port_Status_t *PortStatus);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query whether a particular RFCOMM/SPP Port is currently*/
   /* in use as a local SPP Server. This function returns zero if       */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI SPPM_QueryServerPresent(unsigned int ServerPort, Boolean_t *Present);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_QueryServerPresent_t)(unsigned int ServerPort, Boolean_t *Present);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* local modules to locate an RFCOMM/SPP Port which is currently     */
   /* available to be used as a local SPP Server. This function returns */
   /* a positive, non-zero, value if successful, or a negative return   */
   /* error code if there was an error. If this function is successful, */
   /* the value that is returned represents the SPPM Port Number which  */
   /* is available.                                                     */
BTPSAPI_DECLARATION int BTPSAPI SPPM_FindFreeServerPort(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_FindFreeServerPort_t)(void);
#endif

   /* The following function is provided to allow the programmer a means*/
   /* to change the default Transmit and Receive Buffer Sizes.  This    */
   /* function accepts as the input the Port Handle of the SPPM Port to */
   /* change the buffer size for, and the next two parameters represent */
   /* the requested Buffer size to change the Receive and Transmit      */
   /* Buffer to (respectively).  The special constant                   */
   /* SPPM_BUFFER_SIZE_CURRENT can be used to specify that the requested*/
   /* Buffer Size (either Transmit and/or Receive) NOT be changed.  This*/
   /* function returns zero if the specified Buffer Size(s) were        */
   /* changed, or a negative return error code if there was an error.   */
   /* * NOTE * This function causes ALL Data in each Buffer to be lost. */
   /*          This function clears the each Data Buffer so that all the*/
   /*          available data buffer is available to be used.           */
BTPSAPI_DECLARATION int BTPSAPI SPPM_ChangeBufferSize(unsigned int PortHandle, unsigned int ReceiveBufferSize, unsigned int TransmitBufferSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_ChangeBufferSize_t)(unsigned int PortHandle, unsigned int ReceiveBufferSize, unsigned int TransmitBufferSize);
#endif

   /* The following constant SPPM_BUFFER_SIZE_CURRENT is used with the  */
   /* SPP_Change_Buffer_Size() function to inform the function NOT to   */
   /* change the Buffer Size.                                           */
#define SPPM_BUFFER_SIZE_CURRENT                               0

   /* The following function is provided to allow a mechanism to enable */
   /* MFi support in the Serial Port Profile Manager (SPPM).  This      */
   /* function returns zero if MFi was configured within the Serial Port*/
   /* Profile Manager (SPPM) or a negative return error code if there   */
   /* was an error.                                                     */
   /* * NOTE * MFi settings can only be configured once and are global  */
   /*          in nature.                                               */
   /* * NOTE * Simply enabling MFi support by calling this function     */
   /*          does not mean that MFi will be available for all ports   */
   /*          configured.  Each port that is specified by this module  */
   /*          can specify whether or not MFi is allowed/requested      */
   /*          when it is configured/opened.                            */
BTPSAPI_DECLARATION int BTPSAPI SPPM_ConfigureMFiSettings(SPPM_MFi_Configuration_Settings_t *MFiConfigurationSettings);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_ConfigureMFiSettings_t)(SPPM_MFi_Configuration_Settings_t *MFiConfigurationSettings);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* determine if the specified, connected, port is operating in either*/
   /* SPP or MFi mode.  This function returns zero if successful (and   */
   /* the connection type will be filled into the input buffer) or a    */
   /* negative return error code if there was error.                    */
   /* * NOTE * This function can only be called on ports that are       */
   /*          currently connected.                                     */
BTPSAPI_DECLARATION int BTPSAPI SPPM_QueryConnectionType(unsigned int PortHandle, SPPM_Connection_Type_t *ConnectionType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_QueryConnectionType_t)(unsigned int PortHandle, SPPM_Connection_Type_t *ConnectionType);
#endif

   /* The following function is provided to allow a mechanism to respond*/
   /* to an incoming MFi Open Session request.  This function accepts   */
   /* the Session ID to accept/reject and a BOOLEAN value that specifies*/
   /* whether or not the specified Session is to be accected (TRUE) or  */
   /* rejected (FALSE).  This function returns zero if successful or a  */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI SPPM_OpenSessionRequestResponse(unsigned int PortHandle, Word_t SessionID, Boolean_t Accept);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_OpenSessionRequestResponse_t)(unsigned int PortHandle, Word_t SessionID, Boolean_t Accept);
#endif

   /* The following function is provided to allow a mechanism to send   */
   /* preformatted session data packets to a currently connected session*/
   /* (based on the Session ID).  This function accepts the length of   */
   /* data to send followed by the actual packet data that is to be     */
   /* sent.  This function returns a positive, non-zero, value if       */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value from this function represents  */
   /*          the Packet ID of the packet that can be used to track the*/
   /*          confirmation status.                                     */
BTPSAPI_DECLARATION int BTPSAPI SPPM_SendSessionData(unsigned int PortHandle, Word_t SessionID, Word_t SessionDataLength, unsigned char *SessionDataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_SendSessionData_t)(unsigned int PortHandle, Word_t SessionID, Word_t SessionDataLength, unsigned char *SessionDataBuffer);
#endif

   /* The following function is provided to allow a mechanism to send   */
   /* preformatted non-session data packets to a currently MFi device.  */
   /* This function accepts the Lingo ID, Command ID, and the           */
   /* Transaction ID of the data packet to send, followed by the length */
   /* of data to send followed by the actual packet data that is to be  */
   /* sent.  This function returns a positive, non-zero, value if       */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Non Session data is considered to be any protocol data   */
   /*          that is represented by one of the defined Lingos (note   */
   /*          this also includes the General Lingo).                   */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Packet ID of the packet that can be used to track the*/
   /*          confirmation status.                                     */
BTPSAPI_DECLARATION int BTPSAPI SPPM_SendNonSessionData(unsigned int PortHandle, Byte_t Lingo, Byte_t CommandID, Word_t TransactionID, Word_t DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_SendNonSessionData_t)(unsigned int PortHandle, Byte_t Lingo, Byte_t CommandID, Word_t TransactionID, Word_t DataLength, unsigned char *DataBuffer);
#endif

   /* The following function is provided to allow a mechanism to send   */
   /* preformatted Control Session packets to a currently connected     */
   /* MFi device.  This function accepts the Control Message ID of the  */
   /* message to send, followed by the length of Command Payload data   */
   /* to send followed by the actual Command Payload data that is to    */
   /* be sent.  This function returns a positive, non-zero, value if    */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value from this function represents  */
   /*          the Packet ID of the packet that can be used to track the*/
   /*          confirmation status.                                     */
BTPSAPI_DECLARATION int BTPSAPI SPPM_SendControlMessage(unsigned int PortHandle, Word_t ControlMessageID, Word_t DataLength, unsigned char *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_SendControlMessage_t)(unsigned int PortHandle, Word_t ControlMessageID, Word_t DataLength, unsigned char *DataBuffer);
#endif

   /* The following function is deprecated due to protocol differences  */
   /* between iAP versions.  Any attempt to invoke this function will   */
   /* result in a BTPM_ERROR_CODE_SERIAL_PORT_MFI_NOT_SUPPORTED error   */
   /* code, and no operation will be performed.                         */
BTPSAPI_DECLARATION int BTPSAPI SPPM_CancelPacket(unsigned int PortHandle, unsigned int PacketID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SPPM_CancelPacket_t)(unsigned int PortHandle, unsigned int PacketID);
#endif

#endif
