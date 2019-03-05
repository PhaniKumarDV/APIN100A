/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< udsapi.h >*************************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UDSAPI - Qualcomm Technologies Bluetooth User Data Service (GATT          */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __UDSAPIH__
#define __UDSAPIH__

#include "SS1BTPS.h"         /* Bluetooth Stack API Prototypes/Constants.     */
#include "SS1BTGAT.h"        /* Bluetooth Stack GATT API Prototypes/Constants.*/
#include "UDSTypes.h"        /* User Data Service Types/Constants.            */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define UDS_ERROR_INVALID_PARAMETER                           (-1000)
#define UDS_ERROR_INVALID_BLUETOOTH_STACK_ID                  (-1001)
#define UDS_ERROR_INSUFFICIENT_RESOURCES                      (-1002)
#define UDS_ERROR_INSUFFICIENT_BUFFER_SPACE                   (-1003)
#define UDS_ERROR_SERVICE_ALREADY_REGISTERED                  (-1004)
#define UDS_ERROR_INVALID_INSTANCE_ID                         (-1005)
#define UDS_ERROR_MALFORMATTED_DATA                           (-1006)

#define UDS_ERROR_INVALID_UDS_CHARACTERISTIC_FLAGS            (-1007)
#define UDS_ERROR_INVALID_CHARACTERISTIC_TYPE                 (-1008)
#define UDS_ERROR_INVALID_ATTRIBUTE_HANDLE                    (-1009)
#define UDS_ERROR_INDICATION_OUTSTANDING                      (-1010)
#define UDS_ERROR_NOTIFICATIONS_NOT_SUPPORTED                 (-1011)
#define UDS_ERROR_INVALID_USER_CONTROL_POINT_REQUEST_OP_CODE  (-1012)
#define UDS_ERROR_INVALID_USER_CONTROL_POINT_USER_INDEX       (-1013)
#define UDS_ERROR_INVALID_USER_CONTROL_POINT_CONSENT_CODE     (-1014)
#define UDS_ERROR_INVALID_OFFSET                              (-1015)

   /* UDS Server Constants, Structures, Events, and Enumerations.       */

   /* The following defines the values of the UDS Characeristic Flags   */
   /* parameter that is provided in the UDS_Initialize_XXX() Function.  */
#define UDS_CHARACTERISTIC_FLAGS_FIRST_NAME                       0x00000001
#define UDS_CHARACTERISTIC_FLAGS_LAST_NAME                        0x00000002
#define UDS_CHARACTERISTIC_FLAGS_EMAIL_ADDRESS                    0x00000004
#define UDS_CHARACTERISTIC_FLAGS_AGE                              0x00000008
#define UDS_CHARACTERISTIC_FLAGS_DATE_OF_BIRTH                    0x00000010
#define UDS_CHARACTERISTIC_FLAGS_GENDER                           0x00000020
#define UDS_CHARACTERISTIC_FLAGS_WEIGHT                           0x00000040
#define UDS_CHARACTERISTIC_FLAGS_HEIGHT                           0x00000080
#define UDS_CHARACTERISTIC_FLAGS_VO2_MAX                          0x00000100
#define UDS_CHARACTERISTIC_FLAGS_HEART_RATE_MAX                   0x00000200
#define UDS_CHARACTERISTIC_FLAGS_RESTING_HEART_RATE               0x00000400
#define UDS_CHARACTERISTIC_FLAGS_MAXIMUM_RECOMMENDED_HEART_RATE   0x00000800
#define UDS_CHARACTERISTIC_FLAGS_AEROBIC_THRESHOLD                0x00001000
#define UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_THRESHOLD              0x00002000
#define UDS_CHARACTERISTIC_FLAGS_SPORT_TYPE                       0x00004000
#define UDS_CHARACTERISTIC_FLAGS_DATE_OF_THRESHOLD                0x00008000
#define UDS_CHARACTERISTIC_FLAGS_WAIST_CIRCUMFERENCE              0x00010000
#define UDS_CHARACTERISTIC_FLAGS_HIP_CIRCUMFERENCE                0x00020000
#define UDS_CHARACTERISTIC_FLAGS_FAT_BURN_HEART_RATE_LOWER_LIMIT  0x00040000
#define UDS_CHARACTERISTIC_FLAGS_FAT_BURN_HEART_RATE_UPPER_LIMIT  0x00080000
#define UDS_CHARACTERISTIC_FLAGS_AEROBIC_HEART_RATE_LOWER_LIMIT   0x00100000
#define UDS_CHARACTERISTIC_FLAGS_AEROBIC_HEART_RATE_UPPER_LIMIT   0x00200000
#define UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_HEART_RATE_LOWER_LIMIT 0x00400000
#define UDS_CHARACTERISTIC_FLAGS_ANAEROBIC_HEART_RATE_UPPER_LIMIT 0x00800000
#define UDS_CHARACTERISTIC_FLAGS_FIVE_ZONE_HEART_RATE_LIMITS      0x01000000
#define UDS_CHARACTERISTIC_FLAGS_THREE_ZONE_HEART_RATE_LIMITS     0x02000000
#define UDS_CHARACTERISTIC_FLAGS_TWO_ZONE_HEART_RATE_LIMIT        0x04000000
#define UDS_CHARACTERISTIC_FLAGS_LANGUAGE                         0x08000000

   /* The following structure defines that data that is required to     */
   /* initialize the User Data Service (a parameter for the             */
   /* UDS_Initialize_Service() and UDS_Initialize_Service_Handle_Range()*/
   /* functions).                                                       */
   /* * NOTE * The UDS_Characteristic_Flags CANNOT be zero and at least */
   /*          one value for the (bit mask) MUST be specified.  These   */
   /*          flags have the form UDS_CHARACTERISTIC_FLAGS_XXX, where  */
   /*          XXX identifies the optional UDS Characteristic that      */
   /*          should be included in the User Data Service.  These can  */
   /*          be found directly above this structure.                  */
   /* * NOTE * The Server_Update_Supported field is used to specify if  */
   /*          the UDS Server is capable of updating a UDS Characterisic*/
   /*          through its user interface or Out-of_Band mechanism.  If */
   /*          TRUE, then a Client Characteristic Configuration         */
   /*          descriptor (CCCD) will be included for the Database      */
   /*          Change Increment Characteristic.  This CCCD MUST be      */
   /*          configured for notifications by the UDS Client and allows*/
   /*          the UDS Server to send a notification to inform UDS      */
   /*          Clients that the UDS Server has updates one or more UDS  */
   /*          Characteristics.                                         */
typedef struct _tagUDS_Initialize_Data_t
{
   DWord_t   UDS_Characteristic_Flags;
   Boolean_t Server_Update_Supported;
} UDS_Initialize_Data_t;

#define UDS_INITIALIZE_DATA_SIZE                         (sizeof(UDS_Initialize_Data_t))

   /* The following enmeration defines the optional UDS Characteristic  */
   /* types.  These will be used by several structure and API's.        */
typedef enum
{
   uctFirstName,
   uctLastName,
   uctEmailAddress,
   uctAge,
   uctDateOfBirth,
   uctGender,
   uctWeight,
   uctHeight,
   uctVO2Max,
   uctHeartRateMax,
   uctRestingHeartRate,
   uctMaximumRecommendedHeartRate,
   uctAerobicThreshold,
   uctAnaerobicThreshold,
   uctSportType,
   uctDateOfThreshold,
   uctWaistCircumference,
   uctHipCircumference,
   uctFatBurnHeartRateLowerLimit,
   uctFatBurnHeartRateUpperLimit,
   uctAerobicHeartRateLowerLimit,
   uctAerobicHeartRateUpperLimit,
   uctAnaerobicHeartRateLowerLimit,
   uctAnaerobicHeartRateUpperLimit,
   uctFiveZoneHeartRateLimits,
   uctThreeZoneHeartRateLimits,
   uctTwoZoneHeartRateLimit,
   uctLanguage
} UDS_Characteristic_Type_t;

   /* The following enumeration defines the UDS Characteristics that    */
   /* contain a Client Characteristic Configuration descriptor.         */
typedef enum
{
   cctDatabaseChangeIncrement,
   cctUserControlPoint
} UDS_CCCD_Characteristic_Type_t;

   /* The following structure contains all of the attribute handles for */
   /* the optional UDS Characteristics that may need to be cached by the*/
   /* UDS Client if supported by the UDS Server.                        */
   /* * NOTE * The UDS Server MUST support at least one optional UDS    */
   /*          Characteristic.                                          */
typedef struct _tagUDS_Characteristic_Handles_t
{
   Word_t First_Name;
   Word_t Last_Name;
   Word_t Email_Address;
   Word_t Age;
   Word_t Date_Of_Birth;
   Word_t Gender;
   Word_t Weight;
   Word_t Height;
   Word_t VO2_Max;
   Word_t Heart_Rate_Max;
   Word_t Resting_Heart_Rate;
   Word_t Maximum_Recommended_Heart_Rate;
   Word_t Aerobic_Threshold;
   Word_t Anaerobic_Threshold;
   Word_t Sport_Type;
   Word_t Date_Of_Threshold;
   Word_t Waist_Circumference;
   Word_t Hip_Circumference;
   Word_t Fat_Burn_Heart_Rate_Lower_Limit;
   Word_t Fat_Burn_Heart_Rate_Upper_Limit;
   Word_t Aerobic_Heart_Rate_Lower_Limit;
   Word_t Aerobic_Heart_Rate_Upper_Limit;
   Word_t Anaerobic_Heart_Rate_Lower_Limit;
   Word_t Anaerobic_Heart_Rate_Upper_Limit;
   Word_t Five_Zone_Heart_Rate_Limits;
   Word_t Three_Zone_Heart_Rate_Limits;
   Word_t Two_Zone_Heart_Rate_Limit;
   Word_t Language;
} UDS_Characteristic_Handles_t;

   /* The following structure contains all of the information that will */
   /* need to be stored by a UDS Client.  This is the attribute handle  */
   /* information that will need to be cached by a UDS Client in order  */
   /* to only do service discovery once.                                */
   /* * NOTE * The UDS_Characteristic field contains the optional UDS   */
   /*          Characteristic handles that may need to be cached by a   */
   /*          UDS Client.  The UDS Server MUST support at least one    */
   /*          optional UDS Characteristic.                             */
typedef struct _tagUDS_Client_Information_t
{
   UDS_Characteristic_Handles_t UDS_Characteristic;
   Word_t                       Database_Change_Increment;
   Word_t                       Database_Change_Increment_CCCD;
   Word_t                       User_Index;
   Word_t                       User_Control_Point;
   Word_t                       User_Control_Point_CCCD;
} UDS_Client_Information_t;

#define UDS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(UDS_Client_Information_t))

   /* The following structure contains all of the information that will */
   /* need to be stored by a UDS Server for each UDS Client.            */
typedef struct _tagUDS_Server_Information_t
{
   Word_t Database_Change_Increment_Configuration;
   Word_t User_Control_Point_Configuration;
} UDS_Server_Information_t;

#define UDS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(UDS_Server_Information_t))

   /* The following structure defines the UDS String data for the       */
   /* following UDS Characteristics: First Name, Last Name, Email       */
   /* Address, and Language.                                            */
typedef struct _tagUDS_String_Data_t
{
   Word_t  Buffer_Length;
   Byte_t *Buffer;
} UDS_String_Data_t;

   /* The following structure defines the format for the Date of Birth  */
   /* UDS Characteristic .                                              */
   /* * NOTE * The Day and Month fields start at 1 for the first day and*/
   /*          month.                                                   */
typedef struct _tagUDS_Date_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
} UDS_Date_Data_t;

