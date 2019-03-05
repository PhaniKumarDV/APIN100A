/*****< objpapi.h >************************************************************/
/*      Copyright 2000 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OBJPAPI - Stonestreet One Bluetooth Stack Object Push Profile API Type    */
/*            Definitions, Constants, and Prototypes.                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/01  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __OBJPAPIH__
#define __OBJPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTOBJP_ERROR_INVALID_PARAMETER                            (-1000)
#define BTOBJP_ERROR_NOT_INITIALIZED                              (-1001)
#define BTOBJP_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1002)
#define BTOBJP_ERROR_INSUFFICIENT_RESOURCES                       (-1004)
#define BTOBJP_ERROR_INVALID_ROOT_DIRECTORY                       (-1005)
#define BTOBJP_ERROR_INVALID_DEFAULT_BUSINESS_CARD                (-1006)
#define BTOBJP_ERROR_NOT_ALLOWED_WHILE_CONNECTED                  (-1007)
#define BTOBJP_ERROR_REQUEST_ALREADY_OUTSTANDING                  (-1008)
#define BTOBJP_ERROR_NOT_ALLOWED_WHILE_NOT_CONNECTED              (-1009)
#define BTOBJP_ERROR_UNABLE_TO_CREATE_LOCAL_FILE                  (-1010)
#define BTOBJP_ERROR_UNABLE_TO_READ_LOCAL_FILE                    (-1011)
#define BTOBJP_ERROR_UNABLE_TO_WRITE_LOCAL_FILE                   (-1012)

   /* SDP Profile UUID's for the OBEX Object Push Profile.              */

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_16) to the specified UUID_16_t variable.*/
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the OBJECT_PUSH_PROFILE_UUID_16 Constant value.*/
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_16(_x)      ASSIGN_SDP_UUID_16((_x), 0x11, 0x05)

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_32) to the specified UUID_32_t variable.*/
   /* This MACRO accepts one parameter which is the UUID_32_t variable  */
   /* that is to receive the OBJECT_PUSH_PROFILE_UUID_32 Constant value.*/
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_32(_x)      ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x05)

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Push Profile Bluetooth Universally Unique Identifier              */
   /* (OBJECT_PUSH_PROFILE_UUID_128) to the specified UUID_128_t        */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* OBJECT_PUSH_PROFILE_UUID_128 Constant value.                      */
#define SDP_ASSIGN_OBJECT_PUSH_PROFILE_UUID_128(_x)     ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following definitions represent the defined Object Types      */
   /* that can be supported by the Object Push Server.  These SDP       */
   /* Definitions are used with the Bluetooth Profile Descriptor List   */
   /* Attribute.                                                        */
#define SDP_OBJECT_SUPPORTED_FEATURE_VCARD2_1                        0x01
#define SDP_OBJECT_SUPPORTED_FEATURE_VCARD3_0                        0x02
#define SDP_OBJECT_SUPPORTED_FEATURE_VCALENDAR1_0                    0x03
#define SDP_OBJECT_SUPPORTED_FEATURE_ICALENDAR1_0                    0x04
#define SDP_OBJECT_SUPPORTED_FEATURE_VNOTE                           0x05
#define SDP_OBJECT_SUPPORTED_FEATURE_VMESSAGE                        0x06
#define SDP_OBJECT_SUPPORTED_FEATURE_ANY_OBJECT                      0xFF

   /* The following constant defines the Profile Version Number used    */
   /* within the SDP Record for Object Push Profile Servers (supported  */
   /* by this implementation).                                          */
#define OBJP_PROFILE_VERSION                                         (0x0101)

   /* The following enumerations specify the Type of objects that are   */
   /* supported by the Object Push Module.                              */
typedef enum
{
   obtvCard,
   obtvCalendar,
   obtiCalendar,
   obtvNote,
   obtvMessage,
   obtUnknownObject
} OBJP_ObjectType_t;

   /* The following definitions define the currently defined Object     */
   /* Type File Extensions.                                             */
#define OBJP_VCARD_DEFAULT_FILE_EXTENSION                            "vcf"
#define OBJP_VCALENDAR_DEFAULT_FILE_EXTENSION                        "vcs"
#define OBJP_VNOTE_DEFAULT_FILE_EXTENSION                            "vnt"
#define OBJP_VMESSAGE_DEFAULT_FILE_EXTENSION                         "vmg"

   /* The following Bit Definitions represent the Object Push Server    */
   /* Supported Object Types.  These Bit Definitions are used with the  */
   /* OBJP_Open_Server() function to specify the Objects that the       */
   /* Object Push Server supports.                                      */
