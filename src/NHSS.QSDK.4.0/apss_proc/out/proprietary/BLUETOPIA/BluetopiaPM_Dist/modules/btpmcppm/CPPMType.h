/*****< CPPMTYPE.h >***********************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CPPMTYPE - Cycling Power Manager API Type Definitions and Constants       */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#ifndef __CPPMTYPEH__
#define __CPPMTYPEH__

   /* The Enumerated values to be used to note the local profile role:  */
typedef enum _tagCPPM_Connection_Type_t
{
   cctSensor,
   cctCollector
} CPPM_Connection_Type_t;

   /* The LastWheelEventTime unit is seconds with a resolution of       */
   /* 1/2048. The value rolls over every 32 seconds.                    */
typedef struct _tagCPPM_Wheel_Revolution_Data_t
{
   DWord_t CumulativeWheelRevolutions;
   Word_t  LastWheelEventTime;
} CPPM_Wheel_Revolution_Data_t;

#define CPPM_WHEEL_REVOLUTION_DATA_SIZE                   (sizeof(CPPM_Wheel_Revolution_Data_t))

   /* The LastCrankEventTime unit is seconds with a resolution of       */
   /* 1/1024. The value rolls over every 64 seconds.                    */
typedef struct _tagCPPM_Crank_Revolution_Data_t
{
   Word_t CumulativeCrankRevolutions;
   Word_t LastCrankEventTime;
} CPPM_Crank_Revolution_Data_t;

#define CPPM_CRANK_REVOLUTION_DATA_SIZE                   (sizeof(CPPM_Crank_Revolution_Data_t))

   /* The unit for the force magnitude values is Newtons. The values    */
   /* are maximum and minimum force measurements per crank revolution.  */
typedef struct _tagCPPM_Extreme_Force_Magnitudes_Data_t
{
   SWord_t MaximumForceMagnitude;
   SWord_t MinimumForceMagnitude;
} CPPM_Extreme_Force_Magnitudes_Data_t;

#define CPPM_EXTREME_FORCE_MAGNITUDES_DATA_SIZE           (sizeof(CPPM_Extreme_Force_Magnitudes_Data_t))

   /* The unit for the torque magnitude values is Newton Meters with a  */
   /* resolution of 1/32. The values are the maximum and minimum torque */
   /* measurements for a single crank revolution.                       */
typedef struct _tagCPPM_Extreme_Torque_Magnitudes_Data_t
{
   SWord_t MaximumTorqueMagnitude;
   SWord_t MinimumTorqueMagnitude;
} CPPM_Extreme_Torque_Magnitudes_Data_t;

#define CPPM_EXTREME_TORQUE_MAGNITUDES_DATA_SIZE          (sizeof(CPPM_Extreme_Torque_Magnitudes_Data_t))

   /* The extreme angles values, in degrees, are the angles of the      */
   /* pedal crank when the maximum and minimum force or torque          */
   /* occurred during a single crank revolution. The degree measure     */
   /* starts when the crank is at 12:00 o'clock.                        */
typedef struct _tagCPPM_Extreme_Angles_Data_t
{
   Word_t MaximumAngle;
   Word_t MinimumAngle;
} CPPM_Extreme_Angles_Data_t;

#define CPPM_EXTREMA_ANGLES_DATA_SIZE                     (sizeof(CPPM_Extreme_Angles_Data_t))

   /*********************************************************************/
   /*   Measurement             Unit              Resolution            */
   /*********************************************************************/
   /* InstantaneousPower        Watts                 1                 */
   /* PedalPowerBalance         Percentage           1/2                */
   /* AccumulatedTorque         Newton Meters       1/32                */
   /* WheelRevolutionData                 See Above                     */
   /* CrankRevolutionData                 See Above                     */
   /* ExtremeForceMagnitudes              See Above                     */
   /* ExtremeTorqueMagnitudes             See Above                     */
   /* ExtremeAngles                       See Above                     */
   /* TopDeadSpotAngle          Degrees               1                 */
   /* BottomDeadSpotAngle       Degrees               1                 */
   /* AccumulatedEnergy         KiloJoules            1                 */
   /*********************************************************************/
   /* Notes:                                                            */
   /*   TopDeadSpotAngle is the angle of the crank when the power       */
   /*      measurement becomes positive.                                */
   /*   BottomDeadSpotAngle is the angle of the crank when the power    */
   /*      measurement becomes negative.                                */
   /*********************************************************************/