#define UDS_DATE_DATA_SIZE                               (sizeof(UDS_Date_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date of Birth is valid.  The only parameter to   */
   /* this function is the UDS_Date_Data_t structure to validate.  This */
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
#define UDS_DATE_OF_BIRTH_VALID(_x)                      ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)))

   /* The following structure defines the format for the Three Zone     */
   /* Heart Rate Limits UDS Characteristic.                             */
typedef struct _tagUDS_Five_Zone_Heart_Rate_Limits_Data_t
{
   Byte_t Light_Limit;
   Byte_t Light_Moderate_Limit;
   Byte_t Moderate_Hard_Limit;
   Byte_t Hard_Maximum_Limit;
} UDS_Five_Zone_Heart_Rate_Limits_Data_t;

#define UDS_FIVE_ZONE_HEART_RATE_LIMITS_DATA_SIZE        (sizeof(UDS_Five_Zone_Heart_Rate_Limits_Data_t))

   /* The following structure defines the format for the Three Zone     */
   /* Heart Rate Limits UDS Characteristic .                            */
typedef struct _tagUDS_Three_Zone_Heart_Rate_Limits_Data_t
{
   Byte_t Light_Moderate_Limit;
   Byte_t Moderate_Hard_Limit;
} UDS_Three_Zone_Heart_Rate_Limits_Data_t;

#define UDS_THREE_ZONE_HEART_RATE_LIMITS_DATA_SIZE       (sizeof(UDS_Three_Zone_Heart_Rate_Limits_Data_t))

   /* The following union defines the possible data types for an        */
   /* optional UDS Characteristic.                                      */
