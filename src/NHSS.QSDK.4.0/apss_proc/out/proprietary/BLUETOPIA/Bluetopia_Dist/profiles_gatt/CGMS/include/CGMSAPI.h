/*****< cgmsapi.h >************************************************************/
/*      Copyright 2014 Stonestreet One.                                       */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CGMSAPI - Stonestreet One Bluetooth Continous Glucose Monitor Service     */
/*           (GATT based) API Type Definitions, Constants, and Prototypes.    */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/14  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __CGMSAPIH__
#define __CGMSAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.*/
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API                 */
                            /* Prototypes/Constants.                    */
#include "CGMSTypes.h"      /* CGM Service Types/Constants.             */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define CGMS_ERROR_INVALID_PARAMETER                            (-1000)
#define CGMS_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1001)
#define CGMS_ERROR_INSUFFICIENT_RESOURCES                       (-1002)
#define CGMS_ERROR_INSUFFICIENT_BUFFER_SPACE                    (-1003)
#define CGMS_ERROR_SERVICE_ALREADY_REGISTERED                   (-1004)
#define CGMS_ERROR_INVALID_INSTANCE_ID                          (-1005)
#define CGMS_ERROR_MALFORMATTED_DATA                            (-1006)
#define CGMS_ERROR_INDICATION_OUTSTANDING                       (-1007)
#define CGMS_ERROR_INVALID_RESPONSE_CODE                        (-1008)
#define CGMS_ERROR_UNKNOWN_ERROR                                (-1009)
#define CGMS_ERROR_CRC_MISSING                                  (-1010)
#define CGMS_ERROR_CRC_INVALID                                  (-1011)
#define CGMS_ERROR_MEASUREMENT_SIZE_INVALID                     (-1012)
#define CGMS_ERROR_MEASUREMENT_AND_FLAG_SIZE_NOT_EQUAL          (-1013)
#define CGMS_ERROR_MEASUREMENT_FAILED_TO_DECODE                 (-1014)
#define CGMS_ERROR_BUFFER_NOT_EMPTY                             (-1015)

   /* This following defines the bit values for whether a E2E-CRC       */
   /* calculation is required (Flags parameter for many API's).  For    */
   /* CGMS Server API's this will indicate whether a CRC needs to be    */
   /* calculated and included with the data being sent.  For CGMS Client*/
   /* API's this will indicate whether the client expects a CRC.        */
   /* * NOTE * The CGMS Client can ignore a CRC sent by the CGMS Server.*/
#define CGMS_E2E_CRC_NOT_SUPPORTED                              (0x00)
#define CGMS_E2E_CRC_SUPPORTED                                  (0x01)

   /* The following defines bit values that will be used to indicate    */
   /* whether a CRC is present or valid.  This will allow the CGMS      */
   /* Server and Client to determine if a CRC is present with incoming  */
   /* data and valid for the following situations.                      */
   /* * NOTE * If the Session Start Time Characteristic or the Specific */
   /*          Ops Control Point Characteristic is written, we cannot   */
   /*          internally determine if a CRC is present and valid with  */
   /*          the incoming data from the CGMS Client.  In order to     */
   /*          handle this situation, the CGMS Server will be notified  */
   /*          if a CRC is present and valid by either the generated    */
   /*          etCGMS_Server_Write_Session_Start_Time_Request or        */
   /*          etCGMS_Server_Specific_Ops_Control_Point_Command CGMS    */
   /*          events, in the CGMS_Event_Callback().  These events will */
   /*          contain the following structures that use these bit      */
   /*          values: CGMS_Write_Session_Start_Time_Data_t (Flags      */
   /*          field), and                                              */
   /*          CGMS_Specific_Ops_Control_Point_Command_Data_t (Flags    */
   /*          field).  This way the CGMS Server will be able to        */
   /*          determine if a CRC is present and valid, and send the    */
   /*          appropriate response to the CGMS Client.                 */
   /* * NOTE * Since the Flags field of a CGMS Measurement does not     */
   /*          indicate whether a CRC is present as an optional field   */
   /*          (Size indicates this), the CGMS Measurements, when       */
   /*          decoded, will each be marked in the (CRCFlags field) of  */
   /*          the CGMS_Measurement_Data_t structure by the bit values  */
   /*          CGMS_E2E_CRC_PRESENT and CGMS_E2E_CRC_VALID to indicate  */
   /*          if each CGMS Measurement has a CRC present and if it is  */
   /*          valid.  This way if multiple CGMS Measurements are       */
   /*          decoded and some contain a CRC while others do not, they */
   /*          can still be decoded.                                    */
#define CGMS_E2E_CRC_PRESENT                                    (0x01)
#define CGMS_E2E_CRC_VALID                                      (0x02)

   /* The following structure contains the Handles that will need to be */
   /* cached by a CGMS client in order to only do service discovery     */
   /* once.                                                             */
typedef struct _tagCGMS_Client_Information_t
{
   Word_t CGMS_Measurement;
   Word_t CGMS_Measurement_Client_Configuration;
   Word_t CGMS_Feature;
   Word_t CGMS_Status;
   Word_t CGMS_Session_Start_Time;
   Word_t CGMS_Session_Run_Time;
   Word_t Record_Access_Control_Point;
   Word_t RACP_Client_Configuration;
   Word_t Specific_Ops_Control_Point;
   Word_t SOCP_Client_Configuration;
} CGMS_Client_Information_t;

#define CGMS_CLIENT_INFORMATION_DATA_SIZE                       (sizeof(CGMS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a CGMS Server.                          */
typedef struct _tagCGMS_Server_Information_t
{
   Word_t CGMS_Measurement_Client_Configuration;
   Word_t RACP_Client_Configuration;
   Word_t SOCP_Client_Configuration;
} CGMS_Server_Information_t;

#define CGMS_SERVER_INFORMATION_DATA_SIZE                       (sizeof(CGMS_Server_Information_t))

   /* The following structure contains CGMS Measurement data that is    */
   /* passed to the function that builds the CGMS Measurement packet.   */
   /* * NOTE * The Size Field represents the size of the CGMS           */
   /*          Measurement record (Including the CRC).  The Size Field  */
   /*          itself should be included in the length calculation.     */
   /* * NOTE * If CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT Flag  */
   /*          is set, then a valid value must be entered for           */
   /*          TrendInformation.                                        */
   /* * NOTE * If CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT Flag is set,    */
   /*          then a valid value must be entered for Quality.          */
   /* * NOTE * If                                                       */
   /*          CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          SensorStatusAnnunciation[0] Status field.                */
   /* * NOTE * If                                                       */
   /*          CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TE  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          SensorStatusAnnunciation[1] CAL/Temp field.              */
   /* * NOTE * If                                                       */
   /*          CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNIN  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          SensorStatusAnnunciation[2] Warning field.               */
   /* * NOTE * Since the Flags field of a CGMS Measurement does not     */
   /*          indicate whether a CRC is present as an optional field   */
   /*          (Size indicates this), the CGMS Measurements, when       */
   /*          decoded, will each be marked in the (CRCFlags field) of  */
   /*          the CGMS_Measurement_Data_t structure by the bit values  */
   /*          CGMS_E2E_CRC_PRESENT and CGMS_E2E_CRC_VALID to indicate  */
   /*          if each CGMS Measurement has a CRC present and if it is  */
   /*          valid.  This way if multiple CGMS Measurements are       */
   /*          decoded and some contain a CRC while others do not, they */
   /*          can still be decoded.                                    */
typedef struct _tagCGMS_Measurement_Data_t
{
   unsigned int Size;
   unsigned int Flags;
   Word_t       GlucoseConcentration;
   Word_t       TimeOffset;
   Byte_t       SensorStatus;
   Byte_t       SensorCalTemp;
   Byte_t       SensorWarning;
   Word_t       TrendInformation;
   Word_t       Quality;
   Byte_t       CRCFlags;
} CGMS_Measurement_Data_t;

#define CGMS_MEASUREMENT_DATA_SIZE                              (sizeof(CGMS_Measurement_Data_t))

   /* The following structure defines the format of CGMS Feature        */
   /* characteristic.  The value of Features bitmask field is of the    */
   /* form CGMS_FEATURE_FLAG_XXX and the value of TypeSampleLocation is */
   /* a combination of 2 nibbles bitmask of the form                    */
   /* CGMS_FEATURE_TYPE_XXX or/and CGMS_FEATURE_SAMPLE_XXX.             */
