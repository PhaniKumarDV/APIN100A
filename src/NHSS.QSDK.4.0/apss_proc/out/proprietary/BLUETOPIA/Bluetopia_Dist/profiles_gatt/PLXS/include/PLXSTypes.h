/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< plxstypes.h >**********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PLXSTypes - Qualcomm Technologies Bluetooth Pulse Oximeter Service Types. */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/06/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __PLXSTYPEH__
#define __PLXSTYPEH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following define the defined PLXS Error Codes that may be sent*/
   /* in a GATT Error Response.                                         */
#define PLXS_ERROR_CODE_SUCCESS                          0x00

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define PLXS_ERROR_CODE_INSUFFICIENT_AUTHENTICATION      (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)
#define PLXS_ERROR_CODE_INSUFFICIENT_AUTHORIZATION       (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION)
#define PLXS_ERROR_CODE_INSUFFICIENT_ENCRYPTION          (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION)
#define PLXS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED       (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define PLXS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS    (ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)
#define PLXS_ERROR_CODE_UNLIKELY_ERROR                   (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the Automation*/
   /* IO Service 16 bit UUID to the specified UUID_16_t variable.  This */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the PLXS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_PLXS_SERVICE_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x22)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PLXS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* PLXS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the PLXS Service UUID.                              */
#define PLXS_COMPARE_PLXS_SERVICE_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x22)

   /* The following defines the PLXS UUID that is used when building the*/
   /* PLXS Service Table.                                               */
#define PLXS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x22, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the PLXS      */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the PLXS UUID Constant value.                          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_PLXS_SERVICE_SDP_UUID_16(_x)         ASSIGN_SDP_UUID_16((_x), 0x18, 0x22)

   /* The following MACRO is a utility MACRO that assigns the PLXS Spot */
   /* Check Measurement Characteristic 16 bit UUID to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the PLXS Spot Check         */
   /* Measurement Characteristic UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_SPOT_CHECK_MEASUREMENT_CHARACTERISTIC_UUID_16(_x)           SSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x5E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PLXS Spot Check Measurement Characteristic */
   /* UUID in UUID16 form.  This MACRO only returns whether the         */
   /* UUID_16_t variable is equal to the Spot Check Measurement         */
   /* Characteristic UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the PLXS Spot Check Measurement Characteristic UUID.*/
#define PLXS_COMPARE_SPOT_CHECK_MEASUREMENT_CHARACTERISTIC_UUID_TO_UUID_16(_x)  COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x5E)

   /* The following defines the PLXS Spot Check Measurement             */
   /* Characteristic UUID that is used when building the PLXS Service   */
   /* Table.                                                            */
#define PLXS_SPOT_CHECK_MEASUREMENT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT      { 0x5E, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the PLXS      */
   /* Continuous Measurement Characteristic 16 bit UUID to the specified*/
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the PLXS Continuous         */
   /* Measurement Characteristic UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_CONTINUOUS_MEASUREMENT_CHARACTERISTIC_UUID_16(_x)           SSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x5F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PLXS Continuous Measurement Characteristic */
   /* UUID in UUID16 form.  This MACRO only returns whether the         */
   /* UUID_16_t variable is equal to the Continuous Measurement         */
   /* Characteristic UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the PLXS Continuous Measurement Characteristic UUID.*/
#define PLXS_COMPARE_CONTINUOUS_MEASUREMENT_CHARACTERISTIC_UUID_TO_UUID_16(_x)  COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x5F)

   /* The following defines the PLXS Continuous Measurement             */
   /* Characteristic UUID that is used when building the PLXS Service   */
   /* Table.                                                            */
#define PLXS_CONTINUOUS_MEASUREMENT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT      { 0x5F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the PLXS PLX  */
   /* Features Characteristic 16 bit UUID to the specified UUID_16_t    */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the PLXS PLX Features Characteristic  */
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_PLX_FEATURES_CHARACTERISTIC_UUID_16(_x)           SSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x60)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PLXS PLX Features Characteristic UUID in   */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the PLX Features Characteristic UUID (MACRO  */
   /* returns boolean result) NOT less than/greater than.  The first    */
   /* parameter is the UUID_16_t variable to compare to the PLXS PLX    */
   /* Features Characteristic UUID.                                     */
#define PLXS_COMPARE_PLX_FEATURES_CHARACTERISTIC_UUID_TO_UUID_16(_x)  COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x60)

   /* The following defines the PLXS PLX Features Characteristic UUID   */
   /* that is used when building the PLXS Service Table.                */
#define PLXS_PLX_FEATURES_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT      { 0x60, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the Record    */
   /* Access Control Point Type Characteristic 16 bit UUID to the       */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the Record     */
   /* Access Control Point Type UUID Constant value.                    */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define PLXS_ASSIGN_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_16(_x)           ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x52)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Record Access Control Point Type UUID in   */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the Record Access Control Point Type UUID    */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the Record*/
   /* Access Control Point Type UUID.                                   */