typedef struct _tagCPPM_Measurement_Data_t
{
   Word_t                                Flags;
   SWord_t                               InstantaneousPower;
   Byte_t                                PedalPowerBalance;
   Word_t                                AccumulatedTorque;
   CPPM_Wheel_Revolution_Data_t          WheelRevolutionData;
   CPPM_Crank_Revolution_Data_t          CrankRevolutionData;
   CPPM_Extreme_Force_Magnitudes_Data_t  ExtremeForceMagnitudes;
   CPPM_Extreme_Torque_Magnitudes_Data_t ExtremeTorqueMagnitudes;
   CPPM_Extreme_Angles_Data_t            ExtremeAngles;
   Word_t                                TopDeadSpotAngle;
   Word_t                                BottomDeadSpotAngle;
   Word_t                                AccumulatedEnergy;
} CPPM_Measurement_Data_t;

#define CPPM_MEASUREMENT_DATA_SIZE                        (sizeof(CPPM_Measurement_Data_t))

#define CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT                           0x0001
#define CPPM_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_REFERENCE_LEFT                    0x0002
#define CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT                            0x0004
#define CPPM_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_SOURCE_CRANK_BASED                 0x0008
#define CPPM_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT                         0x0010
#define CPPM_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT                         0x0020
#define CPPM_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT                      0x0040
#define CPPM_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT                     0x0080
#define CPPM_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT                                0x0100
#define CPPM_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT                           0x0200
#define CPPM_MEASUREMENT_FLAGS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT                        0x0400
#define CPPM_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT                            0x0800
#define CPPM_MEASUREMENT_FLAGS_OFFSET_COMPENSATION_INDICATOR                         0x1000

#define CPPM_FEATURE_PEDAL_POWER_BALANCE_SUPPORTED                                   0x00000001
#define CPPM_FEATURE_ACCUMULATED_TORQUE_SUPPORTED                                    0x00000002
#define CPPM_FEATURE_WHEEL_REVOLUTION_DATA_SUPPORTED                                 0x00000004
#define CPPM_FEATURE_CRANK_REVOLUTION_DATA_SUPPORTED                                 0x00000008
#define CPPM_FEATURE_EXTREME_MAGNITUDES_SUPPORTED                                    0x00000010
#define CPPM_FEATURE_EXTREME_ANGLES_SUPPORTED                                        0x00000020
#define CPPM_FEATURE_TOP_AND_BOTTOM_DEAD_SPOT_ANGLES_SUPPORTED                       0x00000040
#define CPPM_FEATURE_ACCUMULATED_ENERGY_SUPPORTED                                    0x00000080
#define CPPM_FEATURE_OFFSET_COMPENSATION_INDICATOR_SUPPORTED                         0x00000100
#define CPPM_FEATURE_OFFSET_COMPENSATION_SUPPORTED                                   0x00000200
#define CPPM_FEATURE_MEASUREMENT_CHARACTERISTIC_CONTENT_MASKING_SUPPORTED            0x00000400
#define CPPM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED                             0x00000800
#define CPPM_FEATURE_CRANK_LENGTH_ADJUSTMENT_SUPPORTED                               0x00001000
#define CPPM_FEATURE_CHAIN_LENGTH_ADJUSTMENT_SUPPORTED                               0x00002000
#define CPPM_FEATURE_CHAIN_WEIGHT_ADJUSTMENT_SUPPORTED                               0x00004000
#define CPPM_FEATURE_SPAN_LENGTH_ADJUSTMENT_SUPPORTED                                0x00008000
#define CPPM_FEATURE_SENSOR_MEASUREMENT_CONTEXT_TORQUE                               0x00010000
#define CPPM_FEATURE_INSTANTANEOUS_MEASUREMENT_DIRECTION_SUPPORTED                   0x00020000
#define CPPM_FEATURE_FACTORY_CALIBRATION_DATE_SUPPORTED                              0x00040000

#define CPPM_SENSOR_LOCATION_OTHER                                                   0x00
#define CPPM_SENSOR_LOCATION_TOP_OF_SHOE                                             0x01
#define CPPM_SENSOR_LOCATION_IN_SHOE                                                 0x02
#define CPPM_SENSOR_LOCATION_HIP                                                     0x03
#define CPPM_SENSOR_LOCATION_FRONT_WHEEL                                             0x04
#define CPPM_SENSOR_LOCATION_LEFT_CRANK                                              0x05
#define CPPM_SENSOR_LOCATION_RIGHT_CRANK                                             0x06
#define CPPM_SENSOR_LOCATION_LEFT_PEDAL                                              0x07
#define CPPM_SENSOR_LOCATION_RIGHT_PEDAL                                             0x08
#define CPPM_SENSOR_LOCATION_FRONT_HUB                                               0x09
#define CPPM_SENSOR_LOCATION_REAR_DROPOUT                                            0x0A
#define CPPM_SENSOR_LOCATION_CHAINSTAY                                               0x0B
#define CPPM_SENSOR_LOCATION_REAR_WHEEL                                              0x0C
#define CPPM_SENSOR_LOCATION_REAR_HUB                                                0x0D
#define CPPM_SENSOR_LOCATION_CHEST                                                   0x0E

