/*****< oppapi.h >*************************************************************/
/*      Copyright 2009 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OPPAPI - Stonestreet One Bluetooth Object Push Profile (OPP)              */
/*           API Type Definitions, Constants, and Prototypes.                 */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/07/09  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __OPPAPIH__
#define __OPPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTOPP_ERROR_INVALID_PARAMETER                            (-1000)
#define BTOPP_ERROR_NOT_INITIALIZED                              (-1001)
#define BTOPP_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1002)
#define BTOPP_ERROR_LIBRARY_INITIALIZATION_ERROR                 (-1003)
#define BTOPP_ERROR_INSUFFICIENT_RESOURCES                       (-1004)
#define BTOPP_ERROR_REQUEST_ALREADY_OUTSTANDING                  (-1005)
#define BTOPP_ERROR_ACTION_NOT_ALLOWED                           (-1006)
#define BTOPP_ERROR_INSUFFICIENT_PACKET_LENGTH                   (-1007)

   /* SDP Profile UUID's for the OBEX Object Push Profile.              */

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_16) to the specified UUID_16_t variable.*/
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the OBJECT_PUSH_PROFILE_UUID_16 Constant value.*/
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_16(_x)               ASSIGN_SDP_UUID_16((_x), 0x11, 0x05)

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_32) to the specified UUID_32_t variable.*/
   /* This MACRO accepts one parameter which is the UUID_32_t variable  */
   /* that is to receive the OBJECT_PUSH_PROFILE_UUID_32 Constant value.*/
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_32(_x)               ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x05)

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_128) to the specified UUID_128_t        */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* OBJECT_PUSH_PROFILE_UUID_128 Constant value.                      */
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_128(_x)              ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following definitions represent the defined Object Types      */
   /* that can be supported by the Object Push Server.  These SDP       */
   /* Definitions are used with the Bluetooth Profile Descriptor List   */
   /* Attribute.                                                        */
#define SDP_OBJECT_SUPPORTED_FEATURE_VCARD2_1                    0x01
#define SDP_OBJECT_SUPPORTED_FEATURE_VCARD3_0                    0x02
#define SDP_OBJECT_SUPPORTED_FEATURE_VCALENDAR1_0                0x03
#define SDP_OBJECT_SUPPORTED_FEATURE_ICALENDAR1_0                0x04
#define SDP_OBJECT_SUPPORTED_FEATURE_VNOTE                       0x05
#define SDP_OBJECT_SUPPORTED_FEATURE_VMESSAGE                    0x06
#define SDP_OBJECT_SUPPORTED_FEATURE_ANY_OBJECT                  0xFF

   /* The following constant defines the Profile Version Number used    */
   /* within the SDP Record for Object Push Profile Servers (supported  */
   /* by this implementation).                                          */
#define OPP_PROFILE_VERSION                                      (0x0101)

   /* The following define the possible OPP Response codes.             */
   /* * NOTE * Response Codes less than 0x10                            */
   /*          (OPP_OBEX_RESPONSE_CONTINUE) are Reserved and CANNOT be  */
   /*          used.                                                    */
#define OPP_OBEX_RESPONSE_CONTINUE                               (OBEX_CONTINUE_RESPONSE)
#define OPP_OBEX_RESPONSE_OK                                     (OBEX_OK_RESPONSE)
#define OPP_OBEX_RESPONSE_PARTIAL_CONTENT                        (OBEX_PARTIAL_CONTENT_RESPONSE)

#define OPP_OBEX_RESPONSE_BAD_REQUEST                            (OBEX_BAD_REQUEST_RESPONSE)
#define OPP_OBEX_RESPONSE_NOT_IMPLEMENTED                        (OBEX_NOT_IMPLEMENTED_RESPONSE)
#define OPP_OBEX_RESPONSE_FORBIDDEN                              (OBEX_FORBIDDEN_RESPONSE)
#define OPP_OBEX_RESPONSE_UNAUTHORIZED                           (OBEX_UNAUTHORIZED_RESPONSE)
#define OPP_OBEX_RESPONSE_PRECONDITION_FAILED                    (OBEX_PRECONDITION_FAILED_RESPONSE)
#define OPP_OBEX_RESPONSE_NOT_FOUND                              (OBEX_NOT_FOUND_RESPONSE)
#define OPP_OBEX_RESPONSE_NOT_ACCEPTABLE                         (OBEX_NOT_ACCEPTABLE_RESPONSE)
#define OPP_OBEX_RESPONSE_SERVICE_UNAVAILABLE                    (OBEX_SERVICE_UNAVAILABLE_RESPONSE)
#define OPP_OBEX_RESPONSE_SERVER_ERROR                           (OBEX_INTERNAL_SERVER_ERROR_RESPONSE)

   /* The following constants represent the Minimum and Maximum Port    */
   /* Numbers that can be opened (both locally and remotely).  These    */
   /* constants specify the range for the Port Number parameters in the */
   /* Open Functions.                                                   */
