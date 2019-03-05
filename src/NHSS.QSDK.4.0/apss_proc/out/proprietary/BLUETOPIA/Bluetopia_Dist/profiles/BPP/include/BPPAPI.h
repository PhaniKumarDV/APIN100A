/*****< bppapi.h >*************************************************************/
/*      Copyright 2005 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BPPAPI - Stonestreet One Bluetooth Stack Basic Printing Profile Type      */
/*           Definitions and Constants.                                       */
/*                                                                            */
/*  Author:  Rory Sledge                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname       Description of Modification                  */
/*   --------  -----------       ---------------------------------------------*/
/*   10/03/05  R. Sledge         Initial creation.                            */
/******************************************************************************/
#ifndef __BPPAPIH__
#define __BPPAPIH__

#include "SS1BTPS.h"            /* Bluetooth Stack API Prototypes/Constants.  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define BTBPP_ERROR_INVALID_PARAMETER                             (-1000)
#define BTBPP_ERROR_NOT_INITIALIZED                               (-1001)
#define BTBPP_ERROR_INVALID_BLUETOOTH_STACK_ID                    (-1002)
#define BTBPP_ERROR_LIBRARY_INITIALIZATION_ERROR                  (-1003)
#define BTBPP_ERROR_INSUFFICIENT_RESOURCES                        (-1004)
#define BTBPP_ERROR_INVALID_OPERATION                             (-1005)
#define BTBPP_ERROR_REQUEST_ALREADY_OUTSTANDING                   (-1006)
#define BTBPP_ERROR_ACTION_NOT_ALLOWED                            (-1007)
#define BTBPP_ERROR_UNKNOWN_ERROR                                 (-1008)

   /* SDP Profile UUID's for the Basic Printing Profile Profile (BPP).  */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Bluetooth Universally Unique Identifier          */
   /* (BPP_PROFILE_UUID_16) to the specified UUID_16_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_16_t variable that  */
   /* is to receive the BPP_PROFILE_UUID_16 Constant value.             */
#define SDP_ASSIGN_BPP_PROFILE_UUID_16(_x)                       ASSIGN_SDP_UUID_16((_x), 0x11, 0x22)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Bluetooth Universally Unique Identifier          */
   /* (BPP_PROFILE_UUID_32) to the specified UUID_32_t variable.  This  */
   /* MACRO accepts one parameter which is the UUID_32_t variable that  */
   /* is to receive the BPP_PROFILE_UUID_32 Constant value.             */
#define SDP_ASSIGN_BPP_PROFILE_UUID_32(_x)                       ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x22)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Bluetooth Universally Unique Identifier          */
   /* (BPP_PROFILE_UUID_128) to the specified UUID_128_t variable.  This*/
   /* MACRO accepts one parameter which is the UUID_128_t variable that */
   /* is to receive the BPP_PROFILE_UUID_128 Constant value.            */
#define SDP_ASSIGN_BPP_PROFILE_UUID_128(_x)                      ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x22, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Reference Printing Service Class UUID's.                      */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reference Printing Service Class Bluetooth       */
   /* Universally Unique Identifier (BPP_REFERENCE_PRINTING_UUID_16) to */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* BPP_REFERENCE_PRINTING_UUID_16 Constant value.                    */
#define SDP_ASSIGN_BPP_REFERENCE_PRINTING_UUID_16(_x)            ASSIGN_SDP_UUID_16((_x), 0x11, 0x19)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reference Printing Service Class Bluetooth       */
   /* Universally Unique Identifier (BPP_REFERENCE_PRINTING_UUID_32) to */
   /* the specified UUID_32_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_32_t variable that is to receive the  */
   /* BPP_REFERENCE_PRINTING_UUID_32 Constant value.                    */
#define SDP_ASSIGN_BPP_REFERENCE_PRINTING_UUID_32(_x)            ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x19)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reference Printing Service Class Bluetooth       */
   /* Universally Unique Identifier (BPP_REFERENCE_PRINTING_UUID_128) to*/
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* BPP_REFERENCE_PRINTING_UUID_128 Constant value.                   */
#define SDP_ASSIGN_BPP_REFERENCE_PRINTING_UUID_128(_x)           ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x19, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Printing Status Service Class UUID's.                         */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Printing Status Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_PRINTING_STATUS_UUID_16) to the*/
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* BPP_PRINTING_STATUS_UUID_16 Constant value.                       */
#define SDP_ASSIGN_BPP_PRINTING_STATUS_UUID_16(_x)               ASSIGN_SDP_UUID_16((_x), 0x11, 0x23)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Printing Status Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_PRINTING_STATUS_UUID_32) to the*/
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* BPP_PRINTING_STATUS_UUID_32 Constant value.                       */
#define SDP_ASSIGN_BPP_PRINTING_STATUS_UUID_32(_x)               ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x23)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Printing Status Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_PRINTING_STATUS_UUID_128) to   */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* BPP_PRINTING_STATUS_UUID_128 Constant value.                      */
#define SDP_ASSIGN_BPP_PRINTING_STATUS_UUID_128(_x)              ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x23, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Direct Printing Service Class UUID's.                         */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_DIRECT_PRINTING_UUID_16) to the*/
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* BPP_DIRECT_PRINTING_UUID_16 Constant value.                       */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_UUID_16(_x)               ASSIGN_SDP_UUID_16((_x), 0x11, 0x18)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_DIRECT_PRINTING_UUID_32) to the*/
   /* specified UUID_32_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_32_t variable that is to receive the            */
   /* BPP_DIRECT_PRINTING_UUID_32 Constant value.                       */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_UUID_32(_x)               ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x18)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Service Class Bluetooth          */
   /* Universally Unique Identifier (BPP_DIRECT_PRINTING_UUID_128) to   */
   /* the specified UUID_128_t variable.  This MACRO accepts one        */
   /* parameter which is the UUID_128_t variable that is to receive the */
   /* BPP_DIRECT_PRINTING_UUID_128 Constant value.                      */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_UUID_128(_x)              ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x18, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Direct Printing Reference Object Service Service Class UUID's.*/

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Reference Object Service Class   */
   /* Bluetooth Universally Unique Identifier                           */
   /* (BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_16) to the specified   */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the                         */
   /* BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_16 Constant value.      */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_16(_x) ASSIGN_SDP_UUID_16((_x), 0x11, 0x20)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Reference Object Service Class   */
   /* Bluetooth Universally Unique Identifier                           */
   /* (BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_32) to the specified   */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the                         */
   /* BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_32 Constant value.      */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_32(_x) ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x20)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Direct Printing Reference Object Service Class   */
   /* Bluetooth Universally Unique Identifier                           */
   /* (BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_128) to the specified  */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_128 Constant value.     */
#define SDP_ASSIGN_BPP_DIRECT_PRINTING_REFERENCE_OBJECT_UUID_128(_x) ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x20, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* SDP Reflected UI Service Class UUID's.                            */

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reflected UI Service Class Bluetooth Universally */
   /* Unique Identifier (BPP_REFLECTED_UI_UUID_16) to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the BPP_REFLECTED_UI_UUID_16*/
   /* Constant value.                                                   */
#define SDP_ASSIGN_BPP_REFLECTED_UI_UUID_16(_x)                  ASSIGN_SDP_UUID_16((_x), 0x11, 0x21)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reflected UI Service Class Bluetooth Universally */
   /* Unique Identifier (BPP_REFLECTED_UI_UUID_32) to the specified     */
   /* UUID_32_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_32_t variable that is to receive the BPP_REFLECTED_UI_UUID_32*/
   /* Constant value.                                                   */
#define SDP_ASSIGN_BPP_REFLECTED_UI_UUID_32(_x)                  ASSIGN_SDP_UUID_32((_x), 0x00, 0x00, 0x11, 0x21)

   /* The following MACRO is a utility MACRO that assigns the Basic     */
   /* Printing Profile Reflected UI Service Class Bluetooth Universally */
   /* Unique Identifier (BPP_REFLECTED_UI_UUID_128) to the specified    */
   /* UUID_128_t variable.  This MACRO accepts one parameter which is   */
   /* the UUID_128_t variable that is to receive the                    */
   /* BPP_REFLECTED_UI_UUID_128 Constant value.                         */
#define SDP_ASSIGN_BPP_REFLECTED_UI_UUID_128(_x)                 ASSIGN_SDP_UUID_128((_x), 0x00, 0x00, 0x11, 0x21, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)

   /* Defines the Profile Version Number used within the SDP Record for */
   /* Basic Printing Profile Servers.                                   */
#define BPP_PROFILE_VERSION                                      (0x0102)

   /* The following define the possible OBEX Response codes.            */
   /* * NOTE * Response Codes less than 0x10 (OBEX_CONTINUE_RESPONSE)   */
   /*          are Reserved and CANNOT be used.                         */