#define PLXS_COMPARE_RECORD_ACCESS_CONTROL_POINT_TYPE_UUID_TO_UUID_16(_x)  COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x52)

   /* The following defines the Record Access Control Point Type        */
   /* Characteristic UUID that is used when building the PLXS Service   */
   /* Table.                                                            */
#define PLXS_RECORD_ACCESS_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT      { 0x52, 0x2A }

   /* The following defines the supported features of the PLX Features  */
   /* Characteristic.                                                   */
#define PLXS_FEATURES_MEASUREMENT_STATUS_SUPPORTED              (0x0001)
#define PLXS_FEATURES_DEVICE_AND_SENSOR_STATUS_SUPPORTED        (0x0002)
#define PLXS_FEATURES_SPOT_CHECK_MEASUREMENT_STORAGE_SUPPORTED  (0x0004)
#define PLXS_FEATURES_TIME_STAMP_SUPPORTED                      (0x0008)
#define PLXS_FEATURES_SPO2PR_FAST_METRIC_SUPPORTED              (0x0010)
#define PLXS_FEATURES_SPO2PR_SLOW_METRIC_SUPPORTED              (0x0020)
#define PLXS_FEATURES_PULSE_AMPLITUDE_INDEX_SUPPORTED           (0x0040)
#define PLXS_FEATURES_MULTIPLE_BONDS_SUPPORTED                  (0x0080)

   /* The following defines the Measurement Status Support of the PLX   */
   /* Features Characteristic.                                          */
   /* * NOTE * If PLXS_FEATURES_MEASUREMENT_STATUS_SUPPORTED is not     */
   /*          defined for the PLX Features Characteristic, then all of */
   /*          the following bits MUST be set to zero.                  */
   /* * NOTE * The PLXS Measurement Status bits have the form           */
   /*          PLXS_MEASUREMENT_STATUS_XXX and can be found below.      */
   /*          These bits MUST NOT be set unless the                    */
   /*          PLXS_FEATURES_MEASUREMENT_STATUS_SUPPORTED bit is        */
   /*          supported by the PLX Features Characteristic.            */
#define PLXS_MSS_MEASUREMENT_ONGOING_SUPPORTED                (0x0020)
#define PLXS_MSS_EARLY_ESTIMATE_DATA_SUPPORTED                (0x0040)
#define PLXS_MSS_VALIDATED_DATA_SUPPORTED                     (0x0080)
#define PLXS_MSS_FULLY_QUALIFIED_DATA_SUPPORTED               (0x0100)
#define PLXS_MSS_MEASUREMENT_STORAGE_SUPPORTED                (0x0200)
#define PLXS_MSS_DATA_FOR_DEMONSTRATION_SUPPORTED             (0x0400)
#define PLXS_MSS_DATA_FOR_TESTING_SUPPORTED                   (0x0800)
#define PLXS_MSS_CALIBRATION_ONGOING_SUPPORTED                (0x1000)
#define PLXS_MSS_MEASUREMENT_UNAVAILABLE_SUPPORTED            (0x2000)
#define PLXS_MSS_QUESTIONABLE_MEASUREMENT_DETECTED_SUPPORTED  (0x4000)
#define PLXS_MSS_INVALID_MEASUREMENT_DETECTED_SUPPORTED       (0x8000)

   /* The following defines the Device and Sensor Status Support of the */
   /* PLX Features Characteristic.                                      */
   /* * NOTE * If PLXS_FEATURES_DEVICE_AND_SENSOR_STATUS_SUPPORTED is   */
   /*          not defined for the PLX Features Characteristic, then all*/
   /*          of the following bits MUST be set to zero.               */
   /* * NOTE * The PLXS Device and Sensor Status bits have the form     */
   /*          PLXS_DEVICE_AND_SENSOR__XXX and can be found below.      */
   /*          These bits MUST NOT be set unless the                    */
   /*          PLXS_FEATURES_DEVICE_AND_SENSOR_STATUS_SUPPORTED bit is  */
   /*          supported by the PLX Features Characteristic.            */
   /* * NOTE * The following constants are 24-bit values, however the   */
   /*          upper 8 bits are reserved for future use.                */
