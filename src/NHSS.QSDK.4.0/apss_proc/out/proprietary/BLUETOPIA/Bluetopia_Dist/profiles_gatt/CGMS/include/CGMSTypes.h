/*****< cgmstypes.h >**********************************************************/
/*      Copyright 2014 Stonestreet One.                                       */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CGMSTypes - Stonestreet One Bluetooth Stack Continous Glucose Monitor     */
/*             Service Types                                                  */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/14  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __CGMSTYPESH__
#define __CGMSTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.           */

   /* The following define the defined CGMS Error Codes that may be sent*/
   /* in a GATT Error Response.                                         */
#define CGMS_ERROR_CODE_SUCCESS                                             0x00
#define CGMS_ERROR_CODE_MISSING_CRC                                         0x80
#define CGMS_ERROR_CODE_INVALID_CRC                                         0x81
#define CGMS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED  0xFD
#define CGMS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS                       0xFE

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the CGMS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_CGMS_SERVICE_UUID_16(_x)                                ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x1F)

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the CGMS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define CGMS_ASSIGN_CGMS_SERVICE_SDP_RECORD(_x)                             ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x1F, 0x18)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* CGMS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CGMS Service UUID.                              */
#define CGMS_COMPARE_CGMS_SERVICE_UUID_TO_UUID_16(_x)                       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x1F)

   /* The following defines the CGMS Service UUID that is used when     */
   /* building the CGMS Service Table.                                  */
#define CGMS_SERVICE_BLUETOOTH_UUID_CONSTANT                                { 0x1F, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Measurement Characteristic 16 bit UUID to the specified UUID_16_t */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the CGMS Measurement UUID Constant    */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_MEASUREMENT_UUID_16(_x)                                 ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA7)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Measurement UUID in UUID16 form.  This*/
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* CGMS Measurement UUID (MACRO returns boolean result) NOT less     */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CGMS Measurement UUID.                          */
#define CGMS_COMPARE_MEASUREMENT_UUID_TO_UUID_16(_x)                        COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA7)

   /* The following defines the CGMS Measurement Characteristic UUID    */
   /* that is used when building the CGMS Service Table.                */
#define CGMS_MEASUREMENT_CHARACTERISTIC_UUID_CONSTANT                       { 0xA7, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Feature Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the CGMS Feature UUID Constant value. */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_FEATURE_UUID_16(_x)                                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA8)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Feature UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* CGMS Feature UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CGMS Feature UUID.                              */
#define CGMS_COMPARE_FEATURE_UUID_TO_UUID_16(_x)                            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA8)

   /* The following defines the CGMS Feature Characteristic UUID that is*/
   /* used when building the CGMS Service Table.                        */
#define CGMS_FEATURE_CHARACTERISTIC_UUID_CONSTANT                           { 0xA8, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Status Characteristic 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the CGMS Status UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_STATUS_UUID_16(_x)                                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA9)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Status UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* CGMS Status UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CGMS Status UUID.                               */
#define CGMS_COMPARE_STATUS_UUID_TO_UUID_16(_x)                             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA9)

   /* The following defines the CGMS Status Characteristic UUID that is */
   /* used when building the CGMS Service Table.                        */
#define CGMS_STATUS_CHARACTERISTIC_UUID_CONSTANT                            { 0xA9, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Session Start Time Characteristic 16 bit UUID to the specified    */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the CGMS Session Start Time */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_SESSION_START_TIME_UUID_16(_x)                          ASSIGN_UUID_16((_x), 0x2A, 0xAA)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Session Start Time UUID in UUID16     */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Session Start Time UUID (MACRO returns boolean       */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the CGMS Session Start time UUID.*/
#define CGMS_COMPARE_CGMS_SESSION_START_TIME_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xAA)

   /* The following defines the CGMS Session Start Time Characteristic  */
   /* UUID that is used when building the CGMS Service Table.           */
#define CGMS_SESSION_START_TIME_CHARACTERISTIC_UUID_CONSTANT                { 0xAA, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Session Run Time Characteristic 16 bit UUID to the specified      */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the CGMS Session Run Time   */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_SESSION_RUN_TIME_UUID_16(_x)                            ASSIGN_UUID_16((_x), 0x2A, 0xAB)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Session Run Time UUID in UUID16 form. */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the Session Run Time UUID (MACRO returns boolean result) NOT less */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CGMS Session Run time UUID.                     */
#define CGMS_COMPARE_CGMS_SESSION_RUN_TIME_UUID_TO_UUID_16(_x)              COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xAB)

   /* The following defines the CGMS Session Run Time Characteristic    */
   /* UUID that is used when building the CGMS Service Table.           */