#define BPP_OBEX_RESPONSE_CONTINUE                               (OBEX_CONTINUE_RESPONSE)
#define BPP_OBEX_RESPONSE_OK                                     (OBEX_OK_RESPONSE)
#define BPP_OBEX_RESPONSE_CREATED                                (OBEX_CREATED_RESPONSE)
#define BPP_OBEX_RESPONSE_ACCEPTED                               (OBEX_ACCEPTED_RESPONSE)
#define BPP_OBEX_RESPONSE_NON_AUTHORITATIVE_INFORMATION          (OBEX_NON_AUTHORITATIVE_INFORMATION_RESPONSE)
#define BPP_OBEX_RESPONSE_NO_CONTENT                             (OBEX_NO_CONTENT_RESPONSE)
#define BPP_OBEX_RESPONSE_RESET_CONTENT                          (OBEX_RESET_CONTENT_RESPONSE)
#define BPP_OBEX_RESPONSE_PARTIAL_CONTENT                        (OBEX_PARTIAL_CONTENT_RESPONSE)
#define BPP_OBEX_RESPONSE_MULTIPLE_CHOICES                       (OBEX_MULTIPLE_CHOICES_RESPONSE)
#define BPP_OBEX_RESPONSE_MOVED_PERMANENTLY                      (OBEX_MOVED_PERMANETLY_RESPONSE)
#define BPP_OBEX_RESPONSE_MOVED_TEMPORARILY                      (OBEX_MOVED_TEMPORARILY_RESPONSE)
#define BPP_OBEX_RESPONSE_SEE_OTHER                              (OBEX_SEE_OTHER_RESPONSE)
#define BPP_OBEX_RESPONSE_NOT_MODIFIED                           (OBEX_NOT_MODIFIED_RESPONSE)
#define BPP_OBEX_RESPONSE_USE_PROXY                              (OBEX_USE_PROXY_RESPONSE)
#define BPP_OBEX_RESPONSE_BAD_REQUEST                            (OBEX_BAD_REQUEST_RESPONSE)
#define BPP_OBEX_RESPONSE_UNAUTHORIZED                           (OBEX_UNAUTHORIZED_RESPONSE)
#define BPP_OBEX_RESPONSE_PAYMENT_REQUIRED                       (OBEX_PAYMENT_REQUIRED_RESPONSE)
#define BPP_OBEX_RESPONSE_FORBIDDEN                              (OBEX_FORBIDDEN_RESPONSE)
#define BPP_OBEX_RESPONSE_NOT_FOUND                              (OBEX_NOT_FOUND_RESPONSE)
#define BPP_OBEX_RESPONSE_METHOD_NOT_ALLOWED                     (OBEX_METHOD_NOT_ALLOWED_RESPONSE)
#define BPP_OBEX_RESPONSE_NOT_ACCEPTABLE                         (OBEX_NOT_ACCEPTABLE_RESPONSE)
#define BPP_OBEX_RESPONSE_PROXY_AUTHENTICATION_REQUIRED          (OBEX_PROXY_AUTHENTICATION_REQUIRED_RESPONSE)
#define BPP_OBEX_RESPONSE_REQUEST_TIMEOUT                        (OBEX_REQUEST_TIMEOUT_RESPONSE)
#define BPP_OBEX_RESPONSE_CONFLICT                               (OBEX_CONFLICT_RESPONSE)
#define BPP_OBEX_RESPONSE_GONE                                   (OBEX_GONE_RESPONSE)
#define BPP_OBEX_RESPONSE_LENGTH_REQUIRED                        (OBEX_LENGTH_REQUIRED_RESPONSE)
#define BPP_OBEX_RESPONSE_PRECONDITION_FAILED                    (OBEX_PRECONDITION_FAILED_RESPONSE)
#define BPP_OBEX_RESPONSE_REQUESTED_ENTITY_TOO_LARGE             (OBEX_REQUESTED_ENTITY_TOO_LARGE_RESPONSE)
#define BPP_OBEX_RESPONSE_REQUESTED_URL_TOO_LARGE                (OBEX_REQUESTED_URL_TOO_LARGE_RESPONSE)
#define BPP_OBEX_RESPONSE_UNSUPPORTED_MEDIA_TYPE                 (OBEX_UNSUPPORTED_MEDIA_TYPE_RESPONSE)
#define BPP_OBEX_RESPONSE_INTERNAL_SERVER_ERROR                  (OBEX_INTERNAL_SERVER_ERROR_RESPONSE)
#define BPP_OBEX_RESPONSE_NOT_IMPLEMENTED                        (OBEX_NOT_IMPLEMENTED_RESPONSE)
#define BPP_OBEX_RESPONSE_BAD_GATEWAY                            (OBEX_BAD_GATEWAY_RESPONSE)
#define BPP_OBEX_RESPONSE_SERVICE_UNAVAILABLE                    (OBEX_SERVICE_UNAVAILABLE_RESPONSE)
#define BPP_OBEX_RESPONSE_GATEWAY_TIMEOUT                        (OBEX_GATEWAY_TIMEOUT_RESPONSE)
#define BPP_OBEX_RESPONSE_HTTP_VERSION_NOT_SUPPORTED             (OBEX_HTTP_VERSION_NOT_SUPPORTED_RESPONSE)
#define BPP_OBEX_RESPONSE_DATABASE_FULL                          (OBEX_DATABASE_FULL_RESPONSE)
#define BPP_OBEX_RESPONSE_DATABASE_LOCKED                        (OBEX_DATABASE_LOCKED_RESPONSE)

   /* The following define the possible operation status codes.         */
#define BPP_OPERATION_STATUS_SUCCESSFUL_OK                       (0x0000)
#define BPP_OPERATION_STATUS_SUCCESSFUL_OK_IGNORED_OR_SUBSTITUTED_ATTRIBUTES (0x0001)
#define BPP_OPERATION_STATUS_SUCCESSFUL_OK_CONFLICTING_ATTRIBUTES (0x0002)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_BAD_REQUEST            (0x0400)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_FORBIDDEN              (0x0401)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_NOT_AUTHENTICATED      (0x0402)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_NOT_AUTHORIZED         (0x0403)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_NOT_POSSIBLE           (0x0404)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_TIMEOUT                (0x0405)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_NOT_FOUND              (0x0406)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_GONE                   (0x0407)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_REQUEST_ENTITY_TOO_LARGE (0x0408)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_REQUEST_VALUE_TOO_LONG (0x0409)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_DOCUMENT_FORMAT_NOT_SUPPORTED (0x040A)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_ATTRIBUTES_OR_VALUES_NOT_SUPPORTED (0x040B)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_URL_SCHEME_NOT_SUPPORTED (0x040C)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_CHARSET_NOT_SUPPORTED  (0x040D)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_CONFLICTING_ATTRIBUTES (0x040E)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_COMPRESSION_NOT_SUPPORTED (0x040F)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_COMPRESSION_ERROR      (0x0410)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_DOCUMENT_FORMAT_ERROR  (0x0411)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_DOCUMENT_ACCESS_ERROR  (0x0412)
#define BPP_OPERATION_STATUS_CLIENT_ERROR_ERROR_MEDIA_NOT_LOADED (0x0418)
#define BPP_OPERATION_STATUS_SERVER_ERROR_INTERNAL_ERROR         (0x0500)
#define BPP_OPERATION_STATUS_SERVER_ERROR_OPERATION_NOT_SUPPORTED (0x0501)
#define BPP_OPERATION_STATUS_SERVER_ERROR_SERVICE_UNAVAILABLE    (0x0502)
#define BPP_OPERATION_STATUS_SERVER_ERROR_VERSION_NOT_SUPPORTED  (0x0503)
#define BPP_OPERATION_STATUS_SERVER_ERROR_DEVICE_ERROR           (0x0504)
#define BPP_OPERATION_STATUS_SERVER_ERROR_TEMPORARY_ERROR        (0x0505)
#define BPP_OPERATION_STATUS_SERVER_ERROR_NOT_ACCEPTING_JOBS     (0x0506)
#define BPP_OPERATION_STATUS_SERVER_ERROR_BUSY                   (0x0507)
#define BPP_OPERATION_STATUS_SERVER_ERROR_JOB_CANCELED           (0x0508)
#define BPP_OPERATION_STATUS_SERVER_ERROR_MULTIPLE_DOCUMENT_JOBS_NOT_SUPPORTED (0x0509)

   /* The following bits represent the possible values that may be      */
   /* passed into the BPP_Open_Print_Server() functions                 */
   /* JobServerPortTargetsMask parameter to indicate which Targets shall*/
   /* be allowed to connect using the Job Server Port.                  */
   /* ** NOTE ** Currently only support for Direct Printing exists in   */
   /*            the current implementation of this profile.  The other */
   /*            bits have be defined merely as a convenience for future*/
   /*            implementation.                                        */
#define BPP_JOB_SERVER_PORT_TARGET_DIRECT_PRINTING_SERVICE_BIT   (0x00000001)
#define BPP_JOB_SERVER_PORT_TARGET_REFERENCE_PRINTING_SERVICE_BIT (0x00000002)
#define BPP_JOB_SERVER_PORT_TARGET_ADMINISTRATIVE_USER_INTERFACE_SERVICE_BIT (0x00000004)

   /* The following constants represent the Minimum and Maximum Port    */
   /* Numbers that can be opened (both locally and remotely).  These    */
   /* constants specify the range for the Port Number parameters in the */
   /* Open Port Functions.                                              */
#define BPP_PORT_NUMBER_MINIMUM                                  (SPP_PORT_NUMBER_MINIMUM)
#define BPP_PORT_NUMBER_MAXIMUM                                  (SPP_PORT_NUMBER_MAXIMUM)

   /* The following constant represent the invalid value that may be    */
   /* used with the BPP_Open_Print_Server() and                         */
   /* BPP_Open_Remote_Print_Server() functions StatusServerPort         */
   /* parameter if a status channel is not required by the applications */
   /* making these calls.                                               */
#define BPP_STATUS_SERVER_PORT_INVALID_VALUE                     (0xFF)

   /* The following enumerated type defines the possible Targets that   */
   /* may be connected to on a Print Servers Job Server Port.  These    */
   /* values should be used with the BPP_Open_Remote_Print_Server()     */
   /* functions JobServerPortTarget parameter.                          */
typedef enum
{
   jtDirectPrintingService,
   jtReferencedPrintingService,
   jtAdministrativeUserInterfaceService
} BPP_Job_Server_Port_Target_t;

   /* The following BIT definitions are used to denote the possible     */
   /* Basic Printing Profile Server Modes that can be applied to a BPP  */
   /* Client Connection.  These BIT definitions are used with the       */
   /* BPP_Set_Server_Mode() and BPP_Get_Server_Mode() mode functions.   */
#define BPP_SERVER_MODE_JOB_SERVER_PORT_AUTOMATIC_ACCEPT_CONNECTION               (0x00000000)
#define BPP_SERVER_MODE_JOB_SERVER_PORT_MANUAL_ACCEPT_CONNECTION                  (0x00000001)
#define BPP_SERVER_MODE_JOB_SERVER_PORT_CONNECTION_MASK                           (0x00000001)
#define BPP_SERVER_MODE_STATUS_SERVER_PORT_AUTOMATIC_ACCEPT_CONNECTION            (0x00000000)
#define BPP_SERVER_MODE_STATUS_SERVER_PORT_MANUAL_ACCEPT_CONNECTION               (0x00000002)
#define BPP_SERVER_MODE_STATUS_SERVER_PORT_CONNECTION_MASK                        (0x00000002)
#define BPP_SERVER_MODE_REFERENCED_OBJECT_SERVER_PORT_AUTOMATIC_ACCEPT_CONNECTION (0x00000000)
#define BPP_SERVER_MODE_REFERENCED_OBJECT_SERVER_PORT_MANUAL_ACCEPT_CONNECTION    (0x00000004)
#define BPP_SERVER_MODE_REFERENCED_OBJECT_SERVER_PORT_CONNECTION_MASK             (0x00000004)

   /* The following constants represent the Open Status Values that are */
   /* possible for the OpenStatus member of the Open Confirmation       */
   /* events.                                                           */
#define BPP_OPEN_STATUS_SUCCESS                                  (0x00)
#define BPP_OPEN_STATUS_CONNECTION_TIMEOUT                       (0x01)
#define BPP_OPEN_STATUS_CONNECTION_REFUSED                       (0x02)
#define BPP_OPEN_STATUS_UNKNOWN_ERROR                            (0x04)

   /* The following constants represent the Job Id minimum, maximum and */
   /* invalid values that may be used with the functions and events     */
   /* using Basic Printing Profile Job Based Printing Job Ids.          */
#define BPP_JOB_ID_MINIMUM_VALUE                                 (1)
#define BPP_JOB_ID_MAXIMUM_VALUE                                 (0xFFFFFFFF)
#define BPP_JOB_ID_INVALID_VALUE                                 (0)

   /* The following constants represent the Count minimum, maximum and  */
   /* value used to request the remainder of the object.                */
#define BPP_COUNT_MINIMUM_VALUE                                 (0)
#define BPP_COUNT_MAXIMUM_VALUE                                 (0x7FFFFFFF)
#define BPP_COUNT_REMAINDER_OF_OBJECT_VALUE                     ((DWord_t)(-1))

   /* The following constants represent the Object Size minimum,        */
   /* maximum, invalid, and unknown object size values that may be used */
   /* with the functions and events that use the Object Size value.     */
