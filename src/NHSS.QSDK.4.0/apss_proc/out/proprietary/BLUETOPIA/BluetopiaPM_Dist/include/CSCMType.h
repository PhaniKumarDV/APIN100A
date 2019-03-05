/*****< cscmtype.h >***********************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CSCMTYPE - Cycling Speed and Cadence Manager API Type Definitions and     */
/*             Constants for Stonestreet One Bluetooth Protocol Stack Platform*/
/*             Manager.                                                       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/07/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __CSCMTYPEH__
#define __CSCMTYPEH__

#include "SS1BTPS.h"      /* BTPS Protocol Stack Prototypes/Constants.        */

#include "CSCMMSG.h"      /* BTPM CSC Manager Message Formats.                */

   /* The following bits are used to indicate which optional            */
   /* characteristics a remote sensor supports.                         */
#define CSCM_SUPPORTED_CHARACTERISTIC_SENSOR_LOCATION          0x00000001
#define CSCM_SUPPORTED_CHARACTERISTIC_CONTROL_POINT            0x00000002

   /* The following bits are used as flags to the Configure Remote      */
   /* Sensor command.                                                   */
   /* * NOTE * The Skip Supported Sensor Locations flag will cause the  */
   /*          configure process to not query the supported locations   */
   /*          even if the remote side declares support for multiple    */
   /*          locations. This is mostly useful with PTS testing since  */
   /*          the Control Point command for getting the locations can  */
   /*          cause tests to fail.                                     */
#define CSCM_CONFIGURE_FLAG_SKIP_SUPPORTED_SENSOR_LOCATIONS    0x00000001

   /* The following constants are used as status codes for a            */
   /* configuration attempt.                                            */
#define CSCM_CONFIGURATION_STATUS_SUCCESS                      0x00000000
#define CSCM_CONFIGURATION_STATUS_GATT_OPERATION_FAILURE       0x00000001
#define CSCM_CONFIGURATION_STATUS_CANCELLED                    0x00000002
#define CSCM_CONFIGURATION_STATUS_UNKNOWN_ERROR                0xFFFFFFFF

   /* The following constants are used as status codes for a get sensor */
   /* location request.                                                 */
#define CSCM_GET_SENSOR_LOCATION_STATUS_SUCCESS                0x00000000
#define CSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_SECURITY       0x00000001
#define CSCM_GET_SENSOR_LOCATION_STATUS_INSUFFICIENT_RESOURCES 0x00000002
#define CSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_TIMEOUT        0x00000003
#define CSCM_GET_SENSOR_LOCATION_STATUS_FAILURE_UNKNOWN        0x00000004

   /* The following constants are used as status codes for a control    */
   /* point procedure.                                                  */
#define CSCM_PROCEDURE_STATUS_SUCCESS                          0x00000000
#define CSCM_PROCEDURE_STATUS_FAILURE_SECURITY                 0x00000001
#define CSCM_PROCEDURE_STATUS_INSUFFICIENT_RESOURCES           0x00000002
#define CSCM_PROCEDURE_STATUS_FAILURE_TIMEOUT                  0x00000003
#define CSCM_PROCEDURE_STATUS_FAILURE_INVALID_RESPONSE         0x00000004
#define CSCM_PROCEDURE_STATUS_FAILURE_ERROR_RESPONSE           0x00000005
#define CSCM_PROCEDURE_STATUS_FAILURE_UNKNOWN                  0x00000006

   /* The following constants indicate control point error codes        */
   /* returned from a remote device.                                    */
#define CSCM_CONTROL_POINT_ERROR_OPCODE_NOT_SUPPORTED          CSCS_SC_CONTROL_POINT_RESPONSE_CODE_OPCODE_NOT_SUPPORTED;
#define CSCM_CONTROL_POINT_ERROR_INVALID_PARAMETER             CSCS_SC_CONTROL_POINT_RESPONSE_CODE_INVALID_PARAMETER;
#define CSCM_CONTROL_POINT_ERROR_OPERATION_FAILED              CSCS_SC_CONTROL_POINT_RESPONSE_CODE_OPERATION_FAILED;

   /* The following constants are used as bitmasks to determine the     */
   /* supported features of a remote sensor.                            */
#define CSCM_FEATURE_WHEEL_REVOLUTION_DATA_SUPPORTED           CSCS_CSC_FEATURE_WHEEL_REVOLUTION_DATA_SUPPORTED
#define CSCM_FEATURE_CRANK_REVOLUTION_DATA_SUPPORTED           CSCS_CSC_FEATURE_CRANK_REVOLUTION_DATA_SUPPORTED
#define CSCM_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED       CSCS_CSC_FEATURE_MULTIPLE_SENSOR_LOCATIONS_SUPPORTED

   /* The following enum represents the defined sensor locations for a  */
   /* remote sensor.                                                    */