#define OPP_PORT_NUMBER_MINIMUM                                  (SPP_PORT_NUMBER_MINIMUM)
#define OPP_PORT_NUMBER_MAXIMUM                                  (SPP_PORT_NUMBER_MAXIMUM)

   /* The following constants represent the Open Status Values that are */
   /* possible for the OpenStatus member of the Open Confirmation       */
   /* events.                                                           */
#define OPP_OPEN_STATUS_SUCCESS                                  (0x00)
#define OPP_OPEN_STATUS_CONNECTION_TIMEOUT                       (0x01)
#define OPP_OPEN_STATUS_CONNECTION_REFUSED                       (0x02)
#define OPP_OPEN_STATUS_UNKNOWN_ERROR                            (0x04)

   /* The following enumerations specify the Type of objects that are   */
   /* supported by the Object Push Module.                              */
typedef enum
{
   oppvCard,
   oppvCalendar,
   oppiCalendar,
   oppvNote,
   oppvMessage,
   oppUnknownObject
} OPP_Object_Type_t;

   /* The following definitions define the currently defined Object     */
   /* Type File Extensions.                                             */
#define OPP_VCARD_DEFAULT_FILE_EXTENSION                         "vcf"
#define OPP_VCALENDAR_DEFAULT_FILE_EXTENSION                     "vcs"
#define OPP_VNOTE_DEFAULT_FILE_EXTENSION                         "vnt"
#define OPP_VMESSAGE_DEFAULT_FILE_EXTENSION                      "vmg"

   /* The following definitions represent the defined Object Types that */
   /* can be supported by the Object Push Server.  These Definitions are*/
   /* used to create a Supported Format Mask supplied when Registering a*/
   /* SDP Record function (used with the                                */
   /* OPP_Register_Object_Push_Server_SDP_Record() function).           */
#define OPP_SUPPORTED_FORMAT_VCARD_2_1                           0x00000001
#define OPP_SUPPORTED_FORMAT_VCARD_3_0                           0x00000002
#define OPP_SUPPORTED_FORMAT_VCALENDAR_1_0                       0x00000004
#define OPP_SUPPORTED_FORMAT_ICALENDAR_1_0                       0x00000008
#define OPP_SUPPORTED_FORMAT_VNOTE                               0x00000010
#define OPP_SUPPORTED_FORMAT_VMESSAGE                            0x00000020
#define OPP_SUPPORTED_FORMAT_ALL_OBJECTS                         0x80000000

#define OPP_SUPPORTED_FORMAT_MASK                                (OPP_SUPPORTED_FORMAT_VCARD_2_1 | OPP_SUPPORTED_FORMAT_VCARD_3_0 | OPP_SUPPORTED_FORMAT_VCALENDAR_1_0 | OPP_SUPPORTED_FORMAT_ICALENDAR_1_0 | OPP_SUPPORTED_FORMAT_VNOTE | OPP_SUPPORTED_FORMAT_VMESSAGE | OPP_SUPPORTED_FORMAT_ALL_OBJECTS)

   /* The following BIT definitions are used to denote the possible     */
   /* Object Push Server Modes that can be applied to a Object Push     */
   /* Client Connection.  These BIT definitions are used with the       */
   /* OPP_Set_Server_Mode() and OPP_Get_Server_Mode() mode functions.   */
#define OPP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION              (0x00000000)
#define OPP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION                 (0x00000001)
#define OPP_SERVER_MODE_CONNECTION_MASK                          (0x00000001)

   /* Object Push Profile Event API Types.                              */
