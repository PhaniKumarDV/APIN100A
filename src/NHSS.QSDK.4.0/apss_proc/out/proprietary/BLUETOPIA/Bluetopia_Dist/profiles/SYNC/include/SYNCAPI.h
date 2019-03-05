/*****< syncapi.h >************************************************************/
/*      Copyright 2006 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SYNCAPI - Stonestreet One Bluetooth Stack Sync Profile API Type           */
/*            Definitions, Constants, and Prototypes.                         */
/*                                                                            */
/*  Author:  Josh Toole                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/22/06  J. Toole       Initial creation.                               */
/******************************************************************************/
#ifndef __SYNCAPIH__
#define __SYNCAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTSYNC_ERROR_INVALID_PARAMETER                           (-1000)
#define BTSYNC_ERROR_NOT_INITIALIZED                             (-1001)
#define BTSYNC_ERROR_INVALID_BLUETOOTH_STACK_ID                  (-1002)
#define BTSYNC_ERROR_LIBRARY_INITIALIZATION_ERROR                (-1003)
#define BTSYNC_ERROR_INSUFFICIENT_RESOURCES                      (-1004)
#define BTSYNC_ERROR_REQUEST_ALREADY_OUTSTANDING                 (-1005)
#define BTSYNC_ERROR_ACTION_NOT_ALLOWED                          (-1006)
#define BTSYNC_ERROR_INVALID_SYNC_TYPE                           (-1007)

   /* OBEX Object SYNC SDP UUID's.                                      */

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Profile Bluetooth Universally Unique Identifier (IrMCSync) to the */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the IrMCSync   */
   /* Constant value.                                                   */
#define SDP_ASSIGN_IRMC_SYNC_PROFILE_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x11, 0x04)

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Profile Bluetooth Universally Unique Identifier (IrMCSync) to the */
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the IrMCSync   */
   /* Constant value.                                                   */
#define SDP_ASSIGN_IRMC_SYNC_PROFILE_UUID_32(_x)                 ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x04)

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Profile Bluetooth Universally Unique Identifier (IrMCSync) to the */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the IrMCSync  */
   /* Constant value.                                                   */
#define SDP_ASSIGN_IRMC_SYNC_PROFILE_UUID_128(_x)                ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x04, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Command Bluetooth Universally Unique Identifier (IrMCSyncCommand) */
   /* to the specified UUID_16_t variable.  This MACRO accepts one      */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* IrMCSyncCommand Constant value.                                   */
#define SDP_ASSIGN_IRMC_SYNC_COMMAND_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x11, 0x07)

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Command Bluetooth Universally Unique Identifier (IrMCSyncCommand) */
   /* to the specified UUID_32_t variable.  This MACRO accepts one      */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* IrMCSyncCommand Constant value.                                   */
#define SDP_ASSIGN_IRMC_SYNC_COMMAND_UUID_32(_x)                 ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x07)

   /* The following MACRO is a utility MACRO that assigns the IrMC Sync */
   /* Command Bluetooth Universally Unique Identifier (IrMCSyncCommand) */
   /* to the specified UUID_128_t variable.  This MACRO accepts one     */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* IrMCSyncCommand Constant value.                                   */
#define SDP_ASSIGN_IRMC_SYNC_COMMAND_UUID_128(_x)                ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x07, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Profile UUID's for SyncML.  The SYNCML_SERVER UUIDs are used  */
   /* by the SyncML Server device, while the SYNCML_CLIENT UUID is used */
   /* by the SyncML Client device.  Note that these designation do not  */
   /* necessarily correspond to the OBEX server/client roles            */

   /* The following MACRO is a utility MACRO that assigns the SyncML    */
   /* Server Universally Unique Identifier (SyncMLServer) to the        */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* SYNCML_SERVER_UUID_128 Constant value.                            */
   /* * NOTE * Because the SyncML specification does not use a standard */
   /*          Bluetooth base UUID, ASSIGN_SDP_UUID_128 must be used    */
   /*          instead of the shorter 16 or 32 versions.                */
#define SDP_ASSIGN_SYNCML_SERVER_UUID_128(_x)                    ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x02)

   /* The following MACRO is a utility MACRO that assigns the SyncML    */
   /* Client Universally Unique Identifier (SyncMLClient) to the        */
   /* specified UUID_128_t variable.  This MACRO accepts one parameter  */
   /* which is the UUID_128_t variable that is to receive the           */
   /* SYNCML_CLIENT_UUID_128 Constant value.                            */
   /* * NOTE * Because the SyncML specification does not use a standard */
   /*          Bluetooth base UUID, ASSIGN_SDP_UUID_128 must be used    */
   /*          instead of the shorter 16 or 32 versions.                */
#define SDP_ASSIGN_SYNCML_CLIENT_UUID_128(_x)                    ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x02, 0xEE, 0x00, 0x00, 0x02)

   /* The following define sets the Sync Profile version included in the*/
   /* SDP record for the sync profile, as specified in the relevant     */
   /* specification.                                                    */
#define SYNC_PROFILE_VERSION                                     (0x0101)

   /* The following constants represent the Minimum and Maximum Port    */
   /* Numbers that can be opened (both locally and remotely).  These    */
   /* constants specify the range for the Port Number parameters in the */
   /* Open Functions.                                                   */
#define SYNC_PORT_NUMBER_MINIMUM                                 (SPP_PORT_NUMBER_MINIMUM)
#define SYNC_PORT_NUMBER_MAXIMUM                                 (SPP_PORT_NUMBER_MAXIMUM)

   /* The following constants are the OBEX response codes that are      */
   /* specifically mentioned in the IrMC Sync or SyncML specifications  */
   /* for particular response situations.  All OBEX responses are       */
   /* possible, valid, and accepted as well.                            */
#define SYNC_OBEX_RESPONSE_CONTINUE                              (OBEX_CONTINUE_RESPONSE)
#define SYNC_OBEX_RESPONSE_OK                                    (OBEX_OK_RESPONSE)
#define SYNC_OBEX_RESPONSE_UNAUTHORIZED                          (OBEX_UNAUTHORIZED_RESPONSE)
#define SYNC_OBEX_RESPONSE_NOT_FOUND                             (OBEX_NOT_FOUND_RESPONSE)
#define SYNC_OBEX_RESPONSE_OBJECT_TOO_LARGE                      (OBEX_REQUESTED_ENTITY_TOO_LARGE_RESPONSE)
#define SYNC_OBEX_RESPONSE_OBJECT_TYPE_NOT_SUPPORTED             (OBEX_UNSUPPORTED_MEDIA_TYPE_RESPONSE)
#define SYNC_OBEX_RESPONSE_DATABASE_LOCKED                       (OBEX_DATABASE_LOCKED_RESPONSE)
#define SYNC_OBEX_RESPONSE_DATABASE_FULL                         (OBEX_DATABASE_FULL_RESPONSE)
#define SYNC_OBEX_RESPONSE_CONFLICT                              (OBEX_CONFLICT_RESPONSE)
#define SYNC_OBEX_RESPONSE_FORBIDDEN                             (OBEX_FORBIDDEN_RESPONSE)
#define SYNC_OBEX_RESPONSE_SERVICE_UNAVAILABLE                   (OBEX_SERVICE_UNAVAILABLE_RESPONSE)
#define SYNC_OBEX_RESPONSE_BAD_REQUEST                           (OBEX_BAD_REQUEST_RESPONSE)
#define SYNC_OBEX_RESPONSE_NOT_IMPLEMENTED                       (OBEX_NOT_IMPLEMENTED_RESPONSE)
#define SYNC_OBEX_RESPONSE_INTERNAL_ERROR                        (OBEX_INTERNAL_SERVER_ERROR_RESPONSE)

   /* The following defines the OBEX Response Final Bit to be appended  */
   /* to the response codes passed into function of this module.        */
#define SYNC_OBEX_RESPONSE_FINAL_BIT                             (OBEX_FINAL_BIT)

   /* The following BIT definitions are used to denote the possible Sync*/
   /* Profile Server Modes that can be applied to a Sync Profile Client */
   /* Connection.  These BIT definitions are used with the              */
   /* SYNC_Set_Server_Mode() and SYNC_Get_Server_Mode() mode functions. */
#define SYNC_SERVER_MODE_AUTOMATIC_ACCEPT_CONNECTION             (0x00000000)
#define SYNC_SERVER_MODE_MANUAL_ACCEPT_CONNECTION                (0x00000001)

   /* The following BIT MASK is used to mask the Server Mode Accept     */
   /* Connection settings from other (undefined) Server Mode bits.      */
#define SYNC_SERVER_MODE_CONNECTION_MASK                         (0x00000001)

   /* The following constants represent the Sync Profile Status Values  */
   /* that are possible in the Sync Profile Connect Confirmation Event. */
#define SYNC_OPEN_STATUS_SUCCESS                                 (0x00)
#define SYNC_OPEN_STATUS_CONNECTION_TIMEOUT                      (0x01)
#define SYNC_OPEN_STATUS_CONNECTION_REFUSED                      (0x02)
#define SYNC_OPEN_STATUS_UNKNOWN_ERROR                           (0x04)

   /* The following definitions are bit-flags used to specify the object*/
   /* stores supported when calling the SYNC_Open_Server_Port()         */
   /* function.  These flags will be converted to the appropriate object*/
   /* store byte values and included in the SDP record when registered. */