typedef union
{
   UDS_String_Data_t                       First_Name;
   UDS_String_Data_t                       Last_Name;
   UDS_String_Data_t                       Email_Address;
   Byte_t                                  Age;
   UDS_Date_Data_t                         Date_Of_Birth;
   Byte_t                                  Gender;
   Word_t                                  Weight;
   Word_t                                  Height;
   Byte_t                                  VO2_Max;
   Byte_t                                  Heart_Rate_Max;
   Byte_t                                  Resting_Heart_Rate;
   Byte_t                                  Maximum_Recommended_Heart_Rate;
   Byte_t                                  Aerobic_Threshold;
   Byte_t                                  Anaerobic_Threshold;
   Byte_t                                  Sport_Type;
   UDS_Date_Data_t                         Date_Of_Threshold;
   Word_t                                  Waist_Circumference;
   Word_t                                  Hip_Circumference;
   Byte_t                                  Fat_Burn_Heart_Rate_Lower_Limit;
   Byte_t                                  Fat_Burn_Heart_Rate_Upper_Limit;
   Byte_t                                  Aerobic_Heart_Rate_Lower_Limit;
   Byte_t                                  Aerobic_Heart_Rate_Upper_Limit;
   Byte_t                                  Anaerobic_Heart_Rate_Lower_Limit;
   Byte_t                                  Anaerobic_Heart_Rate_Upper_Limit;
   UDS_Five_Zone_Heart_Rate_Limits_Data_t  Five_Zone_Heart_Rate_Limits;
   UDS_Three_Zone_Heart_Rate_Limits_Data_t Three_Zone_Heart_Rate_Limits;
   Byte_t                                  Two_Zone_Heart_Rate_Limit;
   UDS_String_Data_t                       Language;
} UDS_Characteristic_t;

   /* The following enumeration defines the possible UDS User Control   */
   /* Point Request types (Request Op Codes) that may be set for the    */
   /* Op_Code field of the UDS_User_Control_Point_Request_Data_t        */
   /* structure and the Request_Op_Code field of the                    */
   /* UDS_User_Control_Point_Response_Data_t structure.                 */
typedef enum
{
   ucpRegisterNewUser = UDS_USER_CONTROL_POINT_OP_CODE_REGISTER_NEW_USER,
   ucpConsent         = UDS_USER_CONTROL_POINT_OP_CODE_CONSENT,
   ucpDeleteUserData  = UDS_USER_CONTROL_POINT_OP_CODE_DELETE_USER_DATA
} UDS_User_Control_Point_Request_Type_t;

   /* The following structure defines the format of the UDS User Control*/
   /* Point request data.                                               */
   /* * NOTE * If the Op_Code field is set to ucpRegisterNewUser, then  */
   /*          the Consent_Code field of the internal Parameter field   */
   /*          MUST be valid.  The Consent code is required to register */
   /*          a new user on the UDS Server.  It is worth noting that   */
   /*          the User Index for the successfully registered user will */
   /*          be received in the User Control Point response indication*/
   /*          if the user was successfully registered.                 */
   /* * NOTE * If the Op_Code field is set to ucpConsent, then the      */
   /*          Consent_Code and User_Index fields of the internal       */
   /*          Parameter field MUST BOTH be valid.  The User_Index is   */
   /*          used to indicate which user on the UDS Server the UDS    */
   /*          Client wishes to access.  The Consent_Code MUST be valid */
   /*          for the consent code that was set when the user was      */
   /*          registered.  It is worth noting that the number of       */
   /*          consent attempts is left up to the implementation of the */
   /*          UDS Server before access is denied.                      */
typedef struct _tagUDS_User_Control_Point_Request_Data_t
{
   UDS_User_Control_Point_Request_Type_t Op_Code;
   struct
   {
      Word_t                             Consent_Code;
      Byte_t                             User_Index;
   } Parameter;
} UDS_User_Control_Point_Request_Data_t;

#define UDS_USER_CONTROL_POINT_REQUEST_DATA_SIZE         (sizeof(UDS_User_Control_Point_Request_Data_t))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Response_Code_Value field of the                    */
   /* UDS_User_Control_Point_Response_Data_t structure.                 */
typedef enum
{
   rcvUDSSuccess            = UDS_USER_CONTROL_POINT_RESPONSE_VALUE_SUCCESS,
   rcvUDSOpCodeNotSupported = UDS_USER_CONTROL_POINT_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED,
   rcvUDSInvalidParameter   = UDS_USER_CONTROL_POINT_RESPONSE_VALUE_INVALID_PARAMETER,
   rcvUDSOperationFailed    = UDS_USER_CONTROL_POINT_RESPONSE_VALUE_OPERATION_FAILED,
   rcvUDSUserNotAuthorized  = UDS_USER_CONTROL_POINT_RESPONSE_VALUE_USER_NOT_AUTHORIZED
} UDS_User_Control_Point_Response_Value_t;

   /* The following structure defines the format of the UDS User Control*/
   /* Point response data.  This structure will be needed to            */
   /* send/receive the UDS User Control Point response indication.      */
   /* * NOTE * If the Request_Op_Code field is set to                   */
   /*          ucpRegisterNewUser, and the Response_Code_Value field is */
   /*          set to rcvSuccess, then the User_Index field of the      */
   /*          internal Parameter field MUST be valid.  This indicates  */
   /*          the User Index for the successfully registered user.  It */
   /*          is worth noting that the User Index Characteristic will  */
   /*          not be updated till the UDS Client has acquired consent  */
   /*          to access the user's data via the User Control Point     */
   /*          Consent procedure.  However, the UDS Client will need    */
   /*          this User Index to request consent.                      */
typedef struct _tagUDS_User_Control_Point_Response_Data_t
{
   UDS_User_Control_Point_Request_Type_t   Request_Op_Code;
   UDS_User_Control_Point_Response_Value_t Response_Code_Value;
   union
   {
      Byte_t                               User_Index;
   } Parameter;
} UDS_User_Control_Point_Response_Data_t;

#define UDS_USER_CONTROL_POINT_RESPONSE_DATA_SIZE        (sizeof(UDS_User_Control_Point_Response_Data_t))

   /* The following enumeration covers all the events generated by the  */
   /* UDS Service.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the UDS_Event_Data_t structure.                                   */