typedef enum
{
   cslOther       = CSCS_SENSOR_LOCATION_OTHER,
   cslTopOfShoe   = CSCS_SENSOR_LOCATION_TOP_OF_SHOE,
   cslInShoe      = CSCS_SENSOR_LOCATION_IN_SHOE,
   cslHip         = CSCS_SENSOR_LOCATION_HIP,
   cslFrontWheel  = CSCS_SENSOR_LOCATION_FRONT_WHEEL,
   cslLeftCrank   = CSCS_SENSOR_LOCATION_LEFT_CRANK,
   cslRightCrank  = CSCS_SENSOR_LOCATION_RIGHT_CRANK,
   cslLeftPedal   = CSCS_SENSOR_LOCATION_LEFT_PEDAL,
   cslRightPedal  = CSCS_SENSOR_LOCATION_RIGHT_PEDAL,
   cslFrontHub    = CSCS_SENSOR_LOCATION_FRONT_HUB,
   cslRearDropout = CSCS_SENSOR_LOCATION_REAR_DROPOUT,
   cslChainstay   = CSCS_SENSOR_LOCATION_CHAINSTAY,
   cslRearWheel   = CSCS_SENSOR_LOCATION_REAR_WHEEL,
   cslRearHub     = CSCS_SENSOR_LOCATION_REAR_HUB,
   cslChest       = CSCS_SENSOR_LOCATION_CHEST
} CSCM_Sensor_Location_t;

   /* The following bits are used in the sensor location bitmask when   */
   /* querying the list of supported locations.                         */
#define CSCM_SUPPORTED_SENSOR_LOCATION_OTHER                   0x00000001
#define CSCM_SUPPORTED_SENSOR_LOCATION_TOP_OF_SHOE             0x00000002
#define CSCM_SUPPORTED_SENSOR_LOCATION_IN_SHOE                 0x00000004
#define CSCM_SUPPORTED_SENSOR_LOCATION_HIP                     0x00000008
#define CSCM_SUPPORTED_SENSOR_LOCATION_FRONT_WHEEL             0x00000010
#define CSCM_SUPPORTED_SENSOR_LOCATION_LEFT_CRANK              0x00000020
#define CSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_CRANK             0x00000040
#define CSCM_SUPPORTED_SENSOR_LOCATION_LEFT_PEDAL              0x00000080
#define CSCM_SUPPORTED_SENSOR_LOCATION_RIGHT_PEDAL             0x00000100
#define CSCM_SUPPORTED_SENSOR_LOCATION_FRONT_HUB               0x00000200
#define CSCM_SUPPORTED_SENSOR_LOCATION_REAR_DROPOUT            0x00000400
#define CSCM_SUPPORTED_SENSOR_LOCATION_CHAINSTAY               0x00000800
#define CSCM_SUPPORTED_SENSOR_LOCATION_REAR_WHEEL              0x00001000
#define CSCM_SUPPORTED_SENSOR_LOCATION_REAR_HUB                0x00002000
#define CSCM_SUPPORTED_SENSOR_LOCATION_CHEST                   0x00004000

   /* The following structure contains data about a connected device    */
   /* returned in Query Connected Devices.                              */
   /* * NOTE * The SupportedFeatures member is only valid if the        */
   /*          Configured member is TRUE.                               */
   /* * NOTE * The SupportedSensorLocations member is only valid if     */
   /*          the Configured member is TRUE and the SupportedFeatures  */
   /*          member indicates multiple sensor locations are supported.*/
typedef struct _tagCSCM_Connected_Sensor_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned long SupportedOptionalCharateristics;
   Boolean_t     Configured;
   unsigned long SupportedFeatures;
   DWord_t       SupportedSensorLocations;
} CSCM_Connected_Sensor_t;

#define CSCM_CONNECTED_SENSOR_SIZE                             (sizeof(CSCM_Connected_Sensor_t))

   /* The following bit fields are used as flags for CSC Measurement    */
   /* data.                                                             */
#define CSCM_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT   CSCS_CSC_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT
#define CSCM_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT   CSCS_CSC_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT

   /* The following structure defines the format of the optional Wheel  */
   /* Revolution Data field of the CSC Mesaurement characteristic.  The */
   /* Cumulative Wheel Revolutions field represents the number of times */
   /* wheel was rotated. The Last Wheel Event Time is free-running-count*/
   /* of 1/1024 second units and it represents the time when the last   */
   /* wheel revolution was detected by the wheel rotation sensor.       */
typedef struct _tagCSCM_Wheel_Revolution_Data_t
{
   DWord_t CumulativeWheelRevolutions;
   Word_t  LastWheelEventTime;
} CSCM_Wheel_Revolution_Data_t;

#define CSCM_WHEEL_REVOLUTIION_DATA_SIZE                       (sizeof(CSCM_Wheel_Revolution_Data_t))

   /* The following structure defines the format of the optional Crank  */
   /* Revolution Data field of the CSC Mesaurement characteristic.  The */
   /* Cumulative Crank Revolutions field represents the number of times */
   /* crank was rotated. The Last Crank Event Time is free-running-count*/
   /* of 1/1024 second units and it represents the time when the last   */
   /* crank revolution was detected by the crank rotation sensor.       */
typedef struct _tagCSCM_Crank_Revolution_Data_t
{
   Word_t CumulativeCrankRevolutions;
   Word_t LastCrankEventTime;
} CSCM_Crank_Revolution_Data_t;

#define CSCM_CRANK_REVOLUTIION_DATA_SIZE                       (sizeof(CSCM_Crank_Revolution_Data_t))

#endif