#define SYNC_PHONEBOOK_OBJECT_STORE_SUPPORTED_BIT                (0x00000001)
#define SYNC_CALENDAR_OBJECT_STORE_SUPPORTED_BIT                 (0x00000004)
#define SYNC_NOTES_OBJECT_STORE_SUPPORTED_BIT                    (0x00000010)
#define SYNC_MESSAGES_OBJECT_STORE_SUPPORTED_BIT                 (0x00000020)
#define SYNC_ALL_OBJECT_STORES_SUPPORTED_BIT                     (SYNC_PHONEBOOK_OBJECT_STORE_SUPPORTED_BIT | SYNC_CALENDAR_OBJECT_STORE_SUPPORTED_BIT | SYNC_NOTES_OBJECT_STORE_SUPPORTED_BIT | SYNC_MESSAGES_OBJECT_STORE_SUPPORTED_BIT)

   /* IrMC Defined types required for OBEX Sync.                        */

   /* The following define specifies the maximum character size of an   */
   /* LUID identifier.  This is the number of Unicode characters for a  */
   /* Unicode string, or ASCII characters for an ASCII format string.   */
   /* This also determines the size of static buffers that handle LUID  */
   /* values.  According to the OBEX/IrMC specification this should be  */
   /* set to 50 characters (+2 bytes for Unicode NULL), but older       */
   /* versions of the Widcomm stack return larger values and for        */
   /* compatilibity this value has been set to a value larger than the  */
   /* LUIDs seen from Widcomm.                                          */
#define SYNC_MAX_LUID_CHARACTER_SIZE                        (160)

   /* The following enumeration defines all the possible object store   */
   /* types defined by the IrMC specification and extensions to this    */
   /* specification defined in the IrMC Errata, PBAP Specification, and */
   /* Sync Specification.  The fundamental IrMC types defined by the    */
   /* IrMC Spec include osPhonebook, osCalendar, osNotes, osMsgIn,      */
   /* osMsgOut, and osMsgSent.  The osMsgIn, osMsgOut, and isMsgSent    */
   /* types could be viewed as part of a unified Message object store.  */
   /* These are divided here because they have independent change       */
   /* counters and change logs.  The errata for IrMC Version 1.1 adds a */
   /* new object store type called storeBookmarks.  Currently the Sync  */
   /* and PBAP profiles do not support this new object store, but it is */
   /* included here for completeness.  storeInbox is used to represent  */
   /* the IrMC Inbox concept for Level 1 access.  The final type,       */
   /* storeUnknown, is for use in implementations that need to mark a   */
   /* store of an unexpected or unknown type and does not correspond to */
   /* an actual defined object store type.                              */
typedef enum
{
   osPhonebook,
   osCalendar,
   osMsgIn,
   osMsgOut,
   osMsgSent,
   osNotes,
   osBookmark,
   osInbox,
   osUnknown
} IrMC_Object_Store_Type_t;

   /* The following structure is used as an internal representation of  */
   /* time.  This TimeDate structure can be converted to an IrMC format */
   /* timestamp.                                                        */
typedef struct _tagIrMC_TimeDate_t
{
   Word_t    Year;
   Word_t    Month;
   Word_t    Day;
   Word_t    Hour;
   Word_t    Minute;
   Word_t    Second;
   Boolean_t UTC_Time;
} IrMC_TimeDate_t;

   /* This enumeration defines the possible Information Exchange Levels */
   /* defined by the IrMC specification.                                */
typedef enum
{
   olLevel1,
   olLevel2,
   olLevel3,
   olLevel4
} IrMC_Operation_Level_t;

   /* This structure contains discrete representations of the           */
   /* information contained within an IrMC request.  Most of this       */
   /* information must be determined by parsing the Object Name/Path.   */
   /* The SYNC_ParseObjectName() performs this parsing and provides the */
   /* output using this structure.                                      */
typedef struct _tagIrMC_Operation_t
{
   IrMC_Operation_Level_t    Level;
   IrMC_Object_Store_Type_t  StoreType;
   char                      Name[SYNC_MAX_LUID_CHARACTER_SIZE + 1];
   DWord_t                   Index;
   DWord_t                   MaxChangeCounter;
   Boolean_t                 UseMaxChangeCounter;
   Boolean_t                 HardDelete;
} IrMC_Operation_t;

   /* The following enumeration defines the valid Sync Anchor types for */
   /* an IrMC sync operation.  These types are used in the IrMC_Anchor_t*/
   /* structure below to determine the data stored within the structure.*/
typedef enum
{
   atChangeCounter,
   atTimestamp,
   atNone
} IrMC_Anchor_Type_t;

   /* The following structure defines a Sync Anchor that can be used or */
   /* received in profile functions and events to specify the Sync      */
   /* Anchor type and data for a given IrMC operation.  The Anchor_Type */
   /* field determines which Anchor_Data union element should be used.  */
   /* This structure is used when calling                               */
   /* SYNC_IrMC_Special_Object_Get_Request() for certain object types.  */
   /* They are also used when calling SYNC_IrMC_Object_Get_Response() or*/
   /* SYNC_IrMC_Object_Put_Response().  This type is also used in       */
   /* relevant events.                                                  */
typedef struct _tagIrMC_Anchor_t
{
   IrMC_Anchor_Type_t Anchor_Type;
   union
   {
      DWord_t         ChangeCounter;
      IrMC_TimeDate_t Timestamp;
   } Anchor_Data;
} IrMC_Anchor_t;

#define SYNC_ANCHOR_SIZE                           (sizeof(IrMC_Anchor_t))

   /* The following enumeration defines the possible services that can  */
   /* be used with a call to SYNC_Open_Server_Port() or                 */
   /* SYNC_Open_Remote_Server_Port().  Note that when using these       */
   /* enumerations in a call to SYNC_Open_Server_Port(), these represent*/
   /* the TYPE of the LOCAL server to create.  When used with the       */
   /* SYNC_Open_Remote_Server_Port() function, these types represent the*/
   /* TYPE of the REMOTE server with which to connect.                  */
typedef enum
{
   stIrMC_Server,
   stIrMC_SyncCommand,
   stSyncML_Server,
   stSyncML_Client
} SYNC_Sync_Type_t;

   /* The following enumeration defines the possible special object     */
   /* types that can be requested with                                  */
   /* SYNC_IrMC_Special_Object_Get_Request().  These types are also     */
   /* provided when receiving Special Object related events.            */
   /* * NOTE * The type sotSyncCommand is included for internal usage,  */
   /*          but the explicit Sync Command related functions should be*/
   /*          used for Sync Command related operations.  Also sotRTC is*/
   /*          included for internal usage, but the explicit RTC related*/
   /*          functions should be used for RTC related operations.     */
typedef enum
{
   sotChangeCounter,
   sotChangeLog,
   sotInfoLog,
   sotDeviceInfo,
   sotRTC,
   sotSyncCommand,
   sotUnknown
} SYNC_Special_Object_Type_t;

   /* The following enumeration defines the supported MIME types for a  */
   /* SyncML payload.  This enumeration is used when calling            */
   /* SYNC_SyncML_Object_Get_Request() or                               */
   /* SYNC_SyncML_Object_Put_Request().  Setting the appropriate MIME   */
   /* type in a SyncML related call will cause the appropriate strings  */
   /* to be sent in the Type OBEX header.  This enumeration will not    */
   /* affect the actual payload passed for the Body of the message, so  */
   /* its up to the caller to conform to the chosen MIME Type.          */
typedef enum
{
   smtXML,
   smtWBXML,
   smtUnknown
} SYNC_SyncML_MIME_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* Sync Profile.  These are used to determine the type of each event */
   /* generated, and to ensure the proper union element is accessed for */
   /* the SYNC_Event_Data_t structure.                                  */
typedef enum
{
   etSYNC_Open_Port_Indication,
   etSYNC_Open_Port_Confirmation,
   etSYNC_Open_Port_Request_Indication,
   etSYNC_Close_Port_Indication,
   etSYNC_Abort_Indication,
   etSYNC_Abort_Confirmation,
   etSYNC_IrMC_Object_Put_Indication,
   etSYNC_IrMC_Object_Put_Confirmation,
   etSYNC_IrMC_Object_Get_Indication,
   etSYNC_IrMC_Object_Get_Confirmation,
   etSYNC_IrMC_Object_Delete_Indication,
   etSYNC_IrMC_Object_Delete_Confirmation,
   etSYNC_IrMC_Special_Object_Get_Indication,
   etSYNC_IrMC_Special_Object_Get_Confirmation,
   etSYNC_IrMC_RTC_Put_Indication,
   etSYNC_IrMC_RTC_Put_Confirmation,
   etSYNC_IrMC_Sync_Command_Indication,
   etSYNC_IrMC_Sync_Command_Confirmation,
   etSYNC_SyncML_Object_Get_Indication,
   etSYNC_SyncML_Object_Get_Confirmation,
   etSYNC_SyncML_Object_Put_Indication,
   etSYNC_SyncML_Object_Put_Confirmation
} SYNC_Event_Type_t;

   /* The following Sync Profile Event is dispatched when a Sync Profile*/
   /* Client Connects to a local Sync Profile Server.  The SYNC ID      */
   /* member specifies the local Server that has been connected to and  */
   /* the BD_ADDR member specifies the Client Bluetooth Device that has */
   /* connected to the specified Server.                                */
typedef struct _tagSYNC_Open_Port_Indication_Data_t
{
   unsigned int SYNCID;
   BD_ADDR_t    BD_ADDR;
} SYNC_Open_Port_Indication_Data_t;