typedef struct _tagCGMS_Feature_Data_t
{
   DWord_t Features;
   Byte_t  TypeSampleLocation;
} CGMS_Feature_Data_t;

#define CGMS_FEATURE_DATA_SIZE                                  (sizeof(CGMS_Feature_Data_t))

   /* The following structure defines the format of CGMS Status         */
   /* characteristic.  The value of Status bitmask field is of the form */
   /* CGMS_SENSOR_STATUS_ANNUNCIATION_XXX.                              */
typedef struct _tagCGMS_Status_Data_t
{
   Word_t  TimeOffset;
   DWord_t Status;
} CGMS_Status_Data_t;

#define CGMS_STATUS_DATA_SIZE                                   (sizeof(CGMS_Status_Data_t))

   /* The followng defines the format of CGMS Date Time.                */
typedef struct _tagCGMS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
}  CGMS_Date_Time_Data_t;

#define CGMS_DATE_TIME_DATA_SIZE                                (sizeof(CGMS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Date Time is valid.  The only parameter to this  */
   /* function is the CGMS_Date_Time_Data_t structure to validate.  This*/
   /* MACRO returns TRUE if the Date Time is valid or FALSE otherwise.  */
#define CGMS_DATE_TIME_VALID(_x)                                ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following enumerated type defines all the valid values of Time*/
   /* Zone and will be used as Time Zone field of CGMS Session Start    */
   /* Time characteristic.                                              */
typedef enum
{
   tizUTCMinus1200 = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_12_00,
   tizUTCMinus1100 = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_11_00,
   tizUTCMinus1000 = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_10_00,
   tizUTCMinus930  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_9_30,
   tizUTCMinus900  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_9_00,
   tizUTCMinus800  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_8_00,
   tizUTCMinus700  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_7_00,
   tizUTCMinus600  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_6_00,
   tizUTCMinus500  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_5_00,
   tizUTCMinus430  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_4_30,
   tizUTCMinus400  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_4_00,
   tizUTCMinus330  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_3_30,
   tizUTCMinus300  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_3_00,
   tizUTCMinus200  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_2_00,
   tizUTCMinus100  = CGMS_TIME_ZONE_UTC_OFFSET_MINUS_1_00,
   tizUTCPlus000   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_0_00,
   tizUTCPlus100   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_1_00,
   tizUTCPlus200   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_2_00,
   tizUTCPlus300   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_3_00,
   tizUTCPlus330   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_3_30,
   tizUTCPlus400   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_4_00,
   tizUTCPlus430   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_4_30,
   tizUTCPlus500   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_00,
   tizUTCPlus530   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_30,
   tizUTCPlus545   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_45,
   tizUTCPlus600   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_6_00,
   tizUTCPlus630   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_6_30,
   tizUTCPlus700   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_7_00,
   tizUTCPlus800   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_8_00,
   tizUTCPlus845   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_8_45,
   tizUTCPlus900   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_9_00,
   tizUTCPlus930   = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_9_30,
   tizUTCPlus1000  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_10_00,
   tizUTCPlus1030  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_10_30,
   tizUTCPlus1100  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_11_00,
   tizUTCPlus1130  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_11_30,
   tizUTCPlus1200  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_12_00,
   tizUTCPlus1245  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_12_45,
   tizUTCPlus1300  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_13_00,
   tizUTCPlus1400  = CGMS_TIME_ZONE_UTC_OFFSET_PLUS_14_00,
   tizUTCUnknown   = CGMS_TIME_ZONE_UTC_OFFSET_UNKNOWN
} CGMS_Time_Zone_Type_t;

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified Time Zone is valid.  The only parameter to this  */
   /* function is the Time Zone value to validate.  This MACRO returns  */
   /* TRUE if the Time Zone is valid or FALSE otherwise.                */
#define CGMS_TIME_ZONE_VALID(_x)                                (((_x) >= tizUTCMinus1200) && ((_x) <= tizUTCPlus1400))

   /* The following enumerated type defines all the valid values of DST */
   /* Offset and will be used as DST Offset field of CGMS Session Start */
   /* Time characteristic.                                              */
typedef enum
{
   dsoStandardTime           = CGMS_DST_OFFSET_STANDARD_TIME,
   dsoHalfAnHourDaylightTime = CGMS_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME,
   dsoDaylightTime           = CGMS_DST_OFFSET_DAYLIGHT_TIME,
   dsoDoubleDaylightTime     = CGMS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME,
   dsoUnknown                = CGMS_DST_OFFSET_UNKNOWN
} CGMS_DST_Offset_Type_t;

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified DST Offset is valid.  The only parameter to this */
   /* function is the DST Offset value to validate.  This MACRO returns */
   /* TRUE if the DST Offset is valid or FALSE otherwise.               */
#define CGMS_DST_OFFSET_VALID(_x)                               (((_x) >= dsoStandardTime) && ((_x) <= dsoDoubleDaylightTime))

   /* The following structure defines the format of CGMS Session Time.  */
   /* This structure will be used to represent CGMS Session Start Time. */
typedef struct _tagCGMS_Session_Start_Time_Data_t
{
   CGMS_Date_Time_Data_t  Time;
   CGMS_Time_Zone_Type_t  TimeZone;
   CGMS_DST_Offset_Type_t DSTOffset;
} CGMS_Session_Start_Time_Data_t;

#define CGMS_SESSION_START_TIME_DATA_SIZE                       (sizeof(CGMS_Session_Start_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* that a specified CGMS Session Time is valid.  The only parameter  */
   /* to this function is the CGMS_Session_Time_Data_t structure to     */
   /* validate.  This MACRO returns TRUE if the CGMS Session Time is    */
   /* valid or FALSE otherwise.                                         */
#define CGMS_SESSION_TIME_VALID(_x)                             ((CGMS_DATE_TIME_VALID((_x).Time)) && (CGMS_TIME_ZONE_VALID((_x).TimeZone)) && (CGMS_DST_OFFSET_VALID((_x).DSTOffset)))

   /* The following structure defines the format of CGMS Session Run    */
   /* Time characteristic.                                              */
typedef struct _tagCGMS_Session_Run_Time_Data_t
{
   Word_t SessionRunTime;
} CGMS_Session_Run_Time_Data_t;

#define CGMS_SESSION_RUN_TIME_DATA_SIZE                         (sizeof(CGMS_Session_Run_Time_Data_t))

   /* The following defines the enum of RACP Response Type that is      */
   /* passed in the RACP Response strcuture.                            */
typedef enum
{
   rarCGMSNumberOfStoredRecords = CGMS_RACP_OPCODE_NUMBER_OF_STORED_RECORDS_RESPONSE,
   rarCGMSResponseCode          = CGMS_RACP_OPCODE_RESPONSE_CODE
} CGMS_RACP_Response_Type_t;

   /* The following defines the structure of RACP Response Code value   */
   /* that is passed in the RACP Response strcuture.  The values of     */
   /* RequestOpCode is of the form CGMS_RACP_OPCODE_XXX and the value of*/
   /* ResponseCodeValue is of the form CGMS_RACP_RESPONSE_CODE_XXX.     */
typedef struct _tagCGMS_RACP_Response_Code_Value_t
{
   Byte_t RequestOpCode;
   Byte_t ResponseCodeValue;
} CGMS_RACP_Response_Code_Value_t;

#define CGMS_RACP_RESPONSE_CODE_VALUE_SIZE                      (sizeof(CGMS_RACP_Response_Code_Value_t))

   /* The following defines the format of a Record Access Control Point */
   /* Response Data.  This structure will hold the RACP response data   */
   /* received from remote CGMS Server.  The first member specifies the */
   /* Response Type.  The second member is a union of response type     */
   /* value based on Response Type.                                     */
typedef struct _tagCGMS_RACP_Response_Data_t
{
   CGMS_RACP_Response_Type_t ResponseType;
   union
   {
      Word_t                          NumberOfStoredRecordsResult;
      CGMS_RACP_Response_Code_Value_t ResponseCodeValue;
   } ResponseData;
} CGMS_RACP_Response_Data_t;