typedef enum _tagCPPM_Sensor_Location_t
{
   slOther       = CPPM_SENSOR_LOCATION_OTHER,
   slTopOfShoe   = CPPM_SENSOR_LOCATION_TOP_OF_SHOE,
   slInShoe      = CPPM_SENSOR_LOCATION_IN_SHOE,
   slHip         = CPPM_SENSOR_LOCATION_HIP,
   slFrontWheel  = CPPM_SENSOR_LOCATION_FRONT_WHEEL,
   slLeftCrank   = CPPM_SENSOR_LOCATION_LEFT_CRANK,
   slRightCrank  = CPPM_SENSOR_LOCATION_RIGHT_CRANK,
   slLeftPedal   = CPPM_SENSOR_LOCATION_LEFT_PEDAL,
   slRightPedal  = CPPM_SENSOR_LOCATION_RIGHT_PEDAL,
   slFrontHub    = CPPM_SENSOR_LOCATION_FRONT_HUB,
   slRearDropout = CPPM_SENSOR_LOCATION_REAR_DROPOUT,
   slChainstay   = CPPM_SENSOR_LOCATION_CHAINSTAY,
   slRearWheel   = CPPM_SENSOR_LOCATION_REAR_WHEEL,
   slRearHub     = CPPM_SENSOR_LOCATION_REAR_HUB,
   slChest       = CPPM_SENSOR_LOCATION_CHEST
} CPPM_Sensor_Location_t;

   /*********************************************************************/
   /*   Measurement               Unit                   Resolution     */
   /*********************************************************************/
   /* CrankRevolutionData                 See Above                     */
   /* FirstCrankMeasurementAngle  Degrees                    1          */
   /* MagnitudeDataLength                 See Below                     */
   /* InstantaneousMagnitude      Newtons(if force)          1          */ 
   /*                             Newtons Meters(if torque) 1/32        */ 
   /*********************************************************************/
   /* Notes:                                                            */
   /*    FirstCrankMeasurementAngle is the angle of the crank when the  */
   /*      first in a series magnitude measurements was taken for a     */
   /*      single crank revolution.                                     */
   /*    MagnitudeDataLength is the number of magnitude measurements in */
   /*      InstantaneousMagnitude array.                                */
   /*    InstantaneousMagnitude can be force or torque measurements.    */ 
   /*      The direction bits of the flags field indicate the direction */
   /*      of the magnitude measurements.                               */
   /*********************************************************************/
typedef struct _tagCPPM_Vector_Data_t
{
   Byte_t                        Flags;
   CPPM_Crank_Revolution_Data_t  CrankRevolutionData;
   Word_t                        FirstCrankMeasurementAngle;
   Byte_t                        MagnitudeDataLength;
   SWord_t                      *InstantaneousMagnitude;
} CPPM_Vector_Data_t;

#define CPPM_VECTOR_DATA_SIZE  (sizeof(CPPM_Vector_Data_t))

#define CPPM_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT                              0x01
#define CPPM_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT                      0x02
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT                0x04
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT               0x08
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_BITS                   0x30
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_TANGENTIAL_COMPONENT   0x10
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_RADIAL_COMPONENT       0x20
#define CPPM_VECTOR_FLAGS_INSTANTANEOUS_MEASUREMENT_DIRECTION_LATERAL_COMPONENT      0x30

#define CPPM_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE                               0x01
#define CPPM_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION                             0x02
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS                 0x03
#define CPPM_CONTROL_POINT_OPCODE_SET_CRANK_LENGTH                                   0x04
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_CRANK_LENGTH                               0x05
#define CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_LENGTH                                   0x06
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_CHAIN_LENGTH                               0x07
#define CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_WEIGHT                                   0x08
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_CHAIN_WEIGHT                               0x09
#define CPPM_CONTROL_POINT_OPCODE_SET_SPAN_LENGTH                                    0x0A
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_SPAN_LENGTH                                0x0B
#define CPPM_CONTROL_POINT_OPCODE_START_OFFSET_COMPENSATION                          0x0C
#define CPPM_CONTROL_POINT_OPCODE_MASK_MEASUREMENT_CHARACTERISTIC_CONTENT            0x0D
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_SAMPLING_RATE                              0x0E
#define CPPM_CONTROL_POINT_OPCODE_REQUEST_FACTORY_CALIBRATION_DATE                   0x0F