typedef enum
{
   etOPP_Open_Request_Indication,
   etOPP_Open_Port_Indication,
   etOPP_Open_Port_Confirmation,
   etOPP_Close_Port_Indication,
   etOPP_Abort_Indication,
   etOPP_Abort_Confirmation,
   etOPP_Push_Object_Indication,
   etOPP_Push_Object_Confirmation,
   etOPP_Pull_Business_Card_Indication,
   etOPP_Pull_Business_Card_Confirmation
} OPP_Event_Type_t;

   /* The following event is dispatched when a remote client requests a */
   /* connection to a local server.  The OPPID member specifies the     */
   /* identifier of the Local Object Push Profile Server being connect  */
   /* with.  The BD_ADDR member specifies the address of the Remote     */
   /* Client requesting the connection to the Local Server.             */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            OPP_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagOPP_Open_Request_Indication_Data_t
{
   unsigned int OPPID;
   BD_ADDR_t    BD_ADDR;
} OPP_Open_Request_Indication_Data_t;

#define OPP_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(OPP_Open_Request_Indication_Data_t))

   /* The following event is dispatched when a remote Object Push       */
   /* Profile Client makes a Connection to a locally Registered Object  */
   /* Push Profile Server Service Port.  The OPPID member specifies the */
   /* identifier of the Local Object Push Profile Server that is being  */
   /* connected with.  The BD_ADDR member specifies the address of the  */
   /* Object Push Profile Client that is being connected to the Local   */
   /* Server.                                                           */
typedef struct _tagOPP_Open_Port_Indication_Data_t
{
   unsigned int OPPID;
   BD_ADDR_t    BD_ADDR;
} OPP_Open_Port_Indication_Data_t;

#define OPP_OPEN_PORT_INDICATION_DATA_SIZE              (sizeof(OPP_Open_Port_Indication_Data_t))

   /* The following event is dispatched to the Local Object Push Profile*/
   /* Client to indicate the success or failure of a previously         */
   /* submitted Connection Attempt to a remote Server Port.  The OPPID  */
   /* member specifies the Identifier of the Local Object Push Profile  */
   /* Client that has requested the Connection.  The OpenStatus member  */
   /* specifies the status of the Connection Attempt.  Valid values are:*/
   /* - OPP_OPEN_STATUS_SUCCESS - OPP_OPEN_STATUS_CONNECTION_TIMEOUT -  */
   /* OPP_OPEN_STATUS_CONNECTION_REFUSED - OPP_OPEN_STATUS_UNKNOWN_ERROR*/
typedef struct _tagOPP_Open_Port_Confirmation_Data_t
{
   unsigned int OPPID;
   Byte_t       OpenStatus;
} OPP_Open_Port_Confirmation_Data_t;

#define OPP_OPEN_PORT_CONFIRMATION_DATA_SIZE            (sizeof(OPP_Open_Port_Confirmation_Data_t))

   /* The following event is dispatched when the Object Push Profile    */
   /* Client or Server disconnects an active connection.  The OPPID     */
   /* member specifies the Identifier for the Local Object Push Profile */
   /* connection being closed.                                          */
typedef struct _tagOPP_Close_Port_Indication_Data_t
{
   unsigned int OPPID;
} OPP_Close_Port_Indication_Data_t;

#define OPP_CLOSE_PORT_INDICATION_DATA_SIZE             (sizeof(OPP_Close_Port_Indication_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Server when the Client sends an Abort Request.  The OPPID member  */
   /* specifies the Identifier of the Local Object Push Profile Server  */
   /* receiving this event.                                             */
typedef struct _tagOPP_Abort_Indication_Data_t
{
   unsigned int OPPID;
} OPP_Abort_Indication_Data_t;

#define OPP_ABORT_INDICATION_DATA_SIZE                  (sizeof(OPP_Abort_Indication_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Client when the Server sends an Abort Response.  The OPPID member */
   /* specifies the Identifier of the Local Object Push Profile Client  */
   /* receiving this event.                                             */
typedef struct _tagOPP_Abort_Confirmation_Data_t
{
   unsigned int OPPID;
} OPP_Abort_Confirmation_Data_t;