#define OBJP_VCARD_2_1_SUPPORTED_BIT                            0x00000001
#define OBJP_VCARD_3_0_SUPPORTED_BIT                            0x00000002
#define OBJP_VCALENDAR_1_0_SUPPORTED_BIT                        0x00000004
#define OBJP_ICALENDAR_1_0_SUPPORTED_BIT                        0x00000008
#define OBJP_VNOTE_SUPPORTED_BIT                                0x00000010
#define OBJP_VMESSAGE_SUPPORTED_BIT                             0x00000020
#define OBJP_ALL_OBJECTS_SUPPORTED_BIT                          0x80000000

   /* The following Bit Definitions represent the Object Push Server    */
   /* Reject Object Request Types.  These Bit Definitions are used with */
   /* the OBJP_Open_Server() function to specify which Objects Requests */
   /* are to be Rejected by the Server.                                 */
#define OBJP_PUSH_VCARD_REJECT_BIT                              0x00000001
#define OBJP_PUSH_VCALENDAR_REJECT_BIT                          0x00000002
#define OBJP_PUSH_ICALENDAR_REJECT_BIT                          0x00000004
#define OBJP_PUSH_VNOTE_REJECT_BIT                              0x00000008
#define OBJP_PUSH_VMESSAGE_REJECT_BIT                           0x00000010
#define OBJP_PULL_DEFAULT_OBJECT_REJECT_BIT                     0x80000000

   /* The following Bit Mask Definitions represent predefined Bit Mask  */
   /* values for the Reject Object Request Bit Mask fields.             */
#define OBJP_ACCEPT_ALL_REQUESTS_BIT_MASK                       0x00000000
#define OBJP_PUSH_ALL_OBJECTS_REJECT_BIT_MASK  (~OBJP_PULL_DEFAULT_OBJECT_REJECT_BIT)

   /* The following BIT definitions are used to denote the possible     */
   /* Object Push Server Modes that can be applied to a Object Push     */
   /* Client Connection.  These BIT definitions are used with the       */
   /* OBJP_Set_Server_Mode() and OBJP_Get_Server_Mode() mode functions. */
#define OBJP_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION            (0x00000000)
#define OBJP_SERVER_MODE_MANUAL_ACCEPT_CONNECTION               (0x00000001)
#define OBJP_SERVER_MODE_CONNECTION_MASK                        (0x00000001)

   /* OBEX Object Push Server Event API Types.                          */
typedef enum
{
   etOBJP_Server_Connect_Indication,
   etOBJP_Server_Disconnect_Indication,
   etOBJP_Server_Object_Put_Indication,
   etOBJP_Server_Object_Get_Indication,
   etOBJP_Server_Connect_Request_Indication
} OBJP_Server_Event_Type_t;

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client Connects to a registered Object Push Server.   */
   /* The OBJP ID member specifies the Local Server that has been       */
   /* connected to and the BD_ADDR member specifies the Client Bluetooth*/
   /* Device that has connected to the specified Server.                */
typedef struct _tagOBJP_Server_Connect_Indication_Data_t
{
   unsigned int OBJPID;
   BD_ADDR_t    BD_ADDR;
} OBJP_Server_Connect_Indication_Data_t;

#define OBJP_SERVER_CONNECT_INDICATION_DATA_SIZE        (sizeof(OBJP_Server_Connect_Indication_Data_t))

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client Disconnects from a registered Object Push      */
   /* Server.  The OBJP ID member specifies the Local Server that the   */
   /* Remote Client has disconnected from.                              */
typedef struct _tagOBJP_Server_Disconnect_Indication_Data_t
{
   unsigned int OBJPID;
} OBJP_Server_Disconnect_Indication_Data_t;

