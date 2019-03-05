/*****< faxapi.h >*************************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  FAXAPI - Stonestreet One Bluetooth Stack FAX Profile API Type             */
/*           Definitions, Constants, and Prototypes.                          */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/01  D. Lange       Initial creation.                               */
/*   08/19/02  R. Sledge      Ported to Linux.                                */
/******************************************************************************/
#ifndef __FAXAPIH__
#define __FAXAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTFAX_ERROR_INVALID_PARAMETER                             (-1000)
#define BTFAX_ERROR_NOT_INITIALIZED                               (-1001)
#define BTFAX_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTFAX_ERROR_INSUFFICIENT_RESOURCES                        (-1003)
#define BTFAX_ERROR_UNABLE_TO_OPEN_LOCAL_PORT                     (-1004)

   /* SDP Service Classes for the FAX Profile.                          */

   /* The following MACRO is a utility MACRO that assigns the FAX       */
   /* Service Class Bluetooth Universally Unique Identifier             */
   /* (FAX_PROFILE_UUID_16) to the specified UUID_16_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_16_t variable that  */
   /* is to receive the FAX_PROFILE_UUID_16 Constant value.             */
#define SDP_ASSIGN_FAX_PROFILE_UUID_16(_x)              ASSIGN_SDP_UUID_16((_x), 0x11, 0x11)

   /* The following MACRO is a utility MACRO that assigns the FAX       */
   /* Service Class Bluetooth Universally Unique Identifier             */
   /* (FAX_PROFILE_UUID_32) to the specified UUID_32_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_32_t variable that  */
   /* is to receive the FAX_PROFILE_UUID_32 Constant value.             */
#define SDP_ASSIGN_FAX_PROFILE_UUID_32(_x)              ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x11)

   /* The following MACRO is a utility MACRO that assigns the FAX       */
   /* Service Class Bluetooth Universally Unique Identifier             */
   /* (FAX_PROFILE_UUID_128) to the specified UUID_32_t variable.  This */
   /* MACRO accepts one parameter which is the UUID_128_t variable that */
   /* is to receive the FAX_PROFILE_UUID_128 Constant value.            */
#define SDP_ASSIGN_FAX_PROFILE_UUID_128(_x)             ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following Bit Masks and Bit Values represent the possible     */
   /* supported FAX Classes used with the FAX_Register_SDP_Record()     */
   /* function.                                                         */
#define FAX_CLASS_SUPPORT_NONE_VALUE                                    0x00000000
#define FAX_CLASS_SUPPORT_ALL_VALUE                                     0xFFFFFFFF
#define FAX_CLASS_SUPPORT_FAX_CLASS_1_BIT_MASK                          0x00000001
#define FAX_CLASS_SUPPORT_FAX_CLASS_2_0_BIT_MASK                        0x00000002
#define FAX_CLASS_SUPPORT_FAX_CLASS_2_BIT_MASK                          0x00000004

   /* Defines the Profile Version Number used within the SDP Record for */
   /* FAX Servers.                                                      */
#define FAX_PROFILE_VERSION                                             (0x0100)

   /* The following constants represent the Port Open Status Values     */
   /* that are possible in the FAX Open Port Confirmation Event Data    */
   /* Information.                                                      */
#define FAX_OPEN_PORT_STATUS_SUCCESS                                    0x00
#define FAX_OPEN_PORT_STATUS_CONNECTION_TIMEOUT                         0x01
#define FAX_OPEN_PORT_STATUS_CONNECTION_REFUSED                         0x02
#define FAX_OPEN_PORT_STATUS_UNKNOWN_ERROR                              0x03

   /* FAX Event API Types.                                              */
typedef enum
{
   etFAX_Open_Port_Indication,
   etFAX_Open_Port_Confirmation,
   etFAX_Close_Port_Indication,
   etFAX_Open_Port_Request_Indication
} FAX_Event_Type_t;

typedef struct _tagFAX_Open_Port_Indication_Data_t
{
   unsigned int FAXPortID;
   BD_ADDR_t    BD_ADDR;
} FAX_Open_Port_Indication_Data_t;

#define FAX_OPEN_PORT_INDICATION_DATA_SIZE              (sizeof(FAX_Open_Port_Indication_Data_t))

typedef struct _tagFAX_Open_Port_Confirmation_Data_t
{
   unsigned int FAXPortID;
   unsigned int PortOpenStatus;
} FAX_Open_Port_Confirmation_Data_t;

#define FAX_OPEN_PORT_CONFIRMATION_DATA_SIZE            (sizeof(FAX_Open_Port_Confirmation_Data_t))

typedef struct _tagFAX_Close_Port_Indication_Data_t
{
   unsigned int FAXPortID;
} FAX_Close_Port_Indication_Data_t;