#define OPP_ABORT_CONFIRMATION_DATA_SIZE                (sizeof(OPP_Abort_Confirmation_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Server when the Client sends a Object Push Request.  The OPPID    */
   /* member specifies the Identifier of the Local Object Push Profile  */
   /* Client receiving this event.  The ObjectName member is a pointer  */
   /* to a NULL Terminated UNICODE string that specifies the name of the*/
   /* Object.  The DataLength member indicates the number of bytes that */
   /* are present in the DataBuffer buffer.  The DataBuffer member is a */
   /* pointer to a portion (maybe all) of the Object being received.    */
   /* The Final member indicates whether this is the Final portion of   */
   /* the object being received.                                        */
   /* ** NOTE ** The Object Name and ObjectTotalLength fields are only  */
   /*            valid on the First segment of a multi-segment          */
   /*            transfer.                                              */
typedef struct _tagOPP_Push_Object_Indication_Data_t
{
   unsigned int       OPPID;
   OPP_Object_Type_t  ObjectType;
   Word_t            *ObjectName;
   DWord_t            ObjectTotalLength;
   unsigned int       DataLength;
   Byte_t            *DataBuffer;
   Boolean_t          Final;
} OPP_Push_Object_Indication_Data_t;

#define OPP_PUSH_OBJECT_INDICATION_DATA_SIZE             (sizeof(OPP_Push_Object_Indication_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Client when the Server sends a Object Push Response.  The OPPID   */
   /* member specifies the Identifier of the Local Object Push Profile  */
   /* Client receiving this event.                                      */
typedef struct _tagOPP_Push_Object_Confirmation_Data_t
{
   unsigned int OPPID;
   Byte_t       ResponseCode;
} OPP_Push_Object_Confirmation_Data_t;

#define OPP_PUSH_OBJECT_CONFIRMATION_DATA_SIZE           (sizeof(OPP_Push_Object_Confirmation_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Server when the Client sends a Pull Business Card Request.  The   */
   /* OPPID member specifies the Identifier of the Local Object Push    */
   /* Profile Server receiving this event.                              */
   /* * NOTE * This event should be responded to with the               */
   /*          OPP_Pull_Business_Card_Response() function.              */
typedef struct _tagOPP_Pull_Business_Card_Indication_Data_t
{
   unsigned int OPPID;
} OPP_Pull_Business_Card_Indication_Data_t;

#define OPP_PULL_BUSINESS_CARD_INDICATION_DATA_SIZE      (sizeof(OPP_Pull_Business_Card_Indication_Data_t))

   /* The following event is dispatched to the Object Push Profile      */
   /* Client when the Server sends a Pull Business Card Response.  The  */
   /* OPPID member specifies the Identifier of the Local Object Push    */
   /* Profile Client receiving this event.  The ResponseCode member     */
   /* specifies the response code associated with the Pull Business Card*/
   /* Response that generated this event.  The DataLength member        */
   /* indicates the number of bytes that are present in the Data Buffer.*/
   /* The DataBuffer member is a pointer to a portion (possibly entire) */
   /* of the Object being received.                                     */
   /* * NOTE * The Response Code should be used to determine if the     */
   /*          Business card is being sent whole or in segments.  If    */
   /*          the result code indicates that the transfer is not       */
   /*          complete, a OPP_Pull_Business_Card_Request() should be   */
   /*          issued again until the entire Business Card is received. */
typedef struct _tagOPP_Pull_Business_Card_Confirmation_Data_t
{
   unsigned int  OPPID;
   Byte_t        ResponseCode;
   DWord_t       TotalLength;
   unsigned int  DataLength;
   Byte_t       *DataBuffer;
} OPP_Pull_Business_Card_Confirmation_Data_t;