#define OBJP_SERVER_DISCONNECT_INDICATION_DATA_SIZE     (sizeof(OBJP_Server_Disconnect_Indication_Data_t))

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client puts an Object on the registered Object Push   */
   /* Server.  The OBJP ID member specifies the Local Server that the   */
   /* Remote Client has put the specified Object on.  The Object Type   */
   /* member specifies the type of the Object that has been put on the  */
   /* specified server.  The Object File Name member specifies the name */
   /* of the Object that is being written to the INBOX Directory.       */
   /* The Transfer Complete flag specifies whether or not the Object    */
   /* Transfer has been completed (TRUE when completed, FALSE while in  */
   /* progress).  The Transferred Length and Total Length members       */
   /* specify how many bytes have been transferred and how many bytes   */
   /* are to be transferred in total.                                   */
   /* * NOTE * The ObjectFileName member is formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagOBJP_Server_Object_Put_Indication_Data_t
{
   unsigned int       OBJPID;
   OBJP_ObjectType_t  ObjectType;
   char              *ObjectFileName;
   Boolean_t          TransferComplete;
   unsigned int       TransferredLength;
   unsigned int       TotalLength;
} OBJP_Server_Object_Put_Indication_Data_t;

#define OBJP_SERVER_OBJECT_PUT_INDICATION_DATA_SIZE     (sizeof(OBJP_Server_Object_Put_Indication_Data_t))

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client retrieves an Object from the registered Object */
   /* Push Server.  The OBJP ID member specifies the Local Server that  */
   /* the Remote Client is retrieving the specified Object from.  The   */
   /* Object Type member specifies the type of the Object that is being */
   /* requested from the specified server.  The Object File Name        */
   /* specifies the Object File Name (No Path Information) of the Object*/
   /* that is currently being requested.  The Transfer Complete flag    */
   /* specifies whether or not the Object Transfer has been completed   */
   /* (TRUE when completed, FALSE while in progress).  The Transferred  */
   /* Length and Total Length members specify how many bytes have been  */
   /* transferred and how many bytes are to be transferred in total.    */
   /* * NOTE * The ObjectFileName member is formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagOBJP_Server_Object_Get_Indication_Data_t
{
   unsigned int       OBJPID;
   OBJP_ObjectType_t  ObjectType;
   char              *ObjectFileName;
   Boolean_t          TransferComplete;
   unsigned int       TransferredLength;
   unsigned int       TotalLength;
} OBJP_Server_Object_Get_Indication_Data_t;

#define OBJP_SERVER_OBJECT_GET_INDICATION_DATA_SIZE     (sizeof(OBJP_Server_Object_Get_Indication_Data_t))

   /* The following Object Push Server Event is dispatched when a Object*/
   /* Push Client requests to connects to a registered Object Push      */
   /* Server.  The OBJP ID member specifies the Local Server that has   */
   /* received the request to connect and the BD_ADDR member specifies  */
   /* the Client Bluetooth Device that is requesting to connect to the  */
   /* specified Server.                                                 */
typedef struct _tagOBJP_Server_Connect_Request_Indication_Data_t
{
   unsigned int OBJPID;
   BD_ADDR_t    BD_ADDR;
} OBJP_Server_Connect_Request_Indication_Data_t;

#define OBJP_SERVER_CONNECT_REQUEST_INDICATION_DATA_SIZE (sizeof(OBJP_Server_Connect_Request_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all OBEX Object Push Profile Server Event Data Data.      */
typedef struct _tagOBJP_Server_Event_Data_t
{
   OBJP_Server_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      OBJP_Server_Connect_Indication_Data_t         *OBJP_Server_Connect_Indication_Data;
      OBJP_Server_Disconnect_Indication_Data_t      *OBJP_Server_Disconnect_Indication_Data;
      OBJP_Server_Object_Put_Indication_Data_t      *OBJP_Server_Object_Put_Indication_Data;
      OBJP_Server_Object_Get_Indication_Data_t      *OBJP_Server_Object_Get_Indication_Data;
      OBJP_Server_Connect_Request_Indication_Data_t *OBJP_Server_Connect_Request_Indication_Data;
   } Event_Data;
} OBJP_Server_Event_Data_t;

#define OBJP_SERVER_EVENT_DATA_SIZE                     (sizeof(OBJP_Server_Event_Data_t))

   /* The following constants represent the Object Push Client Open     */
   /* Status Values that are possible in the Object Push Client Open    */
   /* Confirmation Event.                                               */
#define OBJP_CLIENT_OPEN_STATUS_SUCCESS                                 0x00
#define OBJP_CLIENT_OPEN_STATUS_CONNECTION_TIMEOUT                      0x01
#define OBJP_CLIENT_OPEN_STATUS_CONNECTION_REFUSED                      0x02
#define OBJP_CLIENT_OPEN_STATUS_UNKNOWN_ERROR                           0x04

   /* OBEX Object Push Client Event API Types.                          */
typedef enum
{
   etOBJP_Client_Connect_Confirmation,
   etOBJP_Client_Disconnect_Indication,
   etOBJP_Client_Abort_Confirmation,
   etOBJP_Client_Object_Put_Confirmation,
   etOBJP_Client_Object_Get_Confirmation
} OBJP_Client_Event_Type_t;

   /* The following Object Push Client Event is dispatched when an      */
   /* Object Client receives the Connection Response from a Remote      */
   /* Object Push Server which was previously attempted to be connected */
   /* to.  The Object Push ID member specifies the Local Client that    */
   /* has requested the connection, the Object Push Open Status         */
   /* represents the Connection Status of the Request, and the BD_ADDR  */
   /* member specifies the Remote Bluetooth Device that the Remote      */
   /* Bluetooth Object Push Server resides on.                          */
typedef struct _tagOBJP_Client_Connect_Confirmation_Data_t
{
   unsigned int OBJPID;
   unsigned int OBJPConnectStatus;
   BD_ADDR_t    BD_ADDR;
} OBJP_Client_Connect_Confirmation_Data_t;