#define CGMS_SESSION_RUN_TIME_CHARACTERISTIC_UUID_CONSTANT                  { 0xAB, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the Record    */
   /* Access Control Point Type Characteristic 16 bit UUID to the       */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the Record     */
   /* Access Control Point Type UUID Constant value.                    */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x52)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Record Access Control Point Type UUID in   */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the Record Access Control Point Type UUID    */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the Record*/
   /* Access Control Point Type UUID.                                   */
#define CGMS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x52)

   /* The following defines the Record Access Control Point Type        */
   /* Characteristic UUID that is used when building the CGMS Service   */
   /* Table.                                                            */
#define CGMS_RECORD_ACCESS_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT       { 0x52, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CGMS      */
   /* Specific Operations Control Point Characteristic 16 bit UUID to   */
   /* the specified UUID_16_t variable.  This MACRO accepts one         */
   /* parameter which is the UUID_16_t variable that is to receive the  */
   /* CGMS Specific Operations Control Point UUID Constant value.       */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CGMS_ASSIGN_SPECIFIC_OPS_CONTROL_POINT_UUID_16(_x)                  ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xAC)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CGMS Specific Operations Control Point UUID*/
   /* in UUID16 form.  This MACRO only returns whether the UUID_16_t    */
   /* variable is equal to the Session Start Time UUID (MACRO returns   */
   /* boolean result) NOT less than/greater than.  The first parameter  */
   /* is the UUID_16_t variable to compare to the CGMS Specific         */
   /* Operations Control Point UUID.                                    */
#define CGMS_COMPARE_CGMS_SPECIFIC_OPS_CONTROL_POINT_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xAC)

   /* The following defines the CGMS Specific Operations Control Point  */
   /* Characteristic UUID that is used when building the CGMS Service   */
   /* Table.                                                            */
#define CGMS_SPECIFIC_OPS_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT        { 0xAC, 0x2A }

   /* The following defines the valid CGMS Measurement Flags bits that  */
   /* may be set in the Flags field of a CGMS Measurement               */
   /* characteristic.                                                   */
#define CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT                     0x01
#define CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT                               0x02
#define CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT     0x20
#define CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT   0x40
#define CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT    0x80
#define CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_PRESENT            (CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT | CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT | CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT)
#define CGMS_MEASUREMENT_FLAG_ALL_PRESENT                                   0xE3

   /* The following defines the valid Sensor Status bits that may be set*/
   /* in the Sensor Status characteristic.  Note that these bits also   */
   /* define the valid CGMS Measurement Sensor Status Annunciation Bits */
   /* that may be set in the CGMS Measurement characteristic.           */
#define CGMS_SENSOR_STATUS_SESSION_STOPPED                                              0x000001
#define CGMS_SENSOR_STATUS_DEVICE_BATTERY_LOW                                           0x000002
#define CGMS_SENSOR_STATUS_SENSOR_TYPE_INCORRECT_FOR_DEVICE                             0x000004
#define CGMS_SENSOR_STATUS_SENSOR_MALFUNCTION                                           0x000008
#define CGMS_SENSOR_STATUS_DEVICE_SPECIFIC_ALERT                                        0x000010
#define CGMS_SENSOR_STATUS_GENERAL_DEVICE_FAULT_HAS_OCCURRED_IN_THE_SENSOR              0x000020

#define CGMS_SENSOR_STATUS_TIME_SYNCHRONIZATION_BETWEEN_SENSOR_AND_COLLECTOR_REQUIRED   0x000100
#define CGMS_SENSOR_STATUS_CALIBRATION_NOT_ALLOWED                                      0x000200
#define CGMS_SENSOR_STATUS_CALIBRATION_RECOMMENDED                                      0x000400
#define CGMS_SENSOR_STATUS_CALIBRATION_REQUIRED                                         0x000800
#define CGMS_SENSOR_STATUS_SENSOR_TEMPERATURE_TOO_HIGH                                  0x001000
#define CGMS_SENSOR_STATUS_SENSOR_TEMPERATURE_TOO_LOW                                   0x002000

