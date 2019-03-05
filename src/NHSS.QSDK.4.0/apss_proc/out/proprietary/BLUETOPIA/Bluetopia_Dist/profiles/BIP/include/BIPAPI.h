/*****< bipapi.h >*************************************************************/
/*      Copyright 2004 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BIPAPI - Stonestreet One Bluetooth Basic Imaging Profile (BIP)            */
/*           API Type Definitions, Constants, and Prototypes.                 */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/01/04  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __BIPAPIH__
#define __BIPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTBIP_ERROR_INVALID_PARAMETER                             (-1000)
#define BTBIP_ERROR_NOT_INITIALIZED                               (-1001)
#define BTBIP_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTBIP_ERROR_LIBRARY_INITIALIZATION_ERROR                  (-1003)
#define BTBIP_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTBIP_ERROR_REQUEST_ALREADY_OUTSTANDING                   (-1005)
#define BTBIP_ERROR_ACTION_NOT_ALLOWED                            (-1006)

   /* SDP Profile UUID's for the Basic Imaging Profile.                 */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_PROFILE_UUID_16) to the specified UUID_16_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_16_t variable that  */
   /* is to receive the BIP_PROFILE_UUID_16 Constant value.             */
#define SDP_ASSIGN_BIP_PROFILE_UUID_16(_x)                       ASSIGN_SDP_UUID_16((_x), 0x11, 0x1A)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_PROFILE_UUID_32) to the specified UUID_32_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_32_t variable that  */
   /* is to receive the BIP_PROFILE_UUID_32 Constant value.             */
#define SDP_ASSIGN_BIP_PROFILE_UUID_32(_x)                       ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x1A)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_PROFILE_UUID_128) to the specified UUID_128_t variable.  This*/
   /* MACRO accepts one parameter which is the UUID_128_t variable that */
   /* is to receive the BIP_PROFILE_UUID_128 Constant value.            */
#define SDP_ASSIGN_BIP_PROFILE_UUID_128(_x)                      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x1A, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_RESPONDER_UUID_16) to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the BIP_IMAGING_RESPONDER_UUID_16     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_BIP_IMAGING_RESPONDER_UUID_16(_x)             ASSIGN_SDP_UUID_16((_x), 0x11, 0x1B)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_RESPONDER_UUID_32) to the specified UUID_32_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the BIP_IMAGING_RESPONDER_UUID_32     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_BIP_IMAGING_RESPONDER_UUID_32(_x)             ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x1B)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_RESPONDER_UUID_128) to the specified UUID_128_t      */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* BIP_IMAGING_RESPONDER_UUID_128 Constant value.                    */
#define SDP_ASSIGN_BIP_IMAGING_RESPONDER_UUID_128(_x)            ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x1B, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_AUTOMATIC_ARCHIVE_UUID_16) to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the BIP_AUTOMATIC_ARCHIVE_UUID_16     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_BIP_AUTOMATIC_ARCHIVE_UUID_16(_x)             ASSIGN_SDP_UUID_16((_x), 0x11, 0x1C)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_AUTOMATIC_ARCHIVE_UUID_32) to the specified UUID_32_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_32_t*/
   /* variable that is to receive the BIP_AUTOMATIC_ARCHIVE_UUID_32     */
   /* Constant value.                                                   */
#define SDP_ASSIGN_BIP_AUTOMATIC_ARCHIVE_UUID_32(_x)             ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x1C)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BASIC_AUTOMATIC_ARCHIVE_UUID_128) to the specified UUID_128_t    */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the                        */
   /* BASIC_AUTOMATIC_ARCHIVE_UUID_128 Constant value.                  */
#define SDP_ASSIGN_BIP_AUTOMATIC_ARCHIVE_UUID_128(_x)            ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x1C, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_REFERENCED_OBJECTS_UUID_16) to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the                         */
   /* BIP_IMAGING_REFERENCED_OBJECTS_UUID_16 Constant value.            */
#define SDP_ASSIGN_BIP_IMAGING_REFERENCED_OBJECTS_UUID_16(_x)    ASSIGN_SDP_UUID_16((_x), 0x11, 0x1C)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_REFERENCED_OBJECTS_UUID_32) to the specified         */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the                         */
   /* BIP_IMAGING_REFERENCED_OBJECTS_UUID_32 Constant value.            */
#define SDP_ASSIGN_BIP_IMAGING_REFERENCED_OBJECTS_UUID_32(_x)    ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x1C)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Imaging Profile Bluetooth Universally Unique Identifier           */
   /* (BIP_IMAGING_REFERENCED_OBJECTS_UUID_128) to the specified        */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* BIP_IMAGING_REFERENCED_OBJECTS_UUID_128 Constant value.           */
#define SDP_ASSIGN_BIP_IMAGING_REFERENCED_OBJECTS_UUID_128(_x)   ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x1C, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* The following bits represent the possible values that may be      */
   /* passed into the BIP_Open_Image_Responder_Server() functions       */
   /* ImagingServicePortTargetsMask parameter to indicate which Targets */
   /* shall be allowed to connect using the Imaging Service Server Port.*/
#define BIP_IMAGING_SERVICE_SERVER_PORT_TARGET_IMAGE_PUSH_SERVICE_BIT (0x00000001)
#define BIP_IMAGING_SERVICE_SERVER_PORT_TARGET_IMAGE_PULL_SERVICE_BIT (0x00000002)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* Basic Imaging Profile Servers.                                    */
#define BIP_PROFILE_VERSION                                      (0x0100)

   /* The following constants represent the Minimum and Maximum Port    */
   /* Numbers that can be opened (both locally and remotely).  These    */
   /* constants specify the range for the Port Number parameters in the */
   /* Open Functions.                                                   */
#define BIP_PORT_NUMBER_MINIMUM                                  (SPP_PORT_NUMBER_MINIMUM)
#define BIP_PORT_NUMBER_MAXIMUM                                  (SPP_PORT_NUMBER_MAXIMUM)

   /* The following enumerated type defines the possible Targets that   */
   /* may be connected to on a Imaging Responder Servers Imaging Service*/
   /* Server Port.  These values should be used with the                */
   /* BIP_Open_Remote_Imaging_Responder_Server() functions              */
   /* ImagingServiceServerPortTarget parameter.                         */
typedef enum
{
   istImagePushService,
   istImagePullService
} BIP_Imaging_Service_Server_Port_Target_t;

   /* The following define the Flags that are submitted when registering*/
   /* an SDP Record for a Image Responder Server.  The Capability Flags,*/
   /* Supported Feature Flags and Supported Function Flags are used     */
   /* directly to construct the Bit Flags for the SDP record.           */
#define BIP_SUPPORTED_CAPABILITY_GENERIC_IMAGING                 (0x01)
#define BIP_SUPPORTED_CAPABILITY_CAPTURING                       (0x02)
#define BIP_SUPPORTED_CAPABILITY_PRINTING                        (0x04)
#define BIP_SUPPORTED_CAPABILITY_DISPLAYING                      (0x08)

#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH                         (0x0001)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_STORE                   (0x0002)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_PRINT                   (0x0004)
#define BIP_SUPPORTED_FEATURE_IMAGE_PUSH_DISPLAY                 (0x0008)
#define BIP_SUPPORTED_FEATURE_IMAGE_PULL                         (0x0010)
#define BIP_SUPPORTED_FEATURE_ADVANCED_IMAGE_PRINTING            (0x0020)
#define BIP_SUPPORTED_FEATURE_AUTOMATIC_ARCHIVE                  (0x0040)
#define BIP_SUPPORTED_FEATURE_REMOTE_CAMERA                      (0x0080)
#define BIP_SUPPORTED_FEATURE_REMOTE_DISPLAY                     (0x0100)