#define OBJP_CLIENT_CONNECT_CONFIRMATION_DATA_SIZE      (sizeof(OBJP_Client_Connect_Confirmation_Data_t))

   /* The following Object Push Client Event is dispatched when an      */
   /* Object Push Client Disconnects from a registered Object Push      */
   /* Server.  The Object Push ID member specifies the Local Client that*/
   /* the Remote Server has disconnected from.  This Event is NOT       */
   /* Dispatched in response to an Client Requesting a Disconnection.   */
   /* This Event is dispatched when the Remote Server terminates the    */
   /* Connection (and/or Bluetooth Link).                               */
typedef struct _tagOBJP_Client_Disconnect_Indication_Data_t
{
   unsigned int OBJPID;
} OBJP_Client_Disconnect_Indication_Data_t;

#define OBJP_CLIENT_DISCONNECT_INDICATION_DATA_SIZE     (sizeof(OBJP_Client_Disconnect_Indication_Data_t))

   /* The following Object Push Client Event is dispatched when an      */
   /* Object Push Client Abort Response is received from the Remote     */
   /* Object Push Server.  The Object Push ID member specifies the Local*/
   /* Client that the Remote Server has responded to the Abort Request  */
   /* on.                                                               */
typedef struct _tagOBJP_Client_Abort_Confirmation_Data_t
{
   unsigned int OBJPID;
} OBJP_Client_Abort_Confirmation_Data_t;

#define OBJP_CLIENT_ABORT_CONFIRMATION_DATA_SIZE        (sizeof(OBJP_Client_Abort_Confirmation_Data_t))

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client receives an Object Put Confirmation from the   */
   /* registered Object Push Server.  The OBJP ID member specifies the  */
   /* Local Object Push Client that has requested the specified Object  */
   /* to be put on the Remote Server.  The Object Type member specifies */
   /* specifies the type of the Object that has been put on the remote  */
   /* server.  The Object File Name member specifies the name of the    */
   /* Object that is being written to the remote INBOX Directory.       */
   /* The Transfer Complete flag specifies whether or not the Object    */
   /* Transfer has been completed (TRUE when completed, FALSE while in  */
   /* progress).  The Transferred Length and Total Length members       */
   /* specify how many bytes have been transferred and how many bytes   */
   /* are to be transferred in total.  The Success member specifies     */
   /* whether or not the request has been completed successfully.       */
   /* * NOTE * The ObjectFileName member is formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagOBJP_Client_Object_Put_Confirmation_Data_t
{
   unsigned int       OBJPID;
   Boolean_t          Success;
   OBJP_ObjectType_t  ObjectType;
   char              *ObjectFileName;
   Boolean_t          TransferComplete;
   unsigned int       TransferredLength;
   unsigned int       TotalLength;
} OBJP_Client_Object_Put_Confirmation_Data_t;

#define OBJP_CLIENT_OBJECT_PUT_CONFIRMATION_DATA_SIZE   (sizeof(OBJP_Client_Object_Put_Confirmation_Data_t))

   /* The following Object Push Server Event is dispatched when an      */
   /* Object Push Client receives an Object from the registered Object  */
   /* Push Server.  The OBJP ID member specifies the Remote Server that */
   /* the Local Client is retrieving the specified Object from.  The    */
   /* Object Type member specifies the type of the Object that is being */
   /* requested from the specified server.  The Object File Name        */
   /* specifies the Object File Name that is currently being requested. */
   /* The Transfer Complete flag specifies whether or not the Object    */
   /* Transfer has been completed (TRUE when completed, FALSE while in  */
   /* progress).  The Transferred Length specifies how many bytes have  */
   /* been transferred.  Note that there is no way to determine how     */
   /* many total bytes of the Remote Object will be transferred.  This  */
   /* is an OBEX limitation.  The Success member specifies whether or   */
   /* not the request has been completed successfully.                  */
   /* * NOTE * The ObjectFileName member is formatted as a NULL         */
   /*          terminated ASCII string with UTF-8 encoding.             */
typedef struct _tagOBJP_Client_Object_Get_Confirmation_Data_t
{
   unsigned int       OBJPID;
   Boolean_t          Success;
   OBJP_ObjectType_t  ObjectType;
   char              *ObjectFileName;
   Boolean_t          TransferComplete;
   unsigned int       TransferredLength;
} OBJP_Client_Object_Get_Confirmation_Data_t;

