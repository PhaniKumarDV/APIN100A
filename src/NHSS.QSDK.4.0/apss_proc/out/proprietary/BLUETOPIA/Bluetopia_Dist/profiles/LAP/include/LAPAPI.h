/*****< lapapi.h >*************************************************************/
/*      Copyright 2001 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LAPAPI - Stonestreet One Bluetooth Stack LAN Access Profile API Type      */
/*           Definitions, Constants, and Prototypes.                          */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/01  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __LAPAPIH__
#define __LAPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTLAP_ERROR_INVALID_PARAMETER                             (-1000)
#define BTLAP_ERROR_NOT_INITIALIZED                               (-1001)
#define BTLAP_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTLAP_ERROR_LIBRARY_INITIALIZATION_ERROR                  (-1003)
#define BTLAP_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTLAP_ERROR_UNABLE_TO_OPEN_LOCAL_VIRTUAL_PORT             (-1005)

   /* SDP Service Classes for the LAN Access Profile.                   */

   /* The following MACRO is a utility MACRO that assigns the LAN Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (LAN_ACCESS_PROFILE_UUID_16) to the specified UUID_16_t variable. */
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the LAN_ACCESS_PROFILE_UUID_16 Constant value. */
#define SDP_ASSIGN_LAN_ACCESS_PROFILE_UUID_16(_x)       ASSIGN_SDP_UUID_16((_x), 0x11, 0x02)

   /* The following MACRO is a utility MACRO that assigns the LAN Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (LAN_ACCESS_PROFILE_UUID_32) to the specified UUID_32_t variable. */
   /* This MACRO accepts one parameter which is the UUID_32_t variable  */
   /* that is to receive the LAN_ACCESS_PROFILE_UUID_32 Constant value. */
#define SDP_ASSIGN_LAN_ACCESS_PROFILE_UUID_32(_x)       ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x02)

   /* The following MACRO is a utility MACRO that assigns the LAN Access*/
   /* Profile Service Class Bluetooth Universally Unique Identifier     */
   /* (LAN_ACCESS_PROFILE_UUID_128) to the specified UUID_128_t         */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* LAN_ACCESS_PROFILE_UUID_128 Constant value.                       */
#define SDP_ASSIGN_LAN_ACCESS_PROFILE_UUID_128(_x)      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* LAN Access Profile Servers.                                       */
#define LAP_PROFILE_VERSION                                             (0x0100)

   /* The following constants represent the Port Open Status Values     */
   /* that are possible in the LAP Open Port Confirmation Event Data    */
   /* Information.                                                      */
#define LAP_OPEN_PORT_STATUS_SUCCESS                                    0x00
#define LAP_OPEN_PORT_STATUS_CONNECTION_TIMEOUT                         0x01
#define LAP_OPEN_PORT_STATUS_CONNECTION_REFUSED                         0x02
#define LAP_OPEN_PORT_STATUS_UNKNOWN_ERROR                              0x03

   /* LAP Event API Types.                                              */
typedef enum
{
   etLAP_Open_Port_Indication,
   etLAP_Open_Port_Confirmation,
   etLAP_Close_Port_Indication,
   etLAP_Open_Port_Request_Indication
} LAP_Event_Type_t;

typedef struct _tagLAP_Open_Port_Indication_Data_t
{
   unsigned int LAPPortID;
   BD_ADDR_t    BD_ADDR;
} LAP_Open_Port_Indication_Data_t;

#define LAP_OPEN_PORT_INDICATION_DATA_SIZE              (sizeof(LAP_Open_Port_Indication_Data_t))

typedef struct _tagLAP_Open_Port_Confirmation_Data_t
{
   unsigned int LAPPortID;
   unsigned int PortOpenStatus;
} LAP_Open_Port_Confirmation_Data_t;

#define LAP_OPEN_PORT_CONFIRMATION_DATA_SIZE            (sizeof(LAP_Open_Port_Confirmation_Data_t))

typedef struct _tagLAP_Close_Port_Indication_Data_t
{
   unsigned int LAPPortID;
} LAP_Close_Port_Indication_Data_t;

#define LAP_CLOSE_PORT_INDICATION_DATA_SIZE             (sizeof(LAP_Close_Port_Indication_Data_t))

typedef struct _tagLAP_Open_Port_Request_Indication_Data_t
{
   unsigned int LAPPortID;
   BD_ADDR_t    BD_ADDR;
} LAP_Open_Port_Request_Indication_Data_t;