#define BPP_OBJECT_SIZE_MINIMUM_VALUE                           (0)
#define BPP_OBJECT_SIZE_MAXIMUM_VALUE                           (0x7FFFFFFF)
#define BPP_OBJECT_SIZE_UNKNOWN_VALUE                           ((DWord_t)(-1))
#define BPP_OBJECT_SIZE_INVALID_VALUE                           ((DWord_t)(-2))

   /* The following type definition is to be used with the              */
   /* BPP_Register_Print_Server_SDP_Record() function to describe the   */
   /* Character Repertoires Supported.                                  */
   /* ** NOTE ** The most current version of the bit assignments for the*/
   /*            Character Repertoires Supported field may be found in  */
   /*            the Host Operating Environment Identifiers section of  */
   /*            the Bluetooth Assigned Numbers Document.               */
typedef Byte_t BPP_Character_Repertoires_t[16];

   /* The following structure is used with the                          */
   /* BPP_Register_Print_Server_SDP_Record() function to describe the   */
   /* IEEE 1284ID String that should be added to the SDP Record for the */
   /* Basic Printing Profile Print Server.                              */
typedef struct _tagBPP_1284_ID_t
{
   Word_t  IEEE_1284IDLength;
   Byte_t *IEEE_1284IDString;
} BPP_1284_ID_t;

#define BPP_1284_ID_SIZE                           (sizeof(BPP_1284_ID_t))

   /* The following enumerated type represents all the allowable SOAP   */
   /* Data Element Types that can be used with the BPP API.             */
typedef enum
{
   stTextString,
   stSequence
} BPP_SOAP_Data_Element_Type_t;

   /* The following Data Structure represents a structure that will hold*/
   /* an individual BPP SOAP Data Element.  The Name Length member holds*/
   /* the length of the Name pointed to by the Name member.  The Name   */
   /* member holds the name of the data element.  The ValueType member  */
   /* holds the type of the value data specified.  The ValueLength      */
   /* member holds the number of bytes that the actual data element     */
   /* value occupies (this value repesnets the buffer size that the     */
   /* pointer member of the union points to).                           */
   /* * NOTE * The following structure also supports the Sequence Data  */
   /*          Type.  This is treated as any of the other Data Element  */
   /*          Types, the ValueLength member denotes the Number of the  */
   /*          Data Elements that the DataSequence Member points to.    */
typedef struct _tagBPP_SOAP_Data_Element_t
{
   unsigned int                  NameLength;
   Byte_t                       *Name;
   BPP_SOAP_Data_Element_Type_t  ValueType;
   unsigned int                  ValueLength;
   union
   {
      Byte_t                             *TextString;
      struct _tagBPP_SOAP_Data_Element_t *DataSequence;
   } ValueData;
} BPP_SOAP_Data_Element_t;

#define BPP_SOAP_DATA_ELEMENT_SIZE                 (sizeof(BPP_SOAP_Data_Element_t))

   /* Basic Printing Profile Event API Types.                           */
typedef enum
{
   etBPP_Open_Request_Indication,
   etBPP_Open_Job_Port_Indication,
   etBPP_Open_Status_Port_Indication,
   etBPP_Open_Referenced_Object_Port_Indication,
   etBPP_Open_Job_Port_Confirmation,
   etBPP_Open_Status_Port_Confirmation,
   etBPP_Open_Referenced_Object_Port_Confirmation,
   etBPP_Close_Job_Port_Indication,
   etBPP_Close_Status_Port_Indication,
   etBPP_Close_Referenced_Object_Port_Indication,
   etBPP_Abort_Indication,
   etBPP_Abort_Confirmation,
   etBPP_File_Push_Indication,
   etBPP_File_Push_Confirmation,
   etBPP_Get_Referenced_Objects_Indication,
   etBPP_Get_Referenced_Objects_Confirmation,
   etBPP_Get_Printer_Attributes_Indication,
   etBPP_Get_Printer_Attributes_Confirmation,
   etBPP_Create_Job_Indication,
   etBPP_Create_Job_Confirmation,
   etBPP_Send_Document_Indication,
   etBPP_Send_Document_Confirmation,
   etBPP_Get_Job_Attributes_Indication,
   etBPP_Get_Job_Attributes_Confirmation,
   etBPP_Cancel_Job_Indication,
   etBPP_Cancel_Job_Confirmation,
   etBPP_Get_Event_Indication,
   etBPP_Get_Event_Confirmation
} BPP_Event_Type_t;

   /* The following event is dispatched when a remote client requests a */
   /* connection to a local server.  The BPPID member specifies the     */
   /* identifier of the Local Basic Printing Profile Server being       */
   /* connect with.  The ServerPort member specifies the server port of */
   /* that Local Basic Printing Profile Server being connected with.    */
   /* The BD_ADDR member specifies the address of the Remote Client     */
   /* requesting the connection to the Local Server.                    */
   /* ** NOTE ** This event is only dispatched to servers that are in   */
   /*            Manual Accept Mode.                                    */
   /* ** NOTE ** This event must be responded to with the               */
   /*            BPP_Open_Request_Response() function in order to accept*/
   /*            or reject the outstanding Open Request.                */
typedef struct _tagBPP_Open_Request_Indication_Data_t
{
   unsigned int BPPID;
   unsigned int ServerPort;
   BD_ADDR_t    BD_ADDR;
} BPP_Open_Request_Indication_Data_t;

#define BPP_OPEN_REQUEST_INDICATION_DATA_SIZE           (sizeof(BPP_Open_Request_Indication_Data_t))

   /* The following event is dispatched when a Basic Printing Profile   */
   /* Sender makes a Connection to a Registered Basic Printing Profile  */
   /* Print Server Job Port.  The BPPID member specifies the identifier */
   /* of the Local Basic Printing Profile Print Server that is being    */
   /* connected with.  The BD_ADDR member specifies the address of the  */
   /* Basic Printing Profile Sender that is being connected to the Local*/
   /* Print Server.  The JobServerPortTarget member specifies that      */
   /* Target Service that was connected with on the Local Print Server. */
typedef struct _tagBPP_Open_Job_Port_Indication_Data_t
{
   unsigned int                 BPPID;
   BD_ADDR_t                    BD_ADDR;
   BPP_Job_Server_Port_Target_t JobServerPortTarget;
} BPP_Open_Job_Port_Indication_Data_t;

#define BPP_OPEN_JOB_PORT_INDICATION_DATA_SIZE          (sizeof(BPP_Open_Job_Port_Indication_Data_t))

   /* The following event is dispatched when a Basic Printing Profile   */
   /* Sender makes a Connection to a Registered Basic Printing Profile  */
   /* Print Server Status Port.  The BPPID member specifies the         */
   /* identifier of the Local Basic Printing Profile Print Server that  */
   /* is being connected with.  The BD_ADDR member specifies the address*/
   /* of the Basic Printing Profile Sender that is being connected to   */
   /* the Local Print Server.                                           */
typedef struct _tagBPP_Open_Status_Port_Indication_Data_t
{
   unsigned int BPPID;
   BD_ADDR_t    BD_ADDR;
} BPP_Open_Status_Port_Indication_Data_t;

#define BPP_OPEN_STATUS_PORT_INDICATION_DATA_SIZE       (sizeof(BPP_Open_Status_Port_Indication_Data_t))

   /* The following event is dispatched when a Basic Printing Profile   */
   /* Printer makes a Connection to a Registered Basic Printing Profile */
   /* Server Server Referenced Object Port.  The BPPID member specifies */
   /* the identifier of the Local Basic Printing Profile Sender Server  */
   /* that is being connected with.  The BD_ADDR member specifies the   */
   /* address of the Basic Printing Profile Printer that is being       */
   /* connected to the Local Sender Server.                             */
typedef struct _tagBPP_Open_Referenced_Object_Port_Indication_Data_t
{
   unsigned int BPPID;
   BD_ADDR_t    BD_ADDR;
} BPP_Open_Referenced_Object_Port_Indication_Data_t;

#define BPP_OPEN_REFERENCED_OBJECT_PORT_INDICATION_DATA_SIZE (sizeof(BPP_Open_Referenced_Object_Port_Indication_Data_t))

   /* The following event is dispatched to the Local Basic Printing     */
   /* Profile Sender to indicate the success or failure of a previously */
   /* submitted Connection Attempt to a Job Server Port.  The BPPID     */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender that has requested the Connection.  The OpenStatus */
   /* member specifies the status of the Connection Attempt.  Valid     */
   /* values are:                                                       */
   /*    - BPP_OPEN_STATUS_SUCCESS                                      */
   /*    - BPP_OPEN_STATUS_CONNECTION_TIMEOUT                           */
   /*    - BPP_OPEN_STATUS_CONNECTION_REFUSED                           */
   /*    - BPP_OPEN_STATUS_UNKNOWN_ERROR                                */
typedef struct _tagBPP_Open_Job_Port_Confirmation_Data_t
{
   unsigned int  BPPID;
   unsigned long OpenStatus;
} BPP_Open_Job_Port_Confirmation_Data_t;

#define BPP_OPEN_JOB_PORT_CONFIRMATION_DATA_SIZE        (sizeof(BPP_Open_Job_Port_Confirmation_Data_t))

   /* The following event is dispatched to the Local Basic Printing     */
   /* Profile Sender to indicate the success or failure of a previously */
   /* submitted Connection Attempt to a Status Server Port.  The BPPID  */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender that has requested the Connection.  The OpenStatus */
   /* member specifies the status of the Connection Attempt.  Valid     */
   /* values are:                                                       */
   /*    - BPP_OPEN_STATUS_SUCCESS                                      */
   /*    - BPP_OPEN_STATUS_CONNECTION_TIMEOUT                           */
   /*    - BPP_OPEN_STATUS_CONNECTION_REFUSED                           */
   /*    - BPP_OPEN_STATUS_UNKNOWN_ERROR                                */
typedef struct _tagBPP_Open_Status_Port_Confirmation_Data_t
{
   unsigned int  BPPID;
   unsigned long OpenStatus;
} BPP_Open_Status_Port_Confirmation_Data_t;

#define BPP_OPEN_STATUS_PORT_CONFIRMATION_DATA_SIZE     (sizeof(BPP_Open_Status_Port_Confirmation_Data_t))

   /* The following event is dispatched to the Local Basic Printing     */
   /* Profile Printer to indicate the success or failure of a previously*/
   /* submitted Connection Attempt to a Referenced Object Server Port.  */
   /* The BPPID member specifies the Identifier of the Local Basic      */
   /* Printing Profile Printer that has requested the Connection.  The  */
   /* OpenStatus member specifies the status of the Connection Attempt. */
   /* Valid values are:                                                 */
   /*    - BPP_OPEN_STATUS_SUCCESS                                      */
   /*    - BPP_OPEN_STATUS_CONNECTION_TIMEOUT                           */
   /*    - BPP_OPEN_STATUS_CONNECTION_REFUSED                           */
   /*    - BPP_OPEN_STATUS_UNKNOWN_ERROR                                */
typedef struct _tagBPP_Open_Referenced_Object_Port_Confirmation_Data_t
{
   unsigned int  BPPID;
   unsigned long OpenStatus;
} BPP_Open_Referenced_Object_Port_Confirmation_Data_t;