#define OBJP_CLIENT_OBJECT_GET_CONFIRMATION_DATA_SIZE   (sizeof(OBJP_Client_Object_Get_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all OBEX Object Push Profile Client Event Data Data.      */
typedef struct _tagOBJP_Client_Event_Data_t
{
   OBJP_Client_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      OBJP_Client_Connect_Confirmation_Data_t    *OBJP_Client_Connect_Confirmation_Data;
      OBJP_Client_Disconnect_Indication_Data_t   *OBJP_Client_Disconnect_Indication_Data;
      OBJP_Client_Abort_Confirmation_Data_t      *OBJP_Client_Abort_Confirmation_Data;
      OBJP_Client_Object_Put_Confirmation_Data_t *OBJP_Client_Object_Put_Confirmation_Data;
      OBJP_Client_Object_Get_Confirmation_Data_t *OBJP_Client_Object_Get_Confirmation_Data;
   } Event_Data;
} OBJP_Client_Event_Data_t;

#define OBJP_CLIENT_EVENT_DATA_SIZE                     (sizeof(OBJP_Client_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Object Push Server Profile Event Receive Data Callback.  This  */
   /* function will be called whenever an Object Push Event occurs that */
   /* is associated with the specified Bluetooth Stack ID.  This        */
   /* function passes to the caller the Bluetooth Stack ID, the Object  */
   /* Push Server Event Data that occurred and the Object Push Server   */
   /* Event Callback Parameter that was specified when this Callback    */
   /* was installed.  The caller is free to use the contents of the     */
   /* Object Push Server Event Data ONLY in the context of this         */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  It needs to be  */
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called   */
   /* in the Thread Context of a Thread that the User does NOT own.     */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Object Push  */
   /* Server Profile Event will not be processed while this function    */
   /* call is outstanding).                                             */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving Object Push Server  */
   /*            Event Packets.  A Deadlock WILL occur because NO Object*/
   /*            Push Server Event Callbacks will be issued while this  */
   /*            function is currently outstanding.                     */
typedef void (BTPSAPI *OBJP_Server_Event_Callback_t)(unsigned int BluetoothStackID, OBJP_Server_Event_Data_t *OBJP_Server_Event_Data, unsigned long CallbackParameter);

   /* The following declared type represents the Prototype Function for */
   /* a Object Push Client Event Receive Data Callback.  This function  */
   /* will be called whenever an OBEC Object Push Event occurs that is  */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the Object Push      */
   /* Client Event Data that occurred and the Object Push Client Event  */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the Object  */
   /* Push Client Event Data ONLY in the context of this callback.  If  */
   /* the caller requires the Data for a longer period of time, then    */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  It needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Object Push Client    */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving Object Push Client  */
   /*            Event Packets.  A Deadlock WILL occur because NO Object*/
   /*            Push Event Callbacks will be issued while this function*/
   /*            is currently outstanding.                              */