#define BIP_SUPPORTED_FUNCTION_GET_CAPABILITIES                  (0x00000001)
#define BIP_SUPPORTED_FUNCTION_PUT_IMAGE                         (0x00000002)
#define BIP_SUPPORTED_FUNCTION_PUT_LINKED_ATTACHMENT             (0x00000004)
#define BIP_SUPPORTED_FUNCTION_PUT_LINKED_THUMBNAIL              (0x00000008)
#define BIP_SUPPORTED_FUNCTION_REMOTE_DISPLAY                    (0x00000010)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGES_LIST                   (0x00000020)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGE_PROPERTIES              (0x00000040)
#define BIP_SUPPORTED_FUNCTION_GET_IMAGE                         (0x00000080)
#define BIP_SUPPORTED_FUNCTION_GET_LINKED_THUMBNAIL              (0x00000100)
#define BIP_SUPPORTED_FUNCTION_GET_LINKED_ATTACHMENT             (0x00000200)
#define BIP_SUPPORTED_FUNCTION_DELETE_IMAGE                      (0x00000400)
#define BIP_SUPPORTED_FUNCTION_START_PRINT                       (0x00000800)
#define BIP_SUPPORTED_FUNCTION_START_ARCHIVE                     (0x00002000)
#define BIP_SUPPORTED_FUNCTION_GET_MONITORING_IMAGE              (0x00004000)
#define BIP_SUPPORTED_FUNCTION_GET_STATUS                        (0x00010000)

   /* The following define the possible BIP Response codes.             */
   /* * NOTE * Response Codes less than 0x10                            */
   /*          (BIP_OBEX_RESPONSE_CONTINUE) are Reserved and CANNOT be  */
   /*          used.                                                    */
#define BIP_OBEX_RESPONSE_CONTINUE                               (OBEX_CONTINUE_RESPONSE)
#define BIP_OBEX_RESPONSE_OK                                     (OBEX_OK_RESPONSE)
#define BIP_OBEX_RESPONSE_PARTIAL_CONTENT                        (OBEX_PARTIAL_CONTENT_RESPONSE)

#define BIP_OBEX_RESPONSE_BAD_REQUEST                            (OBEX_BAD_REQUEST_RESPONSE)
#define BIP_OBEX_RESPONSE_NOT_IMPLEMENTED                        (OBEX_NOT_IMPLEMENTED_RESPONSE)
#define BIP_OBEX_RESPONSE_FORBIDDEN                              (OBEX_FORBIDDEN_RESPONSE)
#define BIP_OBEX_RESPONSE_UNAUTHORIZED                           (OBEX_UNAUTHORIZED_RESPONSE)
#define BIP_OBEX_RESPONSE_PRECONDITION_FAILED                    (OBEX_PRECONDITION_FAILED_RESPONSE)
#define BIP_OBEX_RESPONSE_NOT_FOUND                              (OBEX_NOT_FOUND_RESPONSE)
#define BIP_OBEX_RESPONSE_NOT_ACCEPTABLE                         (OBEX_NOT_ACCEPTABLE_RESPONSE)
#define BIP_OBEX_RESPONSE_SERVICE_UNAVAILABLE                    (OBEX_SERVICE_UNAVAILABLE_RESPONSE)

   /* The following BIT definitions are used to denote the possible     */
   /* Basic Imaging Profile Server Modes that can be applied to a BIP   */
   /* Client Connection.  These BIT definitions are used with the       */
   /* BIP_Set_Server_Mode() and BIP_Get_Server_Mode() mode functions.   */
#define BIP_SERVER_MODE_IMAGING_SERVICE_SERVER_PORT_AUTOMATIC_ACCEPT_CONNECTION (0x00000000)
#define BIP_SERVER_MODE_IMAGING_SERVICE_SERVER_PORT_MANUAL_ACCEPT_CONNECTION    (0x00000001)
#define BIP_SERVER_MODE_IMAGING_SERVICE_SERVER_PORT_CONNECTION_MASK             (0x00000001)

   /* The following constants represent the Open Status Values that are */
   /* possible for the OpenStatus member of the Open Confirmation       */
   /* events.                                                           */
#define BIP_OPEN_STATUS_SUCCESS                                  (0x00)
#define BIP_OPEN_STATUS_CONNECTION_TIMEOUT                       (0x01)
#define BIP_OPEN_STATUS_CONNECTION_REFUSED                       (0x02)
#define BIP_OPEN_STATUS_UNKNOWN_ERROR                            (0x04)

   /* The following constant represents the length of Basic Printing    */
   /* Profile Image Handle values.                                      */
   /* ** NOTE ** An image handle is seven characters values '0' through */
   /*            '9'.  The eighth character should be a '\0'.           */
#define BIP_IMAGE_HANDLE_LENGTH                                  (7)

   /* The following constants represent the Number of Returned Handles  */
   /* minimum, maximum and invalid values that may be used with the     */
   /* BIP_Get_Images_List_Request(), BIP_Get_Images_List_Response()     */
   /* functions and the Get Images List Indication and Confirmation     */
   /* events.                                                           */
#define BIP_NUMBER_OF_RETURNED_HANDLES_MINIMUM_VALUE             (0)
#define BIP_NUMBER_OF_RETURNED_HANDLES_MAXIMUM_VALUE             (65535)
#define BIP_NUMBER_OF_RETURNED_HANDLES_INVALID_VALUE             (0xFFFFFFFF)

   /* The following constants represent the List Start Offset minimum,  */
   /* maximum, and invalid values that may be used with the             */
   /* BIP_Get_Images_List_Request() function and the Get Images List    */
   /* Indication event.                                                 */
#define BIP_LIST_START_OFFSET_MINIMUM_VALUE                      (0)
#define BIP_LIST_START_OFFSET_MAXIMUM_VALUE                      (65535)
#define BIP_LIST_START_OFFSET_INVALID_VALUE                      (0xFFFFFFFF)

   /* The following constants represent the Latest Captured Images      */
   /* false, true and invalid values that may be used with the          */
   /* BIP_Get_Images_List_Request() function and the Get Images List    */
   /* Indication event.                                                 */
#define BIP_LATEST_CAPTURED_IMAGES_FALSE_VALUE                   (0x00)
#define BIP_LATEST_CAPTURED_IMAGES_TRUE_VALUE                    (0x01)
#define BIP_LATEST_CAPTURED_IMAGES_INVALID_VALUE                 (0xFF)

   /* The following constant represents the Image Length invalid value  */
   /* that may be used with the BIP_Get_Image_Response() function and   */
   /* the Get Image Confirmation event.                                 */
#define BIP_IMAGE_LENGTH_INVALID_VALUE                           (0)

   /* Basic Imaging Profile Event API Types.                            */