#define BPP_OPEN_REFERENCED_OBJECT_PORT_CONFIRMATION_DATA_SIZE (sizeof(BPP_Open_Referenced_Object_Port_Confirmation_Data_t))

   /* The following event is dispatched when the Basic Printing Profile */
   /* Sender disconnects from the local Basic Printing Profile Print    */
   /* Server or Client Job Port.  The BPPID member specifies the        */
   /* Identifier for the Local Basic Printing Profile connection being  */
   /* closed.                                                           */
typedef struct _tagBPP_Close_Job_Port_Indication_Data_t
{
   unsigned int BPPID;
} BPP_Close_Job_Port_Indication_Data_t;

#define BPP_CLOSE_JOB_PORT_INDICATION_DATA_SIZE         (sizeof(BPP_Close_Job_Port_Indication_Data_t))

   /* The following event is dispatched when the Basic Printing Profile */
   /* Sender disconnects from the local Basic Printing Profile Print    */
   /* Server or Client Status Port.  The BPPID member specifies the     */
   /* Identifier for the Local Basic Printing Profile connection being  */
   /* closed.                                                           */
typedef struct _tagBPP_Close_Status_Port_Indication_Data_t
{
   unsigned int BPPID;
} BPP_Close_Status_Port_Indication_Data_t;

#define BPP_CLOSE_STATUS_PORT_INDICATION_DATA_SIZE      (sizeof(BPP_Close_Status_Port_Indication_Data_t))

   /* The following event is dispatched when the Basic Printing Profile */
   /* Sender disconnects from the local Basic Printing Profile Print    */
   /* Server or Client Referenced Object Port.  The BPPID member        */
   /* specifies the Identifier for the Local Basic Printing Profile     */
   /* connection being closed.                                          */
typedef struct _tagBPP_Close_Referenced_Object_Port_Indication_Data_t
{
   unsigned int BPPID;
} BPP_Close_Referenced_Object_Port_Indication_Data_t;

#define BPP_CLOSE_REFERENCED_OBJECT_PORT_INDICATION_DATA_SIZE (sizeof(BPP_Close_Referenced_Object_Port_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Server Port when the Client sends an Abort Request.  The BPPID    */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Server receiving this event.  The ServerPort member       */
   /* specifies the server port in which the Abort Request was received.*/
typedef struct _tagBPP_Abort_Indication_Data_t
{
   unsigned int BPPID;
   unsigned int ServerPort;
} BPP_Abort_Indication_Data_t;

#define BPP_ABORT_INDICATION_DATA_SIZE                  (sizeof(BPP_Abort_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Client when the Server sends an Abort Response.  The BPPID member */
   /* specifies the Identifier of the Local Basic Printing Profile      */
   /* Client receiving this event.  The ServerPort member specifies the */
   /* server port in which the Abort Response was received.             */
typedef struct _tagBPP_Abort_Confirmation_Data_t
{
   unsigned int BPPID;
   unsigned int ServerPort;
} BPP_Abort_Confirmation_Data_t;

#define BPP_ABORT_CONFIRMATION_DATA_SIZE                (sizeof(BPP_Abort_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Print Server when the Sender sends a File Push Request.  The BPPID*/
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Printer receiving this event.  The MIMEMediaType member   */
   /* specifies the type of the file being pushed with this File Push   */
   /* Request.  The FileName member specifies the name of the file being*/
   /* pushed with this File Push Request.  The Description member       */
   /* specifies the description of the file being pushed and is         */
   /* typically used to specify document type-dependent information.    */
   /* The DataLength member specifies the length of data pointed to by  */
   /* the DataBuffer member.  The DataBuffer member points to a buffer  */
   /* containing the actual file data.  The Final member specifies if   */
   /* this indication completes the file push operation.                */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BPP_File_Push_Response() function.                     */
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
   /* ** NOTE ** The FileName and Description members are optional and  */
   /*            may not be included for the duration of a File Push    */
   /*            Operation.                                             */
typedef struct _tagBPP_File_Push_Indication_Data_t
{
   unsigned int   BPPID;
   char          *MIMEMediaType;
   char          *FileName;
   char          *Description;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
   Boolean_t      Final;
} BPP_File_Push_Indication_Data_t;

#define BPP_FILE_PUSH_INDICATION_DATA_SIZE              (sizeof(BPP_File_Push_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a File Push Response.  The BPPID    */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender receiving this event.  The ResponseCode member     */
   /* specifies the response code associated with the File Push Response*/
   /* that generated this event.                                        */
typedef struct _tagBPP_File_Push_Confirmation_Data_t
{
   unsigned int BPPID;
   Byte_t       ResponseCode;
} BPP_File_Push_Confirmation_Data_t;

#define BPP_FILE_PUSH_CONFIRMATION_DATA_SIZE            (sizeof(BPP_File_Push_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender Server when the Printer sends a Get Referenced Objects     */
   /* Request.  The BPPID member specifies the Identifier of the Local  */
   /* Basic Printing Profile Sender receiving this event.  The          */
   /* ObjectIdentifier member specifies the Name of the Object being    */
   /* request by this Get Referenced Objects Request.  The Offset member*/
   /* specifies the byte offset into the object being requested by this */
   /* Get Referenced Objects Request.  The Count member specifies the   */
   /* number of bytes to being requested to send by this Get Referenced */
   /* Objects Request.  RequestObjectSize member is a BOOLEAN indicating*/
   /* if this requests the return of the total object size in the       */
   /* response.                                                         */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BPP_Get_Referenced_Objects_Response() function.        */
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
typedef struct _tagBPP_Get_Referenced_Objects_Indication_Data_t
{
   unsigned int  BPPID;
   char         *ObjectIdentifier;
   DWord_t       Offset;
   DWord_t       Count;
   Boolean_t     RequestFileSize;
} BPP_Get_Referenced_Objects_Indication_Data_t;

#define BPP_GET_REFERENCED_OBJECTS_INDICATION_DATA_SIZE (sizeof(BPP_Get_Referenced_Objects_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Get Referenced Objects Response.  */
   /* The BPPID member specifies the Identifier of the Local Basic      */
   /* Printing Profile Printer receiving this event.  The ResponseCode  */
   /* member specifies the response code associated with the Get        */
   /* Referenced Objects Response that generated this event.  The       */
   /* ObjectSize member specifies the total length of the referenced    */
   /* object in bytes.  The DataLength member specifies the length of   */
   /* data pointed to by the DataBuffer member.  The DataBuffer member  */
   /* points to a buffer containing the actual referenced object data.  */
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
typedef struct _tagBPP_Get_Referenced_Objects_Confirmation_Data_t
{
   unsigned int   BPPID;
   Byte_t         ResponseCode;
   DWord_t        ObjectSize;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
} BPP_Get_Referenced_Objects_Confirmation_Data_t;

#define BPP_GET_REFERENCED_OBJECTS_CONFIRMATION_DATA_SIZE (sizeof(BPP_Get_Referenced_Objects_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Get Printer Attributes Request.   */
   /* The BPPID member specifies the Identifier of the Local Basic      */
   /* Printing Profile Printer receiving this event.  The ServerPort    */
   /* member specifies the Identifier of the local Server Port (Job or  */
   /* Status) in which the request was received.  The NumberDataElements*/
   /* member specifies the number of SOAP Data Elements pointed to by   */
   /* the DataElementBuffer member.  The DataElementBuffer points to a  */
   /* buffer containing the SOAP data specified for this Get Printer    */
   /* Attributes Request.                                               */
typedef struct _tagBPP_Get_Printer_Attributes_Indication_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Printer_Attributes_Indication_Data_t;

#define BPP_GET_PRINTER_ATTRIBUTES_INDICATION_DATA_SIZE (sizeof(BPP_Get_Printer_Attributes_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Get Printer Attributes Response.  */
   /* The BPPID member specifies the Identifier of the Local Basic      */
   /* Printing Profile Sender receiving this event.  The ServerPort     */
   /* member specifies the server port (Job or Status) in which the     */
   /* response was received.  The ResponseCode member specifies the     */
   /* response code associated with the Get Printer Attributes Response */
   /* that generated this event.  The NumberDataElements member         */
   /* specifies the number of SOAP Data Elements pointed to by the      */
   /* DataElementBuffer member.  The DataElementBuffer points to a      */
   /* buffer containing the SOAP data specified for this Get Printer    */
   /* Attributes Response.                                              */
   /* ** NOTE ** Response Codes other then those indicating success will*/
   /*            have invalid values for the NumberDataElements and     */
   /*            DataElementBuffer members.  Therefore it is required   */
   /*            that parameter checking be used to validate the members*/
   /*            of this event that may not exist in case of a response */
   /*            indicating an error.                                   */
typedef struct _tagBPP_Get_Printer_Attributes_Confirmation_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   Byte_t                   ResponseCode;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Printer_Attributes_Confirmation_Data_t;

#define BPP_GET_PRINTER_ATTRIBUTES_CONFIRMATION_DATA_SIZE (sizeof(BPP_Get_Printer_Attributes_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Create Job Request.  The BPPID    */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Printer receiving this event.  The NumberDataElements     */
   /* member specifies the number of SOAP Data Elements pointed to by   */
   /* the DataElementBuffer member.  The DataElementBuffer points to a  */
   /* buffer containing the SOAP data specified for this Create Job     */
   /* Request.                                                          */