#define SYNC_OPEN_PORT_INDICATION_DATA_SIZE        (sizeof(SYNC_Open_Port_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a Sync Client */
   /* receives the Connection Response from a remote Sync Profile Server*/
   /* to which it has previously attempted to connect.  The Sync Profile*/
   /* ID member specifies the local Client that requested the           */
   /* connection, the Sync Profile Connect Status represents the        */
   /* Connection Status of the Request, and the BD_ADDR member specifies*/
   /* the Bluetooth Device Address of the remote Bluetooth Sync Profile */
   /* Server.                                                           */
typedef struct _tagSYNC_Open_Port_Confirmation_Data_t
{
   unsigned int SYNCID;
   unsigned int SYNCConnectStatus;
   BD_ADDR_t    BD_ADDR;
} SYNC_Open_Port_Confirmation_Data_t;

#define SYNC_OPEN_PORT_CONFIRMATION_DATA_SIZE      (sizeof(SYNC_Open_Port_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when a Sync Profile*/
   /* Client requests a connection to a local Sync Profile Server.  The */
   /* SYNC ID member specifies the local Server that has received the   */
   /* request to connect and the BD_ADDR member specifies the Client    */
   /* Bluetooth Device that is requesting to connect to the specified   */
   /* Server.                                                           */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            SYNC_Open_Request_Response() function in order to      */
   /*            accept or reject the outstanding Open Request.         */
typedef struct _tagSYNC_Open_Port_Request_Indication_Data_t
{
   unsigned int SYNCID;
   unsigned int ServerPort;
   BD_ADDR_t    BD_ADDR;
} SYNC_Open_Port_Request_Indication_Data_t;

#define SYNC_OPEN_PORT_REQUEST_INDICATION_DATA_SIZE (sizeof(SYNC_Open_Port_Request_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a Sync Profile*/
   /* Server or Client connection is disconnected.  The SYNC ID member  */
   /* specifies the local connection from which the remote device has   */
   /* disconnected.                                                     */
typedef struct _tagSYNC_Close_Port_Indication_Data_t
{
   unsigned int SYNCID;
} SYNC_Close_Port_Indication_Data_t;

#define SYNC_CLOSE_PORT_INDICATION_DATA_SIZE       (sizeof(SYNC_Close_Port_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a Sync Profile*/
   /* Abort Request is received from a remote Sync Profile Client.  The */
   /* SYNC ID member specifies the local Server that has received the   */
   /* request.                                                          */
   /* * NOTE * When receiving this indication, the Sync profile has     */
   /*          already automatically dispatched a response to the remote*/
   /*          client.                                                  */
typedef struct _tagSYNC_Abort_Indication_Data_t
{
   unsigned int SYNCID;
} SYNC_Abort_Indication_Data_t;

#define SYNC_ABORT_INDICATION_DATA_SIZE            (sizeof(SYNC_Abort_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a Sync Profile*/
   /* Abort Response is received from a remote Sync Profile Server.  The*/
   /* SYNC ID member specifies the local Client that has received the   */
   /* response.                                                         */
typedef struct _tagSYNC_Abort_Confirmation_Data_t
{
   unsigned int SYNCID;
} SYNC_Abort_Confirmation_Data_t;

#define SYNC_ABORT_CONFIRMATION_DATA_SIZE          (sizeof(SYNC_Abort_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Server receives an Object Get Request from a remote IrMC Sync     */
   /* Client.  The SYNC ID member specifies the local Server that has   */
   /* received the request.  The ObjectName member contains the         */
   /* Name/Path of the object being requested by this Object Get        */
   /* operation.  The ObjectStoreType member contains an attempt at     */
   /* determining the Object Store destination of this Get request.  If */
   /* the store cound not be determined it will be set to osUnknown.    */
   /* The LUID member contains the LUID portion of the ObjectName,      */
   /* including the file extension.  If the LUID portion could not be   */
   /* determined this member will be NULL.  If valid (not NULL) this    */
   /* pointer will always reference the same underlying buffer as the   */
   /* ObjectName pointer member.  Alternately the ObjectName contains   */
   /* the full path and can be used exclusively to determine the        */
   /* appropriate object response.                                      */
typedef struct _tagSYNC_IrMC_Object_Get_Indication_Data_t
{
   unsigned int  SYNCID;
   char         *ObjectName;
} SYNC_IrMC_Object_Get_Indication_Data_t;

#define SYNC_IRMC_OBJECT_GET_INDICATION_DATA_SIZE  (sizeof(SYNC_IrMC_Object_Get_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Client receives an Object Get Response from a remote IrMC Sync    */
   /* Server.  The SYNC ID member specifies the local client that has   */
   /* received the response.  The ResponseCode member contains the      */
   /* response code included in the response packet.  The ObjectName    */
   /* member points to an ASCII, NULL terminated string of the Name/Path*/
   /* included in the Name header of the Get Response (if the response  */
   /* did not include a Name header, this member will be NULL).  The    */
   /* BufferSize and Buffer members contain the size and pointer        */
   /* (respectively) for the object data included in the Object Get     */
   /* Response.  ObjectLength optionally contains the value sent in the */
   /* Length header by the responder, and indicates the total size of   */
   /* all data to be sent (if the Length header is not present, this    */
   /* value will be zero).  The Final member indicates if this is the   */
   /* last chunk of data that will be sent in the Get Object            */
   /* transaction.                                                      */
typedef struct _tagSYNC_IrMC_Object_Get_Confirmation_Data_t
{
   unsigned int  SYNCID;
   Byte_t        ResponseCode;
   char         *ObjectName;
   unsigned int  BufferSize;
   Byte_t       *Buffer;
   DWord_t       ObjectLength;
   Boolean_t     Final;
} SYNC_IrMC_Object_Get_Confirmation_Data_t;

#define SYNC_IRMC_OBJECT_GET_CONFIRMATION_DATA_SIZE (sizeof(SYNC_IrMC_Object_Get_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Server receives an Object Put Request from a remote IrMC Sync     */
   /* Client.  The SYNC ID member specifies the local Server that has   */
   /* received the request.  ObjectName points to an ASCII, NULL        */
   /* terminated string of the Name/Path included in the Name header of */
   /* the Put Request.  The ObjectStoreType member contains the type of */
   /* object store that was parsed from the ObjectName.  If the         */
   /* ObjectName was not successfully parsed this member will be set to */
   /* 'osUnknown'.  ObjectLength contains the value sent in the Length  */
   /* header by the responder, and indicates the total size of all data */
   /* to be sent (if the Length header is not present, this member will */
   /* be zero).  The BufferSize and Buffer members contain the size and */
   /* pointer (respectively) for the object data included in the Object */
   /* Put Request.  The MaxChangeCounter member is a pointer to a       */
   /* variable that contains the value of the optional Max Change       */
   /* Counter application parameter header (this pointer will be NULL if*/
   /* the parameter was not present).  The Final member indicates if    */
   /* this is the last chunk of data that will be sent in the Put Object*/
   /* transaction.                                                      */
typedef struct _tagSYNC_IrMC_Object_Put_Indication_Data_t
{
   unsigned int  SYNCID;
   char         *ObjectName;
   DWord_t       ObjectLength;
   unsigned int  BufferSize;
   Byte_t       *Buffer;
   DWord_t      *MaxChangeCounter;
   Boolean_t     Final;
} SYNC_IrMC_Object_Put_Indication_Data_t;

#define SYNC_IRMC_OBJECT_PUT_INDICATION_DATA_SIZE  (sizeof(SYNC_IrMC_Object_Put_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Client receives an Object Put Response from a remote IrMC Sync    */
   /* Server.  The SYNC ID member specifies the local IrMC Client that  */
   /* has received the response.  The ResponseCode member contains the  */
   /* response code returned in the Object Put Response packet.  The    */
   /* LUID member points to an ASCII, NULL terminated string with the   */
   /* LUID returned by the responder (If the LUID header is not         */
   /* included, this member will be NULL).  The SyncAnchor member is a  */
   /* pointer to a structure that contains information about Sync Anchor*/
   /* related information in the response (if this information is not   */
   /* included, this member will be NULL).                              */
typedef struct _tagSYNC_IrMC_Object_Put_Confirmation_Data_t
{
   unsigned int   SYNCID;
   Byte_t         ResponseCode;
   char          *LUID;
   IrMC_Anchor_t *SyncAnchor;
} SYNC_IrMC_Object_Put_Confirmation_Data_t;

#define SYNC_IRMC_OBJECT_PUT_CONFIRMATION_DATA_SIZE (sizeof(SYNC_IrMC_Object_Put_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Server receives an Object Delete Request from a remote IrMC Sync  */
   /* Client.  The SYNC ID member specifies the local Server that has   */
   /* received the request.  The ObjectName member points to an ASCII,  */
   /* NULL terminated string of the Name/Path included in the Name      */
   /* header of the Delete Request.  The ObjectStoreType member contains*/
   /* the type of object store that was parsed from the ObjectName.  If */
   /* the ObjectName was not successfully parsed this member will be set*/
   /* to 'osUnknown'.  The LUID member is an ASCII, Null-Terminated     */
   /* string containing the parsed LUID and extension from the          */
   /* ObjectName member.  This pointer will reference the same          */
   /* underlying memory as the ObjectName member.  If the LUID could not*/
   /* be parsed from the ObjectName it will be set to NULL.  The        */
   /* MaxChangeCounter member is a pointer to a variable that contains  */
   /* the value of the optional Max Change Counter application parameter*/
   /* header (this pointer will be NULL if the parameter was not        */
   /* present).  The HardDelete member indicates if this is a request to*/
   /* perform a Hard or Soft Delete operation.                          */