#define PLXS_DSSS_EXTENDED_DISPLAY_UPDATE_ONGOING_SUPPORTED          (0x0001)
#define PLXS_DSSS_EQUIPMENT_MALFUNCTION_DETECTED_SUPPORTED           (0x0002)
#define PLXS_DSSS_SIGNAL_PROCESSING_IRREGULARITY_DETECTED_SUPPORTED  (0x0004)
#define PLXS_DSSS_INADEQUITE_SIGNAL_DETECTED_SUPPORTED               (0x0008)
#define PLXS_DSSS_POOR_SIGNAL_DETECTED_SUPPORTED                     (0x0010)
#define PLXS_DSSS_LOW_PERFUSION_DETECTED_SUPPORTED                   (0x0020)
#define PLXS_DSSS_ERRATIC_SIGNAL_DETECTED_SUPPORTED                  (0x0040)
#define PLXS_DSSS_NON_PULSATILE_SIGNAL_DETECTED_SUPPORTED            (0x0080)
#define PLXS_DSSS_QUESTIONABLE_PULSE_DETECTED_SUPPORTED              (0x0100)
#define PLXS_DSSS_SIGNAL_ANALYSIS_ONGOING_SUPPORTED                  (0x0200)
#define PLXS_DSSS_SENSOR_INTERFACE_DETECTED_SUPPORTED                (0x0400)
#define PLXS_DSSS_SENSOR_UNCONNECTED_TO_USER_SUPPORTED               (0x0800)
#define PLXS_DSSS_UNKNOWN_SENSOR_CONNECTED_SUPPORTED                 (0x1000)
#define PLXS_DSSS_SENSOR_DISPLACEMENT_SUPPORTED                      (0x2000)
#define PLXS_DSSS_SENSOR_MALFUNCTIONING_SUPPORTED                    (0x4000)
#define PLXS_DSSS_SENSOR_DISCONNECTED_SUPPORTED                      (0x8000)

   /* The following defines the PLX Spot-Check Measurement flags (bit   */
   /* mask) that are used to control the optional fields to include in a*/
   /* Spot Check Measurement indication.                                */
#define PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT                (0x01)
#define PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT        (0x02)
#define PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT  (0x04)
#define PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_PULSE_AMPLITUDE_INDEX_PRESENT     (0x08)
#define PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_DEVICE_CLOCK_NOT_SET              (0x10)

   /* The following defines the PLX Continuous Measurement flags        */
   /* (bitmask) that are used to control the optional fields to include */
   /* in a Continuous Measurement notification.                         */
#define PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_FAST_FIELD_PRESENT         (0x01)
#define PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_SLOW_FIELD_PRESENT         (0x02)
#define PLXS_CONTINUOUS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT        (0x04)
#define PLXS_CONTINUOUS_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT  (0x08)
#define PLXS_CONTINUOUS_MEASUREMENT_FLAGS_PULSE_AMPLITUDE_INDEX_PRESENT     (0x10)

   /* The following defines the Status bits that may optionally be      */
   /* included in a PLX Spot-Check or Continuous Measurements that are  */
   /* indicated/notified to the PLXS Client.                            */
#define PLXS_MEASUREMENT_STATUS_MEASUREMENT_ONGOING                (0x0020)
#define PLXS_MEASUREMENT_STATUS_EARLY_ESTIMATE_DATA                (0x0040)
#define PLXS_MEASUREMENT_STATUS_VALIDATED_DATA                     (0x0080)
#define PLXS_MEASUREMENT_STATUS_FULLY_QUALIFIED_DATA               (0x0100)
#define PLXS_MEASUREMENT_STATUS_DATA_FROM_STORAGE                  (0x0200)
#define PLXS_MEASUREMENT_STATUS_DATA_FOR_DEMONSTRATION             (0x0400)
#define PLXS_MEASUREMENT_STATUS_DATA_FOR_TESTING                   (0x0800)
#define PLXS_MEASUREMENT_STATUS_CALIBRATION_ONGOING                (0x1000)
#define PLXS_MEASUREMENT_STATUS_MEASUREMENT_UNAVAILABLE            (0x2000)
#define PLXS_MEASUREMENT_STATUS_QUESTIONABLE_MEASUREMENT_DETECTED  (0x4000)
#define PLXS_MEASUREMENT_STATUS_INVALID_MEASUREMENT_DETECTED       (0x8000)

   /* The following defines the Device and Sensor Status bits that may  */
   /* optionally be included in a PLX Spot-Check or Continuous          */
   /* Measurements that are indicated/notified to the PLXS Client.      */
