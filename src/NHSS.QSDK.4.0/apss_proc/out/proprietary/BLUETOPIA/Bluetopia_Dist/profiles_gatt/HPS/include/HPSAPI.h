/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/****< hpsapi.h >**************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HPSAPI - Qualcomm Technologies Bluetooth HTTP Proxy Service (GATT based)  */
/*           API Type Definitions, Constants, and Prototypes.                 */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/17/16  R. McCord      Initial Creation.                               */
/******************************************************************************/
#ifndef __HPSAPIH__
#define __HPSAPIH__

#include "SS1BTPS.h"       /* Bluetooth Stack API Prototypes/Constants.       */
#include "SS1BTGAT.h"      /* Bluetooth Stack GATT API Prototypes/Constants.  */
#include "HPSTypes.h"      /* Indoor Positioning Service Types/Constants.     */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define HPS_ERROR_INVALID_PARAMETER                      (-1000)
#define HPS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define HPS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define HPS_ERROR_INSUFFICIENT_BUFFER_SPACE              (-1003)
#define HPS_ERROR_SERVICE_ALREADY_REGISTERED             (-1004)
#define HPS_ERROR_INVALID_INSTANCE_ID                    (-1005)
#define HPS_ERROR_MALFORMATTED_DATA                      (-1006)
#define HPS_ERROR_INVALID_CHARACTERISTIC_TYPE            (-1007)

   /* HPS Server API Structures, Enums, and Constants.                  */

   /* The following structure defines the HPS Characteristic data for   */
   /* the URI, HTTP Headers, and HTTP Entity Body characteristics.      */
   /* * NOTE * We will include room for the NULL terminator if it is    */
   /*          present.                                                 */
typedef struct _tagHPS_Characteristic_Data_t
{
   Word_t  Buffer_Length;
   Byte_t  Buffer[BTPS_CONFIGURATION_HPS_MAXIMUM_SUPPORTED_STRING_LENGTH+1];
} HPS_Characteristic_Data_t;

   /* The following structure contains the information that will need to*/
   /* be cached by an HPS Server for each HPS Client that is connected. */
   /* * NOTE * The HPS_HTTP_Status_Code_Configuration field may be set  */
   /*          to HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_DISABLED or   */
   /*          HPS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE.   */
typedef struct _tagHPS_Server_Information_t
{
   HPS_Characteristic_Data_t  URI_Data;
   HPS_Characteristic_Data_t  HTTP_Headers_Data;
   HPS_Characteristic_Data_t  HTTP_Entity_Body_Data;

   Word_t                     HPS_HTTP_Status_Code_Configuration;
} HPS_Server_Information_t;

#define HPS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(HPS_Server_Information_t))

   /* The following structure defines the HPS Status Code data.         */
   /* * NOTE * The Status_Code field should be set to the Status-Line of*/
   /*          the first line of the HTTP Response message.             */
   /* * NOTE * Some common HTTP response Status-Codes are: 100 -        */
   /*          Continue 101 - Switching protocols 200 - OK 301 - Moved  */
   /*          permanently 302 - Found 303 - See other 305 - Use proxy  */
   /*          307 - Temporary redirect 404 - Resource not found 504 -  */
   /*          HPS Server timeout                                       */
   /* * NOTE * The HPS Server may impose a timeout on any request sent  */
   /*          to the internet.  If this timeout occurs then the        */
   /*          Status-Code 504 should be used.                          */
   /* * NOTE * The Data_Status field is a bitmask made up of values that*/
   /*          have the form HPS_DATA_STATUS_XXX found in HPSTypes.h.   */
   /*          This field is intended to indicate that the HPS Server   */
   /*          has received the HTTP Headers and HTTP Entity Body from  */
   /*          the HTTP response information and stored it, making it   */
   /*          available for the HPS Client to read the values after    */
   /*          receiving the notification.                              */
typedef struct _tagHPS_HTTP_Status_Code_Data_t
{
   Word_t  Status_Code;
   Byte_t  Data_Status;
} HPS_HTTP_Status_Code_Data_t;

#define HPS_HTTP_STATUS_CODE_DATA_SIZE                   (sizeof(HPS_HTTP_Status_Code_Data_t))

   /* The following enumeration defines the HPS Characteristic that has */
   /* been read/written for the                                         */
   /* etHPS_Server_Read_HPS_Characteristic_Request and                  */
   /* etHPS_Server_Write_HPS_Characteristic_Request events.             */