typedef struct _tagSYNC_IrMC_Object_Delete_Indication_Data_t
{
   unsigned int  SYNCID;
   char         *ObjectName;
   DWord_t      *MaxChangeCounter;
   Boolean_t     HardDelete;
} SYNC_IrMC_Object_Delete_Indication_Data_t;

#define SYNC_IRMC_OBJECT_DELETE_INDICATION_DATA_SIZE (sizeof(SYNC_IrMC_Object_Delete_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Client receives an Object Delete Response from a remote IrMC Sync */
   /* Server.  The SYNC ID member specifies the local IrMC Sync Client  */
   /* that has received the response.  The ResponseCode member contains */
   /* the response code returned in the Object Delete Response packet.  */
   /* The LUID member contains an ASCII, NULL terminated string with the*/
   /* LUID returned by the responder (If LUID is not included, this     */
   /* member will be NULL).  The SyncAnchor member is a pointer to a    */
   /* structure that contains information about Sync Anchor related     */
   /* information in the response (if this information is not included, */
   /* this member will be NULL).                                        */
typedef struct _tagSYNC_IrMC_Object_Delete_Confirmation_Data_t
{
   unsigned int   SYNCID;
   Byte_t         ResponseCode;
   char          *LUID;
   IrMC_Anchor_t *SyncAnchor;
} SYNC_IrMC_Object_Delete_Confirmation_Data_t;

#define SYNC_IRMC_OBJECT_DELETE_CONFIRMATION_DATA_SIZE (sizeof(SYNC_IrMC_Object_Delete_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Server receives a Special Object Get Request from a remote IrMC   */
   /* Sync Client.  The SYNC ID member specifies the local Server that  */
   /* has received the request.  The ObjectName member points to an     */
   /* ASCII, NULL terminated string that contains the complete Name/Path*/
   /* of the request.  The ObjectType enumeration specifies the type of */
   /* special object being requested.  The ObjectStore enumeration      */
   /* specifies the Object Store type to which this request applies.    */
   /* The SyncAnchor parameter is only used for the Change Log object   */
   /* type, and includes the Sync anchor information for which the      */
   /* Change Log is requested.  For all other types this member will be */
   /* NULL.                                                             */
   /* * NOTE * The information provided in the ObjectType and           */
   /*          ObjectStoreType enumerations, as well as the SyncAnchor  */
   /*          structure, are provided for convenience only.  This same */
   /*          information is available by parsing the specified        */
   /*          ObjectName.  In addition, some special object types do   */
   /*          not use an Object Store or Sync Anchor value.  These     */
   /*          types can be handled based on the Object Type enumeration*/
   /*          only.                                                    */
typedef struct _tagSYNC_IrMC_Special_Object_Get_Indication_Data_t
{
   unsigned int                SYNCID;
   char                       *ObjectName;
   IrMC_Object_Store_Type_t    ObjectStoreType;
   SYNC_Special_Object_Type_t  ObjectType;
   IrMC_Anchor_t              *SyncAnchor;
} SYNC_IrMC_Special_Object_Get_Indication_Data_t;

#define SYNC_IRMC_SPECIAL_OBJECT_GET_INDICATION_DATA_SIZE (sizeof(SYNC_IrMC_Special_Object_Get_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Client receives a Special Object Get Response from a remote IrMC  */
   /* Sync Server.  The SYNC ID member specifies the local Client that  */
   /* has received the request.  The ResponseCode member contains the   */
   /* response code included in the response packet.  The ObjectName    */
   /* member points to an ASCII, NULL terminated string containing the  */
   /* Name/Path included in the Name header of the Get Response (if the */
   /* response did not include a Name header, this member will be NULL).*/
   /* The BufferSize and Buffer members contain the size and pointer    */
   /* (respectively) for the object data included in the Object Get     */
   /* Response.  ObjectLength optionally contains the value sent in the */
   /* Length header by the responder, and indicates the total size of   */
   /* all data to be sent (if the Length header is not present, the     */
   /* value will be zero).  The Final member indicates if this is the   */
   /* last chunk of data that will be sent in the Special Object Get    */
   /* transaction.                                                      */
typedef struct _tagSYNC_IrMC_Special_Object_Get_Confirmation_Data_t
{
   unsigned int  SYNCID;
   Byte_t        ResponseCode;
   char         *ObjectName;
   unsigned int  BufferSize;
   Byte_t       *Buffer;
   Boolean_t     Final;
} SYNC_IrMC_Special_Object_Get_Confirmation_Data_t;

#define SYNC_IRMC_SPECIAL_OBJECT_GET_CONFIRMATION_DATA_SIZE (sizeof(SYNC_IrMC_Special_Object_Get_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Server receives a Real Time Clock (RTC) Put Request from a remote */
   /* IrMC Sync Client.  The SYNC ID member specifies the local Server  */
   /* that has received the request.  The TimeDate structure contains   */
   /* the Time/Date information contained within the RTC PUT request.   */
typedef struct _tagSYNC_IrMC_RTC_Put_Indication_Data_t
{
   unsigned int    SYNCID;
   IrMC_TimeDate_t TimeDate;
} SYNC_IrMC_RTC_Put_Indication_Data_t;

#define SYNC_IRMC_RTC_PUT_INDICATION_DATA_SIZE     (sizeof(SYNC_IrMC_RTC_Put_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Client receives a Real Time Clock (RTC) Put Response from a remote*/
   /* IrMC Sync Server.  The SYNC ID member specifies the local Client  */
   /* that has received the response.  The ResponseCode member contains */
   /* the response code included in the response packet.                */
typedef struct _tagSYNC_IrMC_RTC_Put_Confirmation_Data_t
{
   unsigned int SYNCID;
   Byte_t       ResponseCode;
} SYNC_IrMC_RTC_Put_Confirmation_Data_t;

#define SYNC_IRMC_RTC_PUT_CONFIRMATION_DATA_SIZE   (sizeof(SYNC_IrMC_RTC_Put_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Command Server receives a Sync Command Request from a remote IrMC */
   /* Sync Command Client.  The SYNC ID member specifies the local      */
   /* Server that has received the Sync Command.  The BufferSize and    */
   /* Buffer members contain the size and pointer (respectively) of the */
   /* data that was contained in the Sync Command Request, typically a  */
   /* string specifying the action to be executed.  For more information*/
   /* see the IrMC Specification (Section 5.8, p43).                    */
typedef struct _tagSYNC_IrMC_Sync_Command_Indication_Data_t
{
   unsigned int  SYNCID;
   unsigned int  BufferSize;
   Byte_t       *Buffer;
} SYNC_IrMC_Sync_Command_Indication_Data_t;

#define SYNC_IRMC_SYNC_COMMAND_INDICATION_DATA_SIZE (sizeof(SYNC_IrMC_Sync_Command_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when an IrMC Sync  */
   /* Command Client receives a Sync Command Response from a remote IrMC*/
   /* Sync Command Server.  The SYNC ID member specifies the local      */
   /* Client that received the response.  The ResponseCode member       */
   /* contains the response code included in the response packet.       */
typedef struct _tagSYNC_IrMC_Sync_Command_Confirmation_Data_t
{
   unsigned int SYNCID;
   Byte_t       ResponseCode;
} SYNC_IrMC_Sync_Command_Confirmation_Data_t;

#define SYNC_IRMC_SYNC_COMMAND_CONFIRMATION_DATA_SIZE (sizeof(SYNC_IrMC_Sync_Command_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when a SyncML      */
   /* Server receives a SyncML Object Get Request from a remote SyncML  */
   /* Client.  The SYNC ID member specifies the local Server that       */
   /* received the request.  The MIMEType member is an enumeration which*/
   /* specifies the MIME Type indicated by the Type header in the       */
   /* received packet.                                                  */
typedef struct _tagSYNC_SyncML_Object_Get_Indication_Data_t
{
   unsigned int            SYNCID;
   SYNC_SyncML_MIME_Type_t MIMEType;
} SYNC_SyncML_Object_Get_Indication_Data_t;

#define SYNC_SYNCML_OBJECT_GET_INDICATION_DATA_SIZE (sizeof(SYNC_SyncML_Object_Get_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a SyncML      */
   /* Client receives a SyncML Object Get Response from a remote SyncML */
   /* Server.  The SYNC ID member specifies the local Client that has   */
   /* received the response.  The ResponseCode member contains the      */
   /* response code included in the response packet.  The BufferSize and*/
   /* Buffer members contain the size and pointer (respectively) for the*/
   /* object data included in the Object Get Response.  ObjectLength    */
   /* contains the value sent in the Length header by the responder, and*/
   /* indicates the total size of all data to be sent (if the Length    */
   /* header is not present, this member will be zero).  The Final      */
   /* member indicates if this is the last chunk of data that will be   */
   /* sent in the SyncML Get Object transaction.                        */
typedef struct _tagSYNC_SyncML_Object_Get_Confirmation_Data_t
{
   unsigned int  SYNCID;
   Byte_t        ResponseCode;
   DWord_t       ObjectLength;
   unsigned int  BufferSize;
   Byte_t       *Buffer;
   Boolean_t     Final;
} SYNC_SyncML_Object_Get_Confirmation_Data_t;

#define SYNC_SYNCML_OBJECT_GET_CONFIRMATION_DATA_SIZE (sizeof(SYNC_SyncML_Object_Get_Confirmation_Data_t))

   /* The following Sync Profile Event is dispatched when a SyncML      */
   /* Server receives a SyncML Object Put Request from a remote SyncML  */
   /* Client.  The SYNC ID member specifies the local Server that has   */
   /* received the request.  The MIMEType member is an enumeration which*/
   /* specifies the MIME Type indicated by the Type header in the       */
   /* request packet.  ObjectLength indicates the total size of all data*/
   /* to be sent, based on the Length header in the request (if the     */
   /* Length header is not present, this member will be zero).  The     */
   /* BufferSize and Buffer members contain the size and pointer        */
   /* (respectively) for the object data included in the Object Put     */
   /* Request.  The Final member indicates if this is the last chunk of */
   /* data that will be sent in the SyncML Put Object transaction.      */
