/*****< glsapi.h >*************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLSAPI - Stonestreet One Bluetooth Glucose Service (GATT based)           */
/*           API Type Definitions, Constants, and Prototypes.                 */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/22/11  T. Thomas      Initial creation.                               */
/*   07/16/12  Z. Khan        Updated as per new spec                         */
/******************************************************************************/
#ifndef __GLSAPIH__
#define __GLSAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API Prototypes/Constants. */
#include "GLSTypes.h"       /* Glucose Service Types/Constants.               */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define GLS_ERROR_INVALID_PARAMETER                       (-1000)
#define GLS_ERROR_INVALID_BLUETOOTH_STACK_ID              (-1001)
#define GLS_ERROR_INSUFFICIENT_RESOURCES                  (-1002)
#define GLS_ERROR_INSUFFICIENT_BUFFER_SPACE               (-1003)
#define GLS_ERROR_SERVICE_ALREADY_REGISTERED              (-1004)
#define GLS_ERROR_INVALID_INSTANCE_ID                     (-1005)
#define GLS_ERROR_MALFORMATTED_DATA                       (-1006)
#define GLS_ERROR_INDICATION_OUTSTANDING                  (-1007)
#define GLS_ERROR_NOT_CONFIGURED_RACP                     (-1008)
#define GLS_ERROR_NOT_AUTHENTICATION                      (-1009)
#define GLS_ERROR_UNKNOWN_ERROR                           (-1010)

   /* The following structure contains the Handles that will need to be */
   /* cached by a GLS client in order to only do service discovery      */
   /* once.                                                             */
typedef struct _tagGLS_Client_Information_t
{
   Word_t Glucose_Measurement;
   Word_t Glucose_Measurement_Client_Configuration;
   Word_t Measurement_Context;
   Word_t Measurement_Context_Client_Configuration;
   Word_t Glucose_Feature;
   Word_t Record_Access_Control_Point;
   Word_t Record_Access_Control_Point_Client_Configuration;
} GLS_Client_Information_t;

#define GLS_CLIENT_INFORMATION_DATA_SIZE                  (sizeof(GLS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a GLS Server.                           */
typedef struct _tagGLS_Server_Information_t
{
   Word_t Glucose_Measurement_Client_Configuration;
   Word_t Glucose_Context_Client_Configuration;
   Word_t Glucose_Features;
   Word_t Record_Access_Control_Point_Client_Configuration;
} GLS_Server_Information_t;

#define GLS_SERVER_INFORMATION_DATA_SIZE                  (sizeof(GLS_Server_Information_t))

   /* The following structure defines the structure that contains two   */
   /* sequence numbers that are to be used as a starting and ending     */
   /* sequence number to define a sequence number range.                */
typedef struct _tagGLS_Sequence_Number_Range_Data_t
{
   Word_t Minimum;
   Word_t Maximum;
} GLS_Sequence_Number_Range_Data_t;

   /* The following structure defines the structure of the Glucose Base */
   /* Time parameter that may be included in Glucose Measurements.      */
typedef struct _tagGLS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} GLS_Date_Time_Data_t;

#define GLS_DATE_TIME_DATA_SIZE                           (sizeof(GLS_Date_Time_Data_t))

   /* The following structure defines a structure that contains two     */
   /* Data_Time_t structures.  This structure is used to define a       */
   /* Date/Time Range.                                                  */
typedef struct _tagGLS_Date_Time_Range_Data_t
{
   GLS_Date_Time_Data_t Minimum;
   GLS_Date_Time_Data_t Maximum;
} GLS_Date_Time_Range_Data_t;

#define GLS_DATE_TIME_RANGE_DATA_SIZE                     (sizeof(GLS_Date_Time_Range_Data_t))

   /* The following defines the structure that contains all of the      */
   /* information that is associated with the Glucose Concentation      */
   /* Value.  If the Concentration valus is not available, then the     */
   /* ConcentrationValid flag is set to FALSE, otherwaise it is TRUE.   */
   /* * NOTE * The values for Type and SampleLocation are defined in    */
   /*          GLSTypes.h.                                              */