typedef enum _tagUDS_Event_Type_t
{
   etUDS_Server_Read_Characteristic_Request,
   etUDS_Server_Write_Characteristic_Request,
   etUDS_Server_Prepare_Write_Characteristic_Request,
   etUDS_Server_Read_CCCD_Request,
   etUDS_Server_Write_CCCD_Request,
   etUDS_Server_Read_Database_Change_Increment_Request,
   etUDS_Server_Write_Database_Change_Increment_Request,
   etUDS_Server_Read_User_Index_Request,
   etUDS_Server_Write_User_Control_Point_Request,
   etUDS_Server_Confirmation_Data
} UDS_Event_Type_t;

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to read an optional UDS Characteristic.*/
   /* The InstanceID field is the identifier for the instance of UDS    */
   /* that received the request.  The ConnectionID is the identifier for*/
   /* the GATT connection between the UDS Client and UDS Server.  The   */
   /* ConnectionType field specifies the GATT connection type or        */
   /* transport being used for the request.  The TransactionID field    */
   /* identifies the GATT transaction for the request.  The RemoteDevice*/
   /* is the BD_ADDR of the UDS Client that made the request.  The Type */
   /* field indicates the optional UDS Characteristic that has been     */
   /* requested to be read.  The final field is an Offset that is used  */
   /* for UDS Characteristics that have a string data type so that the  */
   /* remaining data of a string may be sent if it could not fit in the */
   /* first GATT Read response due to the length of the string exceeding*/
   /* the maximum size of one response (Connection MTU - 1 bytes).      */
   /* * NOTE * The Offset field is only valid for the following UDS     */
   /*          Characteristic types (will be zero otherwise):           */
   /*                                                                   */
   /*                 uctFirstName                                      */
   /*                 uctLastName                                       */
   /*                 uctEmailAddress                                   */
   /*                 uctLanguage                                       */
   /*                                                                   */
   /* * NOTE * The Offset field will be set to zero for a GATT Read     */
   /*          Value request, however if we receive a GATT Read Long    */
   /*          Value request this field will be used to indicate the    */
   /*          starting offset we should read the string from.  The GATT*/
   /*          Read Long Value request may be issued by a UDS Client if */
   /*          the string was not fully received in the GATT Read       */
   /*          response due to its length exceeding the maximum size    */
   /*          that may fit in one response (Connection MTU - 1 bytes). */
   /*          This way the UDS Client can keep issuing GATT Read Long  */
   /*          Value requests with the Offset updated each time until it*/
   /*          has received the entire string.                          */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Read_Characteristic_Request_Response() function  */
   /*          to send the response to the request.  This response MUST */
   /*          be sent if this event is received.                       */
typedef struct _tagUDS_Read_Characteristic_Request_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   unsigned int              TransactionID;
   BD_ADDR_t                 RemoteDevice;
   UDS_Characteristic_Type_t Type;
   Word_t                    Offset;
} UDS_Read_Characteristic_Request_Data_t;

#define UDS_READ_CHARACTERISTIC_REQUEST_DATA_SIZE        (sizeof(UDS_Read_Characteristic_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to write an optional UDS               */
   /* Characteristic.  The InstanceID field is the identifier for the   */
   /* instance of UDS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the UDS Client and */
   /* UDS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the UDS Client that  */
   /* made the request.  The Type field indicates the optional UDS      */
   /* Characteristic that has been requested to be written.  The final  */
   /* parameter will hold the received optional UDS Characteristic value*/
   /* that has been decoded based on the Type field.                    */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Write_Characteristic_Request_Response() function */
   /*          to send the response to the request.  This response MUST */
   /*          be sent if this event is received.                       */
typedef struct _tagUDS_Write_Characteristic_Request_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   unsigned int              TransactionID;
   BD_ADDR_t                 RemoteDevice;
   UDS_Characteristic_Type_t Type;
   UDS_Characteristic_t      UDS_Characteristic;
} UDS_Write_Characteristic_Request_Data_t;

#define UDS_WRITE_CHARACTERISTIC_REQUEST_DATA_SIZE       (sizeof(UDS_Write_Characteristic_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to prepare an optional UDS             */
   /* Characteristic.  The InstanceID field is the identifier for the   */
   /* instance of UDS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the UDS Client and */
   /* UDS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the UDS Client that  */
   /* made the request.  The Type field indicates the optional UDS      */
   /* Characteristic that has been requested to be written.             */
   /* ** NOTE ** This event is primarily provided to reject a GATT      */
   /*            Prepare Write request for optional security reasons    */
   /*            such as the UDS Client has insufficient authentication,*/
   /*            authorization, or encryption.  Therefore we will not   */
   /*            pass the prepared data up to the application until the */
   /*            the GATT Execute Write request has been received by the*/
   /*            UDS Server, and the prepared writes are not cancelled. */
   /*            If the prepared data is written the                    */
   /*            etUDS_Server_Write_Characteristic_Request event will be*/
   /*            dispatched to the application.  Otherwise the prepared */
   /*            data will be cleared.                                  */
   /* * NOTE * Only the following UDS Characteristic types (Strings) are*/
   /*          valid for this event: uctFirstName uctLastName           */
   /*          uctEmailAddress uctLanguage                              */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Prepare_Write_Characteristic_Request_Response()  */
   /*          function to send the response to the request.  This      */
   /*          response MUST be sent if this event is received.         */
typedef struct _tagUDS_Prepare_Write_Characteristic_Request_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   unsigned int              TransactionID;
   BD_ADDR_t                 RemoteDevice;
   UDS_Characteristic_Type_t Type;
} UDS_Prepare_Write_Characteristic_Request_Data_t;

#define UDS_PREPARE_WRITE_CHARACTERISTIC_REQUEST_DATA_SIZE  (sizeof(UDS_Prepare_Write_Characteristic_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to read a UDS Characteristic's Client  */
   /* Characteristic Configuration descriptor (CCCD).  The InstanceID   */
   /* field is the identifier for the instance of UDS that received the */
   /* request.  The ConnectionID is the identifier for the GATT         */
   /* connection between the UDS Client and UDS Server.  The            */
   /* ConnectionType field specifies the GATT connection type or        */
   /* transport being used for the request.  The TransactionID field    */
   /* identifies the GATT transaction for the request.  The RemoteDevice*/
   /* is the BD_ADDR of the UDS Client that made the request.  The Type */
   /* field indicates the UDS Characteristic that contains the CCCD.    */
   /* ** NOTE ** If the UDS Server does NOT support updating of a UDS   */
   /*            Characteristic through it's user interfrace or         */
   /*            out-of-band mechanism, then the CCCD will not be       */
   /*            included for the Database Change Increment             */
   /*            Characteristic.                                        */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Read_CCCD_Request_Response() function to send the*/
   /*          response to the request.  This response MUST be sent if  */
   /*          this event is received.                                  */
typedef struct _tagUDS_Read_CCCD_Request_Data_t
{
   unsigned int                   InstanceID;
   unsigned int                   ConnectionID;
   GATT_Connection_Type_t         ConnectionType;
   unsigned int                   TransactionID;
   BD_ADDR_t                      RemoteDevice;
   UDS_CCCD_Characteristic_Type_t Type;
} UDS_Read_CCCD_Request_Data_t;