typedef enum
{
   hctURI,
   hctHTTP_Headers,
   hctHTTP_Entity_Body
} HPS_Characteristic_Type_t;

   /* The following enumeration defines the possible values that may be */
   /* set for the HTTP Control Point request type.                      */
   /* * NOTE * The httpCancelRequest may only be used if an HTTP request*/
   /*          is currently oustanding.                                 */
typedef enum
{
   httpGetRequest           = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_GET_REQUEST,
   httpHeadRequest          = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_HEAD_REQUEST,
   httpPostRequest          = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_POST_REQUEST,
   httpPutRequest           = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_PUT_REQUEST,
   httpDeleteRequest        = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_DELETE_REQUEST,
   httpSecureGetRequest     = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_GET_REQUEST,
   httpSecureHeadRequest    = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_HEAD_REQUEST,
   httpSecurePostRequest    = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_POST_REQUEST,
   httpSecurePutRequest     = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_PUT_REQUEST,
   httpSecureDeleteRequest  = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTPS_DELETE_REQUEST,
   httpCancelRequest        = HPS_HTTP_CONTROL_POINT_OP_CODE_HTTP_CANCEL_REQUEST
} HPS_HTTP_Control_Point_Request_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* HPS Service. These are used to determine the type of each event   */
   /* generated and to ensure the proper union element is accessed for  */
   /* the HPS_Event_Data_t structure.                                   */
typedef enum
{
   etHPS_Server_Read_Characteristic_Request,
   etHPS_Server_Write_Characteristic_Request,
   etHPS_Server_Prepare_Characteristic_Request,
   etHPS_Server_Write_HTTP_Control_Point_Request,
   etHPS_Server_Read_HTTPS_Security_Request,
   etHPS_Server_Read_HTTP_Status_Code_CCCD_Request,
   etHPS_Server_Write_HTTP_Status_Code_CCCD_Request
} HPS_Event_Type_t;

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to read one of   */
   /* the following HPS Characteristics: URI, HTTP Headers, or HTTP     */
   /* Entity Body.  The InstanceID field is the identifier for the      */
   /* instance of HPS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the HPS Client and */
   /* HPS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the reqest.  The Type field indicates the HPS Characteristic */
   /* that has been requested to be read.  The final field is the Offset*/
   /* and will be set to zero for a GATT Read Value request, however if */
   /* we receive a GATT Read Long Value request this field will be used */
   /* to indicate the starting offset we should read the HPS            */
   /* Characteristic.  The GATT Read Long request may be issued by an   */
   /* HPS Client if a GATT Read response has been previously sent and   */
   /* the HPS Characteristc, included in the response, has a length that*/
   /* is greater than can fit in the GATT Read response (ATT_MTU-3).    */
   /* This way we can send the remaining data for the HPS Characteristic*/
   /* starting at the specified Offset.                                 */
   /* ** NOTE ** This event SHOULD NOT be received while an HTTP request*/
   /*            is outstanding since the HPS Client should not read the*/
   /*            URI, HTTP Headers, or the HTTP Entity Body while an    */
   /*            HTTP request is being executed.  Since this behaviour  */
   /*            is undefined, it is recommended that if an HTTP request*/
   /*            is currently outstanding that the request to read an   */
   /*            HPS Characteristic identified by the Type field, should*/
   /*            be rejected with the ErrorCode parameter of the        */
   /*            HPS_Read_Characteristic_Request_Response() function set*/
   /*            to HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.       */
   /* ** NOTE ** This event MAY be received after an HTTP request has   */
   /*            been completed (The HTTP Status Code has been notified */
   /*            to the HPS Client).  The HPS Client may wish to read   */
   /*            the HTTP Headers and HTTP Entity Body that will contain*/
   /*            information from the HTTP response.                    */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Read_Characteristic_Request_Response() function  */
   /*          to send the response to the request.                     */
typedef struct _tagHPS_Server_Read_Characteristic_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   HPS_Characteristic_Type_t  Type;
   Word_t                     Offset;
} HPS_Server_Read_Characteristic_Request_Data_t;