#define LAP_OPEN_PORT_REQUEST_INDICATION_DATA_SIZE      (sizeof(LAP_Open_Port_Request_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all LAP Event Data Data.                                  */
typedef struct _tagLAP_Event_Data_t
{
   LAP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      LAP_Open_Port_Indication_Data_t         *LAP_Open_Port_Indication_Data;
      LAP_Open_Port_Confirmation_Data_t       *LAP_Open_Port_Confirmation_Data;
      LAP_Close_Port_Indication_Data_t        *LAP_Close_Port_Indication_Data;
      LAP_Open_Port_Request_Indication_Data_t *LAP_Open_Port_Request_Indication_Data;
   } Event_Data;
} LAP_Event_Data_t;

#define LAP_EVENT_DATA_SIZE                             (sizeof(LAP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an LAP Event Receive Data Callback.  This function will be called */
   /* whenever a LAP Event occurs that is associated with the specified */
   /* Bluetooth Stack ID.  This function passes to the caller the       */
   /* Bluetooth Stack ID, the LAP Event Data that occurred and the LAP  */
   /* Event Callback Parameter that was specified when this Callback    */
   /* was installed.  The caller is free to use the contents of the     */
   /* LAP Event Data ONLY in the context of this callback.  If the      */
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
   /* (this argument holds anyway because another LAP Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving LAP Event Packets.  */
   /*            A Deadlock WILL occur because NO LAP Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
typedef void (BTPSAPI *LAP_Event_Callback_t)(unsigned int BluetoothStackID, LAP_Event_Data_t *LAP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening a LAN Access    */
   /* Server Gateway on the specified Local Virtual Port.  This function*/
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* Instance to use for the LAP Server, the Local Serial Port Server  */
   /* Number to use, the physical Virtual Port Number, and the LAP Event*/
   /* Callback function (and parameter) to associate with the specified */
   /* LAP Port.  The ServerPort parameter *MUST* be between             */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  This        */
   /* function returns a non-zero, positive, value if successful, or a  */
   /* negative return error code if an error occurs.  A successful      */
   /* return code will be a LAP Port ID that can be used to reference   */
   /* the Opened LAP Port in ALL other functions in this module (except */
   /* the LAP_Open_Remote_Port() function).  Once a Server LAP Port is  */
   /* opened, it can only be Un-Registered via a call to the            */
   /* LAP_Close_Server_Port() function (passing the return value from   */
   /* this function).  The LAP_Close_Port() function can be used to     */
   /* Disconnect a Client from the Server Port (if one is connected, it */
   /* will NOT Un-Register the Server Port however).                    */