typedef enum _tagCPPM_Procedure_Opcode_t
{
   pocSetCumulativeValue                   = CPPM_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE,
   pocUdateSensorLocation                  = CPPM_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION,
   pocRequestSupportedSensorLocations      = CPPM_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS,
   pocSetCrankLength                       = CPPM_CONTROL_POINT_OPCODE_SET_CRANK_LENGTH,
   pocRequestCrankLength                   = CPPM_CONTROL_POINT_OPCODE_REQUEST_CRANK_LENGTH,
   pocSetChainLength                       = CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_LENGTH,
   pocRequestChainLength                   = CPPM_CONTROL_POINT_OPCODE_REQUEST_CHAIN_LENGTH,
   pocSetChainWeight                       = CPPM_CONTROL_POINT_OPCODE_SET_CHAIN_WEIGHT,
   pocRequestChainWeight                   = CPPM_CONTROL_POINT_OPCODE_REQUEST_CHAIN_WEIGHT,
   pocSetSpanLength                        = CPPM_CONTROL_POINT_OPCODE_SET_SPAN_LENGTH,
   pocRequestSpanLength                    = CPPM_CONTROL_POINT_OPCODE_REQUEST_SPAN_LENGTH,
   pocStartOffsetCompensation              = CPPM_CONTROL_POINT_OPCODE_START_OFFSET_COMPENSATION,
   pocMaskMeasurementCharacteristicContent = CPPM_CONTROL_POINT_OPCODE_MASK_MEASUREMENT_CHARACTERISTIC_CONTENT,
   pocRequestSamplingRate                  = CPPM_CONTROL_POINT_OPCODE_REQUEST_SAMPLING_RATE,
   pocRequestFactoryCalibrationDate        = CPPM_CONTROL_POINT_OPCODE_REQUEST_FACTORY_CALIBRATION_DATE
} CPPM_Procedure_Opcode_t;

   /* Values to be used in mask measurement content procedure:          */
#define CPPM_MEASUREMENT_CONTENT_PEDAL_POWER_BALANCE                                 0x00000001
#define CPPM_MEASUREMENT_CONTENT_ACCUMULATED_TORQUE                                  0x00000002
#define CPPM_MEASUREMENT_CONTENT_WHEEL_REVOLUTION_DATA                               0x00000004
#define CPPM_MEASUREMENT_CONTENT_CRANK_REVOLUTION_DATA                               0x00000008
#define CPPM_MEASUREMENT_CONTENT_EXTREME_MAGNITUDES                                  0x00000010
#define CPPM_MEASUREMENT_CONTENT_EXTREME_ANGLES                                      0x00000020
#define CPPM_MEASUREMENT_CONTENT_TOP_DEAD_SPOT_ANGLES                                0x00000040
#define CPPM_MEASUREMENT_CONTENT_BOTTOM_DEAD_SPOT_ANGLES                             0x00000080
#define CPPM_MEASUREMENT_CONTENT_ACCUMULATED_ENERGY                                  0x00000100

   /*********************************************************************/
   /* The CPPM_Procedure_Data_t is used when writing the control point  */
   /* to initiate a procedure. When the procedure calls for sending     */
   /* data to the sensor the ProcedureParameter is used.                */
   /*********************************************************************/
   /* Notes:                                                            */
   /* CumulativeValue is only used to update the wheel revolutions      */
   /*   cumulative value.                                               */
   /* The CrankLength unit is millimeters with a 1/2 resolution.        */
   /* The ChainLength unit is millimeters with a 1 resolution.          */
   /* The ChainWeight unit is grams with a 1 resolution.                */
   /* The SpanLength unit is millimeters with a 1 resolution.           */
   /*********************************************************************/
typedef struct _tagCPPM_Procedure_Data_t
{
   CPPM_Procedure_Opcode_t    Opcode;
   union
   {
      DWord_t                 CumulativeValue;
      CPPM_Sensor_Location_t  SensorLocation;
      Word_t                  CrankLength;
      Word_t                  ChainLength;
      Word_t                  ChainWeight;
      Word_t                  SpanLength;
      Word_t                  ContentMask;
   } ProcedureParameter;
} CPPM_Procedure_Data_t;