#define OPP_PULL_BUSINESS_CARD_CONFIRMATION_DATA_SIZE    (sizeof(OPP_Pull_Business_Card_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Bluetooth Object Push Profile Event Data.             */
typedef struct _tagOPP_Event_Data_t
{
   OPP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      OPP_Open_Request_Indication_Data_t         *OPP_Open_Request_Indication_Data;
      OPP_Open_Port_Indication_Data_t            *OPP_Open_Port_Indication_Data;
      OPP_Open_Port_Confirmation_Data_t          *OPP_Open_Port_Confirmation_Data;
      OPP_Close_Port_Indication_Data_t           *OPP_Close_Port_Indication_Data;
      OPP_Abort_Indication_Data_t                *OPP_Abort_Indication_Data;
      OPP_Abort_Confirmation_Data_t              *OPP_Abort_Confirmation_Data;
      OPP_Push_Object_Indication_Data_t          *OPP_Push_Object_Indication_Data;
      OPP_Push_Object_Confirmation_Data_t        *OPP_Push_Object_Confirmation_Data;
      OPP_Pull_Business_Card_Indication_Data_t   *OPP_Pull_Business_Card_Indication_Data;
      OPP_Pull_Business_Card_Confirmation_Data_t *OPP_Pull_Business_Card_Confirmation_Data;
   } Event_Data;
} OPP_Event_Data_t;

#define OPP_EVENT_DATA_SIZE                             (sizeof(OPP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a OPP Event Callback.  This function will be called whenever a    */
   /* Object Push Profile Event occurs that is associated with the      */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the OPP Event Data that occurred, and the */
   /* OPP Event Callback Parameter that was specified when this Callback*/
   /* was installed.  The caller is free to use the contents of the OPP */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another OPP Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving OPP Events.  A      */
   /*            Deadlock WILL occur because NO OPP Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *OPP_Event_Callback_t)(unsigned int BluetoothStackID, OPP_Event_Data_t *OPP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for opening a local Object  */
   /* Push Profile server.  The first parameter to this function is the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack Instance to be */
   /* associated with this Object Push Profile Server.  The second      */
   /* parameter to this function is the Server Port to be used when     */
   /* registering the Server.  The third and fourth parameters to this  */
   /* function are the Event Callback function and application defined  */
   /* Callback Parameter to be used when OPP Events occur.  This        */
   /* function returns a non-zero, positive, number on success or a     */
   /* negative return value if there was an error.  A successful return */
   /* value will be a OPP ID that can used to reference the Opened      */
   /* Object Push Profile Server in ALL other applicable functions in   */
   /* this module.                                                      */
   /* ** NOTE ** The Server Port value must be specified and must be a  */
   /*            value between OPP_PORT_NUMBER_MINIMUM and              */
   /*            OPP_PORT_NUMBER_MAXIMUM.                               */