#define HPS_SERVER_READ_CHARACTERISTIC_REQUEST_DATA_SIZE  (sizeof(HPS_Server_Read_Characteristic_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to write one of  */
   /* the following HPS Characteristics: URI, HTTP Headers, or HTTP     */
   /* Entity Body.  The InstanceID field is the identifier for the      */
   /* instance of HPS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the HPS Client and */
   /* HPS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the reqest.  The Type field indicates the HPS Characteristic */
   /* that has been requested to be written.  The BufferLength field    */
   /* contains the length of Buffer field.  The Buffer field is a       */
   /* pointer the value that has been written by an HPS Client for the  */
   /* HPS Characteristic.                                               */
   /* ** NOTE ** The URI, HTTP Headers, and HTTP Entity Body MUST be    */
   /*            written before an HTTP request may be dispatched by the*/
   /*            HPS Server.  An HPS Client MUST write a null (zero     */
   /*            length) value if it does not wish to use a particular  */
   /*            HPS Characteristic for the HTTP request.               */
   /* ** NOTE ** This event SHOULD NOT be received while an HTTP request*/
   /*            is outstanding since the HPS Client should not write   */
   /*            the URI, HTTP Headers, or the HTTP Entity Body while an*/
   /*            HTTP request is being executed.  Especially since the  */
   /*            HPS Server will update the HTTP Headers and HTTP Entity*/
   /*            Body with information received in the HTTP response.  A*/
   /*            subsequent write by an HPS Client for the HTTP Headers */
   /*            or HTTP Entity Body may overwrite the response         */
   /*            information that has been set by the HPS Server.  Since*/
   /*            this behaviour is undefined, it is recommended that if */
   /*            an HTTP request is currently outstanding that the      */
   /*            request to write an HPS Characteristic identified by   */
   /*            the Type field, should be rejected with the ErrorCode  */
   /*            parameter of the                                       */
   /*            HPS_Write_Characteristic_Request_Response() function   */
   /*            set to HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.   */
   /* ** NOTE ** The HTTP Headers and HTTP Entity Body MUST be          */
   /*            re-configured by an HPS Client after an HTTP request   */
   /*            has completed, and before another HTTP request may be  */
   /*            dispatched, since the HPS Server will update these     */
   /*            characteristics with information from the HTTP         */
   /*            response.                                              */
   /* * NOTE * The BufferLength field may be zero.  If this is the case,*/
   /*          the Buffer field will be NULL.  This indicates that the  */
   /*          HPS Client has written a null (zero length) value for    */
   /*          this HPS Characteristic.  This means that the HPS Client */
   /*          does not wish to use this particular HPS Characteristic  */
   /*          in an HTTP request.                                      */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Write_Characteristic_Request_Response() function */
   /*          to send the response to the request.                     */
typedef struct _tagHPS_Server_Write_Characteristic_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   HPS_Characteristic_Type_t  Type;
   Word_t                     BufferLength;
   Byte_t                    *Buffer;
} HPS_Server_Write_Characteristic_Request_Data_t;

#define HPS_SERVER_WRITE_CHARACTERISTIC_REQUEST_DATA_SIZE  (sizeof(HPS_Server_Write_Characteristic_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to prepare one of*/
   /* the following HPS Characteristics: URI, HTTP Headers, or HTTP     */
   /* Entity Body.  This event simply indicates that the HPS Client is  */
   /* using the GATT Write Long procedure (GATT Prepare Write requests) */
   /* to prepare an HPS Characteristic since its length is greater than */
   /* can fit in a GATT Write Request (ATT_MTU-3).  The InstanceID field*/
   /* is the identifier for the instance of HPS that received the       */
   /* request.  The ConnectionID is the identifier for the GATT         */
   /* connection between the HPS Client and HPS Server.  The            */
   /* ConnectionType field specifies the GATT connection type or        */
   /* transport being used for the request.  The TransactionID field    */
   /* identifies the GATT transaction for the request.  The RemoteDevice*/
   /* is the BD_ADDR of the HPS Client that made the request.  The Type */
   /* field identifies the HPS Characteristic that is being prepared.   */
   /* ** NOTE ** This event is primarily provided to reject a GATT      */
   /*            Prepare Write request for optional security reasons    */
   /*            such as the HPS Client has insufficient authentication,*/
   /*            authorization, or encryption.  This is REQUIRED if the */
   /*            HPS Server needs additional security.  Therefore we    */
   /*            will not pass the prepared data up to the application  */
   /*            until the the GATT Execute Write request has been      */
   /*            received by the HPS Server, and the prepared writes are*/
   /*            not cancelled.  If the prepared data is written the    */
   /*            etHPS_Server_Write_Characteristic_Request event will be*/
   /*            dispatched to the application.  Otherwise the prepared */
   /*            data will be cleared.                                  */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Prepare_Characteristic_Request_Response()        */
   /*          function to send the response to the outstanding request.*/
typedef struct _tagHPS_Server_Prepare_Characteristic_Request_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   unsigned int               TransactionID;
   BD_ADDR_t                  RemoteDevice;
   HPS_Characteristic_Type_t  Type;
} HPS_Server_Prepare_Characteristic_Request_Data_t;