#define CPPM_CONTROL_POINT_RESPONSE_CODE_SUCCESS                0x01
#define CPPM_CONTROL_POINT_RESPONSE_CODE_OPCODE_NOT_SUPPORTED   0x02
#define CPPM_CONTROL_POINT_RESPONSE_CODE_INVALID_PARAMETER      0x03
#define CPPM_CONTROL_POINT_RESPONSE_CODE_OPERATION_FAILED       0x04

typedef enum _tagCPPM_Procedure_Response_Code_t
{
   prcSuccessCPP         = CPPM_CONTROL_POINT_RESPONSE_CODE_SUCCESS,
   prcOpcodeNotSupported = CPPM_CONTROL_POINT_RESPONSE_CODE_OPCODE_NOT_SUPPORTED,
   prcInvalidParameter   = CPPM_CONTROL_POINT_RESPONSE_CODE_INVALID_PARAMETER,
   prcOperationFailed    = CPPM_CONTROL_POINT_RESPONSE_CODE_OPERATION_FAILED
} CPPM_Procedure_Response_Code_t;

typedef struct _tagCPPM_Supported_Sensor_Locations_t
{
   Byte_t                  NumberOfSensorLocations;
   CPPM_Sensor_Location_t *SensorLocations;
} CPPM_Supported_Sensor_Locations_t;

typedef struct _tagCPPM_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} CPPM_Date_Time_Data_t;

   /* Notes:                                                            */
   /* The CrankLength unit is millimeters with a 1/2 resolution.        */
   /* The ChainLength unit is millimeters with a 1 resolution.          */
   /* The ChainWeight unit is grams with a 1 resolution.                */
   /* The SpanLength unit is millimeters with a 1 resolution.           */
   /* The OffsetCompensation unit is either in Newtons with a 1         */
   /*     resolution for a force value or in Newton Meters with 1/32    */
   /*     resolution for a torque value.                                */
   /* The SamplingRate unit is hertz with a 1 resolution.               */
typedef union _tagCPPM_Control_Point_Parameter_t
{
   CPPM_Supported_Sensor_Locations_t SupportedSensorLocations;
   CPPM_Date_Time_Data_t             FactoryCalibrationDate;
   Word_t                            CrankLength;
   Word_t                            ChainLength;
   Word_t                            ChainWeight;
   Word_t                            SpanLength;
   SWord_t                           OffsetCompensation;
   Byte_t                            SamplingRate;
} CPPM_Control_Point_Parameter_t;

   /* The CPPM_Control_Point_Data_t is used to package control point    */
   /* indication data which conclude procedures. When the response      */
   /* includes data from the sensor, the Parameter field is used.       */ 
typedef struct _tagCPPM_Control_Point_Data_t
{
   CPPM_Procedure_Opcode_t        Opcode;
   CPPM_Procedure_Response_Code_t ResponseCode;
   CPPM_Control_Point_Parameter_t Parameter;
} CPPM_Control_Point_Data_t;

  /* The Instance_Record_t structure is used to package instance query  */
  /* data.                                                              */
typedef struct _tagInstance_Record_t
{
   unsigned int            InstanceID;
   unsigned int            StateMask;
   unsigned long           FeatureMask;
   CPPM_Sensor_Location_t  SensorLocation;
} Instance_Record_t;

   /* Values for the instance record state mask                         */
#define CPPM_SENSOR_STATE_MEASUREMENT_ENABLED                            0x0001
#define CPPM_SENSOR_STATE_VECTOR_ENABLED                                 0x0002
#define CPPM_SENSOR_STATE_CONTROL_POINT_ENABLED                          0x0004
#define CPPM_SENSOR_STATE_BROADCAST_ENABLED                              0x0008

   /* ATT protocol error codes                                          */
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE                      0x01
#define BTPM_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED                  0x02
#define BTPM_ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED                 0x03
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_PDU                         0x04
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION         0x05
#define BTPM_ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED               0x06
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET                      0x07
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION          0x08
#define BTPM_ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL                  0x09
#define BTPM_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND                 0x0A
#define BTPM_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG                  0x0B
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE    0x0C
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH      0x0D
#define BTPM_ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR                      0x0E
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION             0x0F
#define BTPM_ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE              0x10
#define BTPM_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES              0x11

#define BTPM_ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED          0xFD
#define BTPM_ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS       0xFE
#define BTPM_ATT_PROTOCOL_ERROR_CODE_OUT_OF_RANGE                        0xFF

#endif
