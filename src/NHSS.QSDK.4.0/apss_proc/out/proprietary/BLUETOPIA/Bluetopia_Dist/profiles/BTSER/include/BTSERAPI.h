/*****< btserapi.h >***********************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTSERAPI - Stonestreet One Bluetooth Stack Serial Port Access API Type    */
/*             Definitions, Constants, and Prototypes.                        */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/01  D. Lange       Initial creation.                               */
/*   07/18/02  R. Sledge      Ported to Linux.                                */
/*   07/29/08  J. Toole       Moved to GEM.                                   */
/******************************************************************************/
#ifndef __BTSERAPIH__
#define __BTSERAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTSER_ERROR_INVALID_PARAMETER                             (-1000)
#define BTSER_ERROR_NOT_INITIALIZED                               (-1001)
#define BTSER_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTSER_ERROR_INSUFFICIENT_RESOURCES                        (-1003)
#define BTSER_ERROR_UNABLE_TO_OPEN_LOCAL_SER_PORT                 (-1004)

   /* The following Bit flag constants are used with the                */
   /* BTSER_Open_Server_Port() and BTSERM_Open_Remote_Port() function.  */
   /* These Bit flags are passed to the LocalHardwareFlowControl        */
   /* parameter when a Virtual Serial Port is specified.                */
#define BTSER_MAP_DTR_DSR_TO_DATA_CARRIER_DETECT                  0x00000001
#define BTSER_ASSERT_DTR_UPON_CONNECTION                          0x00000002

   /* The following Bit Definitions represent the allowable Flags for a */
   /* BTSER_Purge_Buffer() operation.  Either Transmit or Receive can   */
   /* be specified simultaneously.  Abort and Flush may not be used     */
   /* simultaneously.                                                   */
#define BTSER_PURGE_MASK_TRANSMIT_ABORT_BIT                       0x00000001
#define BTSER_PURGE_MASK_RECEIVE_ABORT_BIT                        0x00000002
#define BTSER_PURGE_MASK_TRANSMIT_FLUSH_BIT                       0x00000004

   /* The following structure is used with the                          */
   /* BTSER_Register_SDP_Record() function.  This structure contains    */
   /* additional SDP Service Information that will be added to the SDP  */
   /* Service Record Entry.  The first member of this strucuture        */
   /* specifies the Number of Service Class UUID's that are present in  */
   /* the SDPUUIDEntries Array.  This member does NOT have to be        */
   /* specified.  The SDPUUIDEntries member must point to an array of   */
   /* SDP UUID Entries that contains (at least) as many entries         */
   /* specified by the NumberServiceClassUUID member.  The ProtocolList */
   /* member is an SDP Data Element Sequence that contains a list of    */
   /* Protocol Information that will be added to the generic BTSER SDP  */
   /* Service Record.                                                   */
typedef struct _tagBTSER_SDP_Service_Record_t
{
   unsigned int        NumberServiceClassUUID;
   SDP_UUID_Entry_t   *SDPUUIDEntries;
   SDP_Data_Element_t *ProtocolList;
} BTSER_SDP_Service_Record_t;

#define BTSER_SDP_SERVICE_RECORD_SIZE                    (sizeof(BTSER_SDP_Service_Record_t))

   /* The following constants represent the Port Open Status Values that*/
   /* are possible in the BTSER Open Port Confirmation Event Data       */
   /* Information.                                                      */
#define BTSER_OPEN_PORT_STATUS_SUCCESS                               0x00
#define BTSER_OPEN_PORT_STATUS_CONNECTION_TIMEOUT                    0x01
#define BTSER_OPEN_PORT_STATUS_CONNECTION_REFUSED                    0x02
#define BTSER_OPEN_PORT_STATUS_UNKNOWN_ERROR                         0x03

   /* Bluetooth Serial Port Access Event API Types.                     */
typedef enum
{
   etBTSER_Open_Port_Indication,
   etBTSER_Open_Port_Confirmation,
   etBTSER_Close_Port_Indication,
   etBTSER_Open_Port_Request_Indication,
   etBTSER_Purge_Buffer_Confirmation
} BTSER_Event_Type_t;

