/*****< rscmtype.h >***********************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  RSCMTYPE - Running Speed and Cadence Manager API Type Definitions and     */
/*             Constants for Stonestreet One Bluetooth Protocol Stack Platform*/
/*             Manager.                                                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/18/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __RSCMTYPEH__
#define __RSCMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "RSCMMSG.h"      /* BTPM RSC Manager Message Formats.                */

   /* The following bits are used to indicate which optional            */
   /* characteristics a remote sensor supports.                         */
#define RSCM_SUPPORTED_CHARACTERISTIC_SENSOR_LOCATION          0x00000001
#define RSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT            0x00000002

   /* The following bits are used as flags to the Configure Remote      */
   /* Sensor command.                                                   */
   /* * NOTE * The Skip Supported Sensor Locations flag will cause the  */
   /*          configure process to not query the supported locations   */
   /*          even if the remote side declares support for multiple    */
   /*          locations. This is mostly useful with PTS testing since  */
   /*          the Control Point command for getting the locations can  */
   /*          cause tests to fail.                                     */
#define RSCM_CONFIGURE_FLAG_SKIP_SUPPORTED_SENSOR_LOCATIONS    0x00000001

   /* The following constants are used as status codes for a            */
   /* configuration attempt.                                            */
#define RSCM_CONFIGURATION_STATUS_SUCCESS                      0x00000000
#define RSCM_CONFIGURATION_STATUS_GATT_OPERATION_FAILURE       0x00000001
#define RSCM_CONFIGURATION_STATUS_CANCELLED                    0x00000002
#define RSCM_CONFIGURATION_STATUS_UNKNOWN_ERROR                0xFFFFFFFF

   /* The following constants are used as status codes for a get sensor */
   /* location request.                                                 */
#define RSCM_GET_SENSOR_LOCATION_STATUS_SUCCESS                0x00000000
#define RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_SECURITY       0x00000001
#define RSCM_GET_SENSOR_LOCATION_STATUS_INSUFFICIENT_RESOURCES 0x00000002
#define RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_TIMEOUT        0x00000003
#define RSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN        0x00000004

   /* The following constants are used as status codes for a control    */
   /* point procedure.                                                  */
#define RSCM_PROCEDURE_STATUS_SUCCESS                          0x00000000
#define RSCM_PROCEDURE_STATUS_FAILURE_SECURITY                 0x00000001
#define RSCM_PROCEDURE_STATUS_INSUFFICIENT_RESOURCES           0x00000002
#define RSCM_PROCEDURE_STATUS_FAILURE_TIMEOUT                  0x00000003
#define RSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE         0x00000004
#define RSCM_PROCEDURE_STATUS_FAILURE_ERROR_RESPONSE           0x00000005
#define RSCM_PROCEDURE_STATUS_FAILURE_UNKNOWN                  0x00000006

   /* The following constants indicate control point error codes        */
   /* returned from a remote device.                                    */
#define RSCM_CONTROL_POINT_ERROR_OPCODE_NOT_SUPPORTED          RSCS_SC_CONTROL_POINT_RESPONSE_OPCODE_NOT_SUPPORTED;
#define RSCM_CONTROL_POINT_ERROR_INVALID_PARAMETER             RSCS_SC_CONTROL_POINT_RESPONSE_INVALID_PARAMETER;
#define RSCM_CONTROL_POINT_ERROR_OPERATION_FAILED              RSCS_SC_CONTROL_POINT_RESPONSE_OPERATION_FAILED;

   /* The following constants are used as bitmasks to determine the     */
   /* supported features of a remote sensor.                            */
#define RSCM_FEATURE_INSTANTANEOUS_STRIDE_LENGTH_MEASUREMENT_SUPPORTED  RSCS_FEATURE_FLAG_INSTANTANEOUS_STRIDE_LENGTH_MEASUREMENT_SUPPORTED
#define RSCM_FEATURE_TOTAL_DISTANCE_MEASUREMENT_SUPPORTED               RSCS_FEATURE_FLAG_TOTAL_DISTANCE_MEASUREMENT_SUPPORTED
#define RSCM_FEATURE_WALKING_OR_RUNNING_STATUS_SUPPORTED                RSCS_FEATURE_FLAG_WALKING_OR_RUNNING_STATUS_SUPPORTED
#define RSCM_FEATURE_CALIBRATION_PROCEDURE_SUPPORTED                    RSCS_FEATURE_FLAG_CALIBRATION_PROCEDURE_SUPPORTED
#define RSCM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED                RSCS_FEATURE_FLAG_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED

   /* The following enum represents the defined sensor locations for a  */
   /* remote sensor.                                                    */