#define CGMS_RACP_RESPONSE_DATA_SIZE                            (sizeof(CGMS_RACP_Response_Data_t))

   /* The following define the valid Read Request types that a server   */
   /* may receive in a etCGMS_Server_Read_Client_Configuration_Request  */
   /* or etCGMS_Server_Client_Configuration_Update event.  This is also */
   /* used by the CGMS_Send_Notification to denote the characteristic   */
   /* value to notify.                                                  */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctCGMSMeasurement,
   ctCGMSRecordAccessControlPoint,
   ctCGMSSpecificOpsControlPoint
} CGMS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* CGMS Profile.  These are used to determine the type of each event */
   /* generated, and to ensure the proper union element is accessed for */
   /* the CGMS_Event_Data_t structure.                                  */
typedef enum
{
   etCGMS_Server_Read_Client_Configuration_Request,
   etCGMS_Server_Client_Configuration_Update,
   etCGMS_Server_Read_Feature_Request,
   etCGMS_Server_Read_Status_Request,
   etCGMS_Server_Read_Session_Start_Time_Request,
   etCGMS_Server_Write_Session_Start_Time_Request,
   etCGMS_Server_Read_Session_Run_Time_Request,
   etCGMS_Server_Record_Access_Control_Point_Command,
   etCGMS_Server_Specific_Ops_Control_Point_Command,
   etCGMS_Server_Confirmation_Data
} CGMS_Event_Type_t;

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to read the Client Configuration */
   /* descriptor.  The ConnectionID and RemoteDevice identifies the     */
   /* Client that is making the request.  The TransactionID specifies   */
   /* the TransactionID of the request, this can be used when responding*/
   /* to the request using the CGMS_Client_Configuration_Read_Response()*/
   /* API function.                                                     */
typedef struct _tagCGMS_Read_Client_Configuration_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   CGMS_Characteristic_Type_t ClientConfigurationType;
} CGMS_Read_Client_Configuration_Data_t;

#define CGMS_READ_CLIENT_CONFIGURATION_DATA_SIZE                (sizeof(CGMS_Read_Client_Configuration_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client attempts to write to a Client Configuration    */
   /* descriptor.  The ConnectionID and RemoteDevice identify the Client*/
   /* that is making the update request.  The ClientConfiguration value */
   /* specifies the new Client Configuration value.                     */
typedef struct _tagCGMS_Client_Configuration_Update_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   CGMS_Characteristic_Type_t ClientConfigurationType;
   Word_t                     ClientConfiguration;
} CGMS_Client_Configuration_Update_Data_t;

#define CGMS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE              (sizeof(CGMS_Client_Configuration_Update_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to read the Feature              */
   /* characteristic.  The ConnectionID, ConnectionType and RemoteDevice*/
   /* specifies the Client that is making the request.                  */
typedef struct _tagCGMS_Read_Feature_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} CGMS_Read_Feature_Data_t;

#define CGMS_READ_FEATURE_DATA_SIZE                             (sizeof(CGMS_Read_Feature_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to read the Status               */
   /* characteristic.  The ConnectionID, ConnectionType and RemoteDevice*/
   /* specifies the Client that is making the request.                  */
typedef struct _tagCGMS_Read_Status_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} CGMS_Read_Status_Data_t;

#define CGMS_READ_STATUS_DATA_SIZE                              (sizeof(CGMS_Read_Status_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to read the Session Start Time.  */
   /* The ConnectionID, ConnectionType and RemoteDevice specifies the   */
   /* Client that is making the request.                                */
typedef struct _tagCGMS_Read_Session_Start_Time_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} CGMS_Read_Session_Start_Time_Data_t;

#define CGMS_READ_SESSION_START_TIME_DATA_SIZE                  (sizeof(CGMS_Read_Session_Start_Time_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to write the Session Start Time. */
   /* The ConnectionID, ConnectionType and RemoteDevice specifies the   */
   /* Client that is making the update request.  The Flags indicates if */
   /* a CRC was present with the Session Start Time Data sent to the    */
   /* CGMS Server before this event was generated.  The SessionStartTime*/
   /* value specifies the new Session Start Time value.                 */
   /* ** NOTE * If the Session Start Time Characteristic written, we    */
   /*          cannot internally determine if a CRC is present and valid*/
   /*          with the incoming data from the CGMS Client.  In order to*/
   /*          handle this situation, the CGMS Server will be notified  */
   /*          if a CRC is present and valid by the generated           */
   /*          etCGMS_Server_Write_Session_Start_Time_Request event,    */
   /*          sent to the CGMS_Event_Callback().  This event will      */
   /*          contain the following structure that uses the bit values */
   /*          (below): CGMS_Write_Session_Start_Time_Data_t (Flags     */
   /*          field).  This way the CGMS Server will be able to        */
   /*          determine if a CRC is present and valid, and send the    */
   /*          appropriate response to the CGMS Client.                 */
   /* * NOTE * Valid values for the Flags field are CGMS_E2E_CRC_PRESENT*/
   /*          and CGMS_E2E_CRC_VALID.                                  */
typedef struct _tagCGMS_Write_Session_Start_Time_Data_t
{
   unsigned int                   InstanceID;
   unsigned int                   ConnectionID;
   unsigned int                   TransactionID;
   GATT_Connection_Type_t         ConnectionType;
   BD_ADDR_t                      RemoteDevice;
   Byte_t                         Flags;
   CGMS_Session_Start_Time_Data_t SessionStartTime;
} CGMS_Write_Session_Start_Time_Data_t;

#define CGMS_WRITE_SESSION_START_TIME_DATA_SIZE                 (sizeof(CGMS_Write_Session_Start_Time_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client is attempting to read the Session Run Time.    */
   /* The ConnectionID, ConnectionType and RemoteDevice specifies the   */
   /* Client that is making the request.                                */
typedef struct _tagCGMS_Read_Session_Run_Time_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} CGMS_Read_Session_Run_Time_Data_t;

#define CGMS_READ_SESSION_RUN_TIME_DATA_SIZE                    (sizeof(CGMS_Read_Session_Run_Time_Data_t))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of Record Access Control Point         */
   /* characteristic.                                                   */
typedef enum
{
   racCGMSReportStoredRecordsRequest   = CGMS_RACP_OPCODE_REPORT_STORED_RECORDS,
   racCGMSDeleteStoredRecordsRequest   = CGMS_RACP_OPCODE_DELETE_STORED_RECORDS,
   racCGMSAbortOperationRequest        = CGMS_RACP_OPCODE_ABORT_OPERATION,
   racCGMSNumberOfStoredRecordsRequest = CGMS_RACP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS
} CGMS_RACP_Command_Type_t;

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Operator field of Record Access Control Point       */
   /* characteristic.                                                   */
typedef enum
{
   raoCGMSNull                 = CGMS_RACP_OPERATOR_NULL,
   raoCGMSAllRecords           = CGMS_RACP_OPERATOR_ALL_RECORDS,
   raoCGMSLessThanOrEqualTo    = CGMS_RACP_OPERATOR_LESS_THAN_OR_EQUAL_TO,
   raoCGMSGreaterThanOrEqualTo = CGMS_RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO,
   raoCGMSWithinRangeOf        = CGMS_RACP_OPERATOR_WITHIN_RANGE_OF,
   raoCGMSFirstRecord          = CGMS_RACP_OPERATOR_FIRST_RECORD,
   raoCGMSLastRecord           = CGMS_RACP_OPERATOR_LAST_RECORD
} CGMS_RACP_Operator_Type_t;

   /* The following enumerates the valid values that may be used as the */
   /* Filter Type values of a Record Access Control Point               */
   /* characteristic.                                                   */
typedef enum
{
   rafTimeOffset = CGMS_RACP_FILTER_TYPE_TIME_OFFSET,
} CGMS_RACP_Filter_Type_t;

   /* The following strcuture defines a Time Offset Range from Minimum  */
   /* to Maximum value.                                                 */
typedef struct _tagCGMS_Time_Offset_Range_Data_t
{
   Word_t Minimum;
   Word_t Maximum;
} CGMS_Time_Offset_Range_Data_t;

#define CGMS_TIME_OFFSET_RANGE_DATA_SIZE                        (sizeof(CGMS_Time_Offset_Range_Data_t))

   /* The following structure defines the format of the Record Access   */
   /* Control Point Command Request Data.  This structure is passed as a*/
   /* parameter to CGMS_Format_Record_Access_Control_Point_Command API. */
   /* * NOTE * The CommandType member is an enumerated type that must be*/
   /*          of the form CGMS_RACP_OPCODE_XXX.                        */
   /* * NOTE * The OperatorType member is an enumerated type that must  */
   /*          be of the form CGMS_RACP_OPERATOR_XXX.                   */
   /* * NOTE * The FilterType member is an enumerated type that must be */
   /*          of the form CGMS_RACP_FILTER_TYPE_XXX.                   */
