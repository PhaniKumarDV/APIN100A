/*****< glpmtype.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMTYPE - Glucose Manager API Type Definitions and Constants for         */
/*             Stonestreet One Bluetooth Protocol Stack Platform Manager.     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GLPMTYPEH__
#define __GLPMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

   /* The following defines the maximum GLPM Sequence Number that may be*/
   /* specified.                                                        */
#define GLPM_MAXIMUM_SEQUENCE_NUMBER                           65535

   /* The following enumerated type represents all of the Connection    */
   /* types that are supported by GLPM.                                 */
typedef enum
{
   gctSensor,
   gctCollector
} GLPM_Connection_Type_t;

   /* The following enumerated type represents all of the defined       */
   /* Glucose Procedures that may be performed with a Glucose Sensor.   */
typedef enum
{
   gptReportStoredRecords,
   gptDeleteStoredRecords,
   gptAbortProcedure,
   gptReportNumberStoredRecords
} GLPM_Procedure_Type_t;

   /* The following enumerated type represents all of the defined       */
   /* Glucose Operators that may be used with a Glucose Procedure to a  */
   /* Glucose Sensor.                                                   */
typedef enum
{
   gotAllRecords,
   gotLessThanOrEqualTo,
   gotGreaterThanOrEqualTo,
   gotWithinRangeOf,
   gotFirstRecord,
   gotLastRecord
} GLPM_Operator_Type_t;

   /* The following enumerated type represents all of the defined       */
   /* Glucose Filter Types that may be used with a Glucose Procedure to */
   /* a Glucose Sensor.                                                 */
typedef enum
{
   gftSequenceNumber,
   gftUserFacingTime
} GLPM_Filter_Type_t;

   /* The following structure defines the structure that contains two   */
   /* sequence numbers that are to be used as a starting and ending     */
   /* sequence number to define a sequence number range.                */
typedef struct _tagGLPM_Sequence_Number_Range_Data_t
{
   unsigned int MinimumSequenceNumber;
   unsigned int MaximumSequenceNumber;
} GLPM_Sequence_Number_Range_Data_t;

   /* The following structure defines the structure of the Glucose Base */
   /* Time parameter that may be used as a filter parameter when        */
   /* retrieving information from the remote device.                    */
typedef struct _tagGLPM_Date_Time_Data_t
{
   unsigned int Year;
   unsigned int Month;
   unsigned int Day;
   unsigned int Hours;
   unsigned int Minutes;
   unsigned int Seconds;
} GLPM_Date_Time_Data_t;

   /* The following strcuture defines a strcuture that contains two a   */
   /* GLPM Date Time Range that may be used as a filter parameter when  */
   /* retrieving information from the remote device.                    */
typedef struct _tagGLPM_Date_Time_Range_Data_t
{
   GLPM_Date_Time_Data_t MinimumDateTime;
   GLPM_Date_Time_Data_t MaximumDateTime;
} GLPM_Date_Time_Range_Data_t;

   /* The following structure defines the format of the Glucose         */
   /* Procedure Data.  This structure is used when initiating a         */
   /* procedure to a Glucose Sensor.                                    */
typedef struct _tagGLPM_Procedure_Data_t
{
   GLPM_Procedure_Type_t CommandType;
   GLPM_Operator_Type_t  OperatorType;
   GLPM_Filter_Type_t    FilterType;
   union
   {
      unsigned int                      SequenceNumber;
      GLPM_Date_Time_Data_t             UserFacingTime;
      GLPM_Sequence_Number_Range_Data_t SequenceNumberRange;
      GLPM_Date_Time_Range_Data_t       UserFacingTimeRange;
   } FilterParameters;
} GLPM_Procedure_Data_t;

   /* The following enumerated type represents all of the defined error */
   /* types that may be received in a Glucose Measurement Event.        */
typedef enum
{
   meNoError,
   meNoGlucoseContext,
   meSequenceNumberMismatch,
   meUnexpectedPacket,
   meInvalidData
} GLPM_Measurement_Error_Type_t;

   /* The following enumerated type represents all of the define        */
   /* Response Code types that may be received in a response to a       */
   /* Glucose procedure.                                                */