typedef struct _tagGLS_Concentration_Data_t
{
   Boolean_t ConcentrationValid;
   Word_t    Value;
   Byte_t    Type;
   Byte_t    SampleLocation;
} GLS_Concentration_Data_t;

#define GLS_CONCENTRATION_DATA_SIZE                       (sizeof(GLS_Concentration_Data_t))

   /* The following defines the structure of the Glucose Measurement    */
   /* Data that is passed to the function that builds the Glucose       */
   /* Measurement packet.                                               */
   /* * NOTE * If the GLS_MEASUREMENT_TYPE_SAMPLE_LOCATION_PRESENT Flag */
   /*          is set, then a valid value must be entered for both Type */
   /*          and Sample Location in GlucoseConcentration.             */
typedef struct _tagGLS_Glucose_Measurement_Data_t
{
   Byte_t                   OptionFlags;
   Word_t                   SequenceNumber;
   GLS_Date_Time_Data_t     BaseTime;
   Word_t                   TimeOffset;
   GLS_Concentration_Data_t GlucoseConcentration;
   Word_t                   SensorStatus;
} GLS_Glucose_Measurement_Data_t;

#define GLS_GLUCOSE_MEASUREMENT_DATA_SIZE                         (sizeof(GLS_Glucose_Measurement_Data_t))

   /* The following defines the structure of the Carbohydrate Data that */
   /* is passed in the Glucose Context structure.                       */
typedef struct _tagGLS_Carbohydrate_Data_t
{
   Byte_t ID;
   Word_t Value;
} GLS_Carbohydrate_Data_t;

#define GLS_CARBOHYDRATE_DATA_SIZE                        (sizeof(GLS_Carbohydrate_Data_t))

   /* The following defines the structure of the Exercise Data that is  */
   /* passed in the Glucose Context structure.                          */
typedef struct _tagGLS_Exercise_Data_t
{
   Word_t Duration;
   Byte_t Intensity;
} GLS_Exercise_Data_t;

#define GLS_EXERCISE_DATA_SIZE                            (sizeof(GLS_Exercise_Data_t))

   /* The following defines the structure of the Medication Data that is*/
   /* passed in the Glucose Context structure.                          */
typedef struct _tagGLS_Medication_Data_t
{
   Byte_t ID;
   Word_t Value;
} GLS_Medication_Data_t;

#define GLS_MEDICATION_DATA_SIZE                          (sizeof(GLS_Medication_Data_t))

   /* The following defines the structure of the Glucose Context Data   */
   /* that is passed to the function that builds the Glucose Context    */
   /* packet.                                                           */
   /* * NOTE * If the GLS_CONTEXT_TESTER_HEALTH_PRESENT Flag is set,    */
   /*          then a valid value must be entered for both Tester and   */
   /*          Health.                                                  */
typedef struct _tagGLS_Glucose_Measurement_Context_Data_t
{
   Byte_t                  OptionFlags;
   Word_t                  SequenceNumber;
   Byte_t                  ExtendedFlags;
   GLS_Carbohydrate_Data_t Carbohydrate;
   Byte_t                  Meal;
   Byte_t                  Tester;
   Byte_t                  Health;
   GLS_Exercise_Data_t     ExerciseData;
   GLS_Medication_Data_t   Medication;
   Word_t                  HbA1c;
} GLS_Glucose_Measurement_Context_Data_t;

#define GLS_GLUCOSE_MEASUREMENT_CONTEXT_DATA_SIZE         (sizeof(GLS_Glucose_Measurement_Context_Data_t))

   /* The following defines the enum of RACP Response Type that is      */
   /* passed in the RACP Response structure.                            */
typedef enum
{
   rarGLSNumberOfStoredRecords,
   rarGLSResponseCode
} GLS_RACP_Response_Type_t;

   /* The following defines the structure of RACP Response Code value   */
   /* that is passed in the RACP Response structure.                    */
   /* Values of RequestOpCode must be of the form                       */
   /* GLS_RECORD_ACCESS_OPCODE_XXX                                      */
   /* And the values of ResponseCodeValue must be of the form           */
   /* GLS_RECORD_ACCESS_RESPONSE_CODE_XXX                               */