BTPSAPI_DECLARATION int BTPSAPI OPP_Open_Object_Push_Server(unsigned int BluetoothStackID, unsigned int ServerPort, OPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Open_Object_Push_Server_t)(unsigned int BluetoothStackID, unsigned int ServerPort, OPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a connection    */
   /* from a local Object Push Client to a remote Object Push Server.   */
   /* The first parameter to this function is the Bluetooth Stack ID of */
   /* the Bluetooth Protocol Stack Instance to be associated with this  */
   /* Object Push Profile connection.  The second parameter to this     */
   /* function is the BD_ADDR of the remote Object Push Server in which */
   /* to connect.  The third parameter to this function is the Remote   */
   /* Server Port where the Push Server is registered.  The fourth and  */
   /* fifth parameters are the Event Callback function and application  */
   /* defined Callback Parameter to be used when OPP Events occur.  This*/
   /* function returns a non-zero, positive, number on success or a     */
   /* negative return value if there was an error.  A successful return */
   /* value will be a OPP ID that can used to reference the Opened      */
   /* Object Push Profile connection to a remote Object Push Server in  */
   /* ALL other applicable functions in this module.                    */
   /* ** NOTE ** The Object Push Server Port value must be specified    */
   /*            and must be a value between OPP_PORT_NUMBER_MINIMUM and*/
   /*            OPP_PORT_NUMBER_MAXIMUM.                               */
BTPSAPI_DECLARATION int BTPSAPI OPP_Open_Remote_Object_Push_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, OPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Open_Remote_Object_Push_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int RemoteServerPort, OPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Object Push Profile      */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Object Push   */
   /* Profile Server that is responding to the request.  The second     */
   /* parameter to this function is the OPP ID of the Object Push       */
   /* Profile for which a connection request was received.  The final   */
   /* parameter to this function specifies whether to accept the pending*/
   /* connection request (or to reject the request).  This function     */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etOPP_Open_Imaging_Service_Port_Indication event has   */
   /*            occurred.                                              */
BTPSAPI_DECLARATION int BTPSAPI OPP_Open_Request_Response(unsigned int BluetoothStackID, unsigned int OPPID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int OPPID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for registering an Object   */
   /* Push Profile Servers service record to the SDP database.  The     */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Object Push Profile Server for*/
   /* which this service record is being registered.  The second        */
   /* parameter to this function is the OPP ID of the Object Push       */
   /* Profile Server for which this service record is being registered. */
   /* The third parameter to this function is the Service Name to be    */
   /* associated with this service record.  The fourth parameter to this*/
   /* function is a Bit Mask of the object formats that are supported by*/
   /* this server.  The final parameter to this function is a pointer to*/
   /* a DWord_t which receives the SDP Service Record Handle if this    */
   /* function successfully creates a service record.  If this function */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* (see BTERRORS.H) and the SDPServiceRecordHandle value will be     */
   /* undefined.                                                        */
   /* * NOTE * This function should only be called with the OPP ID that */
   /*          was returned from the OPP_Open_Object_Push_Server()      */
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          OPP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * If the Service Name is NULL, then the default service for*/
   /*          this profile will be used.                               */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI OPP_Register_Object_Push_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int OPPID, char *ServiceName, DWord_t SupportedFormats, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Register_Object_Push_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int OPPID, char *ServiceName, DWord_t SupportedFormats, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* Object Push Profile Server SDP Service Record (specified by the   */
   /* third parameter) from the SDP Database.  This MACRO simply maps to*/
   /* the SDP_Delete_Service_Record() function.  This MACRO is only     */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the OPP ID (returned from a         */
   /* successful call to the OPP_Open_Object_Push_Server() function),   */
   /* and the SDP Service Record Handle.  The SDP Service Record Handle */
   /* was returned via a successful call to the                         */
   /* OPP_Register_Object_Push_Service_SDP_Record() function.  This     */
   /* MACRO returns the result of the SDP_Delete_Service_Record()       */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define OPP_Un_Register_SDP_Record(__BluetoothStackID, __OPPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for closing a currently     */
   /* registered Object Push Profile server.  This function is capable  */
   /* of closing servers opened via a call to                           */
   /* OPP_Open_Object_Push_Server().  The first parameter to this       */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* being closed.  The second parameter to this function is the OPP ID*/
   /* of the Object Push Profile Server to be closed.  This function    */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
BTPSAPI_DECLARATION int BTPSAPI OPP_Close_Server(unsigned int BluetoothStackID, unsigned int OPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int OPPID);
#endif

   /* The following function is responsible for closing a currently     */
   /* on-going Object Push Profile connection.  The first parameter to  */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Profile    */
   /* connection being closed.  The second parameter to this function is*/
   /* the OPP ID of the Object Push Profile connection to be closed.    */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** If this function is called with a server OPP ID (value */
   /*            returned from OPP_Open_Object_Push_Server()) any       */
   /*            clients current connection to this server will be      */
   /*            terminated, but the server will remained registered.   */
   /*            If this function is call using a client OPP ID (value  */
   /*            returned from OPP_Open_Remote_Object_Push_Server()) the*/
   /*            client connection shall be terminated.                 */
BTPSAPI_DECLARATION int BTPSAPI OPP_Close_Connection(unsigned int BluetoothStackID, unsigned int OPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int OPPID);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current Object Push Profile Server Mode.  The first     */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* OPP ID of the Object Push Profile Server in which to get the      */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* a pointer to a variable which will receive the current Server Mode*/
   /* Mask.  This function returns zero if successful, or a negative    */
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI OPP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int OPPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int OPPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current Object Push Profile Server Mode.  The first    */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* OPP ID of the Object Push Profile Server in which to set the      */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* the new Server Mode Mask to use.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI OPP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int OPPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int OPPID, unsigned long ServerModeMask);
#endif

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Object Push Server.  The first parameter to this    */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Client   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Client making this call.  This  */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on-going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
BTPSAPI_DECLARATION int BTPSAPI OPP_Abort_Request(unsigned int BluetoothStackID, unsigned int OPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Abort_Request_t)(unsigned int BluetoothStackID, unsigned int OPPID);
#endif

   /* The following function is responsible for sending an Object Push  */
   /* Request to the remote Object_Push Server.  The first parameter to */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Object Push Client     */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Client making this call.  The third     */
   /* parameter to this function is the actual type of object that is   */
   /* being pushed.  The fourth parameter to this function is the Name  */
   /* of the Object being put (in NULL terminated UNICODE format).  The */
   /* fifth parameter is the total length of the object being put.  The */
   /* sixth and seventh parameters to this function specify the length  */
   /* of the Object Data and a pointer to the Object Data being Pushed. */
   /* The eighth parameter to this function is a pointer to a length    */
   /* variable that will receive the total amount of data actually sent */
   /* in the request.  The final parameter to this function is a Boolean*/
   /* Flag indicating if this is to be the final segment of the object. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** This function should be used to initiate the Push      */
   /*            Object transaction as well as to continue a previously */
   /*            initiated, on-going, Push Object transaction.          */
   /* ** NOTE ** The Object Name is a pointer to a NULL Terminated      */
   /*            UNICODE String.                                        */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            not all parameters to this function are used in each   */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI OPP_Push_Object_Request(unsigned int BluetoothStackID, unsigned int OPPID, OPP_Object_Type_t ObjectType, Word_t *ObjectName, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Push_Object_Request_t)(unsigned int BluetoothStackID, unsigned int OPPID, OPP_Object_Type_t ObjectType, Word_t *ObjectName, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);
#endif

   /* The following function is responsible for sending an Object Push  */
   /* Response to the remote Client.  The first parameter to this       */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI OPP_Push_Object_Response(unsigned int BluetoothStackID, unsigned int OPPID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Push_Object_Response_t)(unsigned int BluetoothStackID, unsigned int OPPID, Byte_t ResponseCode);
#endif

   /* The following function is responsible for sending a Pull Business */
   /* Card Request to the remote Object Push Server.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Object Push Profile Client making this call.  The second parameter*/
   /* to this function is the OPP ID of the Object Push Client making   */
   /* this call.  This function returns zero if successful, or a        */
   /* negative return value if there was an error.                      */
   /* ** NOTE ** This function should be used to initiate the Pull      */
   /*            Business Card transaction as well as to continue a     */
   /*            previously initiated, on-going, Pull Business Card     */
   /*            transaction.                                           */
BTPSAPI_DECLARATION int BTPSAPI OPP_Pull_Business_Card_Request(unsigned int BluetoothStackID, unsigned int OPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Pull_Business_Card_Request_t)(unsigned int BluetoothStackID, unsigned int OPPID);
#endif

   /* The following function is responsible for sending a Pull Business */
   /* Card Response to the remote Client.  The first parameter to this  */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Object Push Profile Server   */
   /* making this call.  The second parameter to this function is the   */
   /* OPP ID of the Object Push Profile Server making this call.  The   */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth parameter specifies the*/
   /* Total Length of the Business card being pulled.  The fifth and    */
   /* sixth parameters to this function specify the length of the data  */
   /* being sent and a pointer to the data being sent.  The seventh     */
   /* parameter to this function is a pointer to a length variable that */
   /* will receive the amount of data actually sent.  This function     */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** If the value returned in AmountWritten is less than the*/
   /*            value specified in DataLength, then an additional call */
   /*            to this function must be made to send the remaining    */
   /*            data.  Note that an additional call cannot be made     */
   /*            until AFTER another Pull Business Card Request Event is*/
   /*            received.                                              */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Object Push Profile function.     */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Object Total       */
   /*            Length), others don't appear in the first segment but  */
   /*            may appear in later segments (i.e. Body).  This being  */
   /*            the case, not all parameters to this function are used */
   /*            in each segment of the transaction.                    */
BTPSAPI_DECLARATION int BTPSAPI OPP_Pull_Business_Card_Response(unsigned int BluetoothStackID, unsigned int OPPID, Byte_t ResponseCode, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OPP_Pull_Business_Card_Response_t)(unsigned int BluetoothStackID, unsigned int OPPID, Byte_t ResponseCode, DWord_t ObjectTotalLength, unsigned int DataLength, Byte_t *DataBuffer, unsigned int *AmountWritten);
#endif

#endif