typedef enum
{
   etBIP_Open_Request_Indication,
   etBIP_Open_Imaging_Service_Port_Indication,
   etBIP_Open_Imaging_Service_Port_Confirmation,
   etBIP_Close_Imaging_Service_Port_Indication,
   etBIP_Abort_Indication,
   etBIP_Abort_Confirmation,
   etBIP_Get_Capabilities_Indication,
   etBIP_Get_Capabilities_Confirmation,
   etBIP_Put_Image_Indication,
   etBIP_Put_Image_Confirmation,
   etBIP_Put_Linked_Thumbnail_Indication,
   etBIP_Put_Linked_Thumbnail_Confirmation,
   etBIP_Get_Images_List_Indication,
   etBIP_Get_Images_List_Confirmation,
   etBIP_Get_Image_Properties_Indication,
   etBIP_Get_Image_Properties_Confirmation,
   etBIP_Get_Image_Indication,
   etBIP_Get_Image_Confirmation,
   etBIP_Get_Linked_Thumbnail_Indication,
   etBIP_Get_Linked_Thumbnail_Confirmation,
   etBIP_Delete_Image_Indication,
   etBIP_Delete_Image_Confirmation
} BIP_Event_Type_t;

   /* The following event is dispatched when a remote client requests a */
   /* connection to a local server.  The BIPID member specifies the     */
   /* identifier of the Local Basic Imaging Profile Server being connect*/
   /* with.  The ServerPort member specifies the server port of that    */
   /* Local Basic Imaging Profile Server being connected with.  The     */
   /* BD_ADDR member specifies the address of the Remote Client         */
   /* requesting the connection to the Local Server.                    */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            BIP_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagBIP_Open_Request_Indication_Data_t
{
   unsigned int BIPID;
   unsigned int ServerPort;
   BD_ADDR_t    BD_ADDR;
} BIP_Open_Request_Indication_Data_t;

#define BIP_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(BIP_Open_Request_Indication_Data_t))

   /* The following event is dispatched when a Basic Imaging Profile    */
   /* Imaging Initiator makes a Connection to a Registered Basic Imaging*/
   /* Profile Imaging Responder Server Imaging Service Port.  The BIPID */
   /* member specifies the identifier of the Local Basic Imaging Profile*/
   /* Imaging Responder Server that is being connected with.  The       */
   /* BD_ADDR member specifies the address of the Basic Imaging Profile */
   /* Imaging Initiator that is being connected to the Local Imaging    */
   /* Responder Server.  The ImagingServiceServerPortTarget member      */
   /* specifies that Target Service that was connected with on the Local*/
   /* Imaging Responder Server.                                         */
typedef struct _tagBIP_Open_Imaging_Service_Port_Indication_Data_t
{
   unsigned int                             BIPID;
   BD_ADDR_t                                BD_ADDR;
   BIP_Imaging_Service_Server_Port_Target_t ImagingServiceServerPortTarget;
} BIP_Open_Imaging_Service_Port_Indication_Data_t;

#define BIP_OPEN_IMAGING_SERVICE_PORT_INDICATION_DATA_SIZE (sizeof(BIP_Open_Imaging_Service_Port_Indication_Data_t))

   /* The following event is dispatched to the Local Basic Imaging      */
   /* Profile Imaging Initiator to indicate the success or failure of a */
   /* previously submitted Connection Attempt to an Imaging Service     */
   /* Server Port.  The BIPID member specifies the Identifier of the    */
   /* Local Basic Imaging Profile Imaging Initiator that has requested  */
   /* the Connection.  The OpenStatus member specifies the status of the*/
   /* Connection Attempt.  Valid values are:                            */
   /*    - BIP_OPEN_STATUS_SUCCESS                                      */
   /*    - BIP_OPEN_STATUS_CONNECTION_TIMEOUT                           */
   /*    - BIP_OPEN_STATUS_CONNECTION_REFUSED                           */
   /*    - BIP_OPEN_STATUS_UNKNOWN_ERROR                                */
typedef struct _tagBIP_Open_Imaging_Service_Port_Confirmation_Data_t
{
   unsigned int  BIPID;
   unsigned long OpenStatus;
} BIP_Open_Imaging_Service_Port_Confirmation_Data_t;

#define BIP_OPEN_IMAGING_SERVICE_PORT_CONFIRMATION_DATA_SIZE (sizeof(BIP_Open_Imaging_Service_Port_Confirmation_Data_t))

   /* The following event is dispatched when the Basic Imaging Profile  */
   /* Imaging Initiator or Responder disconnects from the local Basic   */
   /* Imaging Profile Imaging Initiator or Responder Server or Client   */
   /* Port.  The BIPID member specifies the Identifier for the Local    */
   /* Basic Imaging Profile connection being closed.                    */
typedef struct _tagBIP_Close_Imaging_Service_Port_Indication_Data_t
{
   unsigned int BIPID;
} BIP_Close_Imaging_Service_Port_Indication_Data_t;

#define BIP_CLOSE_IMAGING_SERVICE_PORT_INDICATION_DATA_SIZE (sizeof(BIP_Close_Imaging_Service_Port_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends an Abort       */
   /* Request.  The BIPID member specifies the Identifier of the Local  */
   /* Basic Imaging Profile Imaging Responder receiving this event.     */
typedef struct _tagBIP_Abort_Indication_Data_t
{
   unsigned int BIPID;
} BIP_Abort_Indication_Data_t;

#define BIP_ABORT_INDICATION_DATA_SIZE                  (sizeof(BIP_Abort_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends an Abort       */
   /* Response.  The BIPID member specifies the Identifier of the Local */
   /* Basic Imaging Profile Imaging Initiator receiving this event.     */
typedef struct _tagBIP_Abort_Confirmation_Data_t
{
   unsigned int BIPID;
} BIP_Abort_Confirmation_Data_t;

#define BIP_ABORT_CONFIRMATION_DATA_SIZE                (sizeof(BIP_Abort_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Get          */
   /* Capabilities Request.  The BIPID member specifies the Identifier  */
   /* of the Local Basic Imaging Profile Imaging Responder receiving    */
   /* this event.                                                       */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Get_Capabilities_Response() function.              */
typedef struct _tagBIP_Get_Capabilities_Indication_Data_t
{
   unsigned int BIPID;
} BIP_Get_Capabilities_Indication_Data_t;