typedef enum
{
   grcSuccess,
   grcOpcodeNotSupported,
   grcInvalidOperator,
   grcOperatorNotSupported,
   grcInvalidOperand,
   grcNoRecordsFound,
   grcAbortUnsuccessful,
   grcProcedureNotCompleted,
   grcOperandNotSupported,
   grcProcedureTimeout,
   grcUnknown
} GLPM_Procedure_Response_Code_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Measurement Types.                                                */
typedef enum
{
   gmtCapillaryWholeBlood,
   gmtCapillaryPlasma,
   gmtVenousWholeBlood,
   gmtVenousPlasma,
   gmtArterialWholeBlood,
   gmtArterialPlasma,
   gmtUndeterminedWholeBlood,
   gmtUndeterminedPlasma,
   gmtInterstitialFluid,
   gmtControlSolution,
   gmtUnknown
} GLPM_Measurement_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Sample Location Types.                                            */
typedef enum
{
   gstFinger,
   gstAlternateSiteTest,
   gstEarlobe,
   gstControlSolution,
   gstLocationNotAvailable,
   gstUnknown
} GLPM_Sample_Location_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Context Meal Types.                                               */
typedef enum
{
   gmtPreprandial,
   gmtPostprandial,
   gmtFasting,
   gmtCasual,
   gmtBedtime,
   gmtUnknownMeal
} GLPM_Meal_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Context Tester Types.                                             */
typedef enum
{
   gttSelf,
   gttHealthCareProfessional,
   gttLabTest,
   gttNotAvailable,
   gttUnknown
} GLPM_Tester_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Context Health Types.                                             */
typedef enum
{
   ghtMinorHealthIssues,
   ghtMajorHealthIssues,
   ghtDuringMenses,
   ghtUnderStress,
   ghtNoHealthIssues,
   ghtNotAvailable,
   ghtUnknown
} GLPM_Health_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Context Carbohydrate IDs.                                         */
typedef enum
{
   gciBreakfast,
   gciLunch,
   gciDinner,
   gciSnack,
   gciDrink,
   gciSupper,
   gciBrunch,
   gciUnknown
} GLPM_Carbohydrate_ID_Type_t;

   /* The following enumerated type represents all of the valid Glucose */
   /* Context Medication IDs.                                           */
typedef enum
{
   gmiRapidActingInsulin,
   gmiShortActingInsulin,
   gmiIntermediateActingInsulin,
   gmiLongActingInsulin,
   gmiPremixedInsulin,
   gmiUnknown
} GLPM_Medication_ID_Type_t;

   /* The following enumerated type represents all of the valid Floating*/
   /* Point types.                                                      */
typedef enum
{
   gftValid,
   gftNotANumber,
   gftNotAtThisResolution,
   gftPositiveInfinity,
   gftNegativeInfinity,
   gftRFU
} GLPM_Floating_Point_Type_t;

   /* The following structure defines the structure of a Glucose        */
   /* floating point value (SFLOAT).                                    */
typedef struct _tagGLPM_Floating_Point_Data_t
{
   GLPM_Floating_Point_Type_t ValueType;
   int                        Mantissa;
   int                        Exponent;
} GLPM_Floating_Point_Data_t;

   /* The following defines the structure that contains all of the      */
   /* information that is associated with the Glucose Concentation      */
   /* Value.                                                            */
typedef struct _tagGLPM_Concentration_Data_t
{
   GLPM_Floating_Point_Data_t  Value;
   GLPM_Measurement_Type_t     MeasurementType;
   GLPM_Sample_Location_Type_t SampleLocationType;
} GLPM_Concentration_Data_t;

   /* The following defines the structure that contains all of the      */
   /* information that is associated with the Glucose Measurement data. */
typedef struct _tagGLPM_Glucose_Measurement_Data_t
{
   unsigned long             MeasurementFlags;
   unsigned int              SequenceNumber;
   GLPM_Date_Time_Data_t     BaseTime;
   int                       TimeOffset;
   GLPM_Concentration_Data_t Concentration;
   unsigned long             SensorStatusFlags;
} GLPM_Glucose_Measurement_Data_t;

   /* The following defines the valid bits that may be set in the       */
   /* MeasurementFlags field of the GLPM_Glucose_Measurement_Data_t     */
   /* structure.                                                        */
#define GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_PRESENT             0x00000001
#define GLPM_MEASUREMENT_FLAGS_CONCENTRATION_PRESENT           0x00000002
#define GLPM_MEASUREMENT_FLAGS_CONCENTRATION_IN_MOL_PER_LITER  0x00000004
#define GLPM_MEASUREMENT_FLAGS_SENSOR_STATUS_PRESENT           0x00000010
#define GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_OVERRUN             0x00000020
#define GLPM_MEASUREMENT_FLAGS_TIME_OFFSET_UNDERRUN            0x00000040

   /* The following defines the valid bits that may be set in the       */
   /* SensorStatusFlags field of the GLPM_Glucose_Measurement_Data_t    */
   /* structure.                                                        */