#define CGMS_SENSOR_STATUS_SENSOR_RESULT_LOWER_THAN_THE_PATIENT_LOW_LEVEL               0x010000
#define CGMS_SENSOR_STATUS_SENSOR_RESULT_HIGHER_THAN_THE_PATIENT_HIGH_LEVEL             0x020000
#define CGMS_SENSOR_STATUS_SENSOR_RESULT_LOWER_THAN_THE_HYPO_LEVEL                      0x040000
#define CGMS_SENSOR_STATUS_SENSOR_RESULT_HIGHER_THAN_THE_HYPER_LEVEL                    0x080000
#define CGMS_SENSOR_STATUS_SENSOR_RATE_OF_DECREASE_EXCEEDED                             0x100000
#define CGMS_SENSOR_STATUS_SENSOR_RATE_OF_INCREASE_EXCEEDED                             0x200000
#define CGMS_SENSOR_STATUS_SENSOR_RESULT_TOO_LOW                                        0x400000
#define CGMS_SENSOR_STATUS_SENSOR_RESULT_TOO_HIGH                                       0x800000
#define CGMS_SENSOR_STATUS_RESERVED_BITS                                                0x000040
#define CGMS_SENSOR_STATUS_STATUS_EXTENSION                                             0x004000

   /* The following defines the valid values of CGMS Feature bits that  */
   /* may be set in the Feature field of CGMS Feature characteristic.   */
#define CGMS_FEATURE_FLAG_CALIBRATION_SUPPORTED                                         0x000001
#define CGMS_FEATURE_FLAG_PATIENT_HIGH_LOW_ALERTS_SUPPORTED                             0x000002
#define CGMS_FEATURE_FLAG_HYPO_ALERTS_SUPPORTED                                         0x000004
#define CGMS_FEATURE_FLAG_HYPER_ALERTS_SUPPORTED                                        0x000008
#define CGMS_FEATURE_FLAG_RATE_OF_INCREASE_DECREASE_SUPPORTED                           0x000010
#define CGMS_FEATURE_FLAG_DEVICE_SPECIFIC_ALERT_SUPPORTED                               0x000020
#define CGMS_FEATURE_FLAG_SENSOR_MALFUNCTION_DETECTION_SUPPORTED                        0x000040
#define CGMS_FEATURE_FLAG_SENSOR_TEMPERATURE_HIGH_LOW_DETECTION_SUPPORTED               0x000080
#define CGMS_FEATURE_FLAG_SENSOR_RESULT_HIGH_LOW_DETECTION_SUPPORTED                    0x000100
#define CGMS_FEATURE_FLAG_LOW_BATTERY_DETECTION_SUPPORTED                               0x000200
#define CGMS_FEATURE_FLAG_SENSOR_TYPE_ERROR_DETECTION_SUPPORTED                         0x000400
#define CGMS_FEATURE_FLAG_GENERAL_DEVICE_FAULT_SUPPORTED                                0x000800
#define CGMS_FEATURE_FLAG_E2E_CRC_SUPPORTED                                             0x001000
#define CGMS_FEATURE_FLAG_MULTIPLE_BOND_SUPPORTED                                       0x002000
#define CGMS_FEATURE_FLAG_MULTIPLE_SESSIONS_SUPPORTED                                   0x004000
#define CGMS_FEATURE_FLAG_TREND_INFORMATION_SUPPORTED                                   0x008000
#define CGMS_FEATURE_FLAG_QUALITY_SUPPORTED                                             0x010000
#define CGMS_FEATURE_FLAG_RESERVED_BITS                                                 0x7E0000
#define CGMS_FEATURE_FLAG_FEATURE_EXTENSION                                             0x800000

   /* The following defines the valid values that may be used as the    */
   /* Type value of a CGMS Feature Characteristic.                      */