typedef struct _tagSYNC_SyncML_Object_Put_Indication_Data_t
{
   unsigned int             SYNCID;
   SYNC_SyncML_MIME_Type_t  MIMEType;
   DWord_t                  ObjectLength;
   unsigned int             BufferSize;
   Byte_t                  *Buffer;
   Boolean_t                Final;
} SYNC_SyncML_Object_Put_Indication_Data_t;

#define SYNC_SYNCML_OBJECT_PUT_INDICATION_DATA_SIZE (sizeof(SYNC_SyncML_Object_Put_Indication_Data_t))

   /* The following Sync Profile Event is dispatched when a SyncML      */
   /* Client receives an Object Put Response from a remote SyncML       */
   /* Server.  The SYNC ID member specifies the local Client that has   */
   /* received the response.  The ResponseCode member contains the      */
   /* response code included in the response packet.                    */
typedef struct _tagSYNC_SyncML_Object_Put_Confirmation_Data_t
{
   unsigned int SYNCID;
   Byte_t       ResponseCode;
} SYNC_SyncML_Object_Put_Confirmation_Data_t;

#define SYNC_SYNCML_OBJECT_PUT_CONFIRMATION_DATA_SIZE (sizeof(SYNC_SyncML_Object_Put_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all Sync Profile Event Data.  This structure is received  */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagSYNC_Event_Data_t
{
   SYNC_Event_Type_t Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      SYNC_Open_Port_Indication_Data_t                 *SYNC_Open_Port_Indication_Data;
      SYNC_Open_Port_Confirmation_Data_t               *SYNC_Open_Port_Confirmation_Data;
      SYNC_Open_Port_Request_Indication_Data_t         *SYNC_Open_Port_Request_Indication_Data;
      SYNC_Close_Port_Indication_Data_t                *SYNC_Close_Port_Indication_Data;
      SYNC_Abort_Indication_Data_t                     *SYNC_Abort_Indication_Data;
      SYNC_Abort_Confirmation_Data_t                   *SYNC_Abort_Confirmation_Data;
      SYNC_IrMC_Object_Put_Indication_Data_t           *SYNC_IrMC_Object_Put_Indication_Data;
      SYNC_IrMC_Object_Put_Confirmation_Data_t         *SYNC_IrMC_Object_Put_Confirmation_Data;
      SYNC_IrMC_Object_Get_Indication_Data_t           *SYNC_IrMC_Object_Get_Indication_Data;
      SYNC_IrMC_Object_Get_Confirmation_Data_t         *SYNC_IrMC_Object_Get_Confirmation_Data;
      SYNC_IrMC_Object_Delete_Indication_Data_t        *SYNC_IrMC_Object_Delete_Indication_Data;
      SYNC_IrMC_Object_Delete_Confirmation_Data_t      *SYNC_IrMC_Object_Delete_Confirmation_Data;
      SYNC_IrMC_Special_Object_Get_Indication_Data_t   *SYNC_IrMC_Special_Object_Get_Indication_Data;
      SYNC_IrMC_Special_Object_Get_Confirmation_Data_t *SYNC_IrMC_Special_Object_Get_Confirmation_Data;
      SYNC_IrMC_RTC_Put_Indication_Data_t              *SYNC_IrMC_RTC_Put_Indication_Data;
      SYNC_IrMC_RTC_Put_Confirmation_Data_t            *SYNC_IrMC_RTC_Put_Confirmation_Data;
      SYNC_IrMC_Sync_Command_Indication_Data_t         *SYNC_IrMC_Sync_Command_Indication_Data;
      SYNC_IrMC_Sync_Command_Confirmation_Data_t       *SYNC_IrMC_Sync_Command_Confirmation_Data;
      SYNC_SyncML_Object_Put_Indication_Data_t         *SYNC_SyncML_Object_Put_Indication_Data;
      SYNC_SyncML_Object_Put_Confirmation_Data_t       *SYNC_SyncML_Object_Put_Confirmation_Data;
      SYNC_SyncML_Object_Get_Indication_Data_t         *SYNC_SyncML_Object_Get_Indication_Data;
      SYNC_SyncML_Object_Get_Confirmation_Data_t       *SYNC_SyncML_Object_Get_Confirmation_Data;
   } Event_Data;
} SYNC_Event_Data_t;