typedef struct _tagCGMS_RACP_Format_Data_t
{
   Byte_t CommandType;
   Byte_t OperatorType;
   Byte_t FilterType;
   union
   {
      Word_t                        TimeOffset;
      CGMS_Time_Offset_Range_Data_t TimeOffsetRange;
   } FilterParameters;
} CGMS_RACP_Format_Data_t;

#define CGMS_RACP_FORMAT_DATA_SIZE                              (sizeof(CGMS_RACP_Format_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client has written a command to the CGMS Record Access*/
   /* Control Point.  The ConnectionID, ConnectionType and RemoteDevice */
   /* specifies the Client that is making the write request.  The       */
   /* FormatData specifies the RACP Format that the Client is sending.  */
typedef struct _tagCGMS_RACP_Command_Data_t
{
   unsigned int            InstanceID;
   unsigned int            ConnectionID;
   unsigned int            TransactionID;
   GATT_Connection_Type_t  ConnectionType;
   BD_ADDR_t               RemoteDevice;
   CGMS_RACP_Format_Data_t FormatData;
} CGMS_Record_Access_Control_Point_Command_Data_t;

#define CGMS_RACP_COMMAND_DATA_SIZE                             (sizeof(CGMS_Record_Access_Control_Point_Command_Data_t))

   /* The following defines the values that may be set as the value of  */
   /* CGMS Communication Interval field in CGMS Specific Operation      */
   /* Control Point Characteristic.                                     */
#define CGMS_COMMUNICATION_INTERVAL_VALUE_DISABLE_PERIODIC_COMMUNICATION          0x00
#define CGMS_COMMUNICATION_INTERVAL_SMALLEST_INTERVAL_VALUE                       0xFF

   /* The following defines the values that may be set as the value of  */
   /* Calibration Data Record Number field during the Get Glucose       */
   /* Calibration procedure in CGMS Specific Ops Control Point          */
   /* Characteristic.                                                   */
#define CGMS_GLUCOSE_CALIBRATION_DATA_RECORD_NUMBER_NO_STORED_CALIBRATION_DATA    0x0000
#define CGMS_GLUCOSE_CALIBRATION_DATA_RECORD_NUMBER_LAST_STORED_CALIBRATION_DATA  0xFFFF

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of CGMS Specific Ops Control Point     */
   /* characteristic.                                                   */
typedef enum
{
   cgcSetCGMCommunicationInterval = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_CGM_COMMUNICATION_INTERVAL,
   cgcGetCGMCommunicationInterval = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_CGM_COMMUNICATION_INTERVAL,
   cgcSetGlucoseCalibrationValue  = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_GLUCOSE_CALIBRATION_VALUE,
   cgcGetGlucoseCalibrationValue  = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_GLUCOSE_CALIBRATION_VALUE,
   cgcSetPatientHighAlertLevel    = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_PATIENT_HIGH_ALERT_LEVEL,
   cgcGetPatientHighAlertLevel    = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_PATIENT_HIGH_ALERT_LEVEL,
   cgcSetPatientLowAlertLevel     = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_PATIENT_LOW_ALERT_LEVEL,
   cgcGetPatientLowAlertLevel     = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_PATIENT_LOW_ALERT_LEVEL,
   cgcSetHypoAlertLevel           = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_HYPO_ALERT_LEVEL,
   cgcGetHypoAlertLevel           = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_HYPO_ALERT_LEVEL,
   cgcSetHyperAlertLevel          = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_HYPER_ALERT_LEVEL,
   cgcGetHyperAlertLevel          = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_HYPER_ALERT_LEVEL,
   cgcSetRateOfDecreaseAlertLevel = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_RATE_OF_DECREASE_ALERT_LEVEL,
   cgcGetRateOfDecreaseAlertLevel = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_RATE_OF_DECREASE_ALERT_LEVEL,
   cgcSetRateOfIncreaseAlertLevel = CGMS_SPECIFIC_OPS_CP_OPCODE_SET_RATE_OF_INCREASE_ALERT_LEVEL,
   cgcGetRateOfIncreaseAlertLevel = CGMS_SPECIFIC_OPS_CP_OPCODE_GET_RATE_OF_INCREASE_ALERT_LEVEL,
   cgcResetDeviceSpecificAlert    = CGMS_SPECIFIC_OPS_CP_OPCODE_RESET_DEVICE_SPECIFIC_ALERT,
   cgcStartSession                = CGMS_SPECIFIC_OPS_CP_OPCODE_START_SESSION,
   cgcStopSession                 = CGMS_SPECIFIC_OPS_CP_OPCODE_STOP_SESSION
} CGMS_SOCP_Command_Type_t;

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Response Opcode field of CGMS Specific Ops Control  */
   /* Point characteristic.                                             */
typedef enum
{
   cgrCommunicationIntervalResponse    = CGMS_SPECIFIC_OPS_CP_OPCODE_COMMUNICATION_INTERVAL_RESPONSE ,
   cgrCalibrationValueResponse         = CGMS_SPECIFIC_OPS_CP_OPCODE_CALIBRATION_VALUE_RESPONSE,
   cgrPatientHighAlertLevelResponse    = CGMS_SPECIFIC_OPS_CP_OPCODE_PATIENT_HIGH_ALERT_LEVEL_RESPONSE,
   cgrPatientLowAlertLevelResponse     = CGMS_SPECIFIC_OPS_CP_OPCODE_PATIENT_LOW_ALERT_LEVEL_RESPONSE,
   cgrHypoAlertLevelResponse           = CGMS_SPECIFIC_OPS_CP_OPCODE_HYPO_ALERT_LEVEL_RESPONSE,
   cgrHyperAlertLevelResponse          = CGMS_SPECIFIC_OPS_CP_OPCODE_HYPER_ALERT_LEVEL_RESPONSE,
   cgrRateOfDecreaseAlertLevelResponse = CGMS_SPECIFIC_OPS_CP_OPCODE_RATE_OF_DECREASE_ALERT_LEVEL_RESPONSE,
   cgrRateOfIncreaseAlertLevelResponse = CGMS_SPECIFIC_OPS_CP_OPCODE_RATE_OF_INCREASE_ALERT_LEVEL_RESPONSE,
   cgrResponse                         = CGMS_SPECIFIC_OPS_CP_OPCODE_RESPONSE
} CGMS_SOCP_Response_Type_t;

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Response Opcode field of CGMS Specific Ops Control  */
   /* Point characteristic.                                             */
typedef enum
{
   cgrSuccess               = CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_SUCCESS,
   cgrOpcodeNotSupported    = CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_NOT_SUPPORTED,
   cgrInvalidOperand        = CGMS_SPECIFIC_OPS_CP_RESPONSE_INVALID_OPERAND,
   cgrProcedureNotCompleted = CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PROCEDURE_NOT_COMPLETED,
   cgrParameterOutOfRange   = CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PARAMETER_OUT_OF_RANGE
} CGMS_SOCP_Response_Value_t;

   /* The following defines the structure of CGMS SOCP Response Code    */
   /* value that is passed in the CGMS SOCP Response structure.  The    */
   /* value of RequestOpCode is of the form                             */
   /* CGMS_SPECIFIC_OPS_CP_OPCODE_XXX and the value of ResponseCodeValue*/
   /* is of the form CGMS_SPECIFIC_OPS_CP_RESPONSE_XXX.                 */
typedef struct _tagCGMS_SOCP_Response_Code_Value_t
{
   Byte_t RequestOpCode;
   Byte_t ResponseCodeValue;
} CGMS_SOCP_Response_Code_Value_t;

#define CGMS_SOCP_RESPONSE_CODE_VALUE_SIZE                     (sizeof(CGMS_SOCP_Response_Code_Value_t))

   /* The following structure defines the Calibration Record Structure. */
   /* This is member of CGMS_Specific_Ops_Control_Point_Format_Data_t.  */
   /* The first data member is glucose concetration of calibration it is*/
   /* measured in mg/dl.  Second member is calibration Time, Third      */
   /* member is Sample Location.  Forth Member is Next Calibration Time */
   /* in Minutes and last member is calibration data record number.     */