#define CGMS_FEATURE_TYPE_CAPILLARY_WHOLE_BLOOD                 0x01
#define CGMS_FEATURE_TYPE_CAPILLARY_PLASMA                      0x02
#define CGMS_FEATURE_TYPE_VENOUS_WHOLE_BLOOD                    0x03
#define CGMS_FEATURE_TYPE_VENOUS_PLASMA                         0x04
#define CGMS_FEATURE_TYPE_ARTERIAL_WHOLE_BLOOD                  0x05
#define CGMS_FEATURE_TYPE_ARTERIAL_PLASMA                       0x06
#define CGMS_FEATURE_TYPE_UNDETERMINED_WHOLE_BLOOD              0x07
#define CGMS_FEATURE_TYPE_UNDETERMINED_PLASMA                   0x08
#define CGMS_FEATURE_TYPE_INTERSTITIAL_FLUID                    0x09
#define CGMS_FEATURE_TYPE_CONTROL_SOLUTION                      0x0A

   /* The following defines the valid values that may be used as the    */
   /* Sample Location value of a CGMS Feature Characteristic.           */
#define CGMS_FEATURE_SAMPLE_LOCATION_FINGER                     0x01
#define CGMS_FEATURE_SAMPLE_LOCATION_ALTERNATE_SITE_TEST        0x02
#define CGMS_FEATURE_SAMPLE_LOCATION_EARLOBE                    0x03
#define CGMS_FEATURE_SAMPLE_LOCATION_CONTROL_SOLUTION           0x04
#define CGMS_FEATURE_SAMPLE_LOCATION_SUBCUTANEOUS_TISSUE        0x05
#define CGMS_FEATURE_SAMPLE_LOCATION_NOT_AVAILABLE              0x0F

   /* The following defines the valid values that may be used as the    */
   /* Time Zone value of a CGMS Session Start Time or CGMS Session Run  */
   /* Time characteristic.                                              */
#define CGMS_TIME_ZONE_UTC_OFFSET_UNKNOWN                       (-128)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_12_00                   (-48)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_11_00                   (-44)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_10_00                   (-40)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_9_30                    (-38)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_9_00                    (-36)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_8_00                    (-32)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_7_00                    (-28)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_6_00                    (-24)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_5_00                    (-20)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_4_30                    (-18)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_4_00                    (-16)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_3_30                    (-14)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_3_00                    (-12)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_2_00                    (-8)
#define CGMS_TIME_ZONE_UTC_OFFSET_MINUS_1_00                    (-4)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_0_00                     (0)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_1_00                     (+4)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_2_00                     (+8)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_3_00                     (+12)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_3_30                     (+14)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_4_00                     (+16)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_4_30                     (+18)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_00                     (+20)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_30                     (+22)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_5_45                     (+23)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_6_00                     (+24)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_6_30                     (+26)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_7_00                     (+28)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_8_00                     (+32)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_8_45                     (+35)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_9_00                     (+36)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_9_30                     (+38)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_10_00                    (+40)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_10_30                    (+42)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_11_00                    (+44)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_11_30                    (+46)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_12_00                    (+48)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_12_45                    (+51)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_13_00                    (+52)
#define CGMS_TIME_ZONE_UTC_OFFSET_PLUS_14_00                    (+56)

   /* The following defines the valid values that may be used as the DST*/
   /* Offset value of CGMS Session Start Time or CGMS Session Run Time  */
   /* characteristic.                                                   */
#define CGMS_DST_OFFSET_UNKNOWN                                 (255)
#define CGMS_DST_OFFSET_STANDARD_TIME                           (0)
#define CGMS_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME              (2)
#define CGMS_DST_OFFSET_DAYLIGHT_TIME                           (4)
#define CGMS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME                    (8)

   /* The following defines the valid values that may be set as the     */
   /* value for the OpCode field of Record Access Control Point         */
   /* characteristic.                                                   */
#define CGMS_RACP_OPCODE_REPORT_STORED_RECORDS                  0x01
#define CGMS_RACP_OPCODE_DELETE_STORED_RECORDS                  0x02
#define CGMS_RACP_OPCODE_ABORT_OPERATION                        0x03
#define CGMS_RACP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS        0x04
#define CGMS_RACP_OPCODE_NUMBER_OF_STORED_RECORDS_RESPONSE      0x05
#define CGMS_RACP_OPCODE_RESPONSE_CODE                          0x06

   /* The following defines the valid values that may be set as the     */
   /* value for the Operator field of Record Access Control Point       */
   /* characteristic.                                                   */