typedef enum
{
   rslOther       = RSCS_SENSOR_LOCATION_OTHER,
   rslTopOfShoe   = RSCS_SENSOR_LOCATION_TOP_OF_SHOE,
   rslInShoe      = RSCS_SENSOR_LOCATION_IN_SHOE,
   rslHip         = RSCS_SENSOR_LOCATION_HIP,
   rslFrontWheel  = RSCS_SENSOR_LOCATION_FRONT_WHEEL,
   rslLeftCrank   = RSCS_SENSOR_LOCATION_LEFT_CRANK,
   rslRightCrank  = RSCS_SENSOR_LOCATION_RIGHT_CRANK,
   rslLeftPedal   = RSCS_SENSOR_LOCATION_LEFT_PEDAL,
   rslRightPedal  = RSCS_SENSOR_LOCATION_RIGHT_PEDAL,
   rslFrontHub    = RSCS_SENSOR_LOCATION_FRONT_HUB,
   rslRearDropout = RSCS_SENSOR_LOCATION_REAR_DROPOUT,
   rslChainstay   = RSCS_SENSOR_LOCATION_CHAINSTAY,
   rslRearWheel   = RSCS_SENSOR_LOCATION_REAR_WHEEL,
   rslRearHub     = RSCS_SENSOR_LOCATION_REAR_HUB,
   rslChest       = RSCS_SENSOR_LOCATION_CHEST
} RSCM_Sensor_Location_t;

   /* The following bits are used in the sensor location bitmask when   */
   /* querying the list of supported locations.                         */
#define RSCM_SUPPORTED_SENSOR_LOCATION_OTHER                   0x00000001
#define RSCM_SUPPORTED_SENSOR_LOCATION_TOP_OF_SHOE             0x00000002
#define RSCM_SUPPORTED_SENSOR_LOCATION_IN_SHOE                 0x00000004
#define RSCM_SUPPORTED_SENSOR_LOCATION_HIP                     0x00000008
#define RSCM_SUPPORTED_SENSOR_LOCATION_FRONT_WHEEL             0x00000010
#define RSCM_SUPPORTED_SENSOR_LOCATION_LEFT_CRANK              0x00000020
#define RSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_CRANK             0x00000040
#define RSCM_SUPPORTED_SENSOR_LOCATION_LEFT_PEDAL              0x00000080
#define RSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_PEDAL             0x00000100
#define RSCM_SUPPORTED_SENSOR_LOCATION_FRONT_HUB               0x00000200
#define RSCM_SUPPORTED_SENSOR_LOCATION_REAR_DROPOUT            0x00000400
#define RSCM_SUPPORTED_SENSOR_LOCATION_CHAINSTAY               0x00000800
#define RSCM_SUPPORTED_SENSOR_LOCATION_REAR_WHEEL              0x00001000
#define RSCM_SUPPORTED_SENSOR_LOCATION_REAR_HUB                0x00002000
#define RSCM_SUPPORTED_SENSOR_LOCATION_CHEST                   0x00004000

   /* The following structure contains data about a connected device    */
   /* returned in Query Connected Devices.                              */
   /* * NOTE * The SupportedFeatures member is only valid if the        */
   /*          Configured member is TRUE.                               */
   /* * NOTE * The SupportedSensorLocations member is only valid if     */
   /*          the Configured member is TRUE and the SupportedFeatures  */
   /*          member indicates multiple sensor locations are supported.*/
typedef struct _tagRSCM_Connected_Sensor_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned long SupportedOptionalCharateristics;
   Boolean_t     Configured;
   unsigned long SupportedFeatures;
   DWord_t       SupportedSensorLocations;
} RSCM_Connected_Sensor_t;

#define RSCM_CONNECTED_SENSOR_SIZE                             (sizeof(RSCM_Connected_Sensor_t))

   /* The following structure contains RSC measurement data that is     */
   /* passed to the function that builds the RSC Measurement packet.    */
typedef struct _tagRSCM_Measurement_Data_t
{
   unsigned long Flags;
   Word_t        Instantaneous_Speed;
   Byte_t        Instantaneous_Cadence;
   Word_t        Instantaneous_Stride_Length;
   DWord_t       Total_Distance;
} RSCM_Measurement_Data_t;

#define RSCM_MEASUREMENT_DATA_SIZE                             (sizeof(RSCM_Measurement_Data_t))

   /* The following bit fields are used as flags for RSC Measurement    */
   /* data.                                                             */
#define RSCM_MEASUREMENT_INSTANTANEOUS_STRIDE_LENGTH_PRESENT   RSCS_RSC_MEASUREMENT_INSTANTANEOUS_STRIDE_LENGTH_PRESENT
#define RSCM_MEASUREMENT_TOTAL_DISTANCE_PRESENT                RSCS_RSC_MEASUREMENT_TOTAL_DISTANCE_PRESENT
#define RSCM_MEASUREMENT_STATUS_RUNNING                        RSCS_RSC_MEASUREMENT_STATUS_RUNNING
#define RSCM_MEASUREMENT_WALKING_RUNNING_BIT_VALID             0x80000000

#endif