typedef struct _tagCGMS_Calibration_Data_Record_t
{
   Word_t CalibrationGlucoseConcentration;
   Word_t CalibrationTime;
   Byte_t CalibrationTypeSampleLocation;
   Word_t NextCalibrationTime;
   Word_t CalibrationDataRecordNumber;
   Byte_t CalibrationStatus;
} CGMS_Calibration_Data_Record_t;

#define CGMS_CALIBRATION_DATA_RECORD_SIZE                       (sizeof(CGMS_Calibration_Data_Record_t))

   /* The following structure defines the format of CGMS Specific Ops   */
   /* Control Point Command Request Data.  This structure is passed as a*/
   /* parameter to CGMS_Format_CGMS_Specific_Ops_Control_Point_Command  */
   /* API.                                                              */
typedef struct _tagCGMS_Specific_Ops_Control_Point_Format_Data_t
{
   CGMS_SOCP_Command_Type_t CommandType;
   union
   {
      Byte_t                         CommunicationIntervalMinutes;
      CGMS_Calibration_Data_Record_t CalibrationDataRecord;
      Word_t                         CalibrationDataRecordNumber;
      Word_t                         AlertLevel;
   } CommandParameters;
} CGMS_Specific_Ops_Control_Point_Format_Data_t;

#define CGMS_SPECIFIC_OPS_CONTROL_POINT_FORMAT_DATA_SIZE        (sizeof(CGMS_Specific_Ops_Control_Point_Format_Data_t))

   /* The following defines the format of a CGMS Specific Ops Control   */
   /* Point Response Data.  This structure will hold the CGMS SOCP      */
   /* response data received from remote CGMS Server.  The first member */
   /* specifies the Response Type.  The second member is the            */
   /* ResponseData.                                                     */
typedef struct _tagCGMS_Specific_Ops_Control_Point_Response_Data_t
{
   CGMS_SOCP_Response_Type_t ResponseType;
   union
   {
      Byte_t                          CommunicationIntervalMinutes;
      CGMS_Calibration_Data_Record_t  CalibrationDataRecord;
      Word_t                          AlertLevel;
      CGMS_SOCP_Response_Code_Value_t ResponseCodeValue;
   } ResponseData;
} CGMS_Specific_Ops_Control_Point_Response_Data_t;

#define CGMS_SPECIFIC_OPS_CONTROL_POINT_RESPONSE_DATA_SIZE      (sizeof(CGMS_Specific_Ops_Control_Point_Response_Data_t))

   /* The following is dispatched to a CGMS Server in response to the   */
   /* reception of request from a Client to write to the Specific       */
   /* operations Control Point.  The Flags indicates if a CRC was       */
   /* present and valid with the Specific Ops Control Point Command Data*/
   /* sent to the CGMS Server before this event was generated.  The     */
   /* FormatData specifies the formatted CGMS Specific Ops Control Point*/
   /* Command Data.                                                     */
   /* * NOTE * If the Specific Ops Control Point Characteristic is      */
   /*          written, we cannot internally determine if a CRC is      */
   /*          present and valid with the incoming data from the CGMS   */
   /*          Client.  In order to handle this situation, the CGMS     */
   /*          Server will be notified if a CRC is present and valid by */
   /*          the generated                                            */
   /*          etCGMS_Server_Specific_Ops_Control_Point_Command event,  */
   /*          sent to the CGMS_Event_Callback().  This event will      */
   /*          contain the following structure that uses these bit      */
   /*          values (below):                                          */
   /*          CGMS_Specific_Ops_Control_Point_Command_Data_t (Flags    */
   /*          field).  This way the CGMS Server will be able to        */
   /*          determine if a CRC is present and valid, and send the    */
   /*          appropriate response to the CGMS Client.                 */
   /* * NOTE * Valid values for the Flags field are CGMS_E2E_CRC_PRESENT*/
   /*          and CGMS_E2E_CRC_VALID.                                  */
typedef struct _tagCGMS_Specific_Ops_Control_Point_Command_Data_t
{
   unsigned int                                  InstanceID;
   unsigned int                                  ConnectionID;
   unsigned int                                  TransactionID;
   GATT_Connection_Type_t                        ConnectionType;
   BD_ADDR_t                                     RemoteDevice;
   Byte_t                                        Flags;
   CGMS_Specific_Ops_Control_Point_Format_Data_t FormatData;
} CGMS_Specific_Ops_Control_Point_Command_Data_t;

#define CGMS_SPECIFIC_OPS_CONTROL_POINT_COMMAND_DATA_SIZE       (sizeof(CGMS_Specific_Ops_Control_Point_Command_Data_t))

   /* The following CGMS Profile Event is dispatched to a CGMS Server   */
   /* when a CGMS Client has sent a confirmation to a previously sent   */
   /* confirmation.  The ConnectionID, ConnectionType and RemoteDevice  */
   /* specifies the Client that is making the update.  The              */
   /* CharacteristicType specifies which Indication the Client has sent */
   /* a confirmation for.  The Status field specifies the status of the */
   /* Indication                                                        */
   /* * NOTE * The Characteristic_Type parameter will NEVER be set to   */
   /*          ctIntermediateTemperature for this event.                */
   /* * NOTE *The Status member is set to one of the following values:  */
   /*          GATT_CONFIRMATION_STATUS_SUCCESS                         */
   /*          GATT_CONFIRMATION_STATUS_TIMEOUT.                        */
typedef struct _tagCGMS_Confirmation_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   CGMS_Characteristic_Type_t CharacteristicType;
   Byte_t                     Status;
} CGMS_Confirmation_Data_t;

#define CGMS_CONFIRMATION_DATA_SIZE                             (sizeof(CGMS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all CGMS Profile Event Data.  This structure is received  */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagCGMS_Event_Data_t
{
   CGMS_Event_Type_t Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      CGMS_Read_Client_Configuration_Data_t           *CGMS_Read_Client_Configuration_Data;
      CGMS_Client_Configuration_Update_Data_t         *CGMS_Client_Configuration_Update_Data;
      CGMS_Read_Feature_Data_t                        *CGMS_Read_Feature_Data;
      CGMS_Read_Status_Data_t                         *CGMS_Read_Status_Data;
      CGMS_Read_Session_Start_Time_Data_t             *CGMS_Read_Session_Start_Time_Data;
      CGMS_Write_Session_Start_Time_Data_t            *CGMS_Write_Session_Start_Time_Data;
      CGMS_Read_Session_Run_Time_Data_t               *CGMS_Read_Session_Run_Time_Data;
      CGMS_Record_Access_Control_Point_Command_Data_t *CGMS_Record_Access_Control_Point_Command_Data;
      CGMS_Specific_Ops_Control_Point_Command_Data_t  *CGMS_Specific_Ops_Control_Point_Command_Data;
      CGMS_Confirmation_Data_t                        *CGMS_Confirmation_Data;
   } Event_Data;
} CGMS_Event_Data_t;