#define CGMS_RACP_OPERATOR_NULL                                 0x00
#define CGMS_RACP_OPERATOR_ALL_RECORDS                          0x01
#define CGMS_RACP_OPERATOR_LESS_THAN_OR_EQUAL_TO                0x02
#define CGMS_RACP_OPERATOR_GREATER_THAN_OR_EQUAL_TO             0x03
#define CGMS_RACP_OPERATOR_WITHIN_RANGE_OF                      0x04
#define CGMS_RACP_OPERATOR_FIRST_RECORD                         0x05
#define CGMS_RACP_OPERATOR_LAST_RECORD                          0x06

   /* The following defines the valid values that may be used as the    */
   /* Filter Types value of a Record Access Control Point               */
   /* characteristic.                                                   */
#define CGMS_RACP_FILTER_TYPE_TIME_OFFSET                       0x01

   /* The following defines the valid values that may be used as the    */
   /* Operand value of a Record Access Control Point characteristic.    */
#define CGMS_RACP_RESPONSE_CODE_SUCCESS                         0x01
#define CGMS_RACP_RESPONSE_CODE_OPCODE_NOT_SUPPORTED            0x02
#define CGMS_RACP_RESPONSE_CODE_INVALID_OPERATOR                0x03
#define CGMS_RACP_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED          0x04
#define CGMS_RACP_RESPONSE_CODE_INVALID_OPERAND                 0x05
#define CGMS_RACP_RESPONSE_CODE_NO_RECORDS_FOUND                0x06
#define CGMS_RACP_RESPONSE_CODE_ABORT_UNSUCCESSFUL              0x07
#define CGMS_RACP_RESPONSE_CODE_PROCEDURE_NOT_COMPLETED         0x08
#define CGMS_RACP_RESPONSE_CODE_OPERAND_NOT_SUPPORTED           0x09

   /* The following defines the valid values that may be set as the     */
   /* value for the OpCode field of Specific Ops Control Point          */
   /* characteristic.                                                   */
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_CGM_COMMUNICATION_INTERVAL             0x01
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_CGM_COMMUNICATION_INTERVAL             0x02
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_GLUCOSE_CALIBRATION_VALUE              0x04
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_GLUCOSE_CALIBRATION_VALUE              0x05
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_PATIENT_HIGH_ALERT_LEVEL               0x07
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_PATIENT_HIGH_ALERT_LEVEL               0x08
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_PATIENT_LOW_ALERT_LEVEL                0x0A
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_PATIENT_LOW_ALERT_LEVEL                0x0B
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_HYPO_ALERT_LEVEL                       0x0D
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_HYPO_ALERT_LEVEL                       0x0E
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_HYPER_ALERT_LEVEL                      0x10
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_HYPER_ALERT_LEVEL                      0x11
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_RATE_OF_DECREASE_ALERT_LEVEL           0x13
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_RATE_OF_DECREASE_ALERT_LEVEL           0x14
#define CGMS_SPECIFIC_OPS_CP_OPCODE_SET_RATE_OF_INCREASE_ALERT_LEVEL           0x16
#define CGMS_SPECIFIC_OPS_CP_OPCODE_GET_RATE_OF_INCREASE_ALERT_LEVEL           0x17
#define CGMS_SPECIFIC_OPS_CP_OPCODE_RESET_DEVICE_SPECIFIC_ALERT                0x19
#define CGMS_SPECIFIC_OPS_CP_OPCODE_START_SESSION                              0x1A
#define CGMS_SPECIFIC_OPS_CP_OPCODE_STOP_SESSION                               0x1B


   /* The followimg defines the valid values that may be set a the value*/
   /* for the OpCode field of the Specific Ops Control Point Response   */
   /* OpCode.                                                           */
#define CGMS_SPECIFIC_OPS_CP_OPCODE_COMMUNICATION_INTERVAL_RESPONSE            0x03
#define CGMS_SPECIFIC_OPS_CP_OPCODE_CALIBRATION_VALUE_RESPONSE                 0x06
#define CGMS_SPECIFIC_OPS_CP_OPCODE_PATIENT_HIGH_ALERT_LEVEL_RESPONSE          0x09
#define CGMS_SPECIFIC_OPS_CP_OPCODE_PATIENT_LOW_ALERT_LEVEL_RESPONSE           0x0C
#define CGMS_SPECIFIC_OPS_CP_OPCODE_HYPO_ALERT_LEVEL_RESPONSE                  0x0F
#define CGMS_SPECIFIC_OPS_CP_OPCODE_HYPER_ALERT_LEVEL_RESPONSE                 0x12
#define CGMS_SPECIFIC_OPS_CP_OPCODE_RATE_OF_DECREASE_ALERT_LEVEL_RESPONSE      0x15
#define CGMS_SPECIFIC_OPS_CP_OPCODE_RATE_OF_INCREASE_ALERT_LEVEL_RESPONSE      0x18
#define CGMS_SPECIFIC_OPS_CP_OPCODE_RESPONSE                                   0x1C

   /* The following defines the valid values that may be set as the     */
   /* value for the Response Code value field of Specific Operation     */
   /* Control Point characteristic.                                     */