typedef void (BTPSAPI *OBJP_Client_Event_Callback_t)(unsigned int BluetoothStackID, OBJP_Client_Event_Data_t *OBJP_Client_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for Opening an Object Push  */
   /* Server on the specified Bluetooth SPP Serial Port.  This function */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* Instance to use for the Object Push Server, the Local Serial Port */
   /* Server Number to use, a pointer to a NULL terminated ASCII string */
   /* which specifies the Local Directory Path of the directory to use  */
   /* as the INBOX for the Object Server, the Default Business Card     */
   /* File Name (Must be specified and must be located in the INBOX     */
   /* directory),  the Supported Object List Bit Mask, the Reject       */
   /* Request List Bit Mask, and the Object Push Server Event Callback  */
   /* function (and parameter) to associate with the specified Object   */
   /* Push Server.  The ServerPort parameter *MUST* be between          */
   /* SPP_PORT_NUMBER_MINIMUM and SPP_PORT_NUMBER_MAXIMUM.  The         */
   /* Supported Object List specifies the Bit Mask of supported Objects */
   /* that can be pushed to this server.  This parameter must specify   */
   /* at least (vCard 2.1 support - or Any Object Support).  The Reject */
   /* Requests Mask specifies the Object Requests that are to be        */
   /* Rejected by this Object Server.  This parameter can be zero, which*/
   /* means do NOT Reject ANY Object Requests for the Object Push       */
   /* Server.  This function returns a positive, non zero, value if     */
   /* successful or a negative return error code if an error occurs.  A */
   /* successful return code will be a Object Push ID that can be used  */
   /* to reference the Opened Object Push Server in ALL other Object    */
   /* Push Server functions in this module.  Once an Object Push Server */
   /* is opened, it can only be Un-Registered via a call to the         */
   /* OBJP_Close_Server() function (passing the return value from this  */
   /* function).                                                        */
   /* * NOTE * The InBoxDirectory and DefaultBusinessCardFileName       */
   /*          parameters should be formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Open_Server(unsigned int BluetoothStackID, unsigned int ServerPort, char *InBoxDirectory, char *DefaultBusinessCardFileName, unsigned long SupportedObjectsMask, unsigned long RejectRequestsMask, OBJP_Server_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Open_Server_t)(unsigned int BluetoothStackID, unsigned int ServerPort, char *InBoxDirectory, char *DefaultBusinessCardFileName, unsigned long SupportedObjectsMask, unsigned long RejectRequestsMask, OBJP_Server_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for Un-Registering an Object*/
   /* Push Server (which was Registered by a successful call to the     */
   /* OBJP_Open_Server() function).  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the Object*/
   /* Push Server specified by the Second Parameter is valid for.  This */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred (see BTERRORS.H).  Note that this       */
   /* function does NOT delete any SDP Service Record Handles.          */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Close_Server(unsigned int BluetoothStackID, unsigned int OBJPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int OBJPID);
#endif

   /* The following function is used to terminate a possible connection */
   /* to the local server.  This function can only be called by the     */
   /* Object Push Server.  A successfully call to this function will    */
   /* terminate the remote Object Push Client connection to the local   */
   /* Object Push server.  This function accepts as input the Bluetooth */
   /* Stack ID of the Bluetooth Stack which handles the Server and the  */
   /* Object Push ID that was returned from the OBJP_Open_Server()      */
   /* function.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.  This function does NOT       */
   /* Un-Register an Object Push Server from the system, it ONLY        */
   /* disconnects any connection that is currently active.  The         */
   /* OBJP_Close_Server() function can be used to Un-Register the Object*/
   /* Push Server.                                                      */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Close_Server_Connection(unsigned int BluetoothStackID, unsigned int OBJPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Close_Server_Connection_t)(unsigned int BluetoothStackID, unsigned int OBJPID);
#endif

   /* The following function is provided to allow a means to respond to */
   /* a request to connect to an Object Push Server.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which handles the Server and the OBJP ID that was returned from   */
   /* the OBJP_Open_Server() function.  The final parameter to this     */
   /* function is whether to accept the pending connection.  This       */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Server_Connect_Request_Response(unsigned int BluetoothStackID, unsigned int OBJPID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Server_Connect_Request_Response_t)(unsigned int BluetoothStackID, unsigned int OBJPID, Boolean_t AcceptConnection);
#endif

   /* The following function is provided to allow a means to add a      */
   /* Generic Object Push Service Record to the SDP Database.  This     */
   /* function takes as input the Bluetooth Stack ID of the Local       */
   /* Bluetooth Protocol Stack, the Object Push Server ID (which *MUST* */
   /* have been obtained by calling the OBJP_Open_Server() function.    */
   /* The third parameter specifies the Service Name to associate with  */
   /* the SDP Record.  The final parameter is a pointer to a DWord_t    */
   /* which receives the SDP Service Record Handle if this function     */
   /* successfully creates an SDP Service Record.  If this function     */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be         */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function should only be called with the OBJP ID that*/
   /*          was returned from the OBJP_Open_Server() function.  This */
   /*          function should NEVER be used with OBJP ID returned from */
   /*          the OBJP_Open_Remote_Server() function.                  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until    */
   /*          it is deleted by calling the SDP_Delete_Service_Record() */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from    */
   /*          the SDP Data Base.  This MACRO maps the                  */
   /*          OBJP_Un_Register_SDP_Record() to the                     */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI OBJP_Register_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int OBJPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Register_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int OBJPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* Object Push Server SDP Service Record (specified by the third     */
   /* parameter) from the SDP Database.  This MACRO simply maps to the  */
   /* SDP_Delete_Service_Record() function.  This MACRO is only         */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the Object Push Server ID (returned */
   /* from a successful call to the OBJP_Open_Server() function), and   */
   /* the SDP Service Record Handle.  The SDP Service Record Handle was */
   /* returned via a succesful call to the                              */
   /* OBJP_Register_Server_SDP_Record() function.  See the              */
   /* OBJP_Register_Server_SDP_Record() function for more information.  */
   /* This MACRO returns the result of the SDP_Delete_Service_Record()  */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define OBJP_Un_Register_SDP_Record(__BluetoothStackID, __OBJPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a Remote Object */
   /* Push Server.  This function accepts as input the Bluetooth Stack  */
   /* ID of the Bluetooth Protocol Stack that the Object Server Client  */
   /* is associated with.  The second parameter is the Remote Bluetooth */
   /* Device Address of the Bluetooth Object Push Server to connect     */
   /* with.  The third parameter specifies the Remote Server Port to    */
   /* connect with.  The final two parameters specify the Object Push   */
   /* Client Event Callback Function and the Callback Parameter to      */
   /* associate with this Object Push Client.  The ServerPort parameter */
   /* *MUST* be between SPP_PORT_NUMBER_MINIMUM and                     */
   /* SPP_PORT_NUMBER_MAXIMUM.  This function returns a positive, non   */
   /* zero, value if successful or a negative return error code if an   */
   /* error occurs.  A successful return code will be a Object Push ID  */
   /* that can be used to reference the Opened Object Push Server in ALL*/
   /* other Object Push Client functions in this module.  Once a remote */
   /* server is opened, it can only be closed via a call to the         */
   /* OBJP_Close_Client() function (passing the return value from this  */
   /* function).                                                        */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Open_Remote_Object_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, OBJP_Client_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Open_Remote_Object_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, OBJP_Client_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is used to terminate a possible connection */
   /* to a remote server.  This function can only be called by the      */
   /* Object Push Client.  A successful call to this function will      */
   /* terminate the remote Object Server Connection.  This function     */
   /* accepts as input the Bluetooth Stack ID of the Bluetooth Stack    */
   /* which handles the Client and the Object Push Client ID that was   */
   /* returned from the the OBJP_Open_Remote_Object_Server() function.  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Close_Client(unsigned int BluetoothStackID, unsigned int OBJPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Close_Client_t)(unsigned int BluetoothStackID, unsigned int OBJPID);
#endif

   /* The following function is responsible for retrieving the Default  */
   /* Object from the specified Remote Object Push Server.  This        */
   /* function accepts as its input parameters the Bluetooth Stack ID of*/
   /* the Bluetooth Stack that is associated with this Object Push      */
   /* Client.  The second parameter specifies the Object Push Client ID */
   /* (returned from a successful call to the                           */
   /* OBJP_Open_Remote_Object_Server() function).  The last two         */
   /* parameters specify the local path (and filename) to store the     */
   /* retrieved Object.  The LocalPath member (if specified) contains   */
   /* the Local Path to store the Object, and the final parameter       */
   /* specifies the Filename to store the Object into.  The last        */
   /* parameter is not optional and *MUST* be specified.  If the        */
   /* LocalPath parameter is NULL, then the Object is written to the    */
   /* current directory (on the local machine).  This function returns  */
   /* zero if successful or a negative return error code if there was   */
   /* an error.                                                         */
   /* * NOTE * The LocalPath and LocalObjectFileName parameters should  */
   /*          be formatted as a NULL terminated ASCII strings with     */
   /*          UTF-8 encoding.                                          */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote Object Push Server successfully issued the        */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the Remote Object Push Server     */
   /*          successfully executed the Request.                       */
   /* * NOTE * Due to an OBEX Object Push limitation, there can only be */
   /*          one outstanding Object Push Client Request active at any */
   /*          one time.  Because of this, another Object Push Client   */
   /*          Request cannot be issued until either the current request*/
   /*          is Aborted (by calling the OBJP_Abort() function) or the */
   /*          current Request is complete (this is signified by        */
   /*          receiving a Confirmation Event in the Object Push Client */
   /*          Event Callback that was registered when the Object Push  */
   /*          Client was opened).                                      */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Get_Default_Object(unsigned int BluetoothStackID, unsigned int OBJPID, char *LocalPath, char *LocalObjectFileName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Get_Default_Object_t)(unsigned int BluetoothStackID, unsigned int OBJPID, char *LocalPath, char *LocalObjectFileName);
#endif

   /* The following function is responsible for sending the specified   */
   /* Object to the specified Remote Object Push Server.  This function */
   /* accepts as its input parameters the Bluetooth Stack ID of the     */
   /* Bluetooth Stack that is associated with this Object Push Client.  */
   /* The second parameter specifies the Object Push Client ID (returned*/
   /* from a successful call to the OBJP_Open_Remote_Object_Server()    */
   /* function).  The third parameter specifies the type of Object that */
   /* is being pushed to the Remote Object Push Server.  The fourth and */
   /* fifth parameters specify the Local Path and Local Filename of the */
   /* source Object.  These two parameters are pointers to NULL         */
   /* terminated ASCII strings which specify the Path and File Name     */
   /* (respectively) of the Local File Object.  The Local Path parameter*/
   /* is optional, and if NON-NULL specifies the Local Path of the      */
   /* Local File Object.  The Local File Name parameter is NOT optional */
   /* and specifies the File Name of the Local File Object.  The        */
   /* RemoteObjectName parameter specifies the name of the Object that  */
   /* is to be stored on the Remote Object Push Server.  The Object that*/
   /* is sent to the Remote Object Push Server is stored in the INBOX   */
   /* of the Remote Object Push Server.  This function returns zero if  */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * The LocalPath, LocalObjectFileName, and RemoteObjectName */
   /*          parameters should be formatted as a NULL terminated      */
   /*          ASCII strings with UTF-8 encoding.                       */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          Remote Object Push Server successfully issued the        */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the Remote Object Push Server     */
   /*          successfully executed the Request.                       */
   /* * NOTE * Due to an OBEX Object Push limitation, there can only be */
   /*          one outstanding Object Push Client Request active at any */
   /*          one time.  Because of this, another Object Push Client   */
   /*          Request cannot be issued until either the current request*/
   /*          is Aborted (by calling the OBJP_Abort() function) or the */
   /*          current Request is complete (this is signified by        */
   /*          receiving a Confirmation Event in the Object Push Client */
   /*          Event Callback that was registered when the Object Push  */
   /*          Client was opened).                                      */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Put_Object(unsigned int BluetoothStackID, unsigned int OBJPID, OBJP_ObjectType_t ObjectType, char *LocalPath, char *LocalObjectFileName, char *RemoteObjectName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Put_Object_t)(unsigned int BluetoothStackID, unsigned int OBJPID, OBJP_ObjectType_t ObjectType, char *LocalPath, char *LocalFileName, char *RemoteObjectFileName);
#endif

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding Object Push Client Request.  This function accepts as */
   /* its input parameters the Bluetooth Stack ID of the Bluetooth Stack*/
   /* for which the Object Push Client is valid for.  The second        */
   /* parameter to this function specifies the Object Push Client ID    */
   /* (returned from a successful call to the                           */
   /* OBJP_Open_Remote_Object_Server() function).  This function returns*/
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * This Object Push Client Request is no different than the */
   /*          rest of the Object Push Client Request functions in that */
   /*          a response to this Request must be received before any   */
   /*          other Object Push Client Request can be issued.          */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that an Object Push Client Request that is to be aborted */
   /*          may have completed before the server was able to Abort   */
   /*          the request.  In either case, the caller will be notified*/
   /*          via Object Push Client Callback of the status of the     */
   /*          previous Request.                                        */
   /* * NOTE * Due to the nature of only one outstanding OBEX Command   */
   /*          when an Abort is issued, it may be queued (for           */
   /*          transmission when the response to the currently          */
   /*          outstanding OBEX Command is received.  A problem can     */
   /*          occur if the Remote OBEX Server does not respond to the  */
   /*          original request because the queued Abort Packet will    */
   /*          never be sent.  This is a problem because no new OBEX    */
   /*          commands can be issued because the OBEX layer on the     */
   /*          local machine thinks a Request is outstanding and will   */
   /*          not issue another request.  To aid in error recovery,    */
   /*          this function forces an Abort Request out (and the       */
   /*          clearing of the current OBEX Command Request) if this    */
   /*          function is called twice.  An application can call this  */
   /*          function a second time to force a local cleanup if a     */
   /*          response on the first Abort Packet is never received     */
   /*          (via the Object Push Client Callback).  It should be     */
   /*          noted that under normal circumstances (i.e. the Remote   */
   /*          Server functioning properly) this function will NEVER    */
   /*          have to be called twice.                                 */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Abort(unsigned int BluetoothStackID, unsigned int OBJPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Abort_t)(unsigned int BluetoothStackID, unsigned int OBJPID);
#endif

   /* The following function is responsible for changing the currently  */
   /* active Reject Request Mask.  This function accepts as its input   */
   /* parameters the Bluetooth Stack ID of the Bluetooth Stack for which*/
   /* the Object Push Server is valid for.  The second parameter to     */
   /* this function specifies the Object Push Server ID (returned from  */
   /* a successful call to the OBJP_Open_Server() function).  The final */
   /* parameter specifies the NEW Reject Request.  This Mask will       */
   /* replace the currently active Reject Request Mask that was set     */
   /* either by a prior call to this function OR when the Server was    */
   /* opened via a call to the OBJP_Open_Server() function.  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function DOES NOT abort any current Object Push/Pull*/
   /*          operations that are currently in progress.  After this   */
   /*          function completes successfully, only FUTURE Object      */
   /*          Requests will be affected by a successful call to this   */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Change_Reject_Request_Mask(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long NewRejectRequestsMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Change_Reject_Request_Mask_t)(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long NewRejectRequestsMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current Object Push Server Mode.  This function accepts */
   /* as input the Bluetooth Stack ID of the Bluetooth Stack which      */
   /* handles the Server and the OBJP ID that was returned from the     */
   /* OBJP_Open_Server() function, and as the final parameter a pointer */
   /* to a variable which will receive the current Server Mode Mask.    */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current Object Push Server Mode.  This function accepts*/
   /* as input the Bluetooth Stack ID of the Bluetooth Stack which      */
   /* handles the Server and the OBJP ID that was returned from the     */
   /* OBJP_Open_Server() function, and as the final parameter the new   */
   /* Server Mode Mask to use.  This function returns zero if           */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI OBJP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_OBJP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int OBJPID, unsigned long ServerModeMask);
#endif

#endif