#define CGMS_EVENT_DATA_SIZE                                    (sizeof(CGMS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a CGMS Profile Event Receive Data Callback.  This function will be*/
   /* called whenever an CGMS Profile Event occurs that is associated   */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the CGMS Event Data that       */
   /* occurred and the CGMS Profile Event Callback Parameter that was   */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the CGMS Profile Event Data ONLY in the       */
   /* context of this callback.  If the caller requires the Data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have to be       */
   /* re-entrant).  It needs to be noted however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another CGMS Profile Event will not be processed while this       */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for events that can*/
   /*          only be satisfied by Receiving CGMS Profile Event        */
   /*          Packets.  A Deadlock WILL occur because NO CGMS Event    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
typedef void (BTPSAPI *CGMS_Event_Callback_t)(unsigned int BluetoothStackID, CGMS_Event_Data_t *CGMS_Event_Data, unsigned long CallbackParameter);

   /* CGMS Server API.                                                  */

   /* The following function is responsible for opening a CGMS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the GATT Service Flags.*/
   /* These flags can be used to register GATT for LE, BR/EDR, or DUAL  */
   /* MODE.  The third parameter is the Callback function to call when  */
   /* an event occurs on this Server Port.  The fourth parameter is a   */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered CGMS       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 CGMS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * The GATT Service Flags may be CGMS_SERVICE_FLAGS_LE,     */
   /*          CGMS_SERVICE_FLAGS_BR_EDR, or                            */
   /*          CGMS_SERVICE_FLAGS_DUAL_MODE.  These are defined in      */
   /*          CGMSTypes.h.                                             */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Initialize_Service(unsigned int BluetoothStackID, unsigned int Flags, CGMS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Initialize_Service_t)(unsigned int BluetoothStackID, unsigned int Flags, CGMS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a CGMS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter will specify the GATT Service Flags.*/
   /* These flags can be used to register GATT for LE, BR/EDR, or DUAL  */
   /* MODE.  The third parameter is the Callback function to call when  */
   /* an event occurs on this Server Port.  The fourth parameter is a   */
   /* user-defined callback parameter that will be passed to the        */
   /* callback function with each event.  The fifth parameter is a      */
   /* pointer to store the GATT Service ID of the registered CGMS       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 CGMS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
   /* * NOTE * The GATT Service Flags may be CGMS_SERVICE_FLAGS_LE,     */
   /*          CGMS_SERVICE_FLAGS_BR_EDR, or                            */
   /*          CGMS_SERVICE_FLAGS_DUAL_MODE.  These are defined in      */
   /*          CGMSTypes.h.                                             */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, unsigned int Flags, CGMS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, unsigned int Flags, CGMS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened CGMS Server.  The first parameter is the Bluetooth Stack ID*/
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfull call to           */
   /* CGMS_Initialize_Service().  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI CGMS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_CGMS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* CGMS_Suspend()).  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to CGMS_Suspend().  This    */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the CGMS Service that is         */
   /* registered with a call to CGMS_Initialize_Service().  This        */
   /* function returns the non-zero number of attributes that are       */
   /* contained in a CGMS Server or zero on failure.                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI CGMS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_CGMS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Read Client Configuration Request.  The first parameter is the    */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* CGMS_Initialize_Service().  The third is the Transaction ID of the*/
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for sending CGMS Measurement*/
   /* notifications to a specified remote device.  The first parameter  */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CGMS_Initialize_Service().  The third parameter is the            */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The fourth parameter is a flag that indicates if E2E-CRC is       */
   /* supported and needs to be calculated.  The fifth parameter is the */
   /* number of Measurements to be notified.  The final parameter is a  */
   /* buffer of formatted CGMS Measurements that contain all of the     */
   /* required and optional data for notification.  This function       */
   /* returns a positive value indicating how many measurements were    */
   /* sent in the notification if successful or a negative return error */
   /* code if an error occurs.                                          */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with each CGMS       */
   /*          Measurement.                                             */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Notify_CGMS_Measurements(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, unsigned int NumberOfMeasurements, CGMS_Measurement_Data_t **MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Notify_CGMS_Measurements_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, unsigned int NumberOfMeasurements, CGMS_Measurement_Data_t **MeasurementData);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Feature Read Request received from a remote device.  The first    */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CGMS_Initialize_Service().  The third parameter is the         */
   /* TransactionID that was received in the CGMS Feature Read Request  */
   /* event.  The fourth parameter is a flag to indicate if E2E-CRC is  */
   /* supported and needs to be calculated.  The fifth parameter is an  */
   /* error code that is used to determine if the Request is being      */
   /* accepted by the server or if an error response should be issued   */
   /* instead.  The final parameter contains the CGMS Feature to respond*/
   /* with.  This function returns a zero if successful or a negative   */
   /* return error code if an error occurs.                             */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 then this      */
   /*          function will respond to the Request successfully with a */
   /*          valid value in the CGMSFeature parameter.                */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device and CGMSFeature parameter   */
   /*          will be ignored.                                         */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS        */
   /*          Feature.                                                 */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Feature_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Feature_Data_t *CGMSFeature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Feature_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Feature_Data_t *CGMSFeature);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Status Read Request received from a remote device.  The first     */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CGMS_Initialize_Service().  The third parameter is the         */
   /* TransactionID that was received in the CGMS Status Read Request   */
   /* event.  The fourth parameter is a flag to indicate if E2E-CRC is  */
   /* supported and needs to be calculated.  The fifth parameter is an  */
   /* error code that is used to determine if the Request is being      */
   /* accepted by the server or if an error response should be issued   */
   /* instead.  The final parameter contains the CGMS Status to respond */
   /* with.  This function returns a zero if successful or a negative   */
   /* return error code if an error occurs.                             */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 then this      */
   /*          function will respond to the Request successfully with a */
   /*          valid value in the CGMSStatus parameter.                 */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device and CGMSStatus parameter    */
   /*          will be ignored.                                         */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS Status.*/
BTPSAPI_DECLARATION int BTPSAPI CGMS_Status_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Status_Data_t *CGMSStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Status_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Status_Data_t *CGMSStatus);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Session Start Time Read Request received from a remote device.    */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID that was returned */
   /* from a successful call to CGMS_Initialize_Service().  The third   */
   /* parameter is the TransactionID that was received in the CGMS      */
   /* Session Start Time Read Request event.  The fourth parameter is a */
   /* flag to indicate if E2E-CRC is supported and needs to be          */
   /* calculated.  The fifth parameter is an error code that is used to */
   /* determine if the Request is being accepted by the server or if an */
   /* error response should be issued instead.  The final parameter     */
   /* contains the CGMS Session Start Time to respond with.  This       */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 then this      */
   /*          function will respond to the Request successfully with a */
   /*          valid value in the SessionStartTime parameter.           */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device and SessionStartTime        */
   /*          parameter will be ignored.                               */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS Session*/
   /*          Start Time.                                              */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Session_Start_Time_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Session_Start_Time_Data_t *SessionStartTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Session_Start_Time_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Session_Start_Time_Data_t *SessionStartTime);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Session Start Time Write Request received from a remote device.   */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID that was returned */
   /* from a successful call to CGMS_Initialize_Service().  The third   */
   /* parameter is the TransactionID that was received in the CGMS      */
   /* Session Start Time Write Request event.  The final parameter is an*/
   /* error code that is used to determine if the Request is being      */
   /* accepted by the server or if an error response should be issued   */
   /* instead.  This function returns a zero if successful or a negative*/
   /* return error code if an error occurs.                             */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 then this      */
   /*          function will respond to the Request successfully.       */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device.                            */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Session_Start_Time_Write_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Session_Start_Time_Write_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Session Run Time Read Request received from a remote device.  The */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to CGMS_Initialize_Service().  The third parameter*/
   /* is the TransactionID that was received in the CGMS Session Run    */
   /* Time Read Request event.  The fourth parameter is a flag to       */
   /* indicate if E2E-CRC is supported and needs to be calculated.  The */
   /* fifth parameter is an error code that is used to determine if the */
   /* Request is being accepted by the server or if an error response   */
   /* should be issued instead.  The final parameter contains the CGMS  */
   /* Session Run Time to respond with.  This function returns a zero if*/
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 then this      */
   /*          function will respond to the Request successfully with a */
   /*          valid value in the SessionRunTime parameter.             */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device and SessionRunTime parameter*/
   /*          will be ignored.                                         */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS Session*/
   /*          Run Time.                                                */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Session_Run_Time_Read_Request_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Session_Run_Time_Data_t *SessionRunTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Session_Run_Time_Read_Request_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, unsigned int Flags, Byte_t ErrorCode, CGMS_Session_Run_Time_Data_t *SessionRunTime);
#endif

   /* The following function is responsible for responding to a Record  */
   /* Access Control Point Command received from a remote device.  The  */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID that was returned from a   */
   /* successful call to CGMS_Initialize_Service().  The third parameter*/
   /* is the TransactionID that was received in the Record Access       */
   /* Control Point event.  The final parameter is an error code that is*/
   /* used to determine if the Request is being accepted by the server  */
   /* or if an error response should be issued instead.  This function  */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Record Access Control Point commands when the     */
   /*          Server has not been configured properly for RACP         */
   /*          operation, the Client does not have proper authentication*/
   /*          to write to the RACP characteristic, or a RACP procedure */
   /*          with the Client is already in progress.  All other       */
   /*          reasons should return ZERO for the ErrorCode and then    */
   /*          send RACP Result indication to indicate any other errors.*/
   /*          For Example: If the Operand in the Request is not        */
   /*          supported by the Server this API should be called with   */
   /*          ErrorCode set to ZERO and then the                       */
   /*          CGMS_Indicate_Record_Access_Control_Point_Result() should*/
   /*          be called with the ResponseCode set to                   */
   /*          CGMS_RACP_RESPONSE_CODE_OPERAND_NOT_SUPPORTED.           */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Record_Access_Control_Point_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Record_Access_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for indicating the number of*/
   /* stored records to a specified remote device.  The first parameter */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CGMS_Initialize_Service().  The third parameter is the            */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The last parameter is number of stored records to be indicated.   */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * Only 1 Number of Stored Records indication may be        */
   /*          outstanding per CGMS Instance.                           */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_Number_Of_Stored_Records(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Word_t NumberOfStoredRecords);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_Number_Of_Stored_Records_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Word_t NumberOfStoredRecords);