#define HPS_SERVER_PREPARE_CHARACTERISTIC_REQUEST_DATA_SIZE (sizeof(HPS_Server_Prepare_Characteristic_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to write the HTTP*/
   /* Control Point Characteristic.  The InstanceID field is the        */
   /* identifier for the instance of HPS that received the request.  The*/
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* HPS Client and HPS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the request.  The final field is an enumeration for the HTTP */
   /* Control Point request that has been received.                     */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_HTTP_Control_Point_Request_Response() function to*/
   /*          send the response for the outstanding request.  This     */
   /*          response does not indicate that the HTTP request has     */
   /*          completed, but that the HTTP request has been            */
   /*          accepted/rejected.                                       */
typedef struct _tagHPS_Server_Write_HTTP_Control_Point_Request_Data_t
{
   unsigned int                           InstanceID;
   unsigned int                           ConnectionID;
   GATT_Connection_Type_t                 ConnectionType;
   unsigned int                           TransactionID;
   BD_ADDR_t                              RemoteDevice;
   HPS_HTTP_Control_Point_Request_Type_t  RequestType;
} HPS_Server_Write_HTTP_Control_Point_Request_Data_t;

#define HPS_SERVER_WRITE_HTTP_CONTROL_POINT_REQUEST_DATA_SIZE  (sizeof(HPS_Server_Write_HTTP_Control_Point_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to read the HTTPS*/
   /* Security Characteristic.  The InstanceID field is the identifier  */
   /* for the instance of HPS that received the request.  The           */
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* HPS Client and HPS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Read_HTTPS_Security_Request_Response() function  */
   /*          to send the response to the outstanding request.         */
typedef struct _tagHPS_Server_Read_HTTPS_Security_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
} HPS_Server_Read_HTTPS_Security_Request_Data_t;

#define HPS_SERVER_READ_HTTPS_SECURITY_REQUEST_DATA_SIZE (sizeof(HPS_Server_Read_HTTPS_Security_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to read the HTTP */
   /* Status Code Characteristic's Client Characteristic Configuration  */
   /* descriptor (CCCD).  The InstanceID field is the identifier for the*/
   /* instance of HPS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the HPS Client and */
   /* HPS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Read_HTTP_Status_Code_CCCD_Request_Response()    */
   /*          function to send the response to the outstanding request.*/
typedef struct _tagHPS_Server_Read_HTTP_Status_Code_CCCD_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
} HPS_Server_Read_HTTP_Status_Code_CCCD_Request_Data_t;

#define HPS_SERVER_READ_HTTP_STATUS_CODE_CCCD_REQUEST_DATA_SIZE  (sizeof(HPS_Server_Read_HTTP_Status_Code_CCCD_Request_Data_t))

   /* The following event data is dispatched to an HPS Server to inform */
   /* the application that an HPS Client has requested to write the HTTP*/
   /* Status Code Characteristic's Client Characteristic Configuration  */
   /* descriptor (CCCD).  The InstanceID field is the identifier for the*/
   /* instance of HPS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the HPS Client and */
   /* HPS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the HPS Client that  */
   /* made the request.  The final field is the ClientConfiguration that*/
   /* contains the value that has been written for the CCCD.            */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the HPS_Write_HTTP_Status_Code_CCCD_Request_Response()   */
   /*          function to send the response to the outstanding request.*/
typedef struct _tagHPS_Server_Write_HTTP_Status_Code_CCCD_Request_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   GATT_Connection_Type_t  ConnectionType;
   unsigned int            TransactionID;
   BD_ADDR_t               RemoteDevice;
   Word_t                  ClientConfiguration;
} HPS_Server_Write_HTTP_Status_Code_CCCD_Request_Data_t;

#define HPS_SERVER_WRITE_HTTP_STATUS_CODE_CCCD_REQUEST_DATA_SIZE  (sizeof(HPS_Server_Write_HTTP_Status_Code_CCCD_Request_Data_t))

   /* The following structure holds all HPS Service Event Data. This    */
   /* structure is received for each event generated. The               */
   /* Event_Data_Type member is used to determine the appropriate union */
   /* member element to access the contained data. The Event_Data_Size  */
   /* member contains the total size of the data contained in this      */
   /* event.                                                            */