#define CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_SUCCESS                           0x01
#define CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_NOT_SUPPORTED                     0x02
#define CGMS_SPECIFIC_OPS_CP_RESPONSE_INVALID_OPERAND                          0x03
#define CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PROCEDURE_NOT_COMPLETED           0x04
#define CGMS_SPECIFIC_OPS_CP_RESPONSE_OPCODE_PARAMETER_OUT_OF_RANGE            0x05

   /* The following defines the valid values that may be set as the     */
   /* value for the Calibration Statues field of Specific Operation     */
   /* Control Point Characteristic.                                     */
#define CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_CALIBRATION_DATA_REJECTED      0x01
#define CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_CALIBRATION_DATA_OUT_OF_RANGE  0x02
#define CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_CALIBRATION_PROCESS_PENDING    0x04
#define CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_RESERVED_BITS                  0x78
#define CGMS_SPECIFIC_OPS_CP_CALIBRATION_STATUS_STATUS_EXTENSION               0x80

   /* The following structure defines the format of a CGMS Measurement  */
   /* value that must always be specified in the CGMS Measurement       */
   /* characteristic value.                                             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Measurement_t
{
   NonAlignedByte_t Size;
   NonAlignedByte_t Flags;
   NonAlignedWord_t Glucose_Concentration;
   NonAlignedWord_t Time_Offset;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Measurement_t;

#define CGMS_MEASUREMENT_SIZE(_x)                               (BTPS_STRUCTURE_OFFSET(CGMS_Measurement_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the minimum size of a CGMS Measurement value based on */
   /* the CGMS Measurement Flags.  The only parameter to this MACRO is  */
   /* the CGMS Measurement Flags.                                       */