#define UDS_READ_CCCD_REQUEST_DATA_SIZE                  (sizeof(UDS_Read_CCCD_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to write a UDS Characteristic's Client */
   /* Characteristic Configuration descriptor (CCCD).  The InstanceID   */
   /* field is the identifier for the instance of UDS that received the */
   /* request.  The ConnectionID is the identifier for the GATT         */
   /* connection between the UDS Client and UDS Server.  The            */
   /* ConnectionType field specifies the GATT connection type or        */
   /* transport being used for the request.  The TransactionID field    */
   /* identifies the GATT transaction for the request.  The RemoteDevice*/
   /* is the BD_ADDR of the UDS Client that made the request.  The Type */
   /* field indicates the UDS Characteristic that contains the CCCD.    */
   /* The final field contains the new value for the CCCD that has been */
   /* decoded and validated, and has been requested to be written.      */
   /* ** NOTE ** If the UDS Server does NOT support updating of a UDS   */
   /*            Characteristic through it's user interfrace or         */
   /*            out-of-band mechanism, then the CCCD will not be       */
   /*            included for the Database Change Increment             */
   /*            Characteristic.                                        */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Write_CCCD_Request_Response() function to send   */
   /*          the response to the request.  This response MUST be sent */
   /*          if this event is received.                               */
typedef struct _tagUDS_Write_CCCD_Request_Data_t
{
   unsigned int                   InstanceID;
   unsigned int                   ConnectionID;
   GATT_Connection_Type_t         ConnectionType;
   unsigned int                   TransactionID;
   BD_ADDR_t                      RemoteDevice;
   UDS_CCCD_Characteristic_Type_t Type;
   Word_t                         ClientConfiguration;
} UDS_Write_CCCD_Request_Data_t;

#define UDS_WRITE_CCCD_REQUEST_DATA_SIZE                 (sizeof(UDS_Write_CCCD_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to read the UDS Database Change        */
   /* Increment Characteristic.  The InstanceID field is the identifier */
   /* for the instance of UDS that received the request.  The           */
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* UDS Client and UDS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the UDS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_Database_Change_Increment_Read_Request_Response()*/
   /*          function to send the response to the request.  This      */
   /*          response MUST be sent if this event is received.         */
typedef struct _tagUDS_Read_Database_Change_Increment_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} UDS_Read_Database_Change_Increment_Request_Data_t;

#define UDS_READ_DATABASE_CHANGE_INCREMENT_REQUEST_DATA_SIZE  (sizeof(UDS_Read_Database_Change_Increment_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to write the UDS Database Change       */
   /* Increment Characteristic.  The InstanceID field is the identifier */
   /* for the instance of UDS that received the request.  The           */
   /* ConnectionID is the identifier for the GATT connection between the*/
   /* UDS Client and UDS Server.  The ConnectionType field specifies the*/
   /* GATT connection type or transport being used for the request.  The*/
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the UDS Client that  */
   /* made the request.  The final field contains the decoded Database  */
   /* Change Increment value that has been requested to be written.     */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the                                                      */
   /*          UDS_Database_Change_Increment_Write_Request_Response()   */
   /*          function to send the response to the request.  This      */
   /*          response MUST be sent if this event is received.         */
typedef struct _tagUDS_Write_Database_Change_Increment_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   DWord_t                DatabaseChangeIncrement;
} UDS_Write_Database_Change_Increment_Request_Data_t;

#define UDS_WRITE_DATABASE_CHANGE_INCREMENT_REQUEST_DATA_SIZE  (sizeof(UDS_Write_Database_Change_Increment_Request_Data_t))

   /* The following UDS Server Event is dispatched to a UDS Server when */
   /* a UDS Client is attempting to read the UDS User Index             */
   /* Characteristic.  The InstanceID field is the identifier for the   */
   /* instance of UDS that received the request.  The ConnectionID is   */
   /* the identifier for the GATT connection between the UDS Client and */
   /* UDS Server.  The ConnectionType field specifies the GATT          */
   /* connection type or transport being used for the request.  The     */
   /* TransactionID field identifies the GATT transaction for the       */
   /* request.  The RemoteDevice is the BD_ADDR of the UDS Client that  */
   /* made the request.                                                 */
   /* * NOTE * Many of the following fields will be need to be passed to*/
   /*          the UDS_User_Index_Read_Request_Response() function to   */
   /*          send the response to the request.  This response MUST be */
   /*          sent if this event is received.                          */
typedef struct _tagUDS_Read_User_Index_Request_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
} UDS_Read_User_Index_Request_Data_t;

#define UDS_READ_USER_INDEX_REQUEST_DATA_SIZE            (sizeof(UDS_Read_User_Index_Request_Data_t))

   /* The following is dispatched to a UDS Server in response to the    */
   /* reception of request from a Client to write to the UDS User       */
   /* Control Point.  The FormatData specifies the formatted UDS User   */
   /* Control Point Command Data.                                       */
typedef struct _tagUDS_Write_User_Control_Point_Request_Data_t
{
   unsigned int                          InstanceID;
   unsigned int                          ConnectionID;
   GATT_Connection_Type_t                ConnectionType;
   unsigned int                          TransactionID;
   BD_ADDR_t                             RemoteDevice;
   UDS_User_Control_Point_Request_Data_t UserControlPoint;
} UDS_Write_User_Control_Point_Request_Data_t;

#define UDS_WRITE_USER_CONTROL_POINT_REQUEST_DATA_SIZE   (sizeof(UDS_Write_User_Control_Point_Request_Data_t))

   /* The following is dispatched to a UDS Server when a UDS Client     */
   /* confirms an outstanding indication.  The ConnectionID and         */
   /* RemoteDevice identify the Client that has confirmed the           */
   /* indication.  The BytesWritten field indicates the number of bytes */
   /* that were successfully written in the outstanding indication.     */
typedef struct _tagUDS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   unsigned int           TransactionID;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
   Word_t                 BytesWritten;
} UDS_Confirmation_Data_t;

#define UDS_CONFIRMATION_DATA_SIZE                       (sizeof(UDS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all UDS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagUDS_Event_Data_t
{
   UDS_Event_Type_t  Event_Data_Type;
   Byte_t            Event_Data_Size;
   union
   {
      UDS_Read_Characteristic_Request_Data_t             *UDS_Read_Characteristic_Request_Data;
      UDS_Write_Characteristic_Request_Data_t            *UDS_Write_Characteristic_Request_Data;
      UDS_Prepare_Write_Characteristic_Request_Data_t    *UDS_Prepare_Write_Characteristic_Request_Data;
      UDS_Read_CCCD_Request_Data_t                       *UDS_Read_CCCD_Request_Data;
      UDS_Write_CCCD_Request_Data_t                      *UDS_Write_CCCD_Request_Data;
      UDS_Read_Database_Change_Increment_Request_Data_t  *UDS_Read_Database_Change_Increment_Request_Data;
      UDS_Write_Database_Change_Increment_Request_Data_t *UDS_Write_Database_Change_Increment_Request_Data;
      UDS_Read_User_Index_Request_Data_t                 *UDS_Read_User_Index_Request_Data;
      UDS_Write_User_Control_Point_Request_Data_t        *UDS_Write_User_Control_Point_Request_Data;
      UDS_Confirmation_Data_t                            *UDS_Confirmation_Data;
   } Event_Data;
} UDS_Event_Data_t;