#define PLXS_DEVICE_AND_SENSOR_STATUS_EXTENDED_DISPLAY_UPDATE_ONGOING       (0x0001)
#define PLXS_DEVICE_AND_SENSOR_STATUS_EQUIPMENT_MALFUNCTION_DETECTED        (0x0002)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SIGNAL_PROCESS_IRREGULATORY_DETECTED  (0x0004)
#define PLXS_DEVICE_AND_SENSOR_STATUS_INADEQUITE_SIGNAL_DETECTED            (0x0008)
#define PLXS_DEVICE_AND_SENSOR_STATUS_POOR_SIGNAL_DETECTED                  (0x0010)
#define PLXS_DEVICE_AND_SENSOR_STATUS_LOW_PERFUSION_DETECTED                (0x0020)
#define PLXS_DEVICE_AND_SENSOR_STATUS_ERRATIC_SIGN_DETECTED                 (0x0040)
#define PLXS_DEVICE_AND_SENSOR_STATUS_NONPULSATILE_SIGNAL_DETECTED          (0x0080)
#define PLXS_DEVICE_AND_SENSOR_STATUS_QUESTIONABLE_SIGNAL_DETECTED          (0x0100)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SIGNAL_ANALYSIS_ONGOING               (0x0200)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SENSOR_INTERFACE_DETECTED             (0x0400)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SENSOR_UNCONNECTED_TO_USER            (0x0800)
#define PLXS_DEVICE_AND_SENSOR_STATUS_UNKNOWN_SENSOR_CONNECTED              (0x1000)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SENSOR_DISPLACED                      (0x2000)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SENSOR_MALFUNCTIONING                 (0x4000)
#define PLXS_DEVICE_AND_SENSOR_STATUS_SENSOR_DISCONNECTED                   (0x8000)

   /* The following defines the valid values that may be set for the    */
   /* Op_Code field of the PLXS_RACP_Request_t and PLXS_RACP_Response_t */
   /* structures below.                                                 */
   /* * NOTE * Only PLXS_RACP_OPCODE_RESPONSE_CODE is valid for the     */
   /*          Op_Code field of the PLXS_RACP_Response_t structure and  */
   /*          CANNOT be used for the PLXS_RACP_Request_t structure.    */
#define PLXS_RACP_OPCODE_REPORT_STORED_RECORDS              0x01
#define PLXS_RACP_OPCODE_DELETE_STORED_RECORDS              0x02
#define PLXS_RACP_OPCODE_ABORT_OPERATION                    0x03
#define PLXS_RACP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS    0x04
#define PLXS_RACP_OPCODE_NUMBER_OF_STORED_RECORDS_RESPONSE  0x05
#define PLXS_RACP_OPCODE_RESPONSE_CODE                      0x06

   /* The following defines the valid values that may be set for the    */
   /* Operator field of the PLXS_RACP_Request_t and PLXS_RACP_Response_t*/
   /* structures below.                                                 */
#define PLXS_RACP_OPERATOR_NULL                          0x00
#define PLXS_RACP_OPERATOR_ALL_RECORDS                   0x01

   /* The following defines the valid values that may be set for the    */
   /* Operand field (Variable_Data) of the PLXS_RACP_Response_t         */
   /* structure if the Op_Code field is set to                          */
   /* PLXS_RACP_OPCODE_RESPONSE_CODE.                                   */
#define PLXS_RACP_RESPONSE_CODE_VALUE_SUCCESS                  0x01
#define PLXS_RACP_RESPONSE_CODE_VALUE_OPCODE_NOT_SUPPORTED     0x02
#define PLXS_RACP_RESPONSE_CODE_VALUE_INVALID_OPERATOR         0x03
#define PLXS_RACP_RESPONSE_CODE_VALUE_OPERATOR_NOT_SUPPORTED   0x04
#define PLXS_RACP_RESPONSE_CODE_VALUE_INVALID_OPERAND          0x05
#define PLXS_RACP_RESPONSE_CODE_VALUE_NO_RECORDS_FOUND         0x06
#define PLXS_RACP_RESPONSE_CODE_VALUE_ABORT_UNSUCCESSFUL       0x07
#define PLXS_RACP_RESPONSE_CODE_VALUE_PROCEDURE_NOT_COMPLETED  0x08
#define PLXS_RACP_RESPONSE_CODE_VALUE_OPERAND_NOT_SUPPORTED    0x09

   /* The following structure represents the PLXS INT24 structure.      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagPLXS_INT24_t
{
   NonAlignedWord_t Lower;
   NonAlignedByte_t Upper;
} __PACKED_STRUCT_END__ PLXS_INT24_t;

#define PLXS_INT_24_SIZE                                 (sizeof(PLXS_INT24_t))

   /* The following structure represents the PLXS PLX Features          */
   /* Characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagPLXS_PLX_Features_t
{
   NonAlignedWord_t Supported_Features;
   NonAlignedWord_t Measurement_Status_Support;
   PLXS_INT24_t     Device_And_Sensor_Status_Support;
} __PACKED_STRUCT_END__ PLXS_PLX_Features_t;

#define PLXS_PLX_FEATURES_SIZE                           (sizeof(PLXS_PLX_Features_t))

   /* The following structure represents the PLXS Spot-Check and PLXS   */
   /* Continuous Measurement Characteristic.                            */