#endif

   /* The following function is responsible for sending a Record Access */
   /* Control Point indication to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CGMS_Initialize_Service().  The third parameter the            */
   /* ConnectionID of the remote device to send the indication to.  The */
   /* fourth parameter is the Request data to indicate.  The last       */
   /* parameter is the response code.  This function returns a zero if  */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * Only 1 RACP Request indication may be outstanding per    */
   /*          CGMS Instance.                                           */
   /* * NOTE * The value of RequestOpCode must be of the form           */
   /*          CGMS_RACP_OPCODE_XXX and the value of ResponseCode must  */
   /*          be of the form CGMS_RACP_RESPONSE_CODE_XXX.              */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_Record_Access_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Byte_t RequestOpCode, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_Record_Access_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Byte_t RequestOpCode, Byte_t ResponseCode);
#endif

   /* The following function is responsible for responding to a CGMS    */
   /* Specific Ops Control Point Command received from a remote device. */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to CGMS_Initialize_Service().  The third is the   */
   /* TransactionID that was received in the Specific Ops Control Point */
   /* event.  The final parameter is an error code that is used to      */
   /* determine if the Request is being accepted by the server or if an */
   /* error response should be issued instead.  This function returns a */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject CGMS Specific Ops Control Point commands when the */
   /*          Server has not been configured properly for CGMS SOCP    */
   /*          operation, the Client does not have proper authentication*/
   /*          to write to the CGMS SOCP characteristic, a CGMS SOCP    */
   /*          procedure with the Client is already in progress or when */
   /*          Invalid CRC is found or when CRC is Missing.  All other  */
   /*          reasons should return ZERO for the ErrorCode and then    */
   /*          send CGMS SOCP Result indication to indicate any other   */
   /*          errors.  For Example: If the Operand in the Request is   */
   /*          not supported by the Server this API should be called    */
   /*          with ErrorCode set to ZERO and then the                  */
   /*          CGMS_Indicate_CGM_Specific_Ops_Control_Point_Result()    */
   /*          should be called with the ResponseCode set to            */
   /*          CGMS_SPECIFIC_OPS_CP_RESPONSE_INVALID_OPERAND.           */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Specific_Ops_Control_Point_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Specific_Ops_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending a CGMS Specific */
   /* Ops Control Point indication to a specified remote device.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to CGMS_Initialize_Service().  The third parameter is the    */
   /* ConnectionID of the remote device to send the indication to.  The */
   /* fourth parameter is a flag to indicate if E2E-CRC is supported and*/
   /* needs to be calculated.  The fifth parameter is the Request Op    */
   /* Code to indicate.  The last parameter is Response Code Value to   */
   /* indicate.  This function returns a zero if successful or a        */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 CGMS SOCP Request indication may be outstanding   */
   /*          per CGMS Instance.                                       */
   /* * NOTE * Value of RequestOpCode must be of the form               */
   /*          CGMS_SPECIFIC_OPS_CP_OPCODE_XXX.                         */
   /* * NOTE * Value of ResponseCodeValue must be of the form           */
   /*          CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_XXX.                */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS        */
   /*          Specific Ops Control Point indication.                   */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_CGMS_Specific_Ops_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, Byte_t RequestOpCode, Byte_t ResponseCodeValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_CGMS_Specific_Ops_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, Byte_t RequestOpCode, Byte_t ResponseCodeValue);
#endif

   /* The following function is responsible for sending a Communication */
   /* Interval minutes indication to a specified remote device.  The    */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to CGMS_Initialize_Service().  The third parameter is the    */
   /* ConnectionID of the remote device to send the indication to.  The */
   /* fourth parameter is a flag to indicate if E2E-CRC is supported and*/
   /* needs to be calculated.  The last parameter is an Interval Minutes*/
   /* to be indicated.  This function returns a zero if successful or a */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 CGMS SOCP Request indication may be outstanding   */
   /*          per CGMS Instance.                                       */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS        */
   /*          Communication Interval indication.                       */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_Communication_Interval(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, Byte_t CommunicationIntervalMinutes);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_Communication_Interval_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, Byte_t CommunicationIntervalMinutes);
#endif

   /* The following function is responsible for sending a Calibration   */
   /* Data indication to a specified remote device.  The first parameter*/
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CGMS_Initialize_Service().  The third parameter is the            */
   /* ConnectionID of the remote device to send the indication to.  The */
   /* fourth parameter is a flag to indicate if E2E-CRC is supported and*/
   /* needs to be calculated.  The last parameter is Calibration Data to*/
   /* be indicated.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 CGMS SOCP Request indication may be outstanding   */
   /*          per CGMS Instance.                                       */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS        */
   /*          Calibration Data indication.                             */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_Calibration_Data(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, CGMS_Calibration_Data_Record_t *CalibrationData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_Calibration_Data_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, CGMS_Calibration_Data_Record_t *CalibrationData);
#endif

   /* The following function is responsible for sending a specified     */
   /* Alert level Value indication to a specified remote device.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the InstanceID returned from a successful */
   /* call to CGMS_Initialize_Service().  The third parameter is the    */
   /* ConnectionID of the remote device to send the indication to.  The */
   /* fourth parameter is a flag to indicate if E2E-CRC is supported and*/
   /* needs to be calculated.  The fifth parameter is the Alert Level   */
   /* Response OpCode to indicate.  The final parameter is the Alert    */
   /* Level Value to be indicated.  This function returns a zero if     */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * Only 1 CGMS SOCP Request indication may be outstanding   */
   /*          per CGMS Instance.                                       */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS Alert  */
   /*          Level indication.                                        */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Indicate_Alert_Level(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, CGMS_SOCP_Response_Type_t ResponseOpCode, Word_t AlertLevel);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Indicate_Alert_Level_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, unsigned int Flags, CGMS_SOCP_Response_Type_t ResponseOpCode, Word_t AlertLevel);