typedef struct _tagHPS_Event_Data_t
{
   HPS_Event_Type_t  Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      HPS_Server_Read_Characteristic_Request_Data_t         *HPS_Server_Read_Characteristic_Request_Data;
      HPS_Server_Write_Characteristic_Request_Data_t        *HPS_Server_Write_Characteristic_Request_Data;
      HPS_Server_Prepare_Characteristic_Request_Data_t      *HPS_Server_Prepare_Characteristic_Request_Data;
      HPS_Server_Write_HTTP_Control_Point_Request_Data_t    *HPS_Server_Write_HTTP_Control_Point_Request_Data;
      HPS_Server_Read_HTTPS_Security_Request_Data_t         *HPS_Server_Read_HTTPS_Security_Request_Data;
      HPS_Server_Read_HTTP_Status_Code_CCCD_Request_Data_t  *HPS_Server_Read_HTTP_Status_Code_CCCD_Request_Data;
      HPS_Server_Write_HTTP_Status_Code_CCCD_Request_Data_t *HPS_Server_Write_HTTP_Status_Code_CCCD_Request_Data;
   } Event_Data;
} HPS_Event_Data_t;

#define HPS_EVENT_DATA_SIZE                              (sizeof(HPS_Event_Data_t))

   /* HPS Client API Structures, Enums, and Constants.                  */

   /* The following structure contains the handles that will need to be */
   /* cached by an HPS client in order to only do service discovery     */
   /* once.                                                             */
   /* * NOTE * This information MUST be cached for each HPS Client.     */
typedef struct _tagHPS_Client_Information_t
{
   Word_t  HPS_URI;
   Word_t  HPS_HTTP_Headers;
   Word_t  HPS_HTTP_Entity_Body;
   Word_t  HPS_HTTP_Control_Point;
   Word_t  HPS_HTTP_Status_Code;
   Word_t  HPS_HTTP_Status_Code_CCCD;
   Word_t  HPS_HTTPS_Security;
} HPS_Client_Information_t;

#define HPS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(HPS_Client_Information_t))

   /* The following declared type represents the Prototype Function for */
   /* an HPS Service Event Callback.  This function will be called      */
   /* whenever an HPS Service Event occurs that is associated with the  */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the HPS Event Data that occurred, and the */
   /* HPS Service Event Callback Parameter that was specified when this */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the HPS Service Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be re-entrant).  It needs to be noted      */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another HPS Service Event will*/
   /* not be processed while this function call is outstanding).        */
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by receiving HPS Service Event Packets.*/
   /*          A Deadlock WILL occur because NO HPS Event Callbacks will*/
   /*          be issued while this function is currently outstanding.  */
typedef void (BTPSAPI *HPS_Event_Callback_t)(unsigned int BluetoothStackID, HPS_Event_Data_t *HPS_Event_Data, unsigned long CallbackParameter);

   /* HPS Server API.                                                   */

   /* The following function is responsible for opening an HPS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the HPS Service Flags  */
   /* (HPS_SERVICE_FLAGS_XXX) found in HPSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an HPS  */
   /* Client, for the specified transport.  The third parameter is the  */
   /* Callback function to call when an event occurs on the HPS Server. */
   /* The fourth parameter is a user-defined callback parameter that    */
   /* will be passed to the callback function with each event.  The     */
   /* final parameter is a pointer to store the GATT Service ID of the  */
   /* registered HPS service.  This function returns the positive,      */
   /* non-zero, Instance ID or a negative error code.                   */
   /* * NOTE * Only 1 HPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatched to the            */
   /*          EventCallback function that is specified by the second   */
   /*          parameter to this function.                              */
BTPSAPI_DECLARATION int BTPSAPI HPS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, HPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, HPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening an HPS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the HPS Service Flags  */
   /* (HPS_SERVICE_FLAGS_XXX) found in HPSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an HPS  */
   /* Client, for the specified transport.  The third parameter is the  */
   /* Callback function to call when an event occurs on this Server     */
   /* Port.  The fourth parameter is a user-defined callback parameter  */
   /* that will be passed to the callback function with each event.  The*/
   /* fifth parameter is a pointer to store the GATT Service ID of the  */
   /* registered HPS service.  The final parameter is a pointer, that on*/
   /* input can be used to control the location of the service in the   */
   /* GATT database, and on ouput to store the service handle range.    */
   /* This function returns the positive, non-zero, Instance ID or a    */
   /* negative error code.                                              */
   /* * NOTE * Only 1 HPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatched to the            */
   /*          EventCallback function that is specified by the second   */
   /*          parameter to this function.                              */
BTPSAPI_DECLARATION int BTPSAPI HPS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, HPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, HPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened HPS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* HPS_Initialize_XXX() API's.  This function returns zero if        */
   /* successful or a negative error code.                              */