#define BIP_GET_CAPABILITIES_INDICATION_DATA_SIZE       (sizeof(BIP_Get_Capabilities_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Get          */
   /* Capabilities Response.  The BIPID member specifies the Identifier */
   /* of the Local Basic Imaging Profile Imaging Initiator receiving    */
   /* this event.  The ResponseCode member specifies the response code  */
   /* associated with the Get Capabilities Response that generated this */
   /* event.  The DataLength member specifies the length of data pointed*/
   /* to by the DataBuffer member.  The DataBuffer member points to a   */
   /* buffer containing the actual capabilities data.                   */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Capabilities_Confirmation_Data_t
{
   unsigned int   BIPID;
   Byte_t         ResponseCode;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BIP_Get_Capabilities_Confirmation_Data_t;

#define BIP_GET_CAPABILITIES_CONFIRMATION_DATA_SIZE     (sizeof(BIP_Get_Capabilities_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Put Image    */
   /* Request.  The BIPID member specifies the Identifier of the Local  */
   /* Basic Imaging Profile Imaging Responder receiving this event.  The*/
   /* ImageName member specifies the name of the image being put with   */
   /* this Put Image Request.  The ImageDescriptorLength member         */
   /* specifies the length of the image descriptor pointed to by the    */
   /* ImageDescriptor member.  The ImageDescriptor member points to a   */
   /* buffer containing the actual image descriptor data.  The          */
   /* DataLength member specifies the length of data pointed to by the  */
   /* DataBuffer member.  The DataBuffer member points to a buffer      */
   /* containing the actual image data.  The Final member specifies if  */
   /* this indication completes the put image operation.                */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Put_Image_Response() function.                     */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Put_Image_Indication_Data_t
{
   unsigned int   BIPID;
   char          *ImageName;
   unsigned int   ImageDescriptorLength;
   unsigned char *ImageDescriptor;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
   Boolean_t      Final;
} BIP_Put_Image_Indication_Data_t;

#define BIP_PUT_IMAGE_INDICATION_DATA_SIZE              (sizeof(BIP_Put_Image_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Put Image    */
   /* Response.  The BIPID member specifies the Identifier of the Local */
   /* Basic Imaging Profile Imaging Initiator receiving this event.  The*/
   /* ResponseCode member specifies the response code associated with   */
   /* the Put Image Response that generated this event.  The ImageHandle*/
   /* handle points to the image handle to be associated with the Image */
   /* transferred as part of this request.                              */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Put_Image_Confirmation_Data_t
{
   unsigned int  BIPID;
   Byte_t        ResponseCode;
   char         *ImageHandle;
} BIP_Put_Image_Confirmation_Data_t;

#define BIP_PUT_IMAGE_CONFIRMATION_DATA_SIZE            (sizeof(BIP_Put_Image_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Put Linked   */
   /* Thumbnail Request.  The BIPID member specifies the Identifier of  */
   /* the Local Basic Imaging Profile Imaging Responder receiving this  */
   /* event.  The ImageHandle member specifies the handle of the image  */
   /* the thumbnail is to be linked with.  The DataLength member        */
   /* specifies the length of data pointed to by the DataBuffer member. */
   /* The DataBuffer member points to a buffer containing the actual    */
   /* image data.  The Final member specifies if this indication        */
   /* completes the put linked thumbnail operation.                     */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Put_Linked_Thumbnail_Response() function.          */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Put_Linked_Thumbnail_Indication_Data_t
{
   unsigned int   BIPID;
   char          *ImageHandle;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
   Boolean_t      Final;
} BIP_Put_Linked_Thumbnail_Indication_Data_t;

#define BIP_PUT_LINKED_THUMBNAIL_INDICATION_DATA_SIZE   (sizeof(BIP_Put_Linked_Thumbnail_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Put Linked   */
   /* Thumbnail Response.  The BIPID member specifies the Identifier of */
   /* the Local Basic Imaging Profile Imaging Initiator receiving this  */
   /* event.  The ResponseCode member specifies the response code       */
   /* associated with the Put Linked Thumbnail Response that generated  */
   /* this event.                                                       */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Put_Linked_Thumbnail_Confirmation_Data_t
{
   unsigned int BIPID;
   Byte_t       ResponseCode;
} BIP_Put_Linked_Thumbnail_Confirmation_Data_t;

#define BIP_PUT_LINKED_THUMBNAIL_CONFIRMATION_DATA_SIZE (sizeof(BIP_Put_Linked_Thumbnail_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Get Images   */
   /* List Request.  The BIPID member specifies the Identifier of the   */
   /* Local Basic Imaging Profile Imaging Responder receiving this      */
   /* event.  The NumberOfReturnedHandle member indicates the maximum   */
   /* number of image handles to be returned in the images listing      */
   /* object.  The ListStartOffset member describes a zero-based offset */
   /* from the beginning of the images listing object and may be used to*/
   /* retrieve the images listing object in pieces.  The                */
   /* LatestCapturedImages member is used to restrict the scope of the  */
   /* images listing object to the most recently captured images and    */
   /* control the order of the images within the list.  The             */
   /* ImageHandlesDescriptorLength specifies the length of data pointed */
   /* to by the ImageHandlesDescriptor member.  The                     */
   /* ImageHandlesDescriptor member points to a buffer containing the   */
   /* actual image handle descriptor object data.                       */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Get_Images_List_Response() function.               */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
   /* ** NOTE ** The NumberOfReturnedHandles member will be set to      */
   /*            BIP_NUMBER_OF_RETURNED_HANDLES_INVALID_VALUE when      */
   /*            invalid.                                               */
   /* ** NOTE ** The ListStartOffset member will be set to              */
   /*            BIP_LIST_START_OFFSET_INVALID_VALUE when invalid.      */
   /* ** NOTE ** The LatestCapturedImages will be set to                */
   /*            BIP_LATEST_CAPTURED_IMAGES_INVALID_VALUE when invalid. */
typedef struct _tagBIP_Get_Images_List_Indication_Data_t
{
   unsigned int   BIPID;
   DWord_t        NumberOfReturnedHandles;
   DWord_t        ListStartOffset;
   Byte_t         LatestCapturedImages;
   unsigned int   ImageHandlesDescriptorLength;
   unsigned char *ImageHandlesDescriptor;
} BIP_Get_Images_List_Indication_Data_t;

#define BIP_GET_IMAGES_LIST_INDICATION_DATA_SIZE        (sizeof(BIP_Get_Images_List_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Get Images   */
   /* List Response.  The BIPID member specifies the Identifier of the  */
   /* Local Basic Imaging Profile Imaging Initiator receiving this      */
   /* event.  The ResponseCode member specifies the response code       */
   /* associated with the Get Images List Response that generated this  */
   /* event.  The NumberOfReturnedHandles member specifies indicates the*/
   /* number of image handles that are being returned in the images     */
   /* listing object.  The ImageHandlesDescriptorLength specifies the   */
   /* length of data pointed to by the ImageHandlesDescriptor member.   */
   /* The ImageHandlesDescriptor member points to a buffer containing   */
   /* the actual image handle descriptor object data that specifies the */
   /* filtering mask that was used when creating the image listing      */
   /* object.  The DataLength member specifies length of data pointed to*/
   /* by the DataBuffer member.  The DataBuffer member pointes to a     */
   /* buffer contain the actual image list object being received.       */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
   /* ** NOTE ** The NumberOfReturnedHandles member will be set to      */
   /*            BIP_NUMBER_OF_RETURNED_HANDLES_INVALID_VALUE when      */
   /*            invalid.                                               */
typedef struct _tagBIP_Get_Images_List_Confirmation_Data_t
{
   unsigned int   BIPID;
   Byte_t         ResponseCode;
   DWord_t        NumberOfReturnedHandles;
   unsigned int   ImageHandlesDescriptorLength;
   unsigned char *ImageHandlesDescriptor;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BIP_Get_Images_List_Confirmation_Data_t;

#define BIP_GET_IMAGES_LIST_CONFIRMATION_DATA_SIZE      (sizeof(BIP_Get_Images_List_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Get Image    */
   /* Properties Request.  The BIPID member specifies the Identifier of */
   /* the Local Basic Imaging Profile Imaging Responder receiving this  */
   /* event.  The ImageHandle member specifies the handle of the image  */
   /* in which to retrieve the properties.                              */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Get_Image_Properties_Request() function.           */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Image_Properties_Indication_Data_t
{
   unsigned int  BIPID;
   char         *ImageHandle;
} BIP_Get_Image_Properties_Indication_Data_t;

#define BIP_GET_IMAGE_PROPERTIES_INDICATION_DATA_SIZE   (sizeof(BIP_Get_Image_Properties_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Get Image    */
   /* Properties Response.  The BIPID member specifies the Identifier of*/
   /* the Local Basic Imaging Profile Imaging Initiator receiving this  */
   /* event.  The ResponseCode member specifies the response code       */
   /* associated with the Get Image Properties Response that generated  */
   /* this event.  The DataLength member specifies the length of data   */
   /* pointed to by the DataBuffer member.  The DataBuffer member points*/
   /* to a buffer containing the actual image properties object data.   */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Image_Properties_Confirmation_Data_t
{
   unsigned int   BIPID;
   Byte_t         ResponseCode;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BIP_Get_Image_Properties_Confirmation_Data_t;

#define BIP_GET_IMAGE_PROPERTIES_CONFIRMATION_DATA_SIZE (sizeof(BIP_Get_Image_Properties_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Get Image    */
   /* Request.  The BIPID member specifies the Identifier of the Local  */
   /* Basic Imaging Profile Imaging Responder receiving this event.  The*/
   /* ImageHandle member specifies the handle of the image in which to  */
   /* retrieve.  The ImageDescriptorLength member specifies the length  */
   /* of the image descriptor pointed to by the ImageDescriptor member. */
   /* The ImageDescriptor member points to a buffer containing the      */
   /* actual image descriptor data.                                     */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Get_Image_Response() function.                     */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Image_Indication_Data_t
{
   unsigned int   BIPID;
   char          *ImageHandle;
   unsigned int   ImageDescriptorLength;
   unsigned char *ImageDescriptor;
} BIP_Get_Image_Indication_Data_t;

#define BIP_GET_IMAGE_INDICATION_DATA_SIZE              (sizeof(BIP_Get_Image_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Get Image    */
   /* Response.  The BIPID member specifies the Identifier of the Local */
   /* Basic Imaging Profile Imaging Initiator receiving this event.  The*/
   /* ResponseCode member specifies the response code associated with   */
   /* the Get Image Response that generated this event.  The ImageLength*/
   /* member specifies the total length of the image being retrieved.   */
   /* The DataLength member specifies the length of data pointed to by  */
   /* the DataBuffer member.  The DataBuffer member points to a buffer  */
   /* containing the actual image data.                                 */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e  Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
   /* ** NOTE ** The ImageLength member will be set to                  */
   /*            BIP_IMAGE_LENGTH_INVALID_VALUE when invalid.           */
typedef struct _tagBIP_Get_Image_Confirmation_Data_t
{
   unsigned int   BIPID;
   Byte_t         ResponseCode;
   unsigned int   ImageLength;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BIP_Get_Image_Confirmation_Data_t;

#define BIP_GET_IMAGE_CONFIRMATION_DATA_SIZE            (sizeof(BIP_Get_Image_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Get Linked   */
   /* Thumbnail Request.  The BIPID member specifies the Identifier of  */
   /* the Local Basic Imaging Profile Imaging Responder receiving this  */
   /* event.  The ImageHandle member specifies the handle of the image  */
   /* in which to retrieve the linked thumbnail.                        */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Get_Linked_Thumbnail_Response() function.          */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Linked_Thumbnail_Indication_Data_t
{
   unsigned int  BIPID;
   char         *ImageHandle;
} BIP_Get_Linked_Thumbnail_Indication_Data_t;

#define BIP_GET_LINKED_THUMBNAIL_INDICATION_DATA_SIZE   (sizeof(BIP_Get_Linked_Thumbnail_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Get Linked   */
   /* Thumbnail Response.  The BIPID member specifies the Identifier of */
   /* the Local Basic Imaging Profile Imaging Initiator receiving this  */
   /* event.  The ResponseCode member specifies the response code       */
   /* associated with the Get Linked Thumbnail Response that generated  */
   /* this event.  The DataLength member specifies the length of data   */
   /* pointed to by the DataBuffer member.  The DataBuffer member points*/
   /* to a buffer containing the actual image data.                     */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Get_Linked_Thumbnail_Confirmation_Data_t
{
   unsigned int   BIPID;
   Byte_t         ResponseCode;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BIP_Get_Linked_Thumbnail_Confirmation_Data_t;

#define BIP_GET_LINKED_THUMBNAIL_CONFIRMATION_DATA_SIZE (sizeof(BIP_Get_Linked_Thumbnail_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Responder when the Imaging Initiator sends a Delete Image */
   /* Request.  The BIPID member specifies the Identifier of the Local  */
   /* Basic Imaging Profile Imaging Responder receiving this event.  The*/
   /* ImageHandle member specifies the handle of the image in which to  */
   /* delete.                                                           */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BIP_Delete_Image_Response() function.                  */
   /* ** NOTE ** The members of this structure map to the various       */
   /*            headers that may be present in a single segment of the */
   /*            transaction.  Some headers will only appear once in the*/
   /*            stream associated with a transaction (i.e. Name),      */
   /*            others don't appear in the first segment but may appear*/
   /*            in later segments (i.e. Body).  This being the case,   */
   /*            it is not guaranteed that all members of this structure*/
   /*            will be valid through out the entire transaction.      */
   /*            Therefore it is required that parameter checking be    */
   /*            used to validate the members of this event that may not*/
   /*            have existed in the current segment of the transaction.*/
typedef struct _tagBIP_Delete_Image_Indication_Data_t
{
   unsigned int  BIPID;
   char         *ImageHandle;
} BIP_Delete_Image_Indication_Data_t;

#define BIP_DELETE_IMAGE_INDICATION_DATA_SIZE           (sizeof(BIP_Delete_Image_Indication_Data_t))

   /* The following event is dispatched to the Basic Imaging Profile    */
   /* Imaging Initiator when the Imaging Responder sends a Delete Image */
   /* Response.  The BIPID member specifies the Identifier of the Local */
   /* Basic Imaging Profile Imaging Initiator receiving this event.  The*/
   /* ResponseCode member specifies the response code associated with   */
   /* the Delete Image Response that generated this event.              */
typedef struct _tagBIP_Delete_Image_Confirmation_Data_t
{
   unsigned int BIPID;
   Byte_t       ResponseCode;
} BIP_Delete_Image_Confirmation_Data_t;

#define BIP_DELETE_IMAGE_CONFIRMATION_DATA_SIZE         (sizeof(BIP_Delete_Image_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Bluetooth Imaging Profile Event Data.                 */
typedef struct _tagBIP_Event_Data_t
{
   BIP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      BIP_Open_Request_Indication_Data_t                *BIP_Open_Request_Indication_Data;
      BIP_Open_Imaging_Service_Port_Indication_Data_t   *BIP_Open_Imaging_Service_Port_Indication_Data;
      BIP_Open_Imaging_Service_Port_Confirmation_Data_t *BIP_Open_Imaging_Service_Port_Confirmation_Data;
      BIP_Close_Imaging_Service_Port_Indication_Data_t  *BIP_Close_Imaging_Service_Port_Indication_Data;
      BIP_Abort_Indication_Data_t                       *BIP_Abort_Indication_Data;
      BIP_Abort_Confirmation_Data_t                     *BIP_Abort_Confirmation_Data;
      BIP_Get_Capabilities_Indication_Data_t            *BIP_Get_Capabilities_Indication_Data;
      BIP_Get_Capabilities_Confirmation_Data_t          *BIP_Get_Capabilities_Confirmation_Data;
      BIP_Put_Image_Indication_Data_t                   *BIP_Put_Image_Indication_Data;
      BIP_Put_Image_Confirmation_Data_t                 *BIP_Put_Image_Confirmation_Data;
      BIP_Put_Linked_Thumbnail_Indication_Data_t        *BIP_Put_Linked_Thumbnail_Indication_Data;
      BIP_Put_Linked_Thumbnail_Confirmation_Data_t      *BIP_Put_Linked_Thumbnail_Confirmation_Data;
      BIP_Get_Images_List_Indication_Data_t             *BIP_Get_Images_List_Indication_Data;
      BIP_Get_Images_List_Confirmation_Data_t           *BIP_Get_Images_List_Confirmation_Data;
      BIP_Get_Image_Properties_Indication_Data_t        *BIP_Get_Image_Properties_Indication_Data;
      BIP_Get_Image_Properties_Confirmation_Data_t      *BIP_Get_Image_Properties_Confirmation_Data;
      BIP_Get_Image_Indication_Data_t                   *BIP_Get_Image_Indication_Data;
      BIP_Get_Image_Confirmation_Data_t                 *BIP_Get_Image_Confirmation_Data;
      BIP_Get_Linked_Thumbnail_Indication_Data_t        *BIP_Get_Linked_Thumbnail_Indication_Data;
      BIP_Get_Linked_Thumbnail_Confirmation_Data_t      *BIP_Get_Linked_Thumbnail_Confirmation_Data;
      BIP_Delete_Image_Indication_Data_t                *BIP_Delete_Image_Indication_Data;
      BIP_Delete_Image_Confirmation_Data_t              *BIP_Delete_Image_Confirmation_Data;
   } Event_Data;
} BIP_Event_Data_t;

#define BIP_EVENT_DATA_SIZE                             (sizeof(BIP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a BIP Event Callback.  This function will be called whenever a    */
   /* Basic Imaging Profile Event occurs that is associated with the    */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the BIP Event Data that occurred, and the */
   /* BIP Event Callback Parameter that was specified when this Callback*/
   /* was installed.  The caller is free to use the contents of the BIP */
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
   /* (this argument holds anyway because another BIP Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BIP Events.  A      */
   /*            Deadlock WILL occur because NO BIP Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *BIP_Event_Callback_t)(unsigned int BluetoothStackID, BIP_Event_Data_t *BIP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for opening a local imaging */
   /* responder server.  The first parameter to this function is the    */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack Instance to be */
   /* associated with this Basic Imaging Profile Imaging Responder      */
   /* Server.  The second parameter to this function is Imaging Service */
   /* Server Port to be used by the Imaging Responder Server.  The third*/
   /* parameter to this function is a bit mask used to indicate the     */
   /* targets that should be allowed to connect to the Imaging Service  */
   /* Server Port.  The fourth and fifth parameters are the Event       */
   /* Callback function and application defined Callback Parameter to be*/
   /* used when BIP Events occur.  This function returns a non-zero,    */
   /* positive, number on success or a negative return value if there   */
   /* was an error.  A successful return value will be a BIP ID that can*/
   /* used to reference the Opened Basic Imaging Profile Image Responder*/
   /* Server in ALL other applicable functions in this module.          */
   /* ** NOTE ** The Imaging Service Server Port value must be specified*/
   /*            and must be a value between BIP_PORT_NUMBER_MINIMUM and*/
   /*            BIP_PORT_NUMBER_MAXIMUM.                               */
BTPSAPI_DECLARATION int BTPSAPI BIP_Open_Imaging_Responder_Server(unsigned int BluetoothStackID, unsigned int ImagingServiceServerPort, unsigned long ImagingServicePortTargetsMask, BIP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Open_Imaging_Responder_Server_t)(unsigned int BluetoothStackID, unsigned int ImagingServiceServerPort, unsigned long ImagingServicePortTargetsMask, BIP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a connection    */
   /* from a local Imaging Initiator to a remote Imaging Responder      */
   /* device.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Protocol Stack Instance to be associated*/
   /* with this Basic Imaging Profile connection to a remote Imaging    */
   /* Responder server.  The second parameter to this function is the   */
   /* BD_ADDR of the remote Imaging Responder device in which to        */
   /* connect.  The third parameter to this function is the Imaging     */
   /* Service Server Port to connect with on the remote Imaging         */
   /* Responder Server.  The fourth parameter to this function is the   */
   /* Target Service to connect with on the specified Imaging Service   */
   /* Server Port.  The fifth and sixth parameters are the Event        */
   /* Callback function and application defined Callback Parameter to be*/
   /* used when BIP Events occur.  This function returns a non-zero,    */
   /* positive, number on success or a negative return value if there   */
   /* was an error.  A successful return value will be a BIP ID that can*/
   /* used to reference the Opened Basic Imaging Profile connection to a*/
   /* remote Imaging Responder Server in ALL other applicable functions */
   /* in this module.                                                   */
   /* ** NOTE ** The Imaging Service Server Port value must be specified*/
   /*            and must be a value between BIP_PORT_NUMBER_MINIMUM and*/
   /*            BIP_PORT_NUMBER_MAXIMUM.                               */
BTPSAPI_DECLARATION int BTPSAPI BIP_Open_Remote_Imaging_Responder_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ImagingServiceServerPort, BIP_Imaging_Service_Server_Port_Target_t ImagingServiceServerPortTarget, BIP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Open_Remote_Imaging_Responder_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ImagingServiceServerPort, BIP_Imaging_Service_Server_Port_Target_t ImagingServiceServerPortTarget, BIP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Basic Imaging Profile    */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Basic Imaging */
   /* Profile Server that is responding to the request.  The second     */
   /* parameter to this function is the BIP ID of the Basic Imaging     */
   /* Profile for which a connection request was received.  The final   */
   /* parameter to this function specifies whether to accept the pending*/
   /* connection request (or to reject the request).  This function     */
   /* returns zero if successful, or a negative return error code if an */
   /* error occurred.                                                   */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etBIP_Open_Imaging_Service_Port_Indication event has   */
   /*            occurred.                                              */
BTPSAPI_DECLARATION int BTPSAPI BIP_Open_Request_Response(unsigned int BluetoothStackID, unsigned int BIPID, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for registering an Imaging  */
   /* Responder service servers service record to the SDP database.  The*/
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Basic Imaging Profile Imaging */
   /* Responder Server for which this service record is being           */
   /* registered.  The second parameter to this function is the BIP ID  */
   /* of the Basic Imaging Profile Imaging Responder Server for which   */
   /* this service record is being registered.  The third parameter to  */
   /* this function is the Service Name to be associated with this      */
   /* service record.  The fourth parameter specifies a bit mask that   */
   /* identifies the capabilities supported by the local Imaging        */
   /* Responder Service Server.  The fifth parameter specifies a bit    */
   /* mask that identifies the features supported by the local Imaging  */
   /* Responder Service Server.  The sixth parameter specifies a bit    */
   /* mask that identifies the functions that are supported by the local*/
   /* Imaging Responder Service Server.  The seventh and eighth         */
   /* parameters combine to define the total amount of memory available */
   /* to store image data.  The final parameter to this function is a   */
   /* pointer to a DWord_t which receives the SDP Service Record Handle */
   /* if this function successfully creates a service record.  If this  */
   /* function returns zero, then the SDPServiceRecordHandle entry will */
   /* contain the Service Record Handle of the added SDP Service Record.*/
   /* If this function fails, a negative return error code will be      */
   /* returned (see BTERRORS.H) and the SDPServiceRecordHandle value    */
   /* will be undefined.                                                */
   /* * NOTE * This function should only be called with the BIP ID that */
   /*          was returned from the BIP_Open_Imaging_Responder_Server()*/
   /*          function.                                                */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          BIP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI BIP_Register_Imaging_Responder_Service_SDP_Record(unsigned int BluetoothStackID, unsigned int BIPID, char *ServiceName, Byte_t SupportedCapabilitiesMask, Word_t SupportedFeaturesMask, DWord_t SupportedFunctionsMask, DWord_t TotalCapacityHigh, DWord_t TotalCapacityLow, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Register_Imaging_Responder_Service_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ServiceName, Byte_t SupportedCapabilitiesMask, Word_t SupportedFeaturesMask, DWord_t SupportedFunctionsMask, DWord_t TotalCapacityHigh, DWord_t TotalCapacityLow, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* Basic Imaging Profile Server SDP Service Record (specified by the */
   /* third parameter) from the SDP Database.  This MACRO simply maps to*/
   /* the SDP_Delete_Service_Record() function.  This MACRO is only     */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the BIP ID (returned from a         */
   /* successful call to the BIP_Open_Imaging_Responder_Server()        */
   /* function), and the SDP Service Record Handle.  The SDP Service    */
   /* Record Handle was returned via a successful call to the           */
   /* BIP_Register_Imaging_Responder_Service_SDP_Record() function.     */
   /* This MACRO returns the result of the SDP_Delete_Service_Record()  */
   /* function, which is zero for success or a negative return error    */
   /* code (see BTERRORS.H).                                            */
#define BIP_Un_Register_SDP_Record(__BluetoothStackID, __BIPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for closing a currently     */
   /* open/registered Basic Imaging Profile server.  This function is   */
   /* capable of closing servers opened via a call to                   */
   /* BIP_Open_Imaging_Responder_Server().  The first parameter to this */
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Basic Imaging Profile Server */
   /* being closed.  The second parameter to this function is the BIP ID*/
   /* of the Basic Imaging Profile Server to be closed.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
BTPSAPI_DECLARATION int BTPSAPI BIP_Close_Server(unsigned int BluetoothStackID, unsigned int BIPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int BIPID);
#endif

   /* The following function is responsible for closing a currently     */
   /* ongoing Basic Imaging Profile connection.  The first parameter to */
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Basic Imaging Profile  */
   /* connection being closed.  The second parameter to this function is*/
   /* the BIP ID of the Basic Imaging Profile connection to be closed.  */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** If this function is called with a server BIP ID (value */
   /*            returned from BIP_Open_Imaging_Responder_Server()) any */
   /*            clients current connection to this server will be      */
   /*            terminated, but the server will remained registered.   */
   /*            If this function is call using a client BIP ID (value  */
   /*            returned from                                          */
   /*            BIP_Open_Remote_Imaging_Responder_Server()) the client */
   /*            connection shall be terminated.                        */
BTPSAPI_DECLARATION int BTPSAPI BIP_Close_Connection(unsigned int BluetoothStackID, unsigned int BIPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int BIPID);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current Basic Imaging Profile Server Mode.  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* BIP ID of the Basic Imaging Profile Server in which to get the    */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* a pointer to a variable which will receive the current Server Mode*/
   /* Mask.  This function returns zero if successful, or a negative    */
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int BIPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int BIPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current Basic Imaging Profile Server Mode.  The first  */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* BIP ID of the Basic Imaging Profile Server in which to set the    */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* the new Server Mode Mask to use.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI BIP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int BIPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int BIPID, unsigned long ServerModeMask);
#endif

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Imaging Service Server.  The first parameter to this*/
   /* function is the Bluetooth Stack ID of the Bluetooth Protocol Stack*/
   /* Instance that is associated with the Basic Imaging Profile Imaging*/
   /* Service Client making this call.  The second parameter to this    */
   /* function is the BIP ID of the Basic Imaging Profile Imaging       */
   /* Service Client making this call.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
BTPSAPI_DECLARATION int BTPSAPI BIP_Abort_Request(unsigned int BluetoothStackID, unsigned int BIPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Abort_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID);
#endif

   /* The following function is responsible for sending a Get           */
   /* Capabilities Request to the remote Imaging Service Server.  The   */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Client making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Client making this call.  This    */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function should be used to initiate the Get       */
   /*            Capabilities transaction as well as to continue a      */
   /*            previously initiated, on-going, Get Capabilities       */
   /*            transaction.                                           */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Capabilities_Request(unsigned int BluetoothStackID, unsigned int BIPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Capabilities_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID);
#endif

   /* The following function is responsible for sending a Get           */
   /* Capabilities Response to the remote Imaging Service Client.  The  */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth and fifth parameters to*/
   /* this function specify the length of the data being sent and a     */
   /* pointer to the data being sent.  The final parameter to this      */
   /* function is a pointer to a length variable that will return the   */
   /* amount of data actually sent.  This function returns zero if      */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Capabilities_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Capabilities_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is responsible for sending a Put Image     */
   /* Request to the remote Imaging Service Server.  The first parameter*/
   /* to this function is the Bluetooth Stack ID of the Bluetooth       */
   /* Protocol Stack Instance that is associated with the Basic Imaging */
   /* Profile Imaging Service Client making this call.  The second      */
   /* parameter to this function is the BIP ID of the Basic Imaging     */
   /* Profile Imaging Service Client making this call.  The third       */
   /* parameter to this function is the Image Name of the Image being   */
   /* put.  The fourth and fifth parameter to this function specify the */
   /* length of the Image Descriptor and a pointer to the Image         */
   /* Descriptor associated with the Image being put.  The sixth and    */
   /* seventh parameters to this function specify the length of the data*/
   /* being sent and a pointer to the data being sent.  The eighth      */
   /* parameter to this function is a pointer to a length variable that */
   /* will return the amount of data actually sent.  The final parameter*/
   /* to this function is a Boolean Flag indicating if this is to be the*/
   /* final portion of this operation.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** This function should be used to initiate the Put Image */
   /*            transaction as well as to continue a previously        */
   /*            initiated, on-going, Put Image transaction.            */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Put_Image_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageName, unsigned int ImageDescriptorLength, unsigned char *ImageDescriptor, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Put_Image_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageName, unsigned int ImageDescriptorLength, unsigned char *ImageDescriptor, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);
#endif

   /* The following function is responsible for sending a Put Image     */
   /* Response to the remote Imaging Service Client.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The final parameter to this       */
   /* function is the Imagine Handle to be associated with the Image    */
   /* being received.  This function returns zero if successful, or a   */
   /* negative return value if there was an error.                      */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Put_Image_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, char *ImageHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Put_Image_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, char *ImageHandle);
#endif

   /* The following function is responsible for sending a Put Linked    */
   /* Thumbnail Request to the remote Imaging Service Server.  The first*/
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Client making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Client making this call.  The     */
   /* third parameter to this function the Imagine Handle of the image  */
   /* for which the thumbnail being put is to be associated with.  The  */
   /* fourth and fifth parameters to this function specify the length of*/
   /* the data being sent and a pointer to the data being sent.  The    */
   /* sixth parameter to this function is a pointer to a length variable*/
   /* that will return the amount of data actually sent.  The final     */
   /* parameter to this function is a Boolean Flag indicating if this is*/
   /* to be the final portion of this operation.  This function returns */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
   /* ** NOTE ** This function should be used to initiate the Put Linked*/
   /*            Thumbnail transaction as well as to continue a         */
   /*            previously initiated, on-going, Put Linked Thumbnail   */
   /*            transaction.                                           */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Put_Linked_Thumbnail_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Put_Linked_Thumbnail_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);
#endif

   /* The following function is responsible for sending a Put Linked    */
   /* Thumbnail Response to the remote Imaging Service Client.  The     */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* final parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Put_Linked_Thumbnail_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Put_Linked_Thumbnail_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode);
#endif

   /* The following function is responsible for sending a Get Images    */
   /* List Request to the remote Imaging Service Server.  The first     */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Client making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Client making this call.  The     */
   /* third parameter to this function indicates the maximum number of  */
   /* image handles to be returned in the images listing object.  The   */
   /* fourth parameter to this function describes a zero-based offset   */
   /* from the beginning of the images listing object and may be used to*/
   /* retrieve the images listing object in pieces.  The fifth parameter*/
   /* to this function is used to restrict the scope of the images      */
   /* listing object to the most recently captured images and control   */
   /* the order of the images within the list.  The sixth and seventh   */
   /* parameter to this function specify the length of the Image Handles*/
   /* Descriptor and a pointer to the Image Handles Descriptor object   */
   /* that specifies a filtering mask to apply to the images listing    */
   /* object.  This function returns zero if successful, or a negative  */
   /* return value if there was an error.                               */
   /* ** NOTE ** This function should be used to initiate the Get Images*/
   /*            List transaction as well as to continue a previously   */
   /*            initiated, on-going, Get Images List transaction.      */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Images_List_Request(unsigned int BluetoothStackID, unsigned int BIPID, Word_t NumberOfReturnedHandles, Word_t ListStartOffset, Byte_t LatestCapturedImages, unsigned int ImageHandlesDescriptorLength, unsigned char *ImageHandlesDescriptor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Images_List_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, Word_t NumberOfReturnedHandles, Word_t ListStartOffset, Byte_t LatestCapturedImages, unsigned int ImageHandlesDescriptorLength, unsigned char *ImageHandlesDescriptor);
#endif

   /* The following function is responsible for sending a Get Images    */
   /* List Response to the remote Imaging Service Client.  The first    */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth parameter to this      */
   /* function indicates the number of image handles that are being     */
   /* returned in the images listing object.  The fifth and sixth       */
   /* parameter to this function specify the length of the Image Handles*/
   /* Descriptor and a pointer to the Image Handles Descriptor object   */
   /* that specifies the filtering mask that was used when creating the */
   /* images listing object.  The seventh and eighth parameters to this */
   /* function specify the length of the data being sent and a pointer  */
   /* to the data being sent (image listing object).  The final         */
   /* parameter to this function is a pointer to a length variable that */
   /* will return the amount of data actually sent.  This function      */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Images_List_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, Word_t NumberOfReturnedHandles, unsigned int ImageHandlesDescriptorLength, unsigned char *ImageHandlesDescriptor, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Images_List_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, Word_t NumberOfReturnedHandles, unsigned int ImageHandlesDescriptorLength, unsigned char *ImageHandlesDescriptor, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is responsible for sending a Get Image     */
   /* Properties Request to the remote Imaging Service Server.  The     */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Client making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Client making this call.  The     */
   /* final parameter to this function is the Image Handle of the image */
   /* to retrieve the properties object for.  This function returns zero*/
   /* if successful, or a negative return value if there was an error.  */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
   /* ** NOTE ** This function should be used to initiate the Get Image */
   /*            Properties transaction as well as to continue a        */
   /*            previously initiated, on-going, Get Image Properties   */
   /*            transaction.                                           */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Image_Properties_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Image_Properties_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);
#endif

   /* The following function is responsible for sending a Get Image     */
   /* Properties Response to the remote Imaging Service Client.  The    */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth and fifth parameters to*/
   /* this function specify the length of the data being sent and a     */
   /* pointer to the data being sent (image properties object).  The    */
   /* final parameter to this function is a pointer to a length variable*/
   /* that will return the amount of data actually sent.  This function */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Image_Properties_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Image_Properties_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is responsible for sending a Get Image     */
   /* Request to the remote Imaging Service Server.  The first parameter*/
   /* to this function is the Bluetooth Stack ID of the Bluetooth       */
   /* Protocol Stack Instance that is associated with the Basic Imaging */
   /* Profile Imaging Service Client making this call.  The second      */
   /* parameter to this function is the BIP ID of the Basic Imaging     */
   /* Profile Imaging Service Client making this call.  The third       */
   /* parameter to this function is the Image Handle of the Image to    */
   /* get.  The fourth and fifth parameter to this function specify the */
   /* length of the Image Descriptor and a pointer to the Image         */
   /* Descriptor object describing the image properties the client      */
   /* wishes the server to provide.  This function returns zero if      */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
   /* ** NOTE ** This function should be used to initiate the Get Image */
   /*            transaction as well as to continue a previously        */
   /*            initiated, on-going, Get Image transaction.            */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Image_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle, unsigned int ImageDescriptorLength, unsigned char *ImageDescriptor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Image_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle, unsigned int ImageDescriptorLength, unsigned char *ImageDescriptor);
#endif

   /* The following function is responsible for sending a Get Image     */
   /* Response to the remote Imaging Service Client.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth parameter to this      */
   /* function is the Total Image Length in bytes of the image being    */
   /* retrieved.  The fifth and sixth parameters to this function       */
   /* specify the length of the image data being sent and a pointer to  */
   /* the image data being sent.  The final parameter to this function  */
   /* is a pointer to a length variable that will return the amount of  */
   /* data actually sent.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Image_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int ImageLength, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Image_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int ImageLength, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is responsible for sending a Get Linked    */
   /* Thumbnail Request to the remote Imaging Service Server.  The first*/
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Client making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Client making this call.  The     */
   /* third parameter to this function is the Image Handle of the Image */
   /* in which to get the linked thumbnail.  This function returns zero */
   /* if successful, or a negative return value if there was an error.  */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
   /* ** NOTE ** This function should be used to initiate the Get Linked*/
   /*            Thumbnail transaction as well as to continue a         */
   /*            previously initiated, on-going, Get Linked Thumbnail   */
   /*            transaction.                                           */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Linked_Thumbnail_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Linked_Thumbnail_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);
#endif

   /* The following function is responsible for sending a Get Linked    */
   /* Thumbnail Response to the remote Imaging Service Client.  The     */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* third parameter to this function is the Response Code to be       */
   /* associated with this response.  The fourth and fifth parameters to*/
   /* this function specify the length of the image data being sent and */
   /* a pointer to the image data being sent.  The final parameter to   */
   /* this function is a pointer to a length variable that will return  */
   /* the amount of data actually sent.  This function returns zero if  */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Imaging Profile function.   */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BIP_Get_Linked_Thumbnail_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Get_Linked_Thumbnail_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is responsible for sending a Delete Image  */
   /* Request to the remote Imaging Service Server.  The first parameter*/
   /* to this function is the Bluetooth Stack ID of the Bluetooth       */
   /* Protocol Stack Instance that is associated with the Basic Imaging */
   /* Profile Imaging Service Client making this call.  The second      */
   /* parameter to this function is the BIP ID of the Basic Imaging     */
   /* Profile Imaging Service Client making this call.  The final       */
   /* parameter to this function is the Image Handle of the Image to    */
   /* delete.  This function returns zero if successful, or a negative  */
   /* return value if there was an error.                               */
   /* ** NOTE ** Image Handles are to be '\0' terminated strings with   */
   /*            character values '0' through '9' and                   */
   /*            BIP_IMAGE_HANDLE_LENGTH long not including the '\0'    */
   /*            character.                                             */
BTPSAPI_DECLARATION int BTPSAPI BIP_Delete_Image_Request(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Delete_Image_Request_t)(unsigned int BluetoothStackID, unsigned int BIPID, char *ImageHandle);
#endif

   /* The following function is responsible for sending a Delete Image  */
   /* Response to the remote Imaging Service Client.  The first         */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Imaging Profile Imaging Service Server making this call.    */
   /* The second parameter to this function is the BIP ID of the Basic  */
   /* Imaging Profile Imaging Service Server making this call.  The     */
   /* final parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI BIP_Delete_Image_Response(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BIP_Delete_Image_Response_t)(unsigned int BluetoothStackID, unsigned int BIPID, Byte_t ResponseCode);
#endif

#endif