typedef struct _tagBTSER_Open_Port_Indication_Data_t
{
   unsigned int BTSERID;
   BD_ADDR_t    BD_ADDR;
} BTSER_Open_Port_Indication_Data_t;

#define BTSER_OPEN_PORT_INDICATION_DATA_SIZE             (sizeof(BTSER_Open_Port_Indication_Data_t))

typedef struct _tagBTSER_Open_Port_Confirmation_Data_t
{
   unsigned int BTSERID;
   unsigned int PortOpenStatus;
} BTSER_Open_Port_Confirmation_Data_t;

#define BTSER_OPEN_PORT_CONFIRMATION_DATA_SIZE           (sizeof(BTSER_Open_Port_Confirmation_Data_t))

typedef struct _tagBTSER_Close_Port_Indication_Data_t
{
   unsigned int BTSERID;
} BTSER_Close_Port_Indication_Data_t;

#define BTSER_CLOSE_PORT_INDICATION_DATA_SIZE            (sizeof(BTSER_Close_Port_Indication_Data_t))

typedef struct _tagBTSER_Open_Port_Request_Indication_Data_t
{
   unsigned int BTSERID;
   BD_ADDR_t    BD_ADDR;
} BTSER_Open_Port_Request_Indication_Data_t;

#define BTSER_OPEN_PORT_REQUEST_INDICATION_DATA_SIZE     (sizeof(BTSER_Open_Port_Request_Indication_Data_t))

typedef struct _tagBTSER_Purge_Buffer_Confirmation_Data_t
{
   unsigned int BTSERID;
} BTSER_Purge_Buffer_Confirmation_Data_t;

#define BTSER_PURGE_BUFFER_CONFIRMATION_DATA_SIZE        (sizeof(BTSER_Purge_Buffer_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Bluetooth Serial Port Access Event Data Data.         */
typedef struct _tagBTSER_Event_Data_t
{
   BTSER_Event_Type_t Event_Data_Type;
   Word_t             Event_Data_Size;
   union
   {
      BTSER_Open_Port_Indication_Data_t         *BTSER_Open_Port_Indication_Data;
      BTSER_Open_Port_Confirmation_Data_t       *BTSER_Open_Port_Confirmation_Data;
      BTSER_Close_Port_Indication_Data_t        *BTSER_Close_Port_Indication_Data;
      BTSER_Open_Port_Request_Indication_Data_t *BTSER_Open_Port_Request_Indication_Data;
      BTSER_Purge_Buffer_Confirmation_Data_t    *BTSER_Purge_Buffer_Confirmation_Data;
   } Event_Data;
} BTSER_Event_Data_t;