BTPSAPI_DECLARATION int BTPSAPI HPS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is used to perform a suspend of the        */
   /* Bluetooth stack.  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that Bluetopia is to use to collapse it's state information into. */
   /* This function can be called with BufferSize and Buffer set to 0   */
   /* and NULL, respectively.  In this case this function will return   */
   /* the number of bytes that must be passed to this function in order */
   /* to successfully perform a suspend (or 0 if an error occurred, or  */
   /* this functionality is not supported).  If the BufferSize and      */
   /* Buffer parameters are NOT 0 and NULL, this function will attempt  */
   /* to perform a suspend of the stack.  In this case, this function   */
   /* will return the amount of memory that was used from the provided  */
   /* buffers for the suspend (or zero otherwise).                      */
BTPSAPI_DECLARATION unsigned long BTPSAPI HPS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_HPS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* HPS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to HPS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI HPS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the HPS Service that is          */
   /* registered with a call to the HPS_Initialize_XXX() API's.  This   */
   /* function returns the non-zero number of attributes that are       */
   /* contained in an HPS Server or zero on failure.                    */
   /* * NOTE * This function may be used to determine the attribute     */
   /*          handle range for HPS so that the ServiceHandleRange      */
   /*          parameter of the HPS_Initialize_Service_Handle_Range()   */
   /*          can be configured to register HPS in a specified         */
   /*          attribute handle range in GATT.                          */
   /* * NOTE * Since HPS has a fixed number of mandatory attributes, the*/
   /*          value returned from this function will ALWAYS be the     */
   /*          same.                                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI HPS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_HPS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a request */
   /* from an HPS Client to read the URI, HTTP Headers, or HTTP Entity  */
   /* Body characteristics.  The first parameter is the Bluetooth Stack */
   /* ID of the Bluetooth Device.  The second parameter is the          */
   /* InstanceID returned from a successful call to                     */
   /* HPS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The fifth       */
   /* parameter is an enumeration for the HPS Characteristic that has   */
   /* been requested to be read.  The sixth parameter contains length of*/
   /* the HPS Characterstic that will be sent to the HPS Client.  The   */
   /* final parameter is a pointer to the Buffer for the requested HPS  */
   /* Characteristic, to send to the HPS Client if the request has been */
   /* accepted.  This function returns zero if successful or a negative */
   /* return error code if an error occurs.                             */
   /* ** NOTE ** This function SHOULD NOT be called while an HTTP       */
   /*            request is outstanding since the HPS Client should not */
   /*            read the URI, HTTP Headers, or the HTTP Entity Body    */
   /*            while an HTTP request is being executed.  Since this   */
   /*            behaviour is undefined, it is recommended that if an   */
   /*            HTTP request is currently outstanding and a request has*/
   /*            been dispatched to read the URI, HTTP Headers, or HTTP */
   /*            Entity Body characteristics, it should be rejected with*/
   /*            the ErrorCode parameter set to                         */
   /*            HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.          */
   /* ** NOTE ** If the BufferLength parameter is set to zero this      */
   /*            function will send a zero length response for the HPS  */
   /*            Charactersitic.  The Buffer parameter may be excluded  */
   /*            (NULL).  This simply means that the value has a null   */
   /*            (zero length) value.  This does not mean that the HPS  */
   /*            Client has not written the value since the HPS Client  */
   /*            may write a null (zero length) value for an HPS        */
   /*            Characteristic to indicate that this particular        */
   /*            characteristic should not be used for an HTTP request. */
   /* * NOTE * If this function is called to respond to a GATT Read Long*/
   /*          Value request (The Offset field of the                   */
   /*          HPS_Server_Read_Characteristic_Request_Data_t structure  */
   /*          will be non-zero), then the BufferLength parameter should*/
   /*          be set to the remaining length of the data for the HPS   */
   /*          Characteristic and the Buffer parameter should be set to */
   /*          the location of the HPS Characteristic data indexed by   */
   /*          the specified Offset.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been accepted, then the ErrorCode     */
   /*          parameter MUST be HPS_ERROR_CODE_SUCCESS and the Buffer  */
   /*          parameter is REQUIRED.  The Buffer parameter may be      */
   /*          excluded (NULL) if the request is rejected.  The         */
   /*          BufferLength parameter will simply be ignored if the     */
   /*          request is rejected.                                     */