#define UDS_EVENT_DATA_SIZE                              (sizeof(UDS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a UDS Service Event Receive Data Callback.  This function will be */
   /* called whenever an UDS Service Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the UDS Event Data that        */
   /* occurred and the UDS Service Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the UDS Service Event Data ONLY in  context   */
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another UDS Service  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving UDS Service Event   */
   /*            Packets.  A Deadlock WILL occur because NO UDS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *UDS_Event_Callback_t)(unsigned int BluetoothStackID, UDS_Event_Data_t *UDS_Event_Data, unsigned long CallbackParameter);

   /* UDS Server API.                                                   */

   /* The following function is responsible for opening a UDS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the UDS Service Flags  */
   /* (UDS_SERVICE_FLAGS_XXX) found in UDSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an UDS  */
   /* Client, for the specified transport.  The third parameter is a    */
   /* pointer to the data that is REQUIRED to configure the service.    */
   /* The fourth parameter is the Callback function to call when an     */
   /* event occurs on this Server Port.  The fifth parameter is a       */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered UDS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 UDS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * See the UDS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI UDS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Service_Flags, UDS_Initialize_Data_t *InitializeData, UDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, UDS_Initialize_Data_t *InitializeData, UDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a UDS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the UDS Service Flags  */
   /* (UDS_SERVICE_FLAGS_XXX) found in UDSTypes.h.  These flags will be */
   /* used to configure the service to only allow requests from an UDS  */
   /* Client, for the specified transport.  The third parameter is a    */
   /* pointer to the data that is REQUIRED to configure the service.    */
   /* The fourth parameter is the Callback function to call when an     */
   /* event occurs on this Server Port.  The fifth parameter is a       */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The sixth parameter is a      */
   /* pointer to store the GATT Service ID of the registered UDS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 UDS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * See the UDS_Initialize_Data_t structure above for more   */
   /*          information about the InitializeData parameter.  If this */
   /*          parameter is not configured correctly the service will   */
   /*          FAIL to register.                                        */
BTPSAPI_DECLARATION int BTPSAPI UDS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Service_Flags, UDS_Initialize_Data_t *InitializeData, UDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Service_Flags, UDS_Initialize_Data_t *InitializeData, UDS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened UDS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* UDS_Initialize_XXX().  This function returns a zero if successful */
   /* or a negative return error code if an error occurs.               */
BTPSAPI_DECLARATION int BTPSAPI UDS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
   /* buffer for the suspend (or zero otherwise).                       */
BTPSAPI_DECLARATION unsigned long BTPSAPI UDS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_UDS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* UDS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to UDS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI UDS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the UDS Service that is          */
   /* registered with a call to UDS_Initialize_XXX().  The first        */
   /* parameter is the Bluetooth Stack ID on which to close the server. */
   /* The second parameter is the InstanceID that was returned from a   */
   /* successfull call to UDS_Initialize_XXX().  This function returns  */
   /* the non-zero number of attributes that are contained in a UDS     */
   /* Server or zero on failure.                                        */
   /* * NOTE * This function may be used to determine the attribute     */
   /*          handle range for UDS so that the ServiceHandleRange      */
   /*          parameter of the UDS_Initialize_Service_Handle_Range()   */
   /*          can be configured to register UDS in a specified         */
   /*          attribute handle range in GATT.                          */
   /* * NOTE * The number of attributes in the service is dependent on  */
   /*          the UDS_Initialize_Data_t structure passed in a call to  */
   /*          either of the UDS_Initialize_XXX() API's.                */
BTPSAPI_DECLARATION unsigned int BTPSAPI UDS_Query_Number_Attributes(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_UDS_Query_Number_Attributes_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for responding to a UDS     */
   /* Characteristic Read request received from a UDS Client.  The first*/
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID that was returned from a       */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT TransactionID for the request.  The fourth parameter is  */
   /* an error code that is used to determine if the request has been   */
   /* accepted by the UDS Server or if an error response should be sent.*/
   /* The next parameter is an offset (in bytes) that is used for UDS   */
   /* Characteristics that have a UTF-8 data type so that the remaining */
   /* data of a string may be sent if it could not fit in the first GATT*/
   /* Read response due to the length of the string exceeding the       */
   /* maximum size that would fit in one packet (based on the           */
   /* connection's MTU value).  The fifth parameter is the UDS          */
   /* Characteristic type to identify the UDS Characteristic that UDS   */
   /* Client requested to write.  The next parameter is the UDS         */
   /* Characteristic data that will be sent to the UDS Client if the    */
   /* request is accepted.  This function returns a zero if successful  */
   /* or a negative return error code if an error occurs.               */
   /* * NOTE * The Type parameter MUST be valid and correspond to the   */
   /*          data in the UDS_Characteristic parameter.  Otherwise,    */
   /*          this function will not format the data correctly that    */
   /*          will be sent in the response to the UDS Client.  The Type*/
   /*          parameter is REQUIRED for the error response or the error*/
   /*          response will not be formatted correctly.                */
   /* * NOTE * The Offset parameter MUST match the Offset parameter in  */
   /*          the etUDS_Server_Read_Characteristic_Request event that  */
   /*          generated this response.                                 */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except UDS_ERROR_CODE_SUCCESS.  The     */
   /*          UDS_Characteristic parameter may be excluded (NULL) and  */
   /*          the Offset parameter will be ignored.                    */
BTPSAPI_DECLARATION int BTPSAPI UDS_Read_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t Offset, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Read_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Word_t Offset, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);
#endif

   /* The following function is responsible for responding to a UDS     */
   /* Characteristic Write request received from a UDS Client.  The     */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT TransactionID for the request.  The fourth parameter is  */
   /* an error code that is used to determine if the request has been   */
   /* accepted by the UDS Server or if an error response should be sent.*/
   /* The final parameter is the UDS Characteristic type to identify the*/
   /* UDS Characteristic that UDS Client requested to write.  This      */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI UDS_Write_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Write_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to a UDS     */
   /* Characteristic Prepare Write request received from a UDS Client.  */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT Transaction ID of the request.  The fourth parameter is  */
   /* the ErrorCode to indicate the type of response that will be sent. */
   /* The final parameter is an enumeration for the UDS Characteristic  */
   /* that has been requested to be prepared.  This function returns    */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* ** NOTE ** This event is primarily provided to reject a GATT      */
   /*            Prepare Write request for optional security reasons    */
   /*            such as the UDS Client has insufficient authentication,*/
   /*            authorization, or encryption.  Therefore, we will not  */
   /*            pass the prepared data up to the application until the */
   /*            the GATT Execute Write request has been received by the*/
   /*            UDS Server, and the prepared writes are not cancelled. */
   /*            If the prepared data is written the                    */
   /*            etUDS_Server_Write_Characteristic_Request event will be*/
   /*            dispatched to the application.  Otherwise the prepared */
   /*            data will be cleared.                                  */
   /* * NOTE * Only the following UDS Characteristic types (Strings) are*/
   /*          valid for this function: uctFirstName uctLastName        */
   /*          uctEmailAddress uctLanguage                              */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI UDS_Prepare_Write_Characteristic_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Prepare_Write_Characteristic_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to a UDS     */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) read request received from UDS Client.  The first parameter*/
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* UDS_Initialize_XXX().  The third parameter is the GATT Transaction*/
   /* ID of the request.  The fourth parameter is an error code that is */
   /* used to determine if the request has been accepted by the UDS     */
   /* Server or if an error response should be sent.  The fifth         */
   /* parameter is UDS CCCD Characteristic type that identifies, which  */
   /* UDS Characteristic's CCCD has been requested to be read.  The     */
   /* final parameter is the current Client Characteristic Configuration*/
   /* value to send to the UDS Client if the request has been accepted. */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request is rejected the Client_Configuration      */
   /*          parameter will be IGNORED.                               */