typedef struct _tagGLS_RACP_Response_Code_Data_t
{
   Byte_t RequestOpCode;
   Byte_t ResponseCodeValue;
} GLS_RACP_Response_Code_Value_t;

   /* The following defines the format of a Record Access Control Point */
   /* Response Data. This structure will hold the RACP response data    */
   /* received from remote GLS Server. The first member specifies the   */
   /* Response Type. The second member is a union of response type      */
   /* value based on Response Type.                                     */
typedef struct _tagGLS_Record_Access_Control_Point_Response_Data_t
{
   GLS_RACP_Response_Type_t ResponseType;
   union
   {
      Word_t                         NumberOfStoredRecordsResult;
      GLS_RACP_Response_Code_Value_t ResponseCodeValue;
   } ResponseData;
} GLS_Record_Access_Control_Point_Response_Data_t;

#define GLS_RECORD_ACCESS_CONTROL_POINT_RESPONSE_DATA_SIZE (sizeof(GLS_Record_Access_Control_Point_Response_Data_t))

   /* The following define the valid Read Request types that a server   */
   /* may receive in a etGLS_Server_Read_Client_Configuration_Request or*/
   /* etGLS_Server_Client_Configuration_Update event.  This is also used*/
   /* by the GLS_Send_Notification to denote the characteristic value to*/
   /* notify.                                                           */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctGLSGlucoseMeasurement,
   ctGLSGlucoseMeasurementContext,
   ctGLSRecordAccessControlPoint
} GLS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* GLS Profile.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the GLS_Event_Data_t structure.                                   */
typedef enum
{
   etGLS_Read_Client_Configuration_Request,
   etGLS_Client_Configuration_Update,
   etGLS_Record_Access_Control_Point_Command,
   etGLS_Confirmation_Data
} GLS_Event_Type_t;

   /* The following is dispatched to a GLS Server when a GLS Client     */
   /* is attempting to read the Client Configuration descriptor.  The   */
   /* ConnectionID, and RemoteDevice identifies the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the GLS_Client_Configuration_Read_Response() API function.  */
typedef struct _tagGLS_Read_Client_Configuration_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   GLS_Characteristic_Type_t ClientConfigurationType;
} GLS_Read_Client_Configuration_Data_t;

#define GLS_READ_CLIENT_CONFIGURATION_DATA_SIZE           (sizeof(GLS_Read_Client_Configuration_Data_t))

   /* The following is dispatched to a GLS Server when a GLS Client     */
   /* attempts to write to a Client Configuration descriptor.  The      */
   /* ConnectionID and RemoteDevice identify the Client that is making  */
   /* the update request.  The ClientConfiguration value specifies the  */
   /* new Client Configuration value.                                   */
typedef struct _tagGLS_Client_Configuration_Update_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   GLS_Characteristic_Type_t ClientConfigurationType;
   Word_t                    ClientConfiguration;
} GLS_Client_Configuration_Update_Data_t;

#define GLS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE         (sizeof(GLS_Client_Configuration_Update_Data_t))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of Record Access Control Point         */
   /* characteristic.                                                   */
typedef enum
{
   racGLSReportStoredRecordsRequest   = GLS_RECORD_ACCESS_OPCODE_REPORT_STORED_RECORDS,
   racGLSDeleteStoredRecordsRequest   = GLS_RECORD_ACCESS_OPCODE_DELETE_STORED_RECORDS,
   racGLSAbortOperationRequest        = GLS_RECORD_ACCESS_OPCODE_ABORT_OPERATION,
   racGLSNumberOfStoredRecordsRequest = GLS_RECORD_ACCESS_OPCODE_REPORT_NUM_STORED_RECORDS
} GLS_RACP_Command_Type_t;

   /* The following enumerates the valid values that may be set as the  */
   /* value for the Operator field of Record Access Control Point       */
   /* characteristic.                                                   */