#define FAX_CLOSE_PORT_INDICATION_DATA_SIZE             (sizeof(FAX_Close_Port_Indication_Data_t))

typedef struct _tagFAX_Open_Port_Request_Indication_Data_t
{
   unsigned int FAXPortID;
   BD_ADDR_t    BD_ADDR;
} FAX_Open_Port_Request_Indication_Data_t;

#define FAX_OPEN_PORT_REQUEST_INDICATION_DATA_SIZE      (sizeof(FAX_Open_Port_Request_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all FAX Event Data Data.                                  */
typedef struct _tagFAX_Event_Data_t
{
   FAX_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      FAX_Open_Port_Indication_Data_t         *FAX_Open_Port_Indication_Data;
      FAX_Open_Port_Confirmation_Data_t       *FAX_Open_Port_Confirmation_Data;
      FAX_Close_Port_Indication_Data_t        *FAX_Close_Port_Indication_Data;
      FAX_Open_Port_Request_Indication_Data_t *FAX_Open_Port_Request_Indication_Data;
   } Event_Data;
} FAX_Event_Data_t;

#define FAX_EVENT_DATA_SIZE                             (sizeof(FAX_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an FAX Event Receive Data Callback.  This function will be called */
   /* whenever a FAX Event occurs that is associated with the specified */
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the FAX Event Data that occurred and the FAX  */
   /* Event Callback Parameter that was specified when this Callback    */
   /* was installed.  The caller is free to use the contents of the     */
   /* FAX Event Data ONLY in the context of this callback.  If the      */
   /* caller requires the Data for a longer period of time, then the    */
   /* callback function MUST copy the data into another Data Buffer.    */
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another FAX Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving FAX Event Packets.  */
   /*            A Deadlock WILL occur because NO FAX Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *FAX_Event_Callback_t)(unsigned int BluetoothStackID, FAX_Event_Data_t *FAX_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a FAX Profile   */
   /* Server Gateway on the specified physical Port.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* Instance to use for the FAX Server, the Local Serial Port Server  */
   /* Number to use, the physical Port Settings (Port Number, Baud Rate,*/
   /* and Flow Control Information), and the FAX Event Callback function*/
   /* (and parameter) to associate with the specified FAX Port.  The    */
   /* ServerPort parameter *MUST* be between SPP_PORT_NUMBER_MINIMUM and*/
   /* SPP_PORT_NUMBER_MAXIMUM.  This function returns a non-zero,       */
   /* positive, value if successful or a negative return error code if  */
   /* an error occurs.  A successful return code will be a FAX Port ID  */
   /* that can be used to reference the Opened FAX Port in ALL other    */
   /* functions in this module (except the FAX_Open_Remote_Port()       */
   /* function).  Once a Server FAX Port is opened, it can only be      */
   /* Un-Registered via a call to the FAX_Close_Server_Port() function  */
   /* (passing the return value from this function).  The               */
   /* FAX_Close_Port() function can be used to Disconnect a Client from */
   /* the Server Port (if one is connected, it will NOT Un-Register the */
   /* Server Port however).                                             */