BTPSAPI_DECLARATION int BTPSAPI UDS_Read_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_CCCD_Characteristic_Type_t Type, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Read_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_CCCD_Characteristic_Type_t Type, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to a UDS     */
   /* Characteristic's Client Characteristic Configuration descriptor   */
   /* (CCCD) write request received from a UDS Client.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to UDS_Initialize_XXX().  The third parameter is the GATT         */
   /* Transaction ID for the request.  The fourth parameter is an error */
   /* code that is used to determine if the request has been accepted by*/
   /* the UDS Server or if an error response should be sent.  The final */
   /* parameter is UDS CCCD Characteristic type that identifies, which  */
   /* UDS Characteristic's CCCD has been requested to be written.  This */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI UDS_Write_CCCD_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_CCCD_Characteristic_Type_t Type);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Write_CCCD_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, UDS_CCCD_Characteristic_Type_t Type);
#endif

   /* The following function is responsible for responding to a Database*/
   /* Change Increment read request received from a UDS Client.  The    */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT TransactionID for the request.  The fourth parameter is  */
   /* an error code that is used to determine if the request has been   */
   /* accepted by the UDS Server or if an error response should be sent.*/
   /* The final parameter contains the requested UDS Database Change    */
   /* Increment to respond with.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except UDS_ERROR_CODE_SUCCESS and the   */
   /*          DatabaseChangeIncrement parameter is REQUIRED.           */
   /*          Otherwise, it will be ignored.                           */
BTPSAPI_DECLARATION int BTPSAPI UDS_Database_Change_Increment_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, DWord_t DatabaseChangeIncrement);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Database_Change_Increment_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, DWord_t DatabaseChangeIncrement);
#endif

   /* The following function is responsible for responding to a Database*/
   /* Change Increment write request received from a UDS Client.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT TransactionID for the request.  The final parameter is an*/
   /* error code that is used to determine if the request has been      */
   /* accepted by the UDS Server or if an error response should be sent.*/
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI UDS_Database_Change_Increment_Write_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Database_Change_Increment_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for notifying the UDS       */
   /* Database Change Increment Characteristic to a UDS Client.  The    */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to UDS_Initialize_XXX().  The third parameter is the GATT    */
   /* ConnectionID for the connection to the UDS Client.  The final     */
   /* parameter is the Database Change Increment value that will be     */
   /* notified.  This function returns a positive non-zero value that   */
   /* represents the length of the data that has been notified, if      */
   /* successful, or a negative return error code if an error occurs.   */
   /* * NOTE * The UDS Server MUST have been configured to support      */
   /*          updates through its user interface or out-of-band        */
   /*          mechanism when the service was initialized via a call to */
   /*          either of the UDS_Initialize_XXX() functions.  Otherwise */
   /*          the CCCD for the Database Change Increment will not be   */
   /*          included and this function will fail with error code     */
   /*          UDS_ERROR_NOTIFICATIONS_NOT_SUPPORTED.  This             */
   /*          functionality is included so that if the UDS Server makes*/
   /*          an update it can notify UDS Clients that a UDS           */
   /*          Characteristic has been updated.                         */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the Database Change Increment CCCD has been configured   */
   /*          for notifications by the UDS Client this notification is */
   /*          intended for.  A notification SHOULD NOT be sent if this */
   /*          is not the case.                                         */
   /* * NOTE * There is no guarantee that the UDS Client will receive   */
   /*          the notification, if this function is successful, since  */
   /*          notifications are not confirmed.                         */
BTPSAPI_DECLARATION int BTPSAPI UDS_Notify_Database_Change_Increment(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, DWord_t DatabaseChangeIncrement);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Notify_Database_Change_Increment_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, DWord_t DatabaseChangeIncrement);
#endif

   /* The following function is responsible for responding to a UDS User*/
   /* Index Characteristic read request received from a UDS Client.  The*/
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to UDS_Initialize_XXX().  The third parameter is the GATT    */
   /* TransactionID for the request.  The fourth parameter is an error  */
   /* code that is used to determine if the request has been accepted by*/
   /* the UDS Server or if an error response should be sent.  The final */
   /* parameter contains the User Index to respond with if the request  */
   /* is accepted.  This function returns a zero if successful or a     */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * If the request has been rejected, the ErrorCode parameter*/
   /*          may be any value except UDS_ERROR_CODE_SUCCESS and the   */
   /*          User_Index parameter is REQUIRED.  Otherwise, it will be */
   /*          ignored.                                                 */
BTPSAPI_DECLARATION int BTPSAPI UDS_User_Index_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Byte_t User_Index);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_User_Index_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode, Byte_t User_Index);
#endif

   /* The following function is responsible for responding to a User    */
   /* Control Point Write request received from a UDS Client.  The first*/
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID that was returned from a       */
   /* successful call to UDS_Initialize_XXX().  The third parameter is  */
   /* the GATT TransactionID for the request.  The final parameter is an*/
   /* error code that is used to determine if the request has been      */
   /* accepted by the UDS Server or if an error response should be sent.*/
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject the User Control Point request when the User      */
   /*          Control Point Client Characteristic Configuration        */
   /*          descriptor (CCCD) has not been configured for            */
   /*          indications, the UDS Client does not have proper         */
   /*          Authentication, Authorization, or Encryption to write to */
   /*          the User Control Point , or a User Control Point request */
   /*          is already in progress.  All other reasons should return */
   /*          UDS_ERROR_CODE_SUCCESS for the ErrorCode and then call   */
   /*          the UDS_Indicate_User_Control_Point_Response() to        */
   /*          indicate the result.                                     */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