BTPSAPI_DECLARATION int BTPSAPI LAP_Open_Server_Port(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalVirtualPort, LAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Open_Server_Port_t)(unsigned int BluetoothStackID, unsigned int ServerPort, unsigned int LocalVirtualPort, LAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering a LAP    */
   /* Port Server (which was Registered by a successful call to the     */
   /* LAP_Open_Server_Port() function).  This function accepts as input */
   /* the Bluetooth Stack ID of the Bluetooth Protocol Stack that the   */
   /* LAP Port specified by the Second Parameter is valid for.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred (see BTERRORS.H).  Note that this       */
   /* function does NOT delete any SDP Service Record Handles.          */
BTPSAPI_DECLARATION int BTPSAPI LAP_Close_Server_Port(unsigned int BluetoothStackID, unsigned int LAPPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Close_Server_Port_t)(unsigned int BluetoothStackID, unsigned int LAPPortID);
#endif

   /* The following function is responsible for responding to requests  */
   /* to connect to a LAP Port Server.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Local Bluetooth Protocol Stack, the */
   /* LAP Port ID (which *MUST* have been obtained by calling the       */
   /* LAP_Open_Server_Port() function), and as the final parameter      */
   /* whether to accept the pending connection request.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI LAP_Open_Port_Request_Response(unsigned int BluetoothStackID, unsigned int LAPPortID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Open_Port_Request_Response_t)(unsigned int BluetoothStackID, unsigned int LAPPortID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic LAP Service Record to the SDP Database.  This function    */
   /* takes as input the Bluetooth Stack ID of the Local Bluetooth      */
   /* Protocol Stack, the Serial Port ID (which *MUST* have been        */
   /* obtained by calling the LAP_Open_Server_Port() function).  The    */
   /* third parameter specifies the Service Name to associate with the  */
   /* SDP Record.  The final parameter is a pointer to a DWord_t which  */
   /* receives the SDP Service Record Handle if this function           */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be         */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function should only be called with the LAP Port ID */
   /*          that was returned from the LAP_Open_Server_Port()        */
   /*          function.  This function should NEVER be used with the   */
   /*          LAP Port ID returned from the LAP_Open_Remote_Port()     */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          LAP_Un_Register_SDP_Record() to                          */
   /*          SDP_Delete_Service_Record().                             */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI LAP_Register_SDP_Record(unsigned int BluetoothStackID, unsigned int LAPPortID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Register_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int LAPPortID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* LAP SDP Service Record (specified by the third parameter) from    */
   /* the SDP Database.  This MACRO simply maps to the                  */
   /* SDP_Delete_Service_Record() function.  This MACRO is only         */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the Serial Port ID (returned from   */
   /* a successful call to the LAP_Open_Server_Port() function), and the*/
   /* SDP Service Record Handle.  The SDP Service Record Handle was     */
   /* returned via a succesful call to the LAP_Register_SDP_Record()    */
   /* function.  See the LAP_Register_SDP_Record() function for more    */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success   */
   /* or a negative return error code (see BTERRORS.H).                 */
#define LAP_Un_Register_SDP_Record(__BluetoothStackID, __LAPPortID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for Opening a Remote LAP    */
   /* Port on the specified Remote Device.  This function accepts the   */
   /* Bluetooth Stack ID of the Bluetooth Stack which is to open the LAP*/
   /* Connection as the first parameter.  The second parameter specifies*/
   /* the Board Address (NON NULL) of the Remote Bluetooth Device to    */
   /* connect with.  The next parameter specifies the Remote Server     */
   /* Channel ID to connect.  The fourth parameter specifies the Local  */
   /* Port (Virtual Port to open).  The final two parameters specify the*/
   /* LAP Event Callback function, and callback parameter, respectively,*/
   /* of the LAP Event Callback that is to process any further          */
   /* interaction with the specified Remote Port (Opening Status, Data  */
   /* Writes, etc).  This function returns a non-zero, positive, value  */
   /* if successful, or a negative return error code if this function is*/
   /* unsuccessful.  If this function is successful, the return value   */
   /* will represent the Serial Port ID that can be passed to all other */
   /* functions that require it.  Once a Serial Port is opened, it can  */
   /* only be closed via a call to the LAP_Close_Port() function        */
   /* (passing the return value from this function).                    */
BTPSAPI_DECLARATION int BTPSAPI LAP_Open_Remote_Port(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalVirtualPort, LAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Open_Remote_Port_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, unsigned int LocalVirtualPort, LAP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function exists to close a LAP Port that was        */
   /* previously opened with the LAP_Open_Server_Port() function OR the */
   /* LAP_Open_Remote_Port() function.  This function accepts as input  */
   /* the Bluetooth Stack ID of the Bluetooth Stack which the Open      */
   /* LAP Port resides and the Serial Port ID (return value from one of */
   /* the above mentioned Open functions) of the Port to Close.  This   */
   /* function returns zero if successful, or a negative return value   */
   /* if there was an error.  This function does NOT Un-Register a LAP  */
   /* Server Port from the system, it ONLY disconnects any connection   */
   /* that is currently active on the Server Port.  The                 */
   /* LAP_Close_Server_Port() function can be used to Un-Register the   */
   /* LAP Server Port.                                                  */
BTPSAPI_DECLARATION int BTPSAPI LAP_Close_Port(unsigned int BluetoothStackID, unsigned int LAPPortID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Close_Port_t)(unsigned int BluetoothStackID, unsigned int LAPPortID);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* query the SPP Server Connection Mode.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Local Bluetooth Protocol      */
   /* Stack, the LAP Port ID (which *MUST* have been obtained by calling*/
   /* the LAP_Open_Server_Port() function), and as the final parameter a*/
   /* pointer to a Server Connection Mode variable which will receive   */
   /* the current Server Connection Mode.  This function returns zero if*/
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI LAP_Get_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int LAPPortID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Get_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int LAPPortID, SPP_Server_Connection_Mode_t *SPPServerConnectionMode);
#endif

   /* The following function is responsible for allowing a mechanism to */
   /* change the current SPP Server Connection Mode.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Local Bluetooth    */
   /* Protocol Stack, the LAP Port ID (which *MUST* have been obtained  */
   /* by calling the LAP_Open_Server_Port() function), and as the final */
   /* parameter the new Server Connection Mode to use.  This function   */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI LAP_Set_Server_Connection_Mode(unsigned int BluetoothStackID, unsigned int LAPPortID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LAP_Set_Server_Connection_Mode_t)(unsigned int BluetoothStackID, unsigned int LAPPortID, SPP_Server_Connection_Mode_t SPPServerConnectionMode);
#endif

#endif