typedef enum
{
   raoGLSNull                 = GLS_RECORD_ACCESS_OPERATOR_NULL,
   raoGLSAllRecords           = GLS_RECORD_ACCESS_OPERATOR_ALL_RECORDS,
   raoGLSLessThanOrEqualTo    = GLS_RECORD_ACCESS_OPERATOR_LESS_THAN_OR_EQUAL_TO,
   raoGLSGreaterThanOrEqualTo = GLS_RECORD_ACCESS_OPERATOR_GREATER_THAN_OR_EQUAL_TO,
   raoGLSWithinRangeOf        = GLS_RECORD_ACCESS_OPERATOR_WITHIN_RANGE_OF,
   raoGLSFirstRecord          = GLS_RECORD_ACCESS_OPERATOR_FIRST_RECORD,
   raoGLSLastRecord           = GLS_RECORD_ACCESS_OPERATOR_LAST_RECORD
} GLS_RACP_Operator_Type_t;

   /* The following enumerates the valid values that may be used as the */
   /* Filter Type values of a Record Access Control Point               */
   /* characteristic.                                                   */
typedef enum
{
   rafSequenceNumber = GLS_RECORD_ACCESS_FILTER_TYPE_SEQUENCE_NUMBER,
   rafUserFacingTime = GLS_RECORD_ACCESS_FILTER_TYPE_USER_FACING_TIME
} GLS_RACP_Filter_Type_t;

   /* The following structure defines the format of the Record Access   */
   /* Control Point Command Request Data.  This structure is passed as a*/
   /* parameter to the GLS_Format_Record_Access_Control_Point_Command() */
   /* API.                                                              */
typedef struct _tagGLS_Record_Access_Control_Point_Format_Data_t
{
   GLS_RACP_Command_Type_t  CommandType;
   GLS_RACP_Operator_Type_t OperatorType;
   GLS_RACP_Filter_Type_t   FilterType;
   union
   {
      Word_t                           SequenceNumber;
      GLS_Date_Time_Data_t             UserFacingTime;
      GLS_Sequence_Number_Range_Data_t SequenceNumberRange;
      GLS_Date_Time_Range_Data_t       UserFacingTimeRange;
   } FilterParameters;
} GLS_Record_Access_Control_Point_Format_Data_t;

#define GLS_RECORD_ACCESS_CONTROL_POINT_FORMAT_DATA_SIZE (sizeof(GLS_Record_Access_Control_Point_Format_Data_t))

   /* The following is dispatched to a GLS Server in response to the    */
   /* reception of request from a Client to write to the Record Access  */
   /* Control Point.                                                    */
typedef struct _tagGLS_Record_Access_Control_Point_Command_Data_t
{
   unsigned int                                  InstanceID;
   unsigned int                                  ConnectionID;
   unsigned int                                  TransactionID;
   GATT_Connection_Type_t                        ConnectionType;
   BD_ADDR_t                                     RemoteDevice;
   GLS_Record_Access_Control_Point_Format_Data_t FormatData;
} GLS_Record_Access_Control_Point_Command_Data_t;

#define GLS_RECORD_ACCESS_CONTROL_POINT_COMMAND_DATA_SIZE (sizeof(GLS_Record_Access_Control_Point_Command_Data_t))

   /* The following GLS Profile Event is dispatched to a GLS Server     */
   /* when a GLS Client has sent a confirmation to a previously sent    */
   /* confirmation.  The ConnectionID, ConnectionType, and RemoteDevice */
   /* specifiy the Client that is making the update.  The               */
   /* Characteristic_Type specifies which Indication the Client has sent*/
   /* a confirmation for.  The final parameter specifies the status of  */
   /* the Indication                                                    */
   /* * NOTE * The Characteristic_Type parameter will NEVER be set to   */
   /*          ctIntermediateTemperature for this event.                */
   /* * NOTE * The Status member is set to one of the following values: */
   /*                GATT_CONFIRMATION_STATUS_SUCCESS                   */
   /*                GATT_CONFIRMATION_STATUS_TIMEOUT                   */
typedef struct _tagGLS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   Byte_t                 Status;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} GLS_Confirmation_Data_t;