BTPSAPI_DECLARATION int BTPSAPI HPS_Read_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Read_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type, Word_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for responding to a request */
   /* from an HPS Client to write the URI, HTTP Headers, or HTTP Entity */
   /* Body characteristics.  The first parameter is the Bluetooth Stack */
   /* ID of the Bluetooth Device.  The second parameter is the          */
   /* InstanceID returned from a successful call to                     */
   /* HPS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is an enumeration for the HPS Characteristic that has   */
   /* been requested to be written.  This function returns zero if      */
   /* successful or a negative return error code if an error occurs.    */
   /* ** NOTE ** This function SHOULD NOT be called while an HTTP       */
   /*            request is outstanding since the HPS Client should not */
   /*            write the URI, HTTP Headers, or the HTTP Entity Body   */
   /*            while an HTTP request is being executed.  Especially   */
   /*            since the HPS Server will update the HTTP Headers and  */
   /*            HTTP Entity Body with information received in the HTTP */
   /*            response.  A subsequent write by an HPS Client for the */
   /*            HTTP Headers or HTTP Entity Body may overwrite the     */
   /*            response information that has been set by the HPS      */
   /*            Server.  Since this behaviour is undefined, it is      */
   /*            recommended that if an HTTP request is currently       */
   /*            outstanding and a request has been dispatched to write */
   /*            the URI, HTTP Headers, or HTTP Entity Body             */
   /*            characteristics, it should be rejected with the        */
   /*            ErrorCode parameter set to                             */
   /*            HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI HPS_Write_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Write_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to a request */
   /* from an HPS Client to prepare the URI, HTTP Headers, or HTTP      */
   /* Entity Body characteristics.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* HPS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is an enumeration for the HPS Characteristic that has   */
   /* been requested to be written.  This function returns zero if      */
   /* successful or a negative return error code if an error occurs.    */
   /* ** NOTE ** This function SHOULD NOT be called while an HTTP       */
   /*            request is outstanding since the HPS Client should not */
   /*            prepare/write the URI, HTTP Headers, or the HTTP Entity*/
   /*            Body while an HTTP request is being executed.          */
   /*            Especially since the HPS Server will update the HTTP   */
   /*            Headers and HTTP Entity Body with information received */
   /*            in the HTTP response.  A subsequent write by an HPS    */
   /*            Client for the HTTP Headers or HTTP Entity Body may    */
   /*            overwrite the response information that has been set by*/
   /*            the HPS Server.  Since this behaviour is undefined, it */
   /*            is recommended that if an HTTP request is currently    */
   /*            outstanding and a request has been dispatched to       */
   /*            prepare/write the URI, HTTP Headers, or HTTP Entity    */
   /*            Body characteristics, it should be rejected with the   */
   /*            ErrorCode parameter set to                             */
   /*            HPS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS.          */
   /* ** NOTE ** This event is primarily provided to reject a GATT      */
   /*            Prepare Write request for optional security reasons    */
   /*            such as the HPS Client has insufficient authentication,*/
   /*            authorization, or encryption.  This is REQUIRED if the */
   /*            HPS Server needs additional security.  Therefore we    */
   /*            will not pass the prepared data up to the application  */
   /*            until the the GATT Execute Write request has been      */
   /*            received by the HPS Server, and the prepared writes are*/
   /*            not cancelled.  If the prepared data is written the    */
   /*            etHPS_Server_Write_Characteristic_Request event will be*/
   /*            dispatched to the application.  Otherwise the prepared */
   /*            data will be cleared.                                  */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI HPS_Prepare_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Prepare_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, HPS_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to an HPS    */
   /* HTTP Control Point write request received from an HPS Client.  The*/
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to the HPS_Initialize_XXX() API's.  The third parameter is   */
   /* the GATT Transaction ID of the request.  The fourth parameter is  */
   /* the ErrorCode to indicate the type of response that will be sent. */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* ** NOTE ** This function is primarily provided to allow a way to  */
   /*            reject an HTTP Control Point request when the HTTP     */
   /*            Status Code CCCD has not been configured for           */
   /*            notifications, an outstanding HTTP Control Point       */
   /*            operation is already in progress, the HTTP Server      */
   /*            certificate for an HTTPS request is invalid, or the    */
   /*            HTTP Control Point request has an invalid value.  The  */
   /*            completion of the HTTP Control Point request is        */
   /*            determined by the notification of the HPS HTTP Status  */
   /*            Code via the HPS_Notify_HTTP_Status_Code() function.   */
   /* ** NOTE ** The HPS Server should NEVER dispatch an HTTP request if*/
   /*            the ErrorCode parameter is not HPS_ERROR_CODE_SUCCESS. */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI HPS_HTTP_Control_Point_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_HTTP_Control_Point_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for notifying the HPS HTTP  */
   /* Status Code to an HPS Client for the specified GATT Connection ID.*/
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to the HPS_Initialize_XXX() API's.  The third     */
   /* parameter is the GATT ConnectionID of the connected HPS Client.   */
   /* The final parameter contains a pointer to the HTTP Status Code    */
   /* data to notify to the connected HPS Client.  This function will   */
   /* return a positive non-zero value that represents the actual length*/
   /* of the attribute value that was notified, or a negative return    */
   /* error code if there was an error.                                 */
   /* ** NOTE ** The HPS Server MUST make sure that the HTTP Status Code*/
   /*            CCCD has been configured for notifications for the HPS */
   /*            Client associated with the GATT Connection ID.         */
   /*            Otherwise this function SHOULD NOT be called.          */
   /* ** NOTE ** This function is primarily provided to inform the HPS  */
   /*            Client that the HTTP response has been received by the */
   /*            HPS Server and the HPS Server has set the HTTP Headers */
   /*            and HTTP Entity Body with the required information from*/
   /*            the HTTP response.  This informs the HPS Client that   */
   /*            the HTTP request has been completed.  This does not    */
   /*            mean that it was successful.  The Status_Code field of */
   /*            the HPS_HTTP_Status_Code_Data_t structure will indicate*/
   /*            this.  After receiving the notification, the HPS Client*/
   /*            may read the HTTP response information that has been   */
   /*            previously set by the HPS Server.                      */