typedef __PACKED_STRUCT_BEGIN__ struct _tagPLXS_Measurement_t
{
   NonAlignedByte_t Flags;
   NonAlignedWord_t SPO2;
   NonAlignedWord_t PR;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ PLXS_Measurement_t;

#define PLXS_MEASUREMENT_SIZE(_x)                        (BTPS_STRUCTURE_OFFSET(PLXS_Measurement_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the minimum size of a PLXS Spot-Check Measurement     */
   /* value based on the Flags field of the PLXS_Measurement_t          */
   /* structure.  The only parameter to this MACRO is the Flags field.  */
#define PLXS_SPOT_CHECK_MEASUREMENT_MINIMUM_LENGTH(_x)   (PLXS_MEASUREMENT_SIZE(0)                                                                                                   + \
                                                         (((_x) & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT)               ? GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE : 0) + \
                                                         (((_x) & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)       ? NON_ALIGNED_WORD_SIZE                   : 0) + \
                                                         (((_x) & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT) ? PLXS_INT_24_SIZE                        : 0) + \
                                                         (((_x) & PLXS_SPOT_CHECK_MEASUREMENT_FLAGS_PULSE_AMPLITUDE_INDEX_PRESENT)    ? NON_ALIGNED_WORD_SIZE                   : 0))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the minimum size of a PLXS Continuous Measurement     */
   /* value based on the Flags field of the PLXS_Measurement_t          */
   /* structure.  The only parameter to this MACRO is the Flags field.  */
#define PLXS_CONTINUOUS_MEASUREMENT_MINIMUM_LENGTH(_x)   (PLXS_MEASUREMENT_SIZE(0)                                                                                  + \
                                                         (((_x) & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_FAST_FIELD_PRESENT)        ? NON_ALIGNED_DWORD_SIZE : 0) + \
                                                         (((_x) & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_SPO2PR_SLOW_FIELD_PRESENT)        ? NON_ALIGNED_DWORD_SIZE : 0) + \
                                                         (((_x) & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)       ? NON_ALIGNED_WORD_SIZE  : 0) + \
                                                         (((_x) & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_DEVICE_AND_SENSOR_STATUS_PRESENT) ? PLXS_INT_24_SIZE       : 0) + \
                                                         (((_x) & PLXS_CONTINUOUS_MEASUREMENT_FLAGS_PULSE_AMPLITUDE_INDEX_PRESENT)    ? NON_ALIGNED_WORD_SIZE  : 0))

   /* The following structure represents the format of the Record Access*/
   /* Control Point (RACP) Request structure.                           */
typedef __PACKED_STRUCT_BEGIN__ struct _tagPLXS_RACP_Request_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Operator;
} __PACKED_STRUCT_END__ PLXS_RACP_Request_t;

#define PLXS_RACP_REQUEST_SIZE                           (sizeof(PLXS_RACP_Request_t))

   /* The following structure represents the format of the Record Access*/
   /* Control Point (RACP) Response structure.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagPLXS_RACP_Response_t
{
   NonAlignedByte_t Response_Code;
   NonAlignedByte_t Operator;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ PLXS_RACP_Response_t;

#define PLXS_RACP_RESPONSE_SIZE(_x)                      (BTPS_STRUCTURE_OFFSET(PLXS_RACP_Response_t, Variable_Data) + (_x))

  /* The following defines the valid values (bit mask) that may be set  */
  /* for client characteristic configuration descriptors (CCCD).        */
  /* * NOTE * Both cannot be set simultaneously.                        */
#define PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define PLXS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the PLXS GATT Service Flags MASK that should*/
   /* be passed into GATT_Register_Service when the PLXS Service is     */
   /* registered.                                                       */
#define PLXS_SERVICE_FLAGS_LE                            (GATT_SERVICE_FLAGS_LE_SERVICE)
#define PLXS_SERVICE_FLAGS_BR_EDR                        (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define PLXS_SERVICE_FLAGS_DUAL_MODE                     (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