#define GLS_CONFIRMATION_DATA_SIZE                        (sizeof(GLS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all GLS Profile Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagGLS_Event_Data_t
{
   GLS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      GLS_Read_Client_Configuration_Data_t           *GLS_Read_Client_Configuration_Data;
      GLS_Client_Configuration_Update_Data_t         *GLS_Client_Configuration_Update_Data;
      GLS_Record_Access_Control_Point_Command_Data_t *GLS_Record_Access_Control_Point_Command_Data;
      GLS_Confirmation_Data_t                        *GLS_Confirmation_Data;
   } Event_Data;
} GLS_Event_Data_t;

#define GLS_EVENT_DATA_SIZE                               (sizeof(GLS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a GLS Profile Event Receive Data Callback.  This function will    */
   /* be called whenever an GLS Profile Event occurs that is            */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the GLS Event Data   */
   /* that occurred and the GLS Profile Event Callback Parameter that   */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the GLS Profile Event Data ONLY in    */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* re-entrant).  It needs to be noted however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GLS Profile Event will not be processed while this        */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving GLS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO GLS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *GLS_Event_Callback_t)(unsigned int BluetoothStackID, GLS_Event_Data_t *GLS_Event_Data, unsigned long CallbackParameter);

   /* GLS Server API.                                                   */

   /* The following function is responsible for opening a GLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered GLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 GLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI GLS_Initialize_Service(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Initialize_Service_t)(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a GLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered GLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 GLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI GLS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t  *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened GLS Server.  The first parameter is the Bluetooth Stack    */
   /* ID on which to close the server.  The second parameter is the     */
   /* InstanceID that was returned from a successful call to            */
   /* GLS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI GLS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_GLS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* GLS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfull call to GLS_Suspend().  This      */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI GLS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the GLS Service that is          */
   /* registered with a call to GLS_Initialize_Service().  This function*/
   /* returns the non-zero number of attributes that are contained in a */
   /* GLS Server or zero on failure.                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI GLS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_GLS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for setting the supported   */
   /* Glucose features on the specified GLS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.      */
   /* The second parameter is the InstanceID returned from a successful */
   /* call to GLS_Initialize_Server().  The final parameter is the      */
   /* supported features to set for the specified GLS Instance.  This   */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The SupportedFeatures parameter should be in range       */
   /* between GLS_FEATURE_LOW_BATTERY_DETECTION_DURING_MEASUREMENT to   */
   /* GLS_FEATURE_MULTIPLE_BOND_SUPPORT                                 */
BTPSAPI_DECLARATION int BTPSAPI GLS_Set_Glucose_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Set_Glucose_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);
#endif

   /* GLS Server API.                                                   */
   /* The following function is responsible for querying the current    */
   /* Glucose Features on the specified GLS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The final parameter is a pointer to  */
   /* return the current Glucose Features for the specified GLS         */
   /* Instance.  This function returns a zero if successful or a        */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Query_Glucose_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Query_Glucose_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);
#endif

   /* The following function is responsible for responding to a GLS Read*/
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* GLS_Initialize_Server().  The third is the Transaction ID of the  */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible to responding to a Record   */
   /* Access Control Point Command received from a remote device.  The  */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second is the TransactionID that was received in the Record   */
   /* Access Control Point event.  The final parameter is an error code */
   /* that is used to determine if the Request is being accepted by the */
   /* server or if an error response should be issued instead.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Record Access Control Point commands when the     */
   /*          Server has not been configured properly for RACP         */
   /*          operation, the Client does not have proper authentication*/
   /*          to write to the RACP characteristic or a RACP procedure  */
   /*          with the Client is already in progress.  All other       */
   /*          reasons should return ZERO for the ErrorCode and then    */
   /*          send RACP Result indication to indicate any other errors.*/
   /*          For Example: If the Operand in the Request is not        */
   /*          supported by the Server this API should be called with   */
   /*          ErrorCode set to ZERO and then the                       */
   /*          GLS_Indicate_Record_Access_Control_Point_Result() should */
   /*          be called with the ResponseCode set to                   */
   /*          GLS_RECORD_ACCESS_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED.  */