typedef struct _tagBPP_Create_Job_Indication_Data_t
{
   unsigned int             BPPID;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Create_Job_Indication_Data_t;

#define BPP_CREATE_JOB_INDICATION_DATA_SIZE             (sizeof(BPP_Create_Job_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Create Job Response.  The BPPID   */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender receiving this event.  The ResponseCode member     */
   /* specifies the response code associated with the Create Job        */
   /* Response that generated this event.  The JobId member specifies   */
   /* the Job Id associated with the Create Job Response that generated */
   /* this event.  The NumberDataElements member specifies the number of*/
   /* SOAP Data Elements pointed to by the DataElementBuffer member.    */
   /* The DataElementBuffer points to a buffer containing the SOAP data */
   /* specified for this Create Job Response.                           */
   /* ** NOTE ** Response Codes other then those indicating success will*/
   /*            have invalid values for the NumberDataElements and     */
   /*            DataElementBuffer members.  Therefore it is required   */
   /*            that parameter checking be used to validate the members*/
   /*            of this event that may not exist in case of a response */
   /*            indicating an error.                                   */
typedef struct _tagBPP_Create_Job_Confirmation_Data_t
{
   unsigned int             BPPID;
   Byte_t                   ResponseCode;
   DWord_t                  JobId;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Create_Job_Confirmation_Data_t;

#define BPP_CREATE_JOB_CONFIRMATION_DATA_SIZE           (sizeof(BPP_Create_Job_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Print Server when the Sender sends a Send Document Request.  The  */
   /* BPPID member specifies the Identifier of the Local Basic Printing */
   /* Profile Printer receiving this event.  The JobId member specifies */
   /* the Job Id associated with this Send Document Request.  The       */
   /* MIMEMediaType member specifies the type of the document being     */
   /* pushed with this Send Document Request.  The FileName member      */
   /* specifies the name of the document being sent with this Send      */
   /* Document Request.  The Description member specifies the           */
   /* description of the document being sent and is typically used to   */
   /* specify document type-dependent information.  The DataLength      */
   /* member specifies the length of data pointed to by the DataBuffer  */
   /* member.  The DataBuffer member points to a buffer containing the  */
   /* actual document data.  The Final member specifies if this         */
   /* indication completes the send document operation.                 */
   /* ** NOTE ** This event should be responded to with the             */
   /*            BPP_Send_Document_Response() function.                 */
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
   /* ** NOTE ** The FileName and Description members are optional and  */
   /*            may not be included for the duration of a Send Document*/
   /*            Operation.                                             */
typedef struct _tagBPP_Send_Document_Indication_Data_t
{
   unsigned int   BPPID;
   DWord_t        JobId;
   char          *MIMEMediaType;
   char          *FileName;
   char          *Description;
   unsigned int   DataLength;
   unsigned char *DataBuffer;
   Boolean_t      Final;
} BPP_Send_Document_Indication_Data_t;

#define BPP_SEND_DOCUMENT_INDICATION_DATA_SIZE          (sizeof(BPP_Send_Document_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Send Document Response.  The BPPID*/
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender receiving this event.  The ResponseCode member     */
   /* specifies the response code associated with the Send Document     */
   /* Response that generated this event.                               */
typedef struct _tagBPP_Send_Document_Confirmation_Data_t
{
   unsigned int BPPID;
   Byte_t       ResponseCode;
} BPP_Send_Document_Confirmation_Data_t;

#define BPP_SEND_DOCUMENT_CONFIRMATION_DATA_SIZE        (sizeof(BPP_Send_Document_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Get Job Attributes Request.  The  */
   /* BPPID member specifies the Identifier of the Local Basic Printing */
   /* Profile Printer receiving this event.  The ServerPort member      */
   /* specifies the Identifier of the local Server Port (Job or Status) */
   /* in which the request was received.  The NumberDataElements member */
   /* specifies the number of SOAP Data Elements pointed to by the      */
   /* DataElementBuffer member.  The DataElementBuffer points to a      */
   /* buffer containing the SOAP data specified for this Get Job        */
   /* Attributes Request.                                               */
typedef struct _tagBPP_Get_Job_Attributes_Indication_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Job_Attributes_Indication_Data_t;

#define BPP_GET_JOB_ATTRIBUTES_INDICATION_DATA_SIZE     (sizeof(BPP_Get_Job_Attributes_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Get Job Attributes Response.  The */
   /* BPPID member specifies the Identifier of the Local Basic Printing */
   /* Profile Sender receiving this event.  The ServerPort member       */
   /* specifies the server port (Job or Status) in which the response   */
   /* was received.  The ResponseCode member specifies the response code*/
   /* associated with the Get Job Attributes Response that generated    */
   /* this event.  The NumberDataElements member specifies the number of*/
   /* SOAP Data Elements pointed to by the DataElementBuffer member.    */
   /* The DataElementBuffer points to a buffer containing the SOAP data */
   /* specified for this Get Job Attributes Response.                   */
   /* ** NOTE ** Response Codes other then those indicating success will*/
   /*            have invalid values for the NumberDataElements and     */
   /*            DataElementBuffer members.  Therefore it is required   */
   /*            that parameter checking be used to validate the members*/
   /*            of this event that may not exist in case of a response */
   /*            indicating an error.                                   */
typedef struct _tagBPP_Get_Job_Attributes_Confirmation_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   Byte_t                   ResponseCode;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Job_Attributes_Confirmation_Data_t;

#define BPP_GET_JOB_ATTRIBUTES_CONFIRMATION_DATA_SIZE   (sizeof(BPP_Get_Job_Attributes_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Cancel Job Request.  The BPPID    */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Printer receiving this event.  The ServerPort member      */
   /* specifies the Identifier of the local Server Port (Job or Status) */
   /* in which the request was received.  The NumberDataElements member */
   /* specifies the number of SOAP Data Elements pointed to by the      */
   /* DataElementBuffer member.  The DataElementBuffer points to a      */
   /* buffer containing the SOAP data specified for this Cancel Job     */
   /* Request.                                                          */
typedef struct _tagBPP_Cancel_Job_Indication_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Cancel_Job_Indication_Data_t;

#define BPP_CANCEL_JOB_INDICATION_DATA_SIZE             (sizeof(BPP_Cancel_Job_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Cancel Job Response.  The BPPID   */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender receiving this event.  The ServerPort member       */
   /* specifies the server port (Job or Status) in which the response   */
   /* was received.  The ResponseCode member specifies the response code*/
   /* associated with the Cancel Job Response that generated this event.*/
   /* The NumberDataElements member specifies the number of SOAP Data   */
   /* Elements pointed to by the DataElementBuffer member.  The         */
   /* DataElementBuffer points to a buffer containing the SOAP data     */
   /* specified for this Cancel Job Response.                           */
   /* ** NOTE ** Response Codes other then those indicating success will*/
   /*            have invalid values for the NumberDataElements and     */
   /*            DataElementBuffer members.  Therefore it is required   */
   /*            that parameter checking be used to validate the members*/
   /*            of this event that may not exist in case of a response */
   /*            indicating an error.                                   */
typedef struct _tagBPP_Cancel_Job_Confirmation_Data_t
{
   unsigned int             BPPID;
   unsigned int             ServerPort;
   Byte_t                   ResponseCode;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Cancel_Job_Confirmation_Data_t;

#define BPP_CANCEL_JOB_CONFIRMATION_DATA_SIZE           (sizeof(BPP_Cancel_Job_Confirmation_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Printer when the Sender sends a Get Event Request.  The BPPID     */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Printer receiving this event.  The NumberDataElements     */
   /* member specifies the number of SOAP Data Elements pointed to by   */
   /* the DataElementBuffer member.  The DataElementBuffer points to a  */
   /* buffer containing the SOAP data specified for this Get Event      */
   /* Request.                                                          */
typedef struct _tagBPP_Get_Event_Indication_Data_t
{
   unsigned int             BPPID;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Event_Indication_Data_t;

#define BPP_GET_EVENT_INDICATION_DATA_SIZE              (sizeof(BPP_Get_Event_Indication_Data_t))

   /* The following event is dispatched to the Basic Printing Profile   */
   /* Sender when the Printer sends a Get Event Response.  The BPPID    */
   /* member specifies the Identifier of the Local Basic Printing       */
   /* Profile Sender receiving this event.  The ResponseCode member     */
   /* specifies the response code associated with the Get Event Response*/
   /* that generated this event.  The NumberDataElements member         */
   /* specifies the number of SOAP Data Elements pointed to by the      */
   /* DataElementBuffer member.  The DataElementBuffer points to a      */
   /* buffer containing the SOAP data specified for this Get Event      */
   /* Response.                                                         */
   /* ** NOTE ** Response Codes other then those indicating success will*/
   /*            have invalid values for the NumberDataElements and     */
   /*            DataElementBuffer members.  Therefore it is required   */
   /*            that parameter checking be used to validate the members*/
   /*            of this event that may not exist in case of a response */
   /*            indicating an error.                                   */
typedef struct _tagBPP_Get_Event_Confirmation_Data_t
{
   unsigned int             BPPID;
   Byte_t                   ResponseCode;
   unsigned int             NumberDataElements;
   BPP_SOAP_Data_Element_t *DataElementBuffer;
} BPP_Get_Event_Confirmation_Data_t;

#define BPP_GET_EVENT_CONFIRMATION_DATA_SIZE            (sizeof(BPP_Get_Event_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Bluetooth Printing Profile Event Data.                */
typedef struct _tagBPP_Event_Data_t
{
   BPP_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      BPP_Open_Request_Indication_Data_t                  *BPP_Open_Request_Indication_Data;
      BPP_Open_Job_Port_Indication_Data_t                 *BPP_Open_Job_Port_Indication_Data;
      BPP_Open_Status_Port_Indication_Data_t              *BPP_Open_Status_Port_Indication_Data;
      BPP_Open_Referenced_Object_Port_Indication_Data_t   *BPP_Open_Referenced_Object_Port_Indication_Data;
      BPP_Open_Job_Port_Confirmation_Data_t               *BPP_Open_Job_Port_Confirmation_Data;
      BPP_Open_Status_Port_Confirmation_Data_t            *BPP_Open_Status_Port_Confirmation_Data;
      BPP_Open_Referenced_Object_Port_Confirmation_Data_t *BPP_Open_Referenced_Object_Port_Confirmation_Data;
      BPP_Close_Job_Port_Indication_Data_t                *BPP_Close_Job_Port_Indication_Data;
      BPP_Close_Status_Port_Indication_Data_t             *BPP_Close_Status_Port_Indication_Data;
      BPP_Close_Referenced_Object_Port_Indication_Data_t  *BPP_Close_Referenced_Object_Port_Indication_Data;
      BPP_Abort_Indication_Data_t                         *BPP_Abort_Indication_Data;
      BPP_Abort_Confirmation_Data_t                       *BPP_Abort_Confirmation_Data;
      BPP_File_Push_Indication_Data_t                     *BPP_File_Push_Indication_Data;
      BPP_File_Push_Confirmation_Data_t                   *BPP_File_Push_Confirmation_Data;
      BPP_Get_Referenced_Objects_Indication_Data_t        *BPP_Get_Referenced_Objects_Indication_Data;
      BPP_Get_Referenced_Objects_Confirmation_Data_t      *BPP_Get_Referenced_Objects_Confirmation_Data;
      BPP_Get_Printer_Attributes_Indication_Data_t        *BPP_Get_Printer_Attributes_Indication_Data;
      BPP_Get_Printer_Attributes_Confirmation_Data_t      *BPP_Get_Printer_Attributes_Confirmation_Data;
      BPP_Create_Job_Indication_Data_t                    *BPP_Create_Job_Indication_Data;
      BPP_Create_Job_Confirmation_Data_t                  *BPP_Create_Job_Confirmation_Data;
      BPP_Send_Document_Indication_Data_t                 *BPP_Send_Document_Indication_Data;
      BPP_Send_Document_Confirmation_Data_t               *BPP_Send_Document_Confirmation_Data;
      BPP_Get_Job_Attributes_Indication_Data_t            *BPP_Get_Job_Attributes_Indication_Data;
      BPP_Get_Job_Attributes_Confirmation_Data_t          *BPP_Get_Job_Attributes_Confirmation_Data;
      BPP_Cancel_Job_Indication_Data_t                    *BPP_Cancel_Job_Indication_Data;
      BPP_Cancel_Job_Confirmation_Data_t                  *BPP_Cancel_Job_Confirmation_Data;
      BPP_Get_Event_Indication_Data_t                     *BPP_Get_Event_Indication_Data;
      BPP_Get_Event_Confirmation_Data_t                   *BPP_Get_Event_Confirmation_Data;
   } Event_Data;
} BPP_Event_Data_t;

#define BPP_EVENT_DATA_SIZE                             (sizeof(BPP_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a BPP Event Callback.  This function will be called whenever a    */
   /* Basic Printing Profile Event occurs that is associated with the   */
   /* specified Bluetooth Stack ID.  This function passes to the caller */
   /* the Bluetooth Stack ID, the BPP Event Data that occurred, and the */
   /* BPP Event Callback Parameter that was specified when this Callback*/
   /* was installed.  The caller is free to use the contents of the BPP */
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
   /* (this argument holds anyway because another BPP Event will not be */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BPP Events.  A      */
   /*            Deadlock WILL occur because NO BPP Event Callbacks will*/
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *BPP_Event_Callback_t)(unsigned int BluetoothStackID, BPP_Event_Data_t *BPP_Event_Data, unsigned long CallbackParameter);

   /* The following function is responsible for opening a local print   */
   /* server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Protocol Stack Instance to be associated*/
   /* with this Basic Printing Profile Print Server.  The second        */
   /* parameter to this function is Job Server Port to be used by the   */
   /* Print Server.  The third parameter to this function is a bit mask */
   /* used to indicate the targets that should be allowed to connect to */
   /* the Job Server Port.  The fourth parameter to this function is the*/
   /* Status Server Port to be used by the Print Server.  The fifth     */
   /* parameter to this function is reserved for future use.  The sixth */
   /* and seventh parameters are the Event Callback function and        */
   /* application defined Callback Parameter to be used when BPP Events */
   /* occur.  This function returns a non-zero, positive, number on     */
   /* success or a negative return value if there was an error.  A      */
   /* successful return value will be a BPP ID that can used to         */
   /* reference the Opened Basic Print Profile Print Server in ALL other*/
   /* applicable functions in this module.                              */
   /* ** NOTE ** The Job Server Port value must be specified and must be*/
   /*            a value between BPP_PORT_NUMBER_MINIMUM and            */
   /*            BPP_PORT_NUMBER_MAXIMUM.                               */
   /* ** NOTE ** The Status Server Port value is optional depending on  */
   /*            required support for the optional Status Channel by    */
   /*            this Print Server.  If the Status Channel is required, */
   /*            a value between BPP_PORT_NUMBER_MINIMUM and            */
   /*            BPP_PORT_NUMBER_MAXIMUM must be specified.  If the     */
   /*            Status Channel is not required, a value of             */
   /*            BPP_STATUS_SERVER_PORT_INVALID_VALUE should be         */
   /*            specified.                                             */
BTPSAPI_DECLARATION int BTPSAPI BPP_Open_Print_Server(unsigned int BluetoothStackID, unsigned int JobServerPort, unsigned long JobServerPortTargetsMask, unsigned int StatusServerPort, unsigned int Reserved, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Open_Print_Server_t)(unsigned int BluetoothStackID, unsigned int JobServerPort, unsigned long JobServerPortTargetsMask, unsigned int StatusServerPort, unsigned int Reserved, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a connection    */
   /* from the local printer to the remote Sender device.  This         */
   /* functionality is specifically required when data received by the  */
   /* Printer contains object data that is referenced as existing on the*/
   /* Sender device.  The first parameter to this function is the       */
   /* Bluetooth Stack ID of the Bluetooth Protocol Stack Instance to be */
   /* associated with this Basic Printing Profile connection to a remote*/
   /* Sender server.  The second parameter to this function is the      */
   /* BD_ADDR of the remote Sender device in which to connect.  The     */
   /* third parameter to this function is the Referenced Object Server  */
   /* Port to connect with on the remote Sender Server.  The fourth and */
   /* fifth parameters are the Event Callback function and application  */
   /* defined Callback Parameter to be used when BPP Events occur.  This*/
   /* function returns a non-zero, positive, number on success or a     */
   /* negative return value if there was an error.  A successful return */
   /* value will be a BPP ID that can used to reference the Opened Basic*/
   /* Print Profile connection to a remote Sender in ALL other          */
   /* applicable functions in this module.                              */
   /* ** NOTE ** The Referenced Object Server Port value must be        */
   /*            specified and must be a value between                  */
   /*            BPP_PORT_NUMBER_MINIMUM and BPP_PORT_NUMBER_MAXIMUM.   */
BTPSAPI_DECLARATION int BTPSAPI BPP_Open_Remote_Sender_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ReferencedObjectServerPort, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Open_Remote_Sender_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int ReferencedObjectServerPort, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a local sender  */
   /* server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Protocol Stack Instance to be associated*/
   /* with this Basic Printing Profile Sender Server.  The second       */
   /* parameter to this function is the Referenced Object Server Port to*/
   /* be used by the Sender Server.  The third and fourth parameters are*/
   /* the Event Callback function and application defined Callback      */
   /* Parameter to be used when BPP Events occur.  This function returns*/
   /* a non-zero, positive, number on success or a negative return value*/
   /* if there was an error.  A successful return value will be a BPP ID*/
   /* that can used to reference the Opened Basic Print Profile Sender  */
   /* Server in ALL other applicable functions in this module.          */
   /* ** NOTE ** The Referenced Object Server Port value must be        */
   /*            specified and must be a value between                  */
   /*            BPP_PORT_NUMBER_MINIMUM and BPP_PORT_NUMBER_MAXIMUM.   */
BTPSAPI_DECLARATION int BTPSAPI BPP_Open_Sender_Server(unsigned int BluetoothStackID, unsigned int ReferencedObjectServerPort, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Open_Sender_Server_t)(unsigned int BluetoothStackID, unsigned int ReferencedObjectServerPort, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for opening a connection    */
   /* from a local sender to a remote Print device.  The first parameter*/
   /* to this function is the Bluetooth Stack ID of the Bluetooth       */
   /* Protocol Stack Instance to be associated with this Basic Printing */
   /* Profile connection to a remote Print server.  The second parameter*/
   /* to this function is the BD_ADDR of the remote Sender device in    */
   /* which to connect.  The third parameter to this function is the Job*/
   /* Server Port to connect with on the remote Print Server.  The      */
   /* fourth parameter to this function is the Target Service to connect*/
   /* with on the specified Job Server Port.  The fifth parameter to    */
   /* this function is the Status Server Port to connect with on the    */
   /* remote Print Server.  The sixth parameter to this function is     */
   /* reserved for future use.  The seventh and eighth parameters are   */
   /* the Event Callback function and application defined Callback      */
   /* Parameter to be used when BPP Events occur.  This function returns*/
   /* a non-zero, positive, number on success or a negative return value*/
   /* if there was an error.  A successful return value will be a BPP ID*/
   /* that can used to reference the Opened Basic Print Profile         */
   /* connection to a remote Print Server in ALL other applicable       */
   /* functions in this module.                                         */
   /* ** NOTE ** The Job Server Port value must be specified and must be*/
   /*            a value between BPP_PORT_NUMBER_MINIMUM and            */
   /*            BPP_PORT_NUMBER_MAXIMUM.                               */
   /* ** NOTE ** The Status Server Port value is optional depending on  */
   /*            required support for the optional Status Channel by    */
   /*            this Sender connection to a remote Print Server.  If   */
   /*            the Status Channel is required, a value between        */
   /*            BPP_PORT_NUMBER_MINIMUM and BPP_PORT_NUMBER_MAXIMUM    */
   /*            must be specified.  If the Status Channel is not       */
   /*            required, a value of                                   */
   /*            BPP_STATUS_SERVER_PORT_INVALID_VALUE should be         */
   /*            specified.                                             */
BTPSAPI_DECLARATION int BTPSAPI BPP_Open_Remote_Print_Server(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int JobServerPort, BPP_Job_Server_Port_Target_t JobServerPortTarget, unsigned int StatusServerPort, unsigned int Reserved, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Open_Remote_Print_Server_t)(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, unsigned int JobServerPort, BPP_Job_Server_Port_Target_t JobServerPortTarget, unsigned int StatusServerPort, unsigned int Reserved, BPP_Event_Callback_t EventCallback, unsigned long CallbackParameter);
#endif

   /* The following function is responsible for responding to an        */
   /* individual request to connect to a local Basic Printing Profile   */
   /* Server.  The first parameter to this function is the Bluetooth    */
   /* Stack ID of the Bluetooth Stack associated with the Basic Printing*/
   /* Profile Server that is responding to the request.  The second     */
   /* parameter to this function is the BPP ID of the Basic Printing    */
   /* Profile for which a connection request was received.  The third   */
   /* parameter to this function is the Server Port of the Basic        */
   /* Printing Profile for which a connection request was received.  The*/
   /* final parameter to this function specifies whether to accept the  */
   /* pending connection request (or to reject the request).  This      */
   /* function returns zero if successful, or a negative return error   */
   /* code if an error occurred.                                        */
   /* ** NOTE ** The connection to the server is not established until a*/
   /*            etBPP_Open_Request_Indication event has occurred.      */
   /* ** NOTE ** The Server Port value must be specified and must be a  */
   /*            value between BPP_PORT_NUMBER_MINIMUM and              */
   /*            BPP_PORT_NUMBER_MAXIMUM.                               */
BTPSAPI_DECLARATION int BTPSAPI BPP_Open_Request_Response(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Boolean_t AcceptConnection);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Open_Request_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Boolean_t AcceptConnection);
#endif

   /* The following function is responsible for registering a printer   */
   /* server service record to the SDP database.  The first parameter to*/
   /* this function is the Bluetooth Stack ID of the Bluetooth Stack    */
   /* associated with the Basic Printing Profile Printer Server for     */
   /* which this service record is being registered.  The second        */
   /* parameter to this function is the BPP ID of the Basic Printing    */
   /* Profile Printer Server for which this service record is being     */
   /* registered.  The third parameter to this function is the Service  */
   /* Name to be associated with this service record.  The fourth       */
   /* parameter to this function is a string containing the Document    */
   /* Formats Supported to be associated with this service record.  The */
   /* fifth parameter to this function is a bit field containing the    */
   /* Character Repertoires Supported to be associated with this service*/
   /* record.  The sixth parameter to this function is a string         */
   /* containing the Print Image Formats Supported to be associated with*/
   /* this service record.  The seventh parameter to this function is a */
   /* structure containing the IEEE 1284ID to be associated with this   */
   /* service record.  The final parameter to this function is a pointer*/
   /* to a DWord_t which receives the SDP Service Record Handle if this */
   /* function successfully creates a service record.  If this function */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* (see BTERRORS.H) and the SDPServiceRecordHandle value will be     */
   /* undefined.                                                        */
   /* * NOTE * This function should only be called with the BPP ID that */
   /*          was returned from the BPP_Open_Print_Server() function.  */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          BPP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI BPP_Register_Print_Server_SDP_Record(unsigned int BluetoothStackID, unsigned int BPPID, char *ServiceName, char *DocumentFormatsSupported, BPP_Character_Repertoires_t CharacterRepertoiresSupported, char *PrintImageFormatsSupported, BPP_1284_ID_t *BPP1284ID, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Register_Print_Server_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int BPPID, char *ServiceName, char *DocumentFormatsSupported, BPP_Character_Repertoires_t CharacterRepertoiresSupported, char *PrintImageFormatsSupported, BPP_1284_ID_t *BPP1284ID, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following function is responsible for registering a sender    */
   /* servers direct printing referenced objects service record to the  */
   /* SDP database.  The first parameter to this function is the        */
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* Basic Printing Profile Sender Server for which this service record*/
   /* is being registered.  The second parameter to this function is the*/
   /* BPP ID of the Basic Printing Profile Sender Server for which this */
   /* service record is being registered.  The third parameter to this  */
   /* function is the Service Name to be associated with this service   */
   /* record.  The final parameter to this function is a pointer to a   */
   /* DWord_t which receives the SDP Service Record Handle if this      */
   /* function successfully creates a service record.  If this function */
   /* returns zero, then the SDPServiceRecordHandle entry will contain  */
   /* the Service Record Handle of the added SDP Service Record.  If    */
   /* this function fails, a negative return error code will be returned*/
   /* (see BTERRORS.H) and the SDPServiceRecordHandle value will be     */
   /* undefined.                                                        */
   /* * NOTE * This function should only be called with the BPP ID that */
   /*          was returned from the BPP_Open_Sender_Server() function. */
   /* * NOTE * The Service Record Handle that is returned from this     */
   /*          function will remain in the SDP Record Database until it */
   /*          is deleted by calling the SDP_Delete_Service_Record()    */
   /*          function.                                                */
   /* * NOTE * A MACRO is provided to Delete the Service Record from the*/
   /*          SDP Data Base.  This MACRO maps the                      */
   /*          BPP_Un_Register_SDP_Record() to the                      */
   /*          SDP_Delete_Service_Record() function.                    */
   /* * NOTE * The Service Name is always added at Attribute ID 0x0100. */
   /*          A Language Base Attribute ID List is created that        */
   /*          specifies that 0x0100 is UTF-8 Encoded, English Language.*/
BTPSAPI_DECLARATION int BTPSAPI BPP_Register_Sender_Server_Referenced_Objects_SDP_Record(unsigned int BluetoothStackID, unsigned int BPPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Register_Sender_Server_Referenced_Objects_SDP_Record_t)(unsigned int BluetoothStackID, unsigned int BPPID, char *ServiceName, DWord_t *SDPServiceRecordHandle);
#endif

   /* The following MACRO is a utility MACRO that simply deletes the    */
   /* Basic Printing Profile Server SDP Service Record (specified by the*/
   /* third parameter) from the SDP Database.  This MACRO simply maps to*/
   /* the SDP_Delete_Service_Record() function.  This MACRO is only     */
   /* provided so that the caller doesn't have to sift through the SDP  */
   /* API for very simplistic applications.  This function accepts as   */
   /* input the Bluetooth Stack ID of the Bluetooth Protocol Stack that */
   /* the Service Record exists on, the BPP ID (returned from a         */
   /* successful call to the BPP_Open_Print_Server() or                 */
   /* BPP_Open_Sender_Server() function), and the SDP Service Record    */
   /* Handle.  The SDP Service Record Handle was returned via a         */
   /* successful call to the BPP_Register_Print_Server_SDP_Record or    */
   /* BPP_Register_Sender_Server_Referenced_Objects_SDP_Record()        */
   /* function.  This MACRO returns the result of the                   */
   /* SDP_Delete_Service_Record() function, which is zero for success or*/
   /* a negative return error code (see BTERRORS.H).                    */
#define BPP_Un_Register_SDP_Record(__BluetoothStackID, __BPPID, __SDPRecordHandle) \
        (SDP_Delete_Service_Record(__BluetoothStackID, __SDPRecordHandle))

   /* The following function is responsible for closing a currently     */
   /* open/registered Basic Printing Profile server.  This function is  */
   /* capable of closing servers opened via a call to                   */
   /* BPP_Open_Print_Server() or BPP_Open_Sender_Server().  The first   */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance that is associated with the     */
   /* Basic Printing Profile Server being closed.  The second parameter */
   /* to this function is the BPP ID of the Basic Printing Profile      */
   /* Server to be closed.  This function returns zero if successful, or*/
   /* a negative return value if there was an error.                    */
   /* ** NOTE ** This function only closes/un-registers servers it does */
   /*            NOT delete any SDP Service Record Handles that are     */
   /*            registered for the specified server..                  */
BTPSAPI_DECLARATION int BTPSAPI BPP_Close_Server(unsigned int BluetoothStackID, unsigned int BPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Close_Server_t)(unsigned int BluetoothStackID, unsigned int BPPID);
#endif

   /* The following function is responsible for closing a currently     */
   /* ongoing Basic Printing Profile connection.  The first parameter to*/
   /* this function is the Bluetooth Stack ID of the Bluetooth Protocol */
   /* Stack Instance that is associated with the Basic Printing Profile */
   /* connection being closed.  The second parameter to this function is*/
   /* the BPP ID of the Basic Printing Profile connection to be closed. */
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** If this function is called with a server BPP ID (value */
   /*            returned from BPP_Open_Print_Server() or               */
   /*            BPP_Open_Sender_Server()) any clients current          */
   /*            connection to this server will be terminated, but the  */
   /*            server will remained registered.  If this function is  */
   /*            call using a client BPP ID (value returned from        */
   /*            BPP_Open_Remote_Sender_Server() or                     */
   /*            BPP_Open_Remote_Print_Server()) the client connection  */
   /*            shall be terminated.                                   */
BTPSAPI_DECLARATION int BTPSAPI BPP_Close_Connection(unsigned int BluetoothStackID, unsigned int BPPID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Close_Connection_t)(unsigned int BluetoothStackID, unsigned int BPPID);
#endif

   /* The following function is responsible for sending an Abort Request*/
   /* to the remote Basic Printing Profile Server.  The first parameter */
   /* to this function is the Bluetooth Stack ID of the Bluetooth       */
   /* Protocol Stack Instance that is associated with the Basic Printing*/
   /* Profile Client making this call.  The second parameter to this    */
   /* function is the BPP ID of the Basic Printing Profile Client making*/
   /* this call.  The final parameter to this function is the connected */
   /* Server Port Channel in which it perform the Abort.  This function */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
   /* ** NOTE ** Upon the reception of the Abort Confirmation Event it  */
   /*            may be assumed that the currently on going transaction */
   /*            has been successfully aborted and new requests may be  */
   /*            submitted.                                             */
BTPSAPI_DECLARATION int BTPSAPI BPP_Abort_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Abort_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a simple push transfer model file push request to the     */
   /* remote print server.  The first parameter to this function is the */
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* Basic Printing Profile sender client making the file push request.*/
   /* The second parameter to this function is the BPP ID of the Basic  */
   /* Printing Profile sender client making the file push request.  The */
   /* third parameter to this function is a pointer to the MIME Media   */
   /* Type of the file that is to be pushed.  The fourth parameter to   */
   /* this function is a pointer to the File Name of the file that is to*/
   /* be pushed.  The fifth parameter to this function is a pointer to  */
   /* the description of the file being pushed.  This parameter is      */
   /* typically used to specify document type-dependent information.    */
   /* The sixth and seventh parameter to this function are the Data     */
   /* Length and pointer to a Data Buffer that contain the data which is*/
   /* to be sent with this push request.  The eighth parameter to this  */
   /* function is the Amount of Request Data that was actually able to  */
   /* be sent as part of the file push request.  The final parameter to */
   /* this function is a BOOLEAN flag that is used to indicate if this  */
   /* request is the final request required to push the file.  This     */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function should be used to initiate the File Push */
   /*            transaction as well as to continue a previously        */
   /*            initiated, on-going, File Push transaction.            */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
   /* ** NOTE ** The FileName and Description parameters are optional   */
   /*            and should be set to NULL if not specified.            */
BTPSAPI_DECLARATION int BTPSAPI BPP_File_Push_Request(unsigned int BluetoothStackID, unsigned int BPPID, char *MIMEMediaType, char *FileName, char *Description, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_File_Push_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, char *MIMEMediaType, char *FileName, char *Description, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding simply push transfer model file push request.  The    */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Basic Printing Profile printer*/
   /* server making the file push response.  The second parameter to    */
   /* this function is the BPP ID of the Basic Printing Profile printer */
   /* server making the file push response.  The final parameter to this*/
   /* function is the Response Code to be associated with this response.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_File_Push_Response(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_File_Push_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode);
#endif

   /* The following function is for use by the printer client device to */
   /* send a get referenced objects request to the remote sender server.*/
   /* The first parameter to this function is the Bluetooth Stack ID of */
   /* the Bluetooth Stack associated with the Basic Printing Profile    */
   /* printer client making the get referenced objects request.  The    */
   /* second parameter to this function is the BPP ID of the Basic      */
   /* Printing Profile printer client making the get referenced objects */
   /* request.  The third parameter to this function is the identifier  */
   /* of the object to be retrieved during the get referenced objects   */
   /* operation.  The fourth parameter to this function if the offset of*/
   /* the image or print object to be retrieved during the get          */
   /* referenced objects operation.  The fifth parameter to this        */
   /* function is the number of bytes to be retrieved during the get    */
   /* referenced objects operation.  The final parameter to this        */
   /* function is a BOOLEAN flag indicating if the size of the requested*/
   /* object should be returned in the response to this get referenced  */
   /* objects request.  This function returns zero if successful, or a  */
   /* negative return value if there was an error.                      */
   /* ** NOTE ** This function should be used to initiate the Get       */
   /*            Referenced Objeccts transaction as well as to continue */
   /*            a previously initiated, on-going, Get Referenced       */
   /*            Objects transaction.                                   */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Referenced_Objects_Request(unsigned int BluetoothStackID, unsigned int BPPID, char *ObjectIdentifier, DWord_t Offset, DWord_t Count, Boolean_t RequestObjectSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Referenced_Objects_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, char *ObjectIdentifier, DWord_t Offset, DWord_t Count, Boolean_t RequestObjectSize);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding get referenced objects request.  The first parameter  */
   /* to this function is the Bluetooth Stack ID of the Bluetooth Stack */
   /* associated with the Basic Printing Profile sender server making   */
   /* the get referenced objects response.  The second parameter to this*/
   /* function is the BPP ID of the Basic Printing Profile sender server*/
   /* making the get referenced objects response.  The third parameter  */
   /* to this function is the Response Code to be associated with this  */
   /* response.  The fourth parameter to this function is the size of   */
   /* the referenced object to be associated with this response.  The   */
   /* fifth and sixth parameters to this function specify the length of */
   /* the object data being sent and a pointer to the object data being */
   /* sent.  The final parameter to this function is a pointer to a     */
   /* length variable that will return the amount of data actually sent.*/
   /* This function returns zero if successful, or a negative return    */
   /* value if there was an error.                                      */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Referenced_Objects_Response(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, DWord_t ObjectSize, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Referenced_Objects_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, DWord_t ObjectSize, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model get printer attributes    */
   /* request to the remote print server.  The first parameter to this  */
   /* function is the Bluetooth Stack ID of the Bluetooth Stack         */
   /* associated with the Basic Printing Profile sender client making   */
   /* the get printer attributes request.  The second parameter to this */
   /* function is the BPP ID of the Basic Printing Profile sender client*/
   /* making the get printer attributes request.  The third parameter to*/
   /* this function is the server port (Job or Status) to use when      */
   /* sending the get printer attributes request.  The fourth and fifth */
   /* parameters to this function are the number of data elements and a */
   /* pointer to the data elements being used to build the SOAP object  */
   /* for the get printer attributes request.  This function returns    */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Printer_Attributes_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Printer_Attributes_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model get printer attributes  */
   /* request.  The first parameter to this function is the Bluetooth   */
   /* Stack ID of the Bluetooth Stack associated with the Basic Printing*/
   /* Profile printer server making the get printer attributes response.*/
   /* The second parameter to this function is the BPP ID of the Basic  */
   /* Printing Profile printer server making the get printer attributes */
   /* response.  The third parameter to this function is the server port*/
   /* (Job or Status) to use when sending the get printer attributes    */
   /* response.  The fourth parameter to this function is the Response  */
   /* Code to be associated with this response.  The fifth and sixth    */
   /* parameters to this function are the number of data elements and a */
   /* pointer to the data elements being used to build the SOAP object  */
   /* for the get printer attributes response.  This function returns   */
   /* zero if successful, or a negative return value if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Printer_Attributes_Response(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Printer_Attributes_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model create job request to the */
   /* remote print server.  The first parameter to this function is the */
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* Basic Printing Profile sender client making the create job        */
   /* request.  The second parameter to this function is the BPP ID of  */
   /* the Basic Printing Profile sender client making the create job    */
   /* request.  The third and fourth parameters to this function are the*/
   /* number of data elements and a pointer to the data elements being  */
   /* used to build the SOAP object for the create job request.  This   */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI BPP_Create_Job_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Create_Job_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model create job request.  The*/
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Basic Printing Profile printer*/
   /* server making the create job response.  The second parameter to   */
   /* this function is the BPP ID of the Basic Printing Profile printer */
   /* server making the create job response.  The third parameter to    */
   /* this function is the Response Code to be associated with this     */
   /* response.  The fourth parameter to this function is the Job       */
   /* Identifier of the print job being created to be sent in response  */
   /* to the create job request.  The fifth and sixth parameters to this*/
   /* function are the number of data elements and a pointer to the data*/
   /* elements being used to build the SOAP object for the create job   */
   /* response.  This function returns zero if successful, or a negative*/
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI BPP_Create_Job_Response(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, DWord_t JobId, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Create_Job_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, DWord_t JobId, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model send document request to  */
   /* the remote print server.  The first parameter to this function is */
   /* the Bluetooth Stack ID of the Bluetooth Stack associated with the */
   /* Basic Printing Profile sender client making the send document     */
   /* request.  The second parameter to this function is the BPP ID of  */
   /* the Basic Printing Profile sender client making the send document */
   /* request.  The third parameter to this function is the JobId of the*/
   /* job related to the document being pushed.  The fourth parameter to*/
   /* this function is a pointer to the MIME Media Type of the document */
   /* that is to be sent.  The fifth parameter to this function is a    */
   /* pointer to the File Name of the document that is to be sent.  The */
   /* sixth parameter to this function is a pointer to the description  */
   /* of the document being sent.  This parameter is typically used to  */
   /* specify document type-dependent information.  The seventh and     */
   /* eighth parameter to this function are the Data Length and pointer */
   /* to a Data Buffer that contain the data which is to be sent with   */
   /* this send document request.  The ninth parameter to this function */
   /* is the Amount of Request Data that was actually able to be sent as*/
   /* part of the send document request.  The final parameter to this   */
   /* function is a BOOLEAN flag that is used to indicate if this       */
   /* request is the final request required to send the document.  This */
   /* function returns zero if successful, or a negative return value if*/
   /* there was an error.                                               */
   /* ** NOTE ** This function should be used to initiate the Send      */
   /*            Document transaction as well as to continue a          */
   /*            previously initiated, on-going, Send Document          */
   /*            transaction.                                           */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
   /* ** NOTE ** The FileName and Description parameters are optional   */
   /*            and should be set to NULL if not specified.            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Send_Document_Request(unsigned int BluetoothStackID, unsigned int BPPID, DWord_t JobId, char *MIMEMediaType, char *FileName, char *Description, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Send_Document_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, DWord_t JobId, char *MIMEMediaType, char *FileName, char *Description, unsigned int DataLength, unsigned char *DataBuffer, unsigned int *AmountWritten, Boolean_t Final);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model send document request.  */
   /* The first parameter to this function is the Bluetooth Stack ID of */
   /* the Bluetooth Stack associated with the Basic Printing Profile    */
   /* printer server making the send document response.  The second     */
   /* parameter to this function is the BPP ID of the Basic Printing    */
   /* Profile printer server making the send document response.  The    */
   /* final parameter to this function is the Response Code to be       */
   /* associated with this response.  This function returns zero if     */
   /* successful, or a negative return value if there was an error.     */
   /* ** NOTE ** The parameters to this function map to the headers that*/
   /*            are required by this Basic Printing Profile function.  */
   /*            Some headers will only appear once in the stream       */
   /*            associated with a transaction (i.e. Name), others      */
   /*            don't appear in the first segment but may appear in    */
   /*            later segments (i.e. Body).  This being the case, not  */
   /*            all parameters to this function are used in each       */
   /*            segment of the transaction.                            */
BTPSAPI_DECLARATION int BTPSAPI BPP_Send_Document_Response(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Send_Document_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model get job attributes request*/
   /* to the remote print server.  The first parameter to this function */
   /* is the Bluetooth Stack ID of the Bluetooth Stack associated with  */
   /* the Basic Printing Profile sender client making the get job       */
   /* attributes request.  The second parameter to this function is the */
   /* BPP ID of the Basic Printing Profile sender client making the get */
   /* job attributes request.  The third parameter to this function is  */
   /* the server port (Job or Status) to use when sending the get job   */
   /* attributes request.  The fourth and fifth parameters to this      */
   /* function are the number of data elements and a pointer to the data*/
   /* elements being used to build the SOAP object for the get job      */
   /* attributes request.  This function returns zero if successful, or */
   /* a negative return value if there was an error.                    */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Job_Attributes_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Job_Attributes_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model get job attributes      */
   /* request.  The first parameter to this function is the Bluetooth   */
   /* Stack ID of the Bluetooth Stack associated with the Basic Printing*/
   /* Profile printer server making the get job attributes response.    */
   /* The second parameter to this function is the BPP ID of the Basic  */
   /* Printing Profile printer server making the get job attributes     */
   /* response.  The third parameter to this function is the server port*/
   /* (Job or Status) to use when sending the get job attributes        */
   /* response.  The fourth parameter to this function is the Response  */
   /* Code to be associated with this response.  The fifth and sixth    */
   /* parameters to this function are the number of data elements and a */
   /* pointer to the data elements being used to build the SOAP object  */
   /* for the get job attributes response.  This function returns zero  */
   /* if successful, or a negative return value if there was an error.  */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Job_Attributes_Response(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Job_Attributes_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model cancel job request to the */
   /* remote print server.  The first parameter to this function is the */
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* Basic Printing Profile sender client making the cancel job        */
   /* request.  The second parameter to this function is the BPP ID of  */
   /* the Basic Printing Profile sender client making the cancel job    */
   /* request.  The third parameter to this function is the server port */
   /* (Job or Status) to use when sending the cancel job request.  The  */
   /* fourth and fifth parameters to this function are the number of    */
   /* data elements and a pointer to the data elements being used to    */
   /* build the SOAP object for the cancel job request.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BPP_Cancel_Job_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Cancel_Job_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model cancel job request.  The*/
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Basic Printing Profile printer*/
   /* server making the cancel job response.  The second parameter to   */
   /* this function is the BPP ID of the Basic Printing Profile printer */
   /* server making the cancel job response.  The third parameter to    */
   /* this function is the server port (Job or Status) to use when      */
   /* sending the cancel job response.  The fourth parameter to this    */
   /* function is the Response Code to be associated with this response.*/
   /* The fifth and sixth parameters to this function are the number of */
   /* data elements and a pointer to the data elements being used to    */
   /* build the SOAP object for the cancel job response.  This function */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BPP_Cancel_Job_Response(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Cancel_Job_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int ServerPort, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is for use by the sender client device to  */
   /* sending a job-based data transfer model get event request to the  */
   /* remote print server.  The first parameter to this function is the */
   /* Bluetooth Stack ID of the Bluetooth Stack associated with the     */
   /* Basic Printing Profile sender client making the get event request.*/
   /* The second parameter to this function is the BPP ID of the Basic  */
   /* Printing Profile sender client making the get event request.  The */
   /* third and fourth parameters to this function are the number of    */
   /* data elements and a pointer to the data elements being used to    */
   /* build the SOAP object for the get event request.  This function   */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Event_Request(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Event_Request_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for sending a response to an*/
   /* outstanding job-based data transfer model get event request.  The */
   /* first parameter to this function is the Bluetooth Stack ID of the */
   /* Bluetooth Stack associated with the Basic Printing Profile printer*/
   /* server making the get event response.  The second parameter to    */
   /* this function is the BPP ID of the Basic Printing Profile printer */
   /* server making the get event response.  The third parameter to this*/
   /* function is the Response Code to be associated with this response.*/
   /* The fourth and fifth parameters to this function are the number of*/
   /* data elements and a pointer to the data elements being used to    */
   /* build the SOAP object for the get event response.  This function  */
   /* returns zero if successful, or a negative return value if there   */
   /* was an error.                                                     */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Event_Response(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Event_Response_t)(unsigned int BluetoothStackID, unsigned int BPPID, Byte_t ResponseCode, unsigned int NumberDataElements, BPP_SOAP_Data_Element_t *DataElementBuffer);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* query the current Basic Printing Profile Server Mode.  The first  */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* BPP ID of the Basic Printing Profile Server in which to get the   */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* a pointer to a variable which will receive the current Server Mode*/
   /* Mask.  This function returns zero if successful, or a negative    */
   /* return value if there was an error.                               */
BTPSAPI_DECLARATION int BTPSAPI BPP_Get_Server_Mode(unsigned int BluetoothStackID, unsigned int BPPID, unsigned long *ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Get_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned long *ServerModeMask);
#endif

   /* The following function is responsible for providing a mechanism to*/
   /* change the current Basic Printing Profile Server Mode.  The first */
   /* parameter to this function is the Bluetooth Stack ID of the       */
   /* Bluetooth Protocol Stack Instance associated with the requested   */
   /* servers Server Mode.  The second parameter to this function is the*/
   /* BPP ID of the Basic Printing Profile Server in which to set the   */
   /* current Server Mode Mask.  The final parameter to this function is*/
   /* the new Server Mode Mask to use.  This function returns zero if   */
   /* successful, or a negative return value if there was an error.     */
BTPSAPI_DECLARATION int BTPSAPI BPP_Set_Server_Mode(unsigned int BluetoothStackID, unsigned int BPPID, unsigned long ServerModeMask);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BPP_Set_Server_Mode_t)(unsigned int BluetoothStackID, unsigned int BPPID, unsigned long ServerModeMask);
#endif

#endif