#define GLPM_SENSOR_STATUS_BATTERY_LOW_AT_TIME_OF_MEASUREMENT  0x00000001
#define GLPM_SENSOR_STATUS_SENSOR_MALFUNCTION                  0x00000002
#define GLPM_SENSOR_STATUS_SAMPLE_SIZE_INSUFFICIENT            0x00000004
#define GLPM_SENSOR_STATUS_STRIP_INSERTION_ERROR               0x00000008
#define GLPM_SENSOR_STATUS_STRIP_TYPE_INCORRECT                0x00000010
#define GLPM_SENSOR_STATUS_SENSOR_RESULT_TOO_HIGH              0x00000020
#define GLPM_SENSOR_STATUS_SENSOR_RESULT_TOO_LOW               0x00000040
#define GLPM_SENSOR_STATUS_SENSOR_TEMPERATURE_TOO_HIGH         0x00000080
#define GLPM_SENSOR_STATUS_SENSOR_TEMPERATURE_TOO_LOW          0x00000100
#define GLPM_SENSOR_STATUS_SENSOR_READ_INTERRUPTED             0x00000200
#define GLPM_SENSOR_STATUS_GENERAL_DEVICE_FAULT                0x00000400
#define GLPM_SENSOR_STATUS_TIME_FAULT                          0x00000800

   /* The following defines the structure of the Carbohydrate Data that */
   /* is passed in the Glucose Context structure.                       */
typedef struct _tagGLPM_Carbohydrate_Data_t
{
   GLPM_Carbohydrate_ID_Type_t CarbohydrateID;
   GLPM_Floating_Point_Data_t  CarbohydrateValue;
} GLPM_Carbohydrate_Data_t;

   /* The following defines the structure of the Exercise Data that is  */
   /* passed in the Glucose Context structure.                          */
typedef struct _tagGLPM_Exercise_Data_t
{
   unsigned int Duration;
   unsigned int Intensity;
} GLPM_Exercise_Data_t;

   /* The following defines the structure of the Medication Data that is*/
   /* passed in the Glucose Context structure.                          */
typedef struct _tagGLPM_Medication_Data_t
{
   GLPM_Medication_ID_Type_t  MedicationID;
   GLPM_Floating_Point_Data_t MedicationValue;
} GLPM_Medication_Data_t;

   /* The following defines the structure that contains all of the      */
   /* information that is associated with the Glucose Measurement       */
   /* Context data.                                                     */
typedef struct _tagGLPM_Glucose_Measurement_Context_Data_t
{
   unsigned long              ContextFlags;
   unsigned int               SequenceNumber;
   unsigned int               ExtendedFlags;
   GLPM_Carbohydrate_Data_t   CarbohydrateData;
   GLPM_Meal_Type_t           MealType;
   GLPM_Tester_Type_t         TesterType;
   GLPM_Health_Type_t         HealthType;
   GLPM_Exercise_Data_t       ExerciseData;
   GLPM_Medication_Data_t     MedicationData;
   GLPM_Floating_Point_Data_t HbA1cData;
} GLPM_Glucose_Measurement_Context_Data_t;

   /* The following defines the valid bits that may be set in the       */
   /* ContextFlags field of the GLPM_Glucose_Measurement_Context_Data_t */
   /* structure.                                                        */
#define GLPM_CONTEXT_FLAGS_CARBOHYDRATE_DATA_PRESENT           0x00000001
#define GLPM_CONTEXT_FLAGS_MEAL_TYPE_PRESENT                   0x00000002
#define GLPM_CONTEXT_FLAGS_TESTER_TYPE_AND_HEALTH_TYPE_PRESENT 0x00000004
#define GLPM_CONTEXT_FLAGS_EXERCISE_DATA_PRESENT               0x00000008
#define GLPM_CONTEXT_FLAGS_EXERCISE_DATA_DURATION_OVERRUN      0x00000010
#define GLPM_CONTEXT_FLAGS_MEDICATION_DATA_PRESENT             0x00000020
#define GLPM_CONTEXT_FLAGS_MEDICATION_DATA_UNITS_LITERS        0x00000040
#define GLPM_CONTEXT_FLAGS_HBA1C_PRESENT                       0x00000080
#define GLPM_CONTEXT_FLAGS_EXTENDED_FLAGS_PRESENT              0x00000100

#endif