BTPSAPI_DECLARATION int BTPSAPI UDS_User_Control_Point_Write_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_User_Control_Point_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for indicating the UDS User */
   /* Control Point response to a UDS Client.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* either of the UDS_Initialize_XXX() functions.  The third parameter*/
   /* is the GATT ConnectionID for the connection to the UDS Client.    */
   /* The final parameter is the User Control Point response data to    */
   /* indicate.  This function returns a positive non-zero value (The   */
   /* GATT TransactionID), which may be used to cancel the request.     */
   /* Otherwise a negative return error code will be returned if an     */
   /* error occurs.                                                     */
   /* * NOTE * It is the application's responsibility to make sure that */
   /*          the User Control Point CCCD has been configured for      */
   /*          indications by the UDS Client this indication is intended*/
   /*          for.  An indication SHOULD NOT be sent if this is not the*/
   /*          case.                                                    */
   /* * NOTE * The ErrorCode parameter MUST be a valid value from       */
   /*          UDS_ERROR_CODE_XXX from UDSTypes.h or                    */
   /*          ATT_PROTOCOL_ERROR_CODE_XXX from ATTTypes.h.             */
   /* * NOTE * Only 1 User Control Point Request indication may be      */
   /*          outstanding per UDS Instance.  Otherwise the error code  */
   /*          UDS_ERROR_INDICATION_OUTSTANDING will be returned.       */
BTPSAPI_DECLARATION int BTPSAPI UDS_Indicate_User_Control_Point_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, UDS_User_Control_Point_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Indicate_User_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, UDS_User_Control_Point_Response_Data_t *ResponseData);
#endif

   /* UDS Client API.                                                   */

   /* The following function is responsible for formatting an optional  */
   /* UDS Characteristic into a buffer, for a GATT Write request, that  */
   /* will be sent to the UDS Server.  This function may also be used to*/
   /* determine the size of the buffer to hold the formatted data (see  */
   /* below).  The first parameter is the UDS Characteristic type that  */
   /* will control how the UDS_Characteristic parameter is formatted    */
   /* into the buffer.  The second parameter is a pointer to the UDS    */
   /* Characteristic data identified by the Type parameter.  The third  */
   /* parameter is the size of buffer.  The final parameter is the      */
   /* buffer that will hold the formatted data if this function is      */
   /* successful.  This function returns zero if the UDS Characteristic */
   /* has been successfully formatted into the buffer.  If this function*/
   /* is used to determine the size of the buffer to hold the formatted */
   /* data, then a positive non-zero value will be returned.  Otherwise */
   /* this function will return a negative error code if an error       */
   /* occurs.                                                           */
   /* * NOTE * This function should NOT be used for the following UDS   */
   /*          Characteristics (since they have a UTF-8 string data type*/
   /*          and are already formatted):                              */
   /*                                                                   */
   /*                 uctFirstName                                      */
   /*                 uctLastName                                       */
   /*                 uctEmailAddress                                   */
   /*                 uctLanguage                                       */
   /*                                                                   */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The UDS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI UDS_Format_UDS_Characteristic_Request(UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Format_UDS_Characteristic_Request_t)(UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic, Word_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a response value*/
   /* received from a UDS Server, interpreting it as an optional UDS    */
   /* Characteristic value.  The first parameter is the length of the   */
   /* value returned by the remote UDS Server.  The second parameter is */
   /* a pointer to the data returned by the remote UDS Server.  The     */
   /* third parameter is the UDS Characteristic type that determines how*/
   /* the received Value is decoded.  The final parameter is a pointer  */
   /* to store the parsed UDS Characteristic value.  This function      */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * This function should NOT be used for the following UDS   */
   /*          Characteristics (since they have a UTF-8 string data type*/
   /*          and are already decoded):                                */
   /*                                                                   */
   /*                 uctFirstName                                      */
   /*                 uctLastName                                       */
   /*                 uctEmailAddress                                   */
   /*                 uctLanguage                                       */
   /*                                                                   */
BTPSAPI_DECLARATION int BTPSAPI UDS_Decode_UDS_Characteristic_Response(unsigned int ValueLength, Byte_t *Value, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Decode_UDS_Characteristic_Response_t)(unsigned int ValueLength, Byte_t *Value, UDS_Characteristic_Type_t Type, UDS_Characteristic_t *UDS_Characteristic);
#endif

   /* The following function is responsible for formatting the UDS User */
   /* Control Point request into a buffer, for a GATT Write request,    */
   /* that will be sent to the UDS Server.  This function may also be   */
   /* used to determine the size of the buffer to hold the formatted    */
   /* data (see below).  The first parameter is a pointer to the UDS    */
   /* User Control Point request data that will be formatted into the   */
   /* buffer.  The second parameter is the size of buffer.  The final   */
   /* parameter is the buffer that will hold the formatted data if this */
   /* function is successful.  This function returns zero if the UDS    */
   /* User Control Point has been successfully formatted into the       */
   /* buffer.  If this function is used to determine the size of the    */
   /* buffer to hold the formatted data, then a positive non-zero value */
   /* will be returned.  Otherwise this function will return a negative */
   /* error code if an error occurs.                                    */
   /* * NOTE * If the BufferLength parameter is 0, the Buffer parameter */
   /*          may be excluded (NULL), and this function will return a  */
   /*          positive non-zero value, which represents the size of the*/
   /*          buffer needed to hold the formatted data.  The UDS Client*/
   /*          may use this size to allocate a buffer necessary to hold */
   /*          the formatted data.                                      */
BTPSAPI_DECLARATION int BTPSAPI UDS_Format_User_Control_Point_Request(UDS_User_Control_Point_Request_Data_t *RequestData, Word_t BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Format_User_Control_Point_Request_t)(UDS_User_Control_Point_Request_Data_t *RequestData, Word_t BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a response value*/
   /* received from a UDS Server, interpreting it as the response data  */
   /* for the User Control Point request.  The first parameter is the   */
   /* length of the value returned by the remote UDS Server.  The second*/
   /* parameter is a pointer to the data returned by the remote UDS     */
   /* Server.  The final parameter is a pointer to store the parsed User*/
   /* Control Point response data.  This function returns a zero if     */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI UDS_Decode_User_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, UDS_User_Control_Point_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_UDS_Decode_User_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, UDS_User_Control_Point_Response_Data_t *ResponseData);
#endif

#endif