BTPSAPI_DECLARATION int BTPSAPI GLS_Record_Access_Control_Point_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Record_Access_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending an Glucose      */
   /* Measurement notification to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the Glucose Measurement Data structure that*/
   /* contains all of the required and optional data for the            */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Notify_Glucose_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Notify_Glucose_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for sending an Glucose      */
   /* Measurement Context notification to a specified remote device.    */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device. The second parameter is the InstanceID returned from a    */
   /* successful call to GLS_Initialize_Server().  The third parameter  */
   /* is the ConnectionID of the remote device to send the notification */
   /* to. The final parameter is the Glucose Context Data structure     */
   /* that contains all of the required and optional data for the       */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Notify_Glucose_Measurement_Context(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Context_Data_t *ContextData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Notify_Glucose_Measurement_Context_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Context_Data_t *ContextData);
#endif

   /* GLS Server API.                                                   */
   /* The following function is responsible for Number of Stored        */
   /* Records indication to a specified remote device.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The last parameter is number of stored records to be indicated.   */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * Only 1 Number of Stored Records indication may be        */
   /*          outstanding per GLS Instance.                            */
BTPSAPI_DECLARATION int BTPSAPI GLS_Indicate_Number_Of_Stored_Records(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Word_t NumberOfStoredRecords);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Indicate_Number_Of_Stored_Records_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Word_t NumberOfStoredRecords);
#endif

   /* GLS Server API.  The following function is responsible for sending*/
   /* a Record Access Control Point indication to a specified remote    */
   /* device.  The first parameter is the Bluetooth Stack ID of the     */
   /* Bluetooth Device.  The second parameter is the InstanceID returned*/
   /* from a successful call to GLS_Initialize_Server().  The third     */
   /* parameter the ConnectionID of the remote device to send the       */
   /* indication to.  The fourth parameter is the Request data to       */
   /* indicate.  The last parameter is response code.  This function    */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * Only 1 RACP Request indication may be outstanding per GLS*/
   /*          Instance.                                                */
BTPSAPI_DECLARATION int BTPSAPI GLS_Indicate_Record_Access_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_RACP_Command_Type_t CommandType, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Indicate_Record_Access_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_RACP_Command_Type_t CommandType, Byte_t ResponseCode);
#endif

   /* GLS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a Glucose             */
   /* Measurement characteristic.  The first parameter is the length of */
   /* the value returned by the remote GLS Server.  The second          */
   /* parameter is a pointer to the data returned by the remote GLS     */
   /* Server.  The final parameter is a pointer to store the parsed     */
   /* Glucose Measurement value.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Decode_Glucose_Measurement(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Decode_Glucose_Measurement_t)(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a Glucose Context     */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote GLS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote GLS Server.  The       */
   /* final parameter is a pointer to store the parsed Glucose Context  */
   /* value.  This function returns a zero if successful or a negative  */
   /* return error code if an error occurs.                             */
BTPSAPI_DECLARATION int BTPSAPI GLS_Decode_Glucose_Measurement_Context(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Context_Data_t *ContextData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Decode_Glucose_Measurement_Context_t)(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Context_Data_t *ContextData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a response code of    */
   /* record access control point.The first parameter is the length of  */
   /* the value returned by the remote GLS Server.The second parameter  */
   /* is a pointer to the data returned by the remote GLS Server.The    */
   /* final parameter is a pointer to store the parsed Record Access    */
   /* Control Point Response data value.This function returns a zero if */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI GLS_Decode_Record_Access_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, GLS_Record_Access_Control_Point_Response_Data_t *RACPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Decode_Record_Access_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, GLS_Record_Access_Control_Point_Response_Data_t *RACPData);
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
BTPSAPI_DECLARATION int BTPSAPI GLS_Format_Record_Access_Control_Point_Command(GLS_Record_Access_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLS_Format_Record_Access_Control_Point_Command_t)(GLS_Record_Access_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);
#endif

#endif