#endif

   /* CGMS Client API.                                                  */

   /* The following function is responsible for parsing a buffer        */
   /* received from a remote CGMS Server and decoding a specified number*/
   /* of measurements.  The first parameter is the length of the buffer */
   /* returned by the remote CGMS Server.  The second parameter is a    */
   /* pointer to the buffer returned by the remote CGMS Server.  The    */
   /* third parameter is the number of CGMS Measurements to decode.  The*/
   /* final parameter is an optional double pointer to store the parsed */
   /* CGMS Measurement data.  This function returns the number of       */
   /* measurements successfully parsed in the buffer if successful or a */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * If either the third parameter is 0 or the fourth         */
   /*          parameter is NULL, then the number of measurements       */
   /*          contained in the buffer will be returned.                */
   /* * NOTE * If this function fails for any reason during the decoding*/
   /*          of a measurement, then any following measurements will   */
   /*          not be decoded.                                          */
   /* * NOTE * If a CRC is attached to a decoded CGMS Measurement then  */
   /*          the CRCFlags field of the CGMS_Measurement_Data_t        */
   /*          structure will be marked that there is a CRC present and */
   /*          if it is valid.  See CGMS_E2E_CRC_PRESENT and            */
   /*          CGMS_E2E_CRC_VALID for more information.  This way the   */
   /*          CGMS client can determine if a CRC has been included and */
   /*          is valid for a CGMS Measurement.                         */
   /* * NOTE * Since the Flags field of a CGMS Measurement does not     */
   /*          indicate whether a CRC is present as an optional field   */
   /*          (Size indicates this), the CGMS Measurements, when       */
   /*          decoded, will each be marked in the (CRCFlags field) of  */
   /*          the CGMS_Measurement_Data_t structure by the bit values  */
   /*          CGMS_E2E_CRC_PRESENT and CGMS_E2E_CRC_VALID to indicate  */
   /*          if each CGMS Measurement has a CRC present and if it is  */
   /*          valid.  This way if multiple CGMS Measurements are       */
   /*          decoded and some contain a CRC while others do not, they */
   /*          can still be decoded.                                    */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_CGMS_Measurement(unsigned int BufferLength, Byte_t *Buffer, unsigned int NumberOfMeasurements, CGMS_Measurement_Data_t **MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_CGMS_Measurement_t)(unsigned int BufferLength, Byte_t *Buffer, unsigned int NumberOfMeasurements, CGMS_Measurement_Data_t **MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a CGMS Feature       */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote CGMS Server.  The second parameter is a    */
   /* pointer to the data returned by the remote CGMS Server.  The final*/
   /* parameter is a pointer to store the parsed CGMS Feature Value.    */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * Since the CGMS Client cannot determine if E2E-CRC is     */
   /*          supported by the CGMS Sensor before the features have    */
   /*          been decoded, this function will simply compare the      */
   /*          incoming CRC to the CGMS_E2E_NOT_SUPPORTED_VALUE found in*/
   /*          CGMSTypes.h.  If the incoming CRC is not set to this     */
   /*          value then the CRC will be verified for the CGMS Feature.*/
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_CGMS_Feature(unsigned int ValueLength, Byte_t *Value, CGMS_Feature_Data_t *CGMSFeature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_CGMS_Feature_t)(unsigned int ValueLength, Byte_t *Value, CGMS_Feature_Data_t *CGMSFeature);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a CGMS Status        */
   /* characteristic.  The first parameter is a flag to indicate if     */
   /* E2E-CRC is supported and needs to be calculated.  The second      */
   /* parameter is the length of the value returned by the remote CGMS  */
   /* Server.  The third parameter is a pointer to the data returned by */
   /* the remote CGMS Server.  The final parameter is a pointer to store*/
   /* the parsed CGMS Status Value.  This function returns a zero if    */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated for verification of the CGMS      */
   /*          Status.                                                  */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_CGMS_Status(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Status_Data_t *CGMSStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_CGMS_Status_t)(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Status_Data_t *CGMSStatus);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a CGMS Session Start */
   /* Time characteristic.  The first parameter is a flag to indicate if*/
   /* E2E-CRC is supported and needs to be calculated.  The second      */
   /* parameter is the length of the value returned by the remote CGMS  */
   /* Server.  The third parameter is a pointer to the data returned by */
   /* the remote CGMS Server.  The final parameter is a pointer to store*/
   /* the parsed CGMS Session Start Time value.  This function returns a*/
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated for verification of the CGMS      */
   /*          Session Start Time.                                      */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_Session_Start_Time(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Session_Start_Time_Data_t *SessionTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_Session_Start_Time_t)(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Session_Start_Time_Data_t *SessionTime);
#endif

   /* The following function is responsible for formatting a CGMS       */
   /* Session Start Time characteristic value into a user specified     */
   /* buffer.  The first parameter is a flag to indicate if E2E-CRC is  */
   /* supported and needs to be calculated.  The second parameter is the*/
   /* CGMS Session Start Time to format.  The third parameter is the    */
   /* BufferLength of the Buffer.  The final parameter is the Buffer    */
   /* that will hold the CGMS Session Start Time after formatting.  This*/
   /* function returns the calculated buffer length if successful or a  */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * If either the third parameter is 0 or the fourth         */
   /*          parameter is null, then the calculated buffer length will*/
   /*          simply be returned.  This will allow the caller to       */
   /*          determine the total allocated size for the buffer.       */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated and included with the CGMS Session*/
   /*          Start Time.                                              */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Format_Session_Start_Time(unsigned int Flags, CGMS_Session_Start_Time_Data_t *SessionStartTime, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Format_Session_Start_Time_t)(unsigned int Flags, CGMS_Session_Start_Time_Data_t *SessionStartTime, unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a CGMS Session Run   */
   /* Time characteristic.  The first parameter is a flag to indicate if*/
   /* E2E-CRC is supported and needs to be calculated.  The second      */
   /* parameter is the length of the value returned by the remote CGMS  */
   /* Server.  The third parameter is a pointer to the data returned by */
   /* the remote CGMS Server.  The final parameter is a pointer to store*/
   /* the parsed CGMS Session Run Time value.  This function returns a  */
   /* zero if successful or a negative return error code if an error    */
   /* occurs.                                                           */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated for verification of the CGMS      */
   /*          Session Run Time.                                        */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_Session_Run_Time(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Session_Run_Time_Data_t *CGMSRunTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_Session_Run_Time_t)(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Session_Run_Time_Data_t *CGMSRunTime);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a response data of   */
   /* record access control point.  The first parameter is the length of*/
   /* the value returned by the remote CGMS Server.  The second         */
   /* parameter is a pointer to the data returned by the remote CGMS    */
   /* Server.  The final parameter is a pointer to store the parsed     */
   /* Record Access Control Point Response data value.  This function   */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_Record_Access_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, CGMS_RACP_Response_Data_t *RACPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_Record_Access_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, CGMS_RACP_Response_Data_t *RACPData);
#endif

   /* The following function is responsible for formatting a Record     */
   /* Access Control Point Command into a user specified buffer.  The   */
   /* first parameter is the input command to format.  The second       */
   /* parameter is size of the input Record Access Control Point Request*/
   /* Data.  The final parameter is the output that will contain data in*/
   /* Buffer after formatting.  This function returns a zero if         */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The third parameter BufferLength is the size of input    */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Format_Record_Access_Control_Point_Command(CGMS_RACP_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Format_Record_Access_Control_Point_Command_t)(CGMS_RACP_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CGMS Server interpreting it as a response code of   */
   /* CGMS Specific Ops Control Point.  The first parameter is a flag to*/
   /* indicate if E2E-CRC is supported and needs to be calculated.  The */
   /* second parameter is the length of the value returned by the remote*/
   /* CGMS Server.  The third parameter is a pointer to the data        */
   /* returned by the remote CGMS Server.  The final parameter is a     */
   /* pointer to store the parsed CGMS Specific Ops Control Point       */
   /* Response data value.  This function returns a zero if successful  */
   /* or a negative return error code if an error occurs.               */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated for verification of the CGMS      */
   /*          Specific Ops Control Point Result.                       */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Decode_CGMS_Specific_Ops_Control_Point_Response(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Specific_Ops_Control_Point_Response_Data_t *CGMSCPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Decode_CGMS_Specific_Ops_Control_Point_Response_t)(unsigned int Flags, unsigned int ValueLength, Byte_t *Value, CGMS_Specific_Ops_Control_Point_Response_Data_t *CGMSCPData);
#endif

   /* The following function is responsible for formatting a CGMS       */
   /* Specific Ops Control Point Command into a user specified buffer.  */
   /* The first parameter is a flag to indicate if E2E-CRC is supported */
   /* and needs to be calculated.  The second parameter is the input    */
   /* command to format.  The third parameter is size of the CGMS       */
   /* Specific Ops Control Point request data.  The final parameter is  */
   /* the output that will contain data in Buffer after formatting.     */
   /* This function returns the calculated buffer length if successful  */
   /* or a negative return error code if an error occurs.               */
   /* * NOTE * If either the third parameter is 0 or the fourth         */
   /*          parameter is null, then the calculated buffer length will*/
   /*          simply be returned.  This will allow the caller to       */
   /*          determine the total allocated size for the buffer.       */
   /* * NOTE * The Flags parameter can either be CGMS_E2E_CRC_SUPPORTED */
   /*          or CGMS_E2E_CRC_NOT_SUPPORTED to indicate whether a CRC  */
   /*          needs to be calculated for verification of the CGMS      */
   /*          Specific Ops Control Point Command.                      */
BTPSAPI_DECLARATION int BTPSAPI CGMS_Format_CGMS_Specific_Ops_Control_Point_Command(unsigned int Flags, CGMS_Specific_Ops_Control_Point_Format_Data_t *FormatData, unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CGMS_Format_CGMS_Specific_Ops_Control_Point_Command_t)(unsigned int Flags, CGMS_Specific_Ops_Control_Point_Format_Data_t *FormatData, unsigned int BufferLength, Byte_t *Buffer);
#endif

#endif