#define SYNC_EVENT_DATA_SIZE                       (sizeof(SYNC_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Sync Profile Event Receive Data Callback.  This function will be*/
   /* called whenever an Sync Profile Event occurs that is associated   */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the Sync Event Data that       */
   /* occurred and the Sync Profile Event Callback Parameter that was   */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the Sync Profile Event Data ONLY in the       */
   /* context of this callback.  If the caller requires the Data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* re-entrant).  It needs to be noted however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another Sync Profile Event will not be processed while this       */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving Sync Profile Event  */
   /*            Packets.  A Deadlock WILL occur because NO Sync Event  */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *SYNC_Event_Callback_t)(unsigned int BluetoothStackID, SYNC_Event_Data_t *SYNC_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for opening a local IrMC or */
   /* SyncML Sync Server of the specified type (SyncType).  The first   */
   /* parameter is the Bluetooth Stack ID on which to open the server.  */
   /* The second parameter is the Port on which to open this server, and*/
   /* *MUST* be between SYNC_PORT_NUMBER_MINIMUM and                    */
   /* SYNC_PORT_NUMBER_MAXIMUM.  The third parameter is the type of     */
   /* server to create.  The forth parameter is a mask which determines */
   /* the type of Object Stores supported by this server (This is only  */
   /* valid for an IrMC Sync Server, and uses the                       */
   /* SYNC_xxx_OBJECT_STORE_SUPPORTED_BIT(s).  This value is ignored for*/
   /* other server types).  The fifth parameter is the Callback function*/
   /* to call when an event occurs on this Server Port.  The final      */
   /* parameter is a user-defined callback parameter that will be passed*/
   /* to the callback function with each event.  This function returns a*/
   /* positive, non zero value if successful or a negative return error */
   /* code if an error occurs.  A successful return code will be a Sync */
   /* Profile ID that can be used to reference the Opened Sync Profile  */
   /* Server Port in ALL other Sync Server functions in this module.    */
   /* Once an Sync Profile Server is opened, it can only be             */
   /* Un-Registered via a call to the SYNC_Close_Server() function      */
   /* (passing the return value from this function).                    */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Open_Server_Port(unsigned int BluetoothStackID, unsigned int ServerPort, SYNC_Sync_Type_t SyncType, unsigned int ObjectStoresSupported, SYNC_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Open_Server_Port_t)(unsigned int BluetoothStackID, unsigned int ServerPort, SYNC_Sync_Type_t SyncType, unsigned int ObjectStoresSupported, SYNC_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for closing a Sync Profile  */
   /* Server (which was opened by a successful call to the              */
   /* SYNC_Open_Server_Port() function).  The first parameter is the    */
   /* Bluetooth Stack ID of the previously opened server port.  The     */
   /* second parameter is the SYNC ID returned from the previous call to*/
   /* SYNC_Open_Server_Port().  This function returns zero if           */
   /* successful, or a negative return error code if an error occurred  */
   /* (see BTERRORS.H).  Note that this function does NOT delete any SDP*/
   /* Service Record Handles.                                           */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Close_Server_Port(unsigned int BluetoothStackID, unsigned int SYNCID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Close_Server_Port_t)(unsigned int BluetoothStackID, unsigned int SYNCID);
#endif

   /* The following function adds a Sync Related Service Record to the  */
   /* SDP Database.  The first parameter is the Bluetooth Stack ID of   */
   /* the previously opened Server Port.  The second parameter is the   */
   /* SYNC ID that was returned by a previous call to                   */
   /* SYNC_Open_Server_Port().  The third parameter is a pointer to     */
   /* ASCII, NULL terminated string containing the Service Name to      */
   /* include within the SDP Record.  The final parameter is a pointer  */
   /* to a DWord_t which receives the SDP Service Record Handle if this */
   /* function successfully creates an SDP Service Record.  If this     */
   /* function returns zero, then the SDPServiceRecordHandle entry will */
   /* contain the Service Record Handle of the added SDP Service Record.*/
   /* If this function fails, a negative return error code will be      */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function will register an SDP record based on the   */
   /*          type of server that was opened with a call to            */
   /*          SYNC_Open_Server_Port().  There are four possible types  */
   /*          available (see SYNC_Sync_Type_t enumeration) and each    */
   /*          registers a different SDP record).                       */
   /* * NOTE * This function should only be called with the SYNC ID that*/
   /*          was returned from the SYNC_Open_Server_Port() function.  */
   /*          This function should NEVER be used with SYNC ID returned */
   /*          from the SYNC_Open_Remote_Server_Port() function.        */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SYNC_Un_Register_SDP_Record()  */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          SYNC_Un_Register_SDP_Record() to the                     */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI SYNC_Register_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int SYNCID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Register_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int SYNCID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that deletes the Sync      */
   /* Profile Server SDP Service Record (specified by the third         */
   /* parameter) from the SDP Database.  This MACRO maps to the         */
   /* SDP_Delete_Service_Record() function.  This MACRO is only provided*/
   /* so that the caller doesn't have to sift through the SDP API for   */
   /* very simplistic applications.  This function accepts as input the */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack that the       */
   /* Service Record exists on, the SYNC ID (returned from a successful */
   /* call to the SYNC_Open_Server_Port() function), and the SDP Service*/
   /* Record Handle.  The SDP Service Record Handle was returned via a  */
   /* successful call to the SYNC_Register_Server_SDP_Record() function.*/
   /* See the SYNC_Register_Server_SDP_Record() function for more       */
   /* information.  This MACRO returns the result of the                */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define SYNC_Un_Register_SDP_Record(__BluetoothStackID, __SYNCID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for opening a connection to */
   /* a remote IrMC or SyncML Sync Server.  The first parameter is the  */
   /* Bluetooth Stack ID of the local Bluetooth stack.  The second      */
   /* parameter is the remote Bluetooth Device Address of the Bluetooth */
   /* Sync Profile Server with which to connect.  The third parameter   */
   /* specifies the remote server port with which to connect.  The      */
   /* fourth parameter specifies the type of remote server with which to*/
   /* connect.  The final two parameters specify the Sync Profile Event */
   /* Callback Function and the Callback Parameter to associate with    */
   /* this Sync Profile Client.  The ServerPort parameter *MUST* be     */
   /* between SYNC_PORT_NUMBER_MINIMUM and SYNC_PORT_NUMBER_MAXIMUM.    */
   /* This function returns a positive, non zero, value if successful or*/
   /* a negative return error code if an error occurs.  A successful    */
   /* return code will be a SYNC ID that can be used to reference the   */
   /* remote opened Sync Profile Server in ALL other Sync Profile Client*/
   /* functions in this module.  Once a remote server is opened, it can */
   /* only be closed via a call to the SYNC_Close_Connection() function */
   /* (passing the return value from this function).                    */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Open_Remote_Server_Port(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, SYNC_Sync_Type_t SyncType, SYNC_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Open_Remote_Server_Port_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ServerPort, SYNC_Sync_Type_t SyncType, SYNC_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is provided to allow a means to respond to */
   /* a request to connect to the local Sync Profile Server.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Stack on     */
   /* which this server resides.  The second parameter is the SYNC ID   */
   /* that was returned from a previous SYNC_Open_Server_Port() function*/
   /* for this server.  The final parameter to this function is a       */
   /* Boolean_t that indicates whether to accept the pending connection.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etSYNC_Open_Port_Indication event has occurred.        */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Open_Request_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for closing a currently     */
   /* ongoing Sync Profile connection.  The first parameter is the      */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack Instance that  */
   /* is associated with the Sync Profile connection being closed.  The */
   /* second parameter to this function is the SYNC ID of the Sync      */
   /* Profile connection to be closed.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
   /* * NOTE * If this function is called with a Server SYNC ID (a value*/
   /*          returned from a call to SYNC_Open_Server_Port()) any     */
   /*          clients currently connected to this server will be       */
   /*          terminated, but the server will remained open and        */
   /*          registered.  If this function is called using a Client   */
   /*          SYNC ID (a value returned from a call to                 */
   /*          SYNC_Open_Remote_Server_Port()), the client connection   */
   /*          will be terminated/closed entirely.                      */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Close_Connection(unsigned int BluetoothStackID, unsigned int SYNCID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int SYNCID);
#endif

   /* The following function is responsible for sending an Disconnect   */
   /* Request to the remote device/entity.  The first parameter to this */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Sync Profile connection being*/
   /* disconnected.  The second parameter to this function is the SYNC  */
   /* ID of the Sync Profile connection to be disconnected (that was    */
   /* returned by a call to SYNC_Open_Remote_Server_Port()).  This      */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* * NOTE * Use of this function is optional and is provided for full*/
   /*          compliance with some OBEX applications.  Calling the     */
   /*          SYNC_Close_Connection() function will achieve the same   */
   /*          results without sending the OBEX disconnect packet       */
   /*          beforehand.  If is also possible to call this function   */
   /*          and then immediately call SYNC_Close_Connection() without*/
   /*          waiting for a confirmation because a Disconnect Request  */
   /*          cannot be failed.  Calling this function by itself and   */
   /*          waiting for a response will cause the underlying         */
   /*          connection to automatically be closed once the response  */
   /*          is received.  This will generate a Close Port Indication.*/
BTPSAPI_DECLARATION int BTPSAPI SYNC_OBEX_Disconnect_Request(unsigned int BluetoothStackID, unsigned int SYNCID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_OBEX_Disconnect_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID);
#endif

   /* The following function is responsible for Aborting ANY currently  */
   /* outstanding Sync Profile Client Request (IrMC or SyncML).  The    */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Stack  */
   /* for which the Sync Profile Client is valid.  The second parameter */
   /* to this function specifies the SYNC ID (returned from a successful*/
   /* call to the SYNC_Open_Remote_Server_Port() function).  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
   /* * NOTE * Because of transmission latencies, it may be possible    */
   /*          that a Sync Profile Client Request that is to be aborted */
   /*          may have completed before the server was able to Abort   */
   /*          the request.  In either case, the caller will be notified*/
   /*          via Sync Profile Callback of the status of the previous  */
   /*          Request.                                                 */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Abort_Request(unsigned int BluetoothStackID, unsigned int SYNCID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Abort_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID);
#endif

   /* The following function generates an IrMC Object Get Request for   */
   /* requesting the specified Object from the specified remote IrMC    */
   /* Sync Server.  This function accepts as its input parameters the   */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* this IrMC Sync Client.  The second parameter specifies the SYNC ID*/
   /* (returned from a successful call to the                           */
   /* SYNC_Connect_Remote_Server_Port() function).  The final parameter */
   /* points to an ASCII, NULL terminated string containing the Object  */
   /* Name and/or Path that specifies the Object being requested.  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * A successful return code does not mean that the remote   */
   /*          Sync Profile Server successfully processed the command.  */
   /*          The caller needs to check the confirmation result to     */
   /*          determine if the remote Sync Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Get_Request(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Get_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName);
#endif

   /* The following function sends an IrMC Object Get Response to the   */
   /* specified remote IrMC Sync Client.  This is used for responding to*/
   /* an IrMC Object Get Indication.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* this IrMC Sync Server.  The second parameter specifies the SYNC ID*/
   /* of the IrMC Sync Server responding to the request.  The third     */
   /* parameter is the OBEX response code to include in the response.   */
   /* The fourth parameter points to an optional ASCII, NULL terminated */
   /* string representing the ObjectName to include in the response (if */
   /* set, an appropriate Name header is included, otherwise set to NULL*/
   /* and no header is included).  The fifth parameter is an optional   */
   /* ObjectLength (if non-zero, a Length header is added to the        */
   /* response) that specifies the total data size of the Object being  */
   /* sent in the response.  The sixth parameter is the BufferSize for  */
   /* the data included in the specified Buffer.  The seventh parameter */
   /* is a pointer to the Buffer containing the object data to be       */
   /* included in this response packet.  The final parameter is a       */
   /* pointer to variable which will be written with the actual amount  */
   /* of data that was able to be included in the packet.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * Including a Buffer pointer and setting BufferSize > 0    */
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          Setting the Final parameter to true causes an End-of-Body*/
   /*          header to be used instead of a Body header.  The         */
   /*          exception is that if the stack cannot include all the    */
   /*          requested object data (BufferSize) in the current packet,*/
   /*          the Final flag will be forced to FALSE, and AmountWritten*/
   /*          will reflect that not all data was sent.                 */
   /* * NOTE * If AmountWritten returns an amount smaller than the      */
   /*          specified BufferSize, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted BufferSize and Buffer pointer to account for the*/
   /*          data that was successfully sent.  The Final flag is      */
   /*          automatically cleared when the entire buffer is NOT sent,*/
   /*          and the caller should specify the Final flag anytime it  */
   /*          has passed all the remaining data in Buffer/BufferSize.  */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Get_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Get_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);
#endif

   /* The following function sends an IrMC Object Put Request to the    */
   /* specified remote IrMC Sync Server.  The first parameter is the    */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* the IrMC Sync Client sending the request.  The second parameter   */
   /* specifies the SYNC ID for the Client (returned from a successful  */
   /* call to the SYNC_Open_Remote_Object_Server_Port() function).  The */
   /* third parameter point to an ASCII, NULL terminated string         */
   /* containing the Object Name and Path of the object to be Put.  The */
   /* fourth parameter is the total size of object to be Put, regardless*/
   /* of the amount of data included in this particular Request packet  */
   /* (i.e.  BufferSize).  The fifth parameter is the size of the data  */
   /* to be sent in this call, from the location specified by 'Buffer'  */
   /* pointer.  The sixth parameter is a pointer to the Buffer that     */
   /* contains the object data for this call.  The seventh parameter is */
   /* a pointer to an optional Max Change Counter application parameter */
   /* to include with this packet (to include a MaxChangeCounter value  */
   /* set this pointer to a DWord_t variable containing the value,      */
   /* otherwise set to NULL).  The eighth parameter indicates if the    */
   /* caller intends this to be the final Buffer of data to include for */
   /* this PUT request.  The final parameter is a pointer to a variable */
   /* that upon return will contain the amount of data from the Buffer  */
   /* that was successfully written in the packet.  The AmountWritten   */
   /* parameter should always be checked to determine if the PUT        */
   /* operation was able to include any or all of the data.  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * Including a Buffer pointer and setting BufferSize > 0    */
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          Setting the Final parameter to true causes an End-of-Body*/
   /*          header to be used instead of a Body header.  The         */
   /*          exception is that if the stack cannot include all the    */
   /*          requested object data (BufferSize) in the current packet,*/
   /*          the Final flag will be forced to False, and AmountWritten*/
   /*          will reflect that not all data was sent.                 */
   /* * NOTE * If AmountWritten returns an amount smaller than the      */
   /*          specified BufferSize, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted BufferSize and Buffer pointer to account for the*/
   /*          data that was successfully sent.  The Final flag is      */
   /*          automatically cleared when the entire buffer is NOT sent,*/
   /*          and the caller should specify the Final flag anytime it  */
   /*          has passed all the remaining data in Buffer/BufferSize.  */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Put_Request(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, DWord_t *MaxChangeCounter, Boolean_t Final, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Put_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, DWord_t *MaxChangeCounter, Boolean_t Final, unsigned int *AmountWritten);
#endif

   /* The following function sends an IrMC Object Put Response to the   */
   /* specified remote IrMC Sync Client.  This response should be used  */
   /* for responding to an IrMC Object Put Indication.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Stack that is*/
   /* associated with this IrMC Sync Server.  The second parameter      */
   /* specifies the SYNC ID returned from a previous call to            */
   /* SYNC_Open_Server_Port().  The third parameter is the OBEX response*/
   /* code to include in the response.  The fourth parameter points to  */
   /* an ASCII, NULL terminated string representing the LUID to be      */
   /* included in the application parameter header of the packet (to    */
   /* exclude the LUID header, set to NULL).  The fifth parameter is a  */
   /* pointer to a structure representing the Sync Anchor information to*/
   /* include in the packet (Timestamp or Change Counter, to exclude    */
   /* this information, set to NULL).  This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Put_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *LUID, IrMC_Anchor_t *SyncAnchor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Put_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *LUID, IrMC_Anchor_t *SyncAnchor);
#endif

   /* The following function sends an IrMC Object Delete Request to     */
   /* the specified remote IrMC Sync Server.  The first parameter       */
   /* is the Bluetooth Stack ID of the Bluetooth Stack that is          */
   /* associated with this IrMC Sync Client.  The second parameter      */
   /* specifies the SYNC ID (returned from a successful call to the     */
   /* SYNC_Open_Remote_Object_Server_Port() function).  The third       */
   /* parameter is a pointer to an ASCII, NULL terminated string        */
   /* representing the Object Name and Path of the object being deleted.*/
   /* The fourth parameter is a pointer to an optional Max Change       */
   /* Counter application parameter to include with this packet (to     */
   /* include a MaxChangeCounter value set this pointer to a DWord_t    */
   /* variable containing the value, otherwise set to NULL).  The final */
   /* parameter is a Boolean_t representing the desired state of the    */
   /* Hard Delete Flag - If TRUE, the Hard Delete flag will be included */
   /* in the packet.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Delete_Request(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName, DWord_t *MaxChangeCounter, Boolean_t HardDelete);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Delete_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, char *ObjectName, DWord_t *MaxChangeCounter, Boolean_t HardDelete);
#endif

   /* The following function sends an IrMC Object Delete Response to the*/
   /* specified remote IrMC Sync Client.  This response should be used  */
   /* for responding to an IrMC Object Delete Indication.  The first    */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Stack that is*/
   /* associated with the local IrMC Sync Server.  The second parameter */
   /* specifies the SYNC ID returned from a previous call to            */
   /* SYNC_Open_Server_Port().  The third parameter is the OBEX response*/
   /* code to include in the response.  The fourth parameter is a       */
   /* pointer to an ASCII, NULL terminated string representing the LUID */
   /* to be included in the application parameters portion of the packet*/
   /* (to exclude this header, set to NULL).  The fifth parameter is a  */
   /* pointer to a structure representing the Sync Anchor information to*/
   /* include in the packet (Timestamp or Change Counter, to exclude    */
   /* this header, set to NULL).  This function returns zero if         */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Object_Delete_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *LUID, IrMC_Anchor_t *SyncAnchor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Object_Delete_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *LUID, IrMC_Anchor_t *SyncAnchor);
#endif

   /* The following function sends an IrMC Special Object Get Request to*/
   /* the specified remote IrMC Sync Server.  This function handles     */
   /* additional formulation required for creating Special Object       */
   /* formats specified in the IrMC Sync specification.  The first      */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Stack that is*/
   /* associated with this IrMC Sync Client.  The second parameter      */
   /* specifies the SYNC ID (returned from a successful call to the     */
   /* SYNC_Open_Remote_Object_Server_Port() function).  The third       */
   /* parameter is the Object Store Type to use for this Special Object */
   /* Get (the store value is only required for some object types and   */
   /* may be ignored).  The fourth paramter is the Special Object Type  */
   /* for this Get Request.  The final parameter is a pointer to the    */
   /* SyncAnchor structure representing the desired Sync anchor for use */
   /* with certain object types (Because this is not required for all   */
   /* object types it can be set to NULL when not required).  This      */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * Only the "sotChangeLog" Special Object Type requires the */
   /*          SyncAnchor structure.  Other object types should set this*/
   /*          parameter to NULL.  The ObjectStoreType parameter is     */
   /*          required for the following Special Object Types (and     */
   /*          ignored for all others): sotChangeCounter, sotInfoLog,   */
   /*          sotChangeLog.                                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Special_Object_Get_Request(unsigned int BluetoothStackID, unsigned int SYNCID, IrMC_Object_Store_Type_t ObjectStoreType, SYNC_Special_Object_Type_t ObjectType, IrMC_Anchor_t *SyncAnchor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Special_Object_Get_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, IrMC_Object_Store_Type_t ObjectStoreType, SYNC_Special_Object_Type_t ObjectType, IrMC_Anchor_t *SyncAnchor);
#endif

   /* The following function sends an IrMC Special Object Get Response  */
   /* to the specified remote IrMC Sync Client.  This response should be*/
   /* used for responding to an IrMC Special Object Get Indication.  The*/
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Stack  */
   /* that is associated with the local IrMC Sync Server.  The second   */
   /* parameter specifies the SYNC ID (returned from a previous call to */
   /* SYNC_Open_Server_Port()).  The third parameter is the OBEX        */
   /* response code to include in the response.  The fourth parameter   */
   /* points to an optional ASCII, NULL terminated string containing the*/
   /* ObjectName to include in the response (if set, an appropriate Name*/
   /* header is included, otherwise set to NULL).  The fifth parameter  */
   /* is an optional ObjectLength (if non-zero, a Length header is added*/
   /* to the response) that specifies the total data size of the Object */
   /* being sent in the response.  The sixth parameter is the BufferSize*/
   /* for the data included in the specified Buffer.  The seventh       */
   /* parameter is a pointer to the Buffer containing the object data to*/
   /* be included in this response packet.  The final parameter is a    */
   /* pointer to variable which will be written with the actual amount  */
   /* of data that was able to be included in the packet.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * Including a Buffer pointer and setting BufferSize > 0    */
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          Setting the Final parameter to true causes an End-of-Body*/
   /*          header to be used instead of a Body header.  The         */
   /*          exception is that if the stack cannot include all the    */
   /*          requested object data (BufferSize) in the current packet,*/
   /*          the Final flag will be forced to False, and AmountWritten*/
   /*          will reflect that not all data was sent.                 */
   /* * NOTE * If AmountWritten returns an amount smaller than the      */
   /*          specified BufferSize, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted BufferSize and Buffer pointer to account for the*/
   /*          data that was successfully sent.  The Final flag is      */
   /*          automatically cleared when the entire buffer is NOT sent,*/
   /*          and the caller should specify the Final flag anytime it  */
   /*          has passed all the remaining data in Buffer/BufferSize.  */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Special_Object_Get_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Special_Object_Get_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, char *ObjectName, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);
#endif

   /* The following function sends an IrMC Real Time Clock Put Request  */
   /* to the specified remote IrMC Sync Server.  The first parameter    */
   /* is the Bluetooth Stack ID of the Bluetooth Stack that is          */
   /* associated with the local IrMC Sync Client.  The second parameter */
   /* specifies the SYNC ID (returned from a successful call to the     */
   /* SYNC_Open_Remote_Object_Server_Port() function).  The final       */
   /* parameter is a pointer to a TimeDate structure that should contain*/
   /* the RTC clock value to send (required).  This function returns    */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_RTC_Put_Request(unsigned int BluetoothStackID, unsigned int SYNCID, IrMC_TimeDate_t *TimeDate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_RTC_Put_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, IrMC_TimeDate_t *TimeDate);
#endif

   /* The following function sends an IrMC Real Time Clock Put Response */
   /* to the specified remote IrMC Sync Client.  This response should be*/
   /* used for responding to an IrMC RTC Put Indication.  The first     */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Stack that is*/
   /* associated with the local IrMC Sync Server.  The second parameter */
   /* specifies the SYNC ID (returned from a previous call to           */
   /* SYNC_Open_Server_Port()).  The third parameter is the OBEX        */
   /* response code to include in the response.  This function returns  */
   /* zero if successful or a negative return error code if there was an*/
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_RTC_Put_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_RTC_Put_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);
#endif

   /* The following function sends an IrMC Sync/Push Command to the     */
   /* specified remote IrMC Sync Command Server .  The first parameter  */
   /* is the Bluetooth Stack ID of the Bluetooth Stack that is          */
   /* associated with the local IrMC Sync Command Client.  The second   */
   /* parameter specifies the SYNC ID (returned from a successful call  */
   /* to the SYNC_Open_Remote_Object_Server_Port() function).  The final*/
   /* parameter is a pointer to an ASCII, NULL terminated string to     */
   /* place in the payload of the Sync Command, specifying the desired  */
   /* Sync action (required).  For more information see the IrMC        */
   /* Specification (Section 5.8, p43).  This function returns zero if  */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * This command request is sent to a special server type - a*/
   /*          Sync Command Server (stIrMC_SyncCommand).  Typically an  */
   /*          IrMC Client will create a Sync Command Server to receive */
   /*          commands from IrMC Server devices, a standard IrMC Server*/
   /*          will not accept this command and will return an error.   */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Sync_Command_Request(unsigned int BluetoothStackID, unsigned int SYNCID, char *SyncCommand);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Sync_Command_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, char *SyncCommand);