BTPSAPI_DECLARATION int BTPSAPI FAX_Open_Server_Port(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalPort, unsigned int LocalBaudRate, Boolean_t LocalHardwareFlowControl, FAX_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Open_Server_Port_t)(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalPort, unsigned int LocalBaudRate, Boolean_t LocalHardwareFlowControl, FAX_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a FAX    */
   /* Port Server (which was Registered by a successful call to the     */
   /* FAX_Open_Server_Port() function).  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* FAX Port specified by the Second Parameter is valid for.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred (see BTERRORS.H).  Note that this       */
   /* function does NOT delete any SDP Service Record Handles.          */
BTPSAPI_DECLARATION int BTPSAPI FAX_Close_Server_Port(unsigned int BluetoothStackID, unsigned int FAXPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Close_Server_Port_t)(unsigned int BluetoothStackID, unsigned int FAXPortID);
#endif

   /* The following function is responsible for responding to requests  */
   /* to connect to a FAX Port Server.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Local Bluetooth Protocol Stack, the */
   /* FAX Port ID (which *MUST* have been obtained by calling the       */
   /* FAX_Open_Server_Port() function), and as the final parameter      */
   /* whether to accept the pending connection request.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI FAX_Open_Port_Request_Response(unsigned int BluetoothStackID, unsigned int FAXPortID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Open_Port_Request_Response_t)(unsigned int BluetoothStackID, unsigned int FAXPortID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic FAX Service Record to the SDP Database.  This function    */
   /* takes as input the Bluetooth Stack ID of the Local Bluetooth      */
   /* Protocol Stack, the Serial Port ID (which *MUST* have been        */
   /* obtained by calling the FAX_Open_Server_Port() function).  The    */
   /* third parameter is a BIT Mask that contains a list of all         */
   /* the supported FAX Classes that this FAX Server Supports.  The     */
   /* fourth parameter specifies the Service Name to associate with the */
   /* SDP Record.  The final parameter is a pointer to a DWord_t which  */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be         */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function should only be called with the FAX Port ID */
   /*          that was returned from the FAX_Open_Server_Port()        */
   /*          function.  This function should NEVER be used with the   */
   /*          FAX Port ID returned from the FAX_Open_Remote_Port()     */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          FAX_Un_Register_SDP_Record() to                          */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI FAX_Register_SDP_Record(unsigned int BluetoothStackID, unsigned int FAXPortID, unsigned int FAXClassSupport, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Register_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int FAXPortID, unsigned int FaxClassSupport, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* FAX SDP Service Record (specified by the third parameter) from    */
   /* the SDP Database.  This MACRO simply maps to the                  */
   /* SDP_Delete_Service_Record() function.  This MACRO is only         */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the Serial Port ID (returned from   */
   /* a successful call to the FAX_Open_Server_Port() function), and the*/
   /* SDP Service Record Handle.  The SDP Service Record Handle was     */
   /* returned via a succesful call to the FAX_Register_SDP_Record()    */
   /* function.  See the FAX_Register_SDP_Record() function for more    */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success   */
   /* or a negative return error code (see BTERRORS.H).                 */
#define FAX_Un_Register_SDP_Record(__BluetoothStackID, __FAXPortID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for Opening a Remote FAX    */
   /* Port on the specified Remote Device.  This function accepts the   */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to open the FAX*/
   /* Connection as the first parameter.  The second parameter specifies*/
   /* the Board Address (NON NULL) of the Remote Bluetooth Device to    */
   /* connect with.  The next parameter specifies the Remote Server     */
   /* Channel ID to connect.  The fourth parameter specifies the Local  */
   /* Port (Virtual Port to open).  The final two parameters specify the*/
   /* FAX Event Callback function, and callback parameter, respectively,*/
   /* of the FAX Event Callback that is to process any further          */
   /* interaction with the specified Remote Port (Opening Status, Data  */
   /* Writes, etc).  This function returns a non-zero, positive, value  */
   /* if successful, or a negative return error code if this function is*/
   /* unsuccessful.  If this function is successful, the return value   */
   /* will represent the Serial Port ID that can be passed to all other */
   /* functions that require it.  Once a Serial Port is opened, it can  */
   /* only be closed via a call to the FAX_Close_Port() function        */
   /* (passing the return value from this function).                    */
BTPSAPI_DECLARATION int BTPSAPI FAX_Open_Remote_Port(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalVirtualPort, FAX_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Open_Remote_Port_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalVirtualPort, FAX_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function exists to close a FAX Port that was        */
   /* previously opened with the FAX_Open_Server_Port() function OR the */
   /* FAX_Open_Remote_Port() function.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Bluetooth Stack which the Open      */
   /* FAX Port resides and the Serial Port ID (return value from one of */
   /* the above mentioned Open functions) of the Port to Close.  This   */
   /* function returns zero if successful, or a negative return value   */
   /* if there was an error.  This function does NOT Un-Register a FAX  */
   /* Server Port from the system, it ONLY disconnects any connection   */
   /* that is currently active on the Server Port.  The                 */
   /* FAX_Close_Server_Port() function can be used to Un-Register the   */
   /* FAX Server Port.                                                  */
BTPSAPI_DECLARATION int BTPSAPI FAX_Close_Port(unsigned int BluetoothStackID, unsigned int FAXPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Close_Port_t)(unsigned int BluetoothStackID, unsigned int FAXPortID);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* query the SPP Server Connection Mode.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol      */
   /* Stack, the FAX Port ID (which *MUST* have been obtained by calling*/
   /* the FAX_Open_Server_Port() function), and as the final parameter a*/
   /* pointer to a Server Connection Mode variable which will receive   */
   /* the current Server Connection Mode.  This function returns zero if*/
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI FAX_Get_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int FAXPortID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int FAXPortID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* change the current SPP Server Connection Mode.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Local Bluetooth    */
   /* Protocol Stack, the FAX Port ID (which *MUST* have been obtained  */
   /* by calling the FAX_Open_Server_Port() function), and as the final */
   /* parameter the new Server Connection Mode to use.  This function   */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI FAX_Set_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int FAXPortID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_FAX_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int FAXPortID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);
#endif

#endif