#define BTSER_EVENT_DATA_SIZE                            (sizeof(BTSER_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an BTSER Event Receive Data Callback.  This function will be      */
   /* called whenever a BTSER Event occurs that is associated with the  */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the BTSER Event Data that occurred and    */
   /* the BTSER Event Callback Parameter that was specified when this   */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the BTSER Event Data ONLY in the context of this callback.  If    */
   /* the caller requires the Data for a longer period of time, then the*/
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another BTSER Event will not  */
   /* be processed while this function call is outstanding).            */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BTSER Event         */
   /*            Packets.  A Deadlock WILL occur because NO BTSER Event */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *BTSER_Event_Callback_t)(unsigned int BluetoothStackID, BTSER_Event_Data_t *BTSER_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a BT Serial Port*/
   /* Server Gateway on the specified physical Serial Port.  This       */
   /* function accepts as input the Bluetooth Stack ID of the Bluetooth */
   /* Stack Instance to use for the BT Serial Port Server, the Local    */
   /* Serial Port Number to use, the physical Serial Port Settings      */
   /* (Serial Port Number, Baud Rate, and Flow Control Information), and*/
   /* the BTSER Event Callback function (and parameter) to associate    */
   /* with the specified BTSER Port.  The ServerPort parameter *MUST* be*/
   /* between SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  This*/
   /* function returns a non-zero, positive, value if successful or a   */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a BTSER Port ID that can be used to reference */
   /* the Opened BTSER Port in ALL other functions in this module       */
   /* (except the BTSER_Open_Remote_Port() function).  Once a Server    */
   /* BTSER Port is opened, it can only be Un-Registered via a call to  */
   /* the BTSER_Close_Server_Port() function (passing the return value  */
   /* from this function).  The BTSER_Close_Port() function can be used */
   /* to Disconnect a Client from the Server Port (if one is connected, */
   /* it will NOT Un-Register the Server Port however).                 */
   /* * NOTE * If the LocalBaudRate parameter is zero then the Serial   */
   /*          Port that is open will specify a Virtual Serial Port not */
   /*          a physical Serial Port.                                  */
   /* * NOTE * If the LocalBaudRate parameter is zero, then the the     */
   /*          LocalHardwareFlowControl flag has special meaning.  The  */
   /*          meaning will change if this flag is set to any of the    */
   /*          constants specified above.  Currently the flags that can */
   /*          be specified are:                                        */
   /*             - BTSER_MAP_DTR_DSR_TO_DATA_CARRIER_DETECT            */
   /*               If specified, then the DTR signal is mapped to the  */
   /*               Data Carrier Detect (DCD) signal for output, and    */
   /*               on input, the DSR Signal is mapped to the Data      */
   /*               Carrier Detect (DCD) signal.  This is required for  */
   /*               true NULL Modem Emulation which requires the Data   */
   /*               Carrier Detect signal for proper functionality.     */
   /*               Note that this meaning ONLY applies when a Virtual  */
   /*               Serial Port is specified (the LocalBaudRate         */
   /*               parameter is zero).                                 */
   /*             - BTSER_ASSERT_DTR_UPON_CONNECTION                    */
   /*               If specified, then the DTR signal will be asserted  */
   /*               as long as the connection is valid (i.e. DTR will   */
   /*               be asserted as long as the Bluetooth Connection is  */
   /*               established).  Note that this meaning ONLY applies  */
   /*               when a Virtual Serial Port is specified (the        */
   /*               LocalBaudRate parameter is zero).                   */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Open_Server_Port(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalSerialPort, unsigned int LocalBaudRate, unsigned int LocalHardwareFlowControl, BTSER_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Open_Server_Port_t)(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalSerialPort, unsigned int LocalBaudRate, unsigned int LocalHardwareFlowControl, BTSER_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a BT     */
   /* Serial Port Server (which was Registered by a successful call to  */
   /* the BTSER_Open_Server_Port() function).  This function accepts as */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the BT Serial Port specified by the Second Parameter is valid for.*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if an error occurred (see BTERRORS.H).  Note that this */
   /* function does NOT delete any SDP Service Record Handles.          */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Close_Server_Port(unsigned int BluetoothStackID, unsigned int BTSERID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Close_Server_Port_t)(unsigned int BluetoothStackID, unsigned int BTSERID);
#endif

   /* The following function is responsible for responding to requests  */
   /* to connect to a Serial Port Server.  This function accepts as     */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol      */
   /* Stack, the BTSER ID (which *MUST* have been obtained by calling   */
   /* the BTSER_Open_Server_Port() function), and as the final parameter*/
   /* whether to accept the pending connection request.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Open_Port_Request_Response(unsigned int BluetoothStackID, unsigned int BTSERID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Open_Port_Request_Response_t)(unsigned int BluetoothStackID, unsigned int BTSERID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic BT Serial Port Service Record to the SDP Database.  This  */
   /* function takes as input the Bluetooth Stack ID of the Local       */
   /* Bluetooth Protocol Stack, the BTSER Port ID (which *MUST* have    */
   /* been obtained by calling the BTSER_Open_Server_Port() function.   */
   /* The third parameter (optional) specifies any additional SDP       */
   /* Information to add to the record.  The fourth parameter specifies */
   /* the Service Name to associate with the SDP Record.  The final     */
   /* parameter is a pointer to a DWord_t which receives the SDP Service*/
   /* Record Handle if this function successfully creates an SDP Service*/
   /* Record.  If this function returns zero, then the                  */
   /* SDPServiceRecordHandle entry will contain the Service Record      */
   /* Handle of the added SDP Service Record.  If this function fails, a*/
   /* negative return error code will be returned (see BTERRORS.H) and  */
   /* the SDPServiceRecordHandle value will be undefined.               */
   /* * NOTE * This function should only be called with the BTSER ID    */
   /*          that was returned from the BTSER_Open_Server_Port()      */
   /*          function.  This function should NEVER be used with the   */
   /*          BTSER ID returned from the BTSER_Open_Remote_Port()      */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          BTSER_Un_Register_SDP_Record() to                        */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * If no SDP Information is specified, then a generic SPP   */
   /*          SPP Record is added.                                     */
   /* * NOTE * If there is NO UUID Information specified then the SPP   */
   /*          UUID is added to the record, if UUID information is      */
   /*          specifies it is added to the SDP Record.                 */
   /* * NOTE * If there is Protocol Information specified in the        */
   /*          SDPServiceRecord Parameter, the Protocol information is  */
   /*          inserted AFTER the default SPP Protocol List (L2CAP and  */
   /*          RFCOMM).                                                 */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI BTSER_Register_SDP_Record(unsigned int BluetoothStackID, unsigned int BTSERID, BTSER_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Register_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int BTSERID, BTSER_SDP_Service_Record_t *SDPServiceRecord, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the BT */
   /* Serial Port SDP Service Record (specified by the third parameter) */
   /* from the SDP Database.  This MACRO simply maps to the             */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on, the Serial Port ID (returned from a     */
   /* successful call to the BTSER_Open_Server_Port() function), and the*/
   /* SDP Service Record Handle.  The SDP Service Record Handle was     */
   /* returned via a succesful call to the BTSER_Register_SDP_Record()  */
   /* function.  See the BTSER_Register_SDP_Record() function for more  */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define BTSER_Un_Register_SDP_Record(__BluetoothStackID, __BTSERID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for Opening a Remote BT     */
   /* Serial Port on the specified Remote Device.  This function accepts*/
   /* the Bluetooth Stack ID of the Bluetooth Stack which is to open the*/
   /* BT Serial Connection as the first parameter.  The second parameter*/
   /* specifies the Board Address (NON NULL) of the Remote Bluetooth    */
   /* Device to connect with.  The next parameter specifies the Remote  */
   /* Server Channel ID to connect.  The fourth parameter specifies the */
   /* Local Serial Port to open.  The fifth and sixth parameters specify*/
   /* the Baud Rate and the Flow Control mechanism.  The final two      */
   /* parameters specify the BTSER Event Callback function, and callback*/
   /* parameter, respectively, of the BTSER Event Callback that is to   */
   /* process any further interaction with the specified Remote Port    */
   /* (Opening Status, Data Writes, etc).  This function returns a      */
   /* non-zero, positive, value if successful, or a negative return     */
   /* error code if this function is unsuccessful.  If this function is */
   /* successful, the return value will represent the Serial Port ID    */
   /* that can be passed to all other functions that require it.  Once a*/
   /* Serial Port is opened, it can only be closed via a call to the    */
   /* BTSER_Close_Port() function (passing the return value from this   */
   /* function).                                                        */
   /* * NOTE * If the LocalBaudRate parameter is zero then the Serial   */
   /*          Port that is open will specify a Virtual Serial Port not */
   /*          a physical Serial Port.                                  */
   /* * NOTE * If the LocalBaudRate parameter is zero, then the the     */
   /*          LocalHardwareFlowControl flag has special meaning.  The  */
   /*          meaning will change if this flag is set to any of the    */
   /*          constants specified above.  Currently the flags that can */
   /*          be specified are:                                        */
   /*             - BTSER_MAP_DTR_DSR_TO_DATA_CARRIER_DETECT            */
   /*               If specified, then the DTR signal is mapped to the  */
   /*               Data Carrier Detect (DCD) signal for output, and    */
   /*               on input, the DSR Signal is mapped to the Data      */
   /*               Carrier Detect (DCD) signal.  This is required for  */
   /*               true NULL Modem Emulation which requires the Data   */
   /*               Carrier Detect signal for proper functionality.     */
   /*               Note that this meaning ONLY applies when a Virtual  */
   /*               Serial Port is specified (the LocalBaudRate         */
   /*               parameter is zero).                                 */
   /*             - BTSER_ASSERT_DTR_UPON_CONNECTION                    */
   /*               If specified, then the DTR signal will be asserted  */
   /*               as long as the connection is valid (i.e. DTR will   */
   /*               be asserted as long as the Bluetooth Connection is  */
   /*               established).  Note that this meaning ONLY applies  */
   /*               when a Virtual Serial Port is specified (the        */
   /*               LocalBaudRate parameter is zero).                   */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Open_Remote_Port(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalSerialPort, unsigned int LocalBaudRate, unsigned int LocalHardwareFlowControl, BTSER_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Open_Remote_Port_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalSerialPort, unsigned int LocalBuadRate, unsigned int LocalHardwareFlowControl, BTSER_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function exists to close a BT Serial Port that was  */
   /* previously opened with the BTSER_Open_Server_Port() function OR   */
   /* the BTSER_Open_Remote_Port() function.  This function accepts as  */
   /* input the Bluetooth Stack ID of the Bluetooth Stack which the Open*/
   /* BT Serial Port resides and the Serial Port ID (return value from  */
   /* one ot the above mentioned Open functions) of the Port to Close.  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.  This function does NOT Un-Register a*/
   /* BT Serial Server Port from the system, it ONLY disconnects any    */
   /* connection that is currently active on the Server Port.  The      */
   /* BTSER_Close_Server_Port() function can be used to Un-Register the */
   /* BTSER Server Port.                                                */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Close_Port(unsigned int BluetoothStackID, unsigned int BTSERID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Close_Port_t)(unsigned int BluetoothStackID, unsigned int BTSERID);
#endif

   /* The following function exists to send data out the SPP side of the*/
   /* Virtual Port.  This function accepts as input the Bluetooth Stack */
   /* ID of the Bluetooth Stack which the Open BT Serial Port resides   */
   /* and the BT Serial Port ID of the Port in which to send data, the  */
   /* Length of the Data to send and a pointer to the Data Buffer to    */
   /* Send.  This function returns the number of data bytes that were   */
   /* successfully sent, or a negative return error code if             */
   /* unsuccessful.                                                     */
   /* * NOTE * Under normal circumstances this function should never be */
   /*          called, it exists as a means to poke data into the SPP   */
   /*          data stream in the rare case this might be necessary.    */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Data_Write(unsigned int BluetoothStackID, unsigned int BTSERID, Word_t DataLength, Byte_t *DataBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Data_Write_t)(unsigned int BluetoothStackID, unsigned int BTSERID, Word_t DataLength, Byte_t *DataBuffer);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* query the SPP Server Connection Mode.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol      */
   /* Stack, the BT Serial Port ID (which *MUST* have been obtained by  */
   /* calling the BTSER_Open_Server_Port() function), and as the final  */
   /* parameter a pointer to a Server Connection Mode variable which    */
   /* will receive the current Server Connection Mode.  This function   */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Get_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int BTSERID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int BTSERID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* change the current SPP Server Connection Mode.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Local Bluetooth    */
   /* Protocol Stack, the Bluetooth Serial Port ID (which *MUST* have   */
   /* been obtained by calling the BTSER_Open_Server_Port() function),  */
   /* and as the final parameter the new Server Connection Mode to use. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Set_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int BTSERID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int BTSERID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);
#endif

   /* The following function exists to allow the user a mechanism for   */
   /* either aborting ALL Data present in either an Input or an Output  */
   /* Buffer, or a means to wait until a ALL Data present in either an  */
   /* Output buffer has been removed.  This function accepts as input   */
   /* the Bluetooth Stack ID of the Local Bluetooth Protocol Stack, the */
   /* BT Serial Port ID, and the Purge Buffer Mask containing which     */
   /* Purge Buffer functionality to perform.  This function returns zero*/
   /* if successful, or a negative return error code if unsuccessful.   */
   /* ** NOTE ** When using a PurgeBufferMask of                        */
   /*            BTSER_PURGE_MASK_TRANSMIT_FLUSH_BIT, if the Transmit   */
   /*            Buffer is already empty this function will return      */
   /*            BTPS_ERROR_SPP_BUFFER_EMPTY (See BTErrors.h).          */
BTPSAPI_DECLARATION int BTPSAPI BTSER_Purge_Buffer(unsigned int BluetoothStackID, unsigned int BTSERID, unsigned long PurgeBufferMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BTSER_Purge_Buffer_t)(unsigned int BluetoothStackID, unsigned int BTSERID, unsigned long PurgeBufferMask);
#endif

#endif