#define CGMS_MEASUREMENT_MINIMUM_LENGTH(_x)                     (CGMS_MEASUREMENT_SIZE(0)                                                                                + \
                                                                (((_x) & CGMS_MEASUREMENT_FLAG_TREND_INFORMATION_PRESENT)                   ? NON_ALIGNED_WORD_SIZE : 0) + \
                                                                (((_x) & CGMS_MEASUREMENT_FLAG_QUALITY_PRESENT)                             ? NON_ALIGNED_WORD_SIZE : 0) + \
                                                                (((_x) & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_STATUS_PRESENT)   ? NON_ALIGNED_BYTE_SIZE : 0) + \
                                                                (((_x) & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_CAL_TEMP_PRESENT) ? NON_ALIGNED_BYTE_SIZE : 0) + \
                                                                (((_x) & CGMS_MEASUREMENT_FLAG_SENSOR_STATUS_ANNUNCIATION_WARNING_PRESENT)  ? NON_ALIGNED_BYTE_SIZE : 0))

   /* The following structure defines the format of CGMS Feature        */
   /* characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Feature_t
{
   NonAlignedByte_t Features[3];
   NonAlignedByte_t Type_Sample_Location;
   NonAlignedWord_t E2E_CRC;
} __PACKED_STRUCT_END__ CGMS_Feature_t;

#define CGMS_FEATURE_SIZE                                       (sizeof(CGMS_Feature_t))

   /* The following structure defines the format of CGMS Status         */
   /* characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Status_t
{
   NonAlignedWord_t Time_Offset;
   NonAlignedByte_t Status[3];
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Status_t;

#define CGMS_STATUS_SIZE(_x)                                    (BTPS_STRUCTURE_OFFSET(CGMS_Status_t, Variable_Data) + (_x))

   /* The following type definition defines the structure of the CGMS   */
   /* Session Start Time characteristic value.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Session_Start_Time_t
{
   GATT_Date_Time_Characteristic_t Time;
   NonAlignedSByte_t               Time_Zone;
   NonAlignedByte_t                DST_Offset;
   NonAlignedByte_t                Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Session_Start_Time_t;

#define CGMS_SESSION_START_TIME_SIZE(_x)                        (BTPS_STRUCTURE_OFFSET(CGMS_Session_Start_Time_t, Variable_Data) + (_x))

   /* The following structure defines the format of CGMS Session Run    */
   /* Time characteristic.                                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Session_Run_Time_t
{
   NonAlignedWord_t Time;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Session_Run_Time_t;

#define CGMS_SESSION_RUN_TIME_SIZE(_x)                          (BTPS_STRUCTURE_OFFSET(CGMS_Session_Run_Time_t, Variable_Data) + (_x))

   /* The following strcuture defines a Time Offset Range from Minimum  */
   /* to Maximum value.                                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Time_Offset_Range_t
{
   NonAlignedWord_t Minimum;
   NonAlignedWord_t Maximum;
} __PACKED_STRUCT_END__ CGMS_Time_Offset_Range_t;

#define CGMS_TIME_OFFSET_RANGE_SIZE                             (sizeof(CGMS_Time_Offset_Range_t))

   /* The following defines the format of RACP Response Code value that */
   /* is passed in the RACP Response strcuture.                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_RACP_Response_Code_t
{
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Response_Code_Value;
} __PACKED_STRUCT_END__ CGMS_RACP_Response_Code_t;

#define CGMS_RACP_RESPONSE_CODE_SIZE                            (sizeof(CGMS_RACP_Response_Code_t))

   /* The following defines the format of the Record Access Control     */
   /* Point Request/Response structure.                                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Record_Access_Control_Point_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Operator;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Record_Access_Control_Point_t;

#define CGMS_RECORD_ACCESS_CONTROL_POINT_SIZE(_x)               (BTPS_STRUCTURE_OFFSET(CGMS_Record_Access_Control_Point_t, Variable_Data) + (_x))

   /* The following defines the Calibration Data Record structure.      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Calibration_Record_t
{
   NonAlignedWord_t Calibration_Glucose_Concentration;
   NonAlignedWord_t Calibration_Time;
   NonAlignedByte_t Calibration_Type_Sample_Location;
   NonAlignedWord_t Next_Calibration_Time;
   NonAlignedWord_t Calibration_Data_Record_Number;
   NonAlignedByte_t Calibration_Status;
} __PACKED_STRUCT_END__ CGMS_Calibration_Record_t;

#define CGMS_CALIBRATION_RECORD_SIZE                            (sizeof(CGMS_Calibration_Record_t))

  /* The following structure defines the format of the                  */
  /* Specific_Operations_Control_Point_Response.                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCGMS_Specific_Ops_Control_Point_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ CGMS_Specific_Ops_Control_Point_t;

#define CGMS_SPECIFIC_OPS_CONTROL_POINT_SIZE(_x)                (BTPS_STRUCTURE_OFFSET(CGMS_Specific_Ops_Control_Point_t, Variable_Data) + (_x))

   /* The following defines the length of the E2E-CRC value.            */
#define CGMS_E2E_CRC_VALUE_LENGTH                               (NON_ALIGNED_WORD_SIZE)

   /* The following defines the E2E-CRC not supported value.            */
#define CGMS_E2E_NOT_SUPPORTED_VALUE                            (0xFFFF)

   /* The following defines the CGMS GATT Service Flags MASK that should*/
   /* be passed into GATT_Register_Service when the CGMS Service is     */
   /* registered.                                                       */
#define CGMS_SERVICE_FLAGS_LE                                   (GATT_SERVICE_FLAGS_LE_SERVICE)
#define CGMS_SERVICE_FLAGS_BR_EDR                               (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define CGMS_SERVICE_FLAGS_DUAL_MODE                            (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

   /* The following defines the Special Short Float values.             */
#define CGMS_SFLOAT_NOT_A_NUMBER                                (0x07FF)
#define CGMS_SFLOAT_NOT_AT_THIS_RESOLUTION                      (0x0800)
#define CGMS_SFLOAT_POSITIVE_INFINITY                           (0x07FE)
#define CGMS_SFLOAT_NEGATIVE_INFINITY                           (0x0802)

#endif