#endif

   /* The following function sends an IrMC Sync Command Response to the */
   /* specified remote IrMC Sync Command Client.  This response should  */
   /* be used for responding to an IrMC Sync Command Indication.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Stack  */
   /* that is associated with the local IrMC Sync Command Server.  The  */
   /* second parameter specifies the SYNC ID (returned from a previous  */
   /* call to the SYNC_Open_Server_Port() function).  The third         */
   /* parameter is the OBEX response code to include in the response.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI SYNC_IrMC_Sync_Command_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_IrMC_Sync_Command_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);
#endif

   /* The following function sends a SyncML Object Get Request to the   */
   /* specified remote SyncML Server.  The first parameter is the       */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* the local SyncML Client.  The second parameter specifies the SYNC */
   /* ID (returned from a successful call to the                        */
   /* SYNC_Open_Remote_Server_Port() function).  The final parameter is */
   /* the MIME Type to use for this Request (XML or WBXML).  This       */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          SyncML Server successfully processed the command.  The   */
   /*          caller needs to check the confirmation result to         */
   /*          determine if the remote Sync Profile Server successfully */
   /*          executed the Request.                                    */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_SyncML_Object_Get_Request(unsigned int BluetoothStackID, unsigned int SYNCID, SYNC_SyncML_MIME_Type_t MIMEType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_SyncML_Object_Get_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, SYNC_SyncML_MIME_Type_t MIMEType);