BTPSAPI_DECLARATION int BTPSAPI HPS_Notify_HTTP_Status_Code(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, HPS_HTTP_Status_Code_Data_t *HTTPStatusCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Notify_HTTP_Status_Code_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, HPS_HTTP_Status_Code_Data_t *HTTPStatusCode);
#endif

   /* The following function is responsible for responding to an HPS    */
   /* HTTPS Security read request.  The first parameter is the Bluetooth*/
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* HPS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is the CertificateValid and indicates whether the HTTP  */
   /* Server certificate for the URI is valid.  This function returns a */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* ** NOTE ** The requested HTTPS Security MUST be set before an     */
   /*            HTTPS request is dispatched by the HPS Server.  This   */
   /*            means that the HPS Client should be able to read the   */
   /*            value before an HTTP request is dispatched.  This is   */
   /*            because the HTTP Server certificate should be validated*/
   /*            before HTTPS request is dispatched by the HPS Server.  */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request is REJECTED the CertificateValid parameter*/
   /*          will be ignored.                                         */
BTPSAPI_DECLARATION int BTPSAPI HPS_Read_HTTPS_Security_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Boolean_t CertificateValid);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Read_HTTPS_Security_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Boolean_t CertificateValid);
#endif

   /* The following function is responsible for responding to an HPS    */
   /* HTTP Status Code Characteristic's Client Characteristic           */
   /* Configuration descriptor (CCCD) read request.  The first parameter*/
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* HPS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is the ErrorCode to      */
   /* indicate the type of response that will be sent.  The final       */
   /* parameter is the ClientConfiguration and is the current Client    */
   /* Characteristic Configuration value to send to the HPS Client if   */
   /* the request has been accepted.  This function returns a zero if   */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request is rejected the ClientConfiguration       */
   /*          parameter will be IGNORED.                               */
BTPSAPI_DECLARATION int BTPSAPI HPS_Read_HTTP_Status_Code_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Read_HTTP_Status_Code_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to an HPS    */
   /* HTTP Status Code Characteristic's Client Characteristic           */
   /* Configuration descriptor (CCCD) write request.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to HPS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID of the request.  The fourth parameter is the       */
   /* ErrorCode to indicate the type of response that will be sent.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          HPS_ERROR_CODE_XXX from HPSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI HPS_Write_HTTP_Status_Code_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Write_HTTP_Status_Code_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* HPS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote HPS Server interpreting it as the HTTP Status Code. */
   /* The first parameter is the length of the value returned by the    */
   /* remote HPS Server.  The second parameter is a pointer to the data */
   /* returned by the remote HPS Server.  The final parameter is a      */
   /* pointer to store the decoded HTTP Status Code.  This function     */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI HPS_Decode_HTTP_Status_Code(unsigned int ValueLength, Byte_t *Value, HPS_HTTP_Status_Code_Data_t *StatusCodeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HPS_Decode_HTTP_Status_Code_t)(unsigned int ValueLength, Byte_t *Value, HPS_HTTP_Status_Code_Data_t *StatusCodeData);
#endif

#endif