#endif

   /* The following function sends a SyncML Object Get Response to the  */
   /* specified remote SyncML Client.  The first parameter is the       */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* this SyncML Server.  The second parameter specifies the SYNC ID   */
   /* (returned from a successful call to the SYNC_Open_Server_Port()   */
   /* function).  The third parameter is the OBEX ResponseCode to return*/
   /* with this response.  The fourth parameter is the ObjectLength that*/
   /* specifies the total data size of the Object being sent in the     */
   /* response.  The fifth parameter is the BufferSize for the data     */
   /* included in the specified Buffer.  The sixth parameter is a       */
   /* pointer to the Buffer containing the object data to be included in*/
   /* this response packet.  The final parameter is a pointer to        */
   /* variable which will be written with the actual amount of data that*/
   /* was able to be included in the packet.  This function returns zero*/
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * Including a Buffer pointer and setting BufferSize > 0    */
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          Setting the Final parameter to true causes an End-of-Body*/
   /*          header to be used instead of a Body header.  The         */
   /*          exception is that if the stack cannot include all the    */
   /*          object data (BufferSize) in the current packet, the Final*/
   /*          flag will be forced to False, and AmountWritten will     */
   /*          reflect that not all data was sent.                      */
   /* * NOTE * If AmountWritten returns an amount smaller than the      */
   /*          specified BufferSize, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted BufferSize and Buffer pointer to account for the*/
   /*          data that was successfully sent.                         */
BTPSAPI_DECLARATION int BTPSAPI SYNC_SyncML_Object_Get_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_SyncML_Object_Get_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, unsigned int *AmountWritten);
#endif

   /* The following function sends an SyncML Object Put Request         */
   /* to the specified remote SyncML Server.  The first parameter       */
   /* is the Bluetooth Stack ID of the Bluetooth Stack that is          */
   /* associated with the local SyncML Client.  The second parameter    */
   /* specifies the SYNC ID (returned from a successful call to the     */
   /* SYNC_Open_Remote_Object_Server_Port() function).  The third       */
   /* parameter is the MIME Type to use for this Request (XML or        */
   /* WBXML).  The fourth parameter is the ObjectLength that specifies  */
   /* the total data size of the Object being sent in the request.  The */
   /* fifth parameter is the BufferSize for the data included in the    */
   /* specified Buffer.  The sixth parameter is a pointer to the Buffer */
   /* containing the object data to be included in this response packet.*/
   /* The seventh parameter is a Boolean_t that determines if this is   */
   /* the last chunk of object data to send.  The final parameter is a  */
   /* pointer to variable which will be written with the actual amount  */
   /* of data that was able to be included in the packet.  This function*/
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
   /* * NOTE * Including a Buffer pointer and setting BufferSize > 0    */
   /*          will cause a Body or End-of-Body header to be added to   */
   /*          the packet, either on the first or subsequent packets.   */
   /*          Setting the Final parameter to true causes an End-of-Body*/
   /*          header to be used instead of a Body header.  The         */
   /*          exception is that if the stack cannot include all the    */
   /*          object data (BufferSize) in the current packet, the Final*/
   /*          flag will be forced to False, and AmountWritten will     */
   /*          reflect that not all data was sent.                      */
   /* * NOTE * If AmountWritten returns an amount smaller than the      */
   /*          specified BufferSize, not all the data was able to be    */
   /*          sent.  This function should be called again with an      */
   /*          adjusted BufferSize and Buffer pointer to account for the*/
   /*          data that was successfully sent.  The Final flag is      */
   /*          automatically cleared when the entire buffer is NOT sent,*/
   /*          and the caller should specify the Final flag anytime it  */
   /*          has passed all the remaining data in Buffer/BufferSize.  */
   /* * NOTE * Issuing this command successfully does not mean that the */
   /*          remote Sync Profile Server successfully carried out the  */
   /*          command.  The caller needs to check the confirmation     */
   /*          result to determine if the remote Sync Profile Server    */
   /*          successfully executed the Request.                       */
   /* * NOTE * There can only be one outstanding Sync Profile Request   */
   /*          active at any one time.  Because of this, another Sync   */
   /*          Profile Request cannot be issued until either the current*/
   /*          request is Aborted (by calling the SYNC_Abort_Request()  */
   /*          function) or the current Request is completed (this is   */
   /*          signified by receiving a Confirmation Event in the Sync  */
   /*          Profile Event Callback that was registered when the Sync */
   /*          Profile Port was opened).                                */
BTPSAPI_DECLARATION int BTPSAPI SYNC_SyncML_Object_Put_Request(unsigned int BluetoothStackID, unsigned int SYNCID, SYNC_SyncML_MIME_Type_t MIMEType, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_SyncML_Object_Put_Request_t)(unsigned int BluetoothStackID, unsigned int SYNCID, SYNC_SyncML_MIME_Type_t MIMEType, DWord_t ObjectLength, unsigned int BufferSize, Byte_t *Buffer, Boolean_t Final, unsigned int *AmountWritten);
#endif

   /* The following function sends a SyncML Object Put Response to the  */
   /* specified local SyncML Client.  The first parameter is the        */
   /* Bluetooth Stack ID of the Bluetooth Stack that is associated with */
   /* the local SyncML Server.  The second parameter specifies the SYNC */
   /* ID (returned from a successful call to the SYNC_Open_Server_Port()*/
   /* function).  The final parameter is the OBEX response code to      */
   /* return with the response.  This function returns zero if          */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI SYNC_SyncML_Object_Put_Response(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_SyncML_Object_Put_Response_t)(unsigned int BluetoothStackID, unsigned int SYNCID, Byte_t ResponseCode);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current Sync Profile Server Mode.  The first parameter  */
   /* is the Bluetooth Stack ID of the Bluetooth Stack of the local Sync*/
   /* Profile Server.  The second parameter is the SYNC ID that was     */
   /* returned from the SYNC_Open_Server_Port() function.  The final    */
   /* parameter is a pointer to a variable which will receive the       */
   /* current Server Mode Mask.  This function returns zero if          */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int SYNCID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int SYNCID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current Sync Profile Server Mode.  The first parameter */
   /* is the Bluetooth Stack ID of the Bluetooth Stack of the local Sync*/
   /* Profile Server.  The second parameter is the SYNC ID that was     */
   /* returned from the SYNC_Open_Server_Port() function.  The final    */
   /* parameter is the new Server Mode Mask to use.  This function      */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI SYNC_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int SYNCID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_SYNC_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int SYNCID, unsigned long ServerModeMask);
#endif

   /* The following function provides a standardized method of parsing  */
   /* an Object Name string into the various component elements         */
   /* represented in the IrMC_Operation_t structure.  The first         */
   /* parameter is a pointer to an ASCII, Null-terminated string        */
   /* containing the Name header to be parsed.  The second parameter is */
   /* a pointer to the IrMC Operation structure to populate with the    */
   /* results of parsing.  The last parameter is a Boolean_t flag that  */
   /* determines if the file extension should be removed from the parsed*/
   /* LUID stored in the Operation structure.  The return value for this*/
   /* function is a Boolean_t that indicates if parsing was successful. */
BTPSAPI_DECLARATION Boolean_t BTPSAPI SYNC_Parse_Object_Name(char *ObjectName, IrMC_Operation_t *Operation, Boolean_t RemoveExtension);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_SYNC_Parse_Object_Name_t)(char *ObjectName, IrMC_Operation_t *Operation, Boolean_t RemoveExtension);
#endif

#endif
