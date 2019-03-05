/*****< lnstypes.h >***********************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LNSTypes - Stonestreet One Bluetooth Location and Navigation Service      */
/*             Types.                                                         */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/26/13  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __LNSTYPESH__
#define __LNSTYPESH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following define the defined LNS Error Codes that may be      */
   /* sent in a GATT Error Response.                                    */
#define LNS_ERROR_CODE_SUCCESS                                              0x00
#define LNS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS                        0xFE
#define LNS_ERROR_CODE_CHARACTERISTIC_CONFIGURATION_IMPROPERLY_CONFIGURED   0xFD

   /* The following MACRO is a utility MACRO that assigns the LNS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the LNS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_LNS_SERVICE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x19)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LNS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* LNS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the LNS Service UUID.                               */
#define LNS_COMPARE_LNS_SERVICE_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x19)

   /* The following defines the LNS Service UUID that is used when      */
   /* building the LNS Service Table.                                   */
#define LNS_SERVICE_UUID_CONSTANT                              { 0x19, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the LNS       */
   /* Feature Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the LNS feature UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_LN_FEATURE_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x6A)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LNS LN Feature UUID in UUID16 form.        */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the LNS LN Feature UUID (MACRO returns boolean result) NOT        */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the LNS Measurement UUID.                  */
#define LNS_COMPARE_LN_FEATURE_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x6A)

   /* The following defines the LNS LN Feature Characteristic UUID      */
   /* that is used when building the LNS Service Table.                 */
#define LNS_LN_FEATURE_CHARACTERISTIC_UUID_CONSTANT            { 0x6A, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the LNS       */
   /* Location and Speed Characteristic 16 bit UUID to the specified    */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the LNS Feature UUID        */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_LOCATION_AND_SPEED_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x67)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LNS Feature UUID in UUID16 form.           */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the LNS Feature UUID (MACRO returns boolean result) NOT           */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the LNS Feature UUID.                      */
#define LNS_COMPARE_LOCATION_AND_SPEED_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x67)

   /* The following defines the LNS Location and Speed Characteristic   */
   /* UUID that is used when building the LNS Service Table.            */
#define LNS_LOCATION_AND_SPEED_CHARACTERISTIC_UUID_CONSTANT    { 0x67, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the Position  */
   /* Quality Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the LNS Position Quality UUID Constant*/
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_POSITION_QUALITY_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x69)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Position Quality UUID in UUID16 form.      */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the Sensor Location UUID (MACRO returns boolean result) NOT       */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the LNS Position Quality UUID.             */
#define LNS_COMPARE_POSITION_QUALITY_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x69)

   /* The following defines the Position Quality Characteristic UUID    */
   /* that is used when building the LNS Service Table.                 */
#define LNS_POSITION_QUALITY_CHARACTERISTIC_UUID_CONSTANT      { 0x69, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the LN        */
   /* Control point Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the LNS LN CONTROL POINT    */
   /* UUID Constant Value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_LN_CONTROL_POINT_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x6B)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LN Control Point UUID in UUID16 form.      */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the LN Control Point UUID (MACRO returns boolean result) NOT      */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the LN Control Point UUID.                 */
#define LNS_COMPARE_LN_CONTROL_POINT_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x6B)

   /* The following defines the LN Control Point Characteristic UUID    */
   /* that is used when building the LNS Service Table.                 */
#define LNS_LN_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT      { 0x6B, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the Navigation*/
   /* Characteristic 16 bit UUID to the specified UUID_16_t variable.   */
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the LNS NAVIGATION UUID Constant Value.        */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define LNS_ASSIGN_NAVIGATION_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x68)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Navigation UUID in UUID16 form.            */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the Navigation UUID (MACRO returns boolean result)                */
   /* NOT less than/greater than.  The first parameter is the UUID_16_t */
   /* variable to compare to the navigation UUID.                       */
#define LNS_COMPARE_NAVIGATION_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x68)

   /* The following defines the Navigation Characteristic UUID that is  */
   /* used when building the LNS Service Table.                         */
#define LNS_NAVIGATION_CHARACTERISTIC_UUID_CONSTANT            { 0x68, 0x2A }

   /* The following defines the valid values that may be uses as the    */
   /* Flag value of a LN Feature Characteristic.                        */
#define LNS_LN_FEATURE_FLAG_INSTANTANEOUS_SPEED_SUPPORTED                  0x00000001
#define LNS_LN_FEATURE_FLAG_TOTAL_DISTANCE_SUPPORTED                       0x00000002
#define LNS_LN_FEATURE_FLAG_LOCATION_SUPPORTED                             0x00000004
#define LNS_LN_FEATURE_FLAG_ELEVATION_SUPPORTED                            0x00000008
#define LNS_LN_FEATURE_FLAG_HEADING_SUPPORTED                              0x00000010
#define LNS_LN_FEATURE_FLAG_ROLLING_TIME_SUPPORTED                         0x00000020
#define LNS_LN_FEATURE_FLAG_UTC_TIME_SUPPORTED                             0x00000040
#define LNS_LN_FEATURE_FLAG_REMAINING_DISTANCE_SUPPORTED                   0x00000080
#define LNS_LN_FEATURE_FLAG_REMAINING_VERTICAL_DISTANCE_SUPPORTED          0x00000100
#define LNS_LN_FEATURE_FLAG_ESTIMATED_TIME_OF_ARRIVAL_SUPPORTED            0x00000200
#define LNS_LN_FEATURE_FLAG_NUMBER_OF_SATELLITES_IN_SOLUTIONS_SUPPORTED    0x00000400
#define LNS_LN_FEATURE_FLAG_NUMBER_OF_SATELLITES_IN_VIEW_SUPPORTED         0x00000800
#define LNS_LN_FEATURE_FLAG_TIME_TO_FIRST_FIX_SUPPORTED                    0x00001000
#define LNS_LN_FEATURE_FLAG_ESTIMATED_HORIZONTAL_POSITION_ERROR_SUPPORTED  0x00002000
#define LNS_LN_FEATURE_FLAG_ESTIMATED_VERTICAL_POSITION_ERROR_SUPPORTED    0x00004000
#define LNS_LN_FEATURE_FLAG_HORIZONTAL_DILUTION_OF_PERCISION_SUPPORTED     0x00008000
#define LNS_LN_FEATURE_FLAG_VERTICAL_DILUTION_OF_PERCISION_SUPPORTED       0x00010000
#define LNS_LN_FEATURE_FLAG_LOCATION_SPEED_CONTENT_MASKING_SUPPORTED       0x00020000
#define LNS_LN_FEATURE_FLAG_FIX_RATE_SETTING_SUPPORTED                     0x00040000
#define LNS_LN_FEATURE_FLAG_ELEVATION_SETTING_SUPPORTED                    0x00080000
#define LNS_LN_FEATURE_FLAG_POSITION_STATUS_SUPPORTED                      0x00100000

   /* The following defines the valid values that may be uses as the    */
   /* Flag value of a Location and Speed Characteristic.                */
#define LNS_LOCATION_AND_SPEED_FLAG_INSTANTANEOUS_SPEED_PRESENT                     0x0001
#define LNS_LOCATION_AND_SPEED_FLAG_TOTAL_DISTANCE_PRESENT                          0x0002
#define LNS_LOCATION_AND_SPEED_FLAG_LOCATION_PRESENT                                0x0004
#define LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_PRESENT                               0x0008
#define LNS_LOCATION_AND_SPEED_FLAG_HEADING_PRESENT                                 0x0010
#define LNS_LOCATION_AND_SPEED_FLAG_ROLLING_TIME_PRESENT                            0x0020
#define LNS_LOCATION_AND_SPEED_FLAG_UTC_TIME_PRESENT                                0x0040
#define LNS_LOCATION_AND_SPEED_FLAG_POSITION_STATUS_OK                              0x0080
#define LNS_LOCATION_AND_SPEED_FLAG_POSITION_STATUS_ESTIMATED                       0x0100
#define LNS_LOCATION_AND_SPEED_FLAG_POSITION_STATUS_LAST_KNOWN                      0x0180
#define LNS_LOCATION_AND_SPEED_FLAG_SPEED_DISTANCE_3D_FORMAT                        0x0200
#define LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_SOURCE_SATELLITE_POSITIONING_SYSTEM   0x0000
#define LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_SOURCE_BAROMETRIC_AIR_PRESSURE        0x0400
#define LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_SOURCE_DATABASE_SERVICE               0x0800
#define LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_SOURCE_OTHER                          0x0C00
#define LNS_LOCATION_AND_SPEED_FLAG_HEADING_SOURCE_BASED_ON_MAGNETIC_COMPASS        0x1000

   /* The following defines the valid values that may be used as the    */
   /* Flag value of a LN Control Point filter parameter.                */
   /* Location and Speed Content Mask.                                  */
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_INSTANTANEOUS_SPEED_TURN_OFF 0x0001
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_TOTAL_DISTANCE_TURN_OFF      0x0002
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_LOCATION_TURN_OFF            0x0004
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_ELEVATION_TURN_OFF           0x0008
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_HEADING_TURN_OFF             0x0010
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_ROLLING_TIME_TURN_OFF        0x0020
#define LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_UTC_TIME_TURN_OFF            0x0040

   /* The following defines the valid values that may be used as the    */
   /* Flag value of a Position Quality  Characteristic.                 */
#define LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT 0x0001
#define LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_VIEW_PRESENT     0x0002
#define LNS_POSITION_QUALITY_FLAG_TIME_TO_FIRST_FIX_PRESENT             0x0004
#define LNS_POSITION_QUALITY_FLAG_EHPE_PRESENT                          0x0008
#define LNS_POSITION_QUALITY_FLAG_EVPE_PRESENT                          0x0010
#define LNS_POSITION_QUALITY_FLAG_HDOP_PRESENT                          0x0020
#define LNS_POSITION_QUALITY_FLAG_VDOP_PRESENT                          0x0040
#define LNS_POSITION_QUALITY_FLAG_POSITION_STATUS_OK                    0x0080
#define LNS_POSITION_QUALITY_FLAG_POSITION_STATUS_ESTIMATED_            0x0100
#define LNS_POSITION_QUALITY_FLAG_POSITION_STATUS_LAST_KNOWN            0x0080

   /* The following defines the valid values that may be set as the     */
   /* value for the OpCode field of LN Control Point characteristic.    */
#define LNS_LN_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE                0x01
#define LNS_LN_CONTROL_POINT_MASK_LOCATION_SPEED_CHARACTERISTIC_CONTENT 0x02
#define LNS_LN_CONTROL_POINT_NAVIGATION_CONTROL                         0x03
#define LNS_LN_CONTROL_POINT_REQUEST_NUMBER_OF_ROUTES                   0x04
#define LNS_LN_CONTROL_POINT_REQUEST_NAME_OF_ROUTE                      0x05
#define LNS_LN_CONTROL_POINT_SELECT_ROUTE                               0x06
#define LNS_LN_CONTROL_POINT_SET_FIX_RATE                               0x07
#define LNS_LN_CONTROL_POINT_SET_ELEVATION                              0x08
#define LNS_LN_CONTROL_POINT_OPCODE_RESPONSE                            0x20

   /* The following defines the valid values that may be set as the     */
   /* value for the Response Code value field of LN Control Point       */
   /* characteristic.                                                   */
#define LNS_LN_CONTROL_POINT_RESPONSE_CODE_SUCCESS             0x01
#define LNS_LN_CONTROL_POINT_RESPONSE_OPCODE_NOT_SUPPORTED     0x02
#define LNS_LN_CONTROL_POINT_RESPONSE_INVALID_PARAMETER        0x03
#define LNS_LN_CONTROL_POINT_RESPONSE_OPERATION_FAILED         0x04

   /* The following defines the valid values that may be uses as the    */
   /* Flag value of a Navigation Characteristic.                        */
#define LNS_NAVIGATION_FLAG_REMAINING_DISTANCE_PRESENT               0x0001
#define LNS_NAVIGATION_FLAG_REMAINING_VERTICAL_DISTANCE_PRESENT      0x0002
#define LNS_NAVIGATION_FLAG_ESTIMATED_TIME_OF_ARRIVAL_PRESENT        0x0004
#define LNS_NAVIGATION_FLAG_POSITION_STATUS_OK                       0x0008
#define LNS_NAVIGATION_FLAG_POSITION_STATUS_ESTIMATED                0x0010
#define LNS_NAVIGATION_FLAG_POSITION_STATUS_LAST_KNOWN_              0x0018
#define LNS_NAVIGATION_FLAG_HEADING_SOURCE_BASED_ON_MAGNETIC_COMPASS 0x0020
#define LNS_NAVIGATION_INDICATOR_TYPE_TO_WAYPOINT                    0x0000
#define LNS_NAVIGATION_INDICATOR_TYPE_TO_DESTINATION                 0x0040
#define LNS_NAVIGATION_WAYPOINT_REACHED                              0x0080
#define LNS_NAVIGATION_DESTINATION_REACHED                           0x0100

   /* The following defines the valid values that may be use as the     */
   /* value of a LN Control Point filter parameter Navigation Control.  */
#define LNS_NAVIGATION_CONTROL_STOP_NAVIGATION                             0x0000
#define LNS_NAVIGATION_CONTROL_START_NAVIGATION_TO_FIRST_WAYPOINT_ON_ROUTE 0x0001
#define LNS_NAVIGATION_CONTROL_PAUSE_NAVIGATION                            0x0002
#define LNS_NAVIGATION_CONTROL_CONTINUE_NAVIGATION                         0x0003
#define LNS_NAVIGATION_CONTROL_SKIP_WAYPOINT                               0x0004
#define LNS_NAVIGATION_CONTROL_SELECT_NEAREST_WAYPOINT                     0x0005

   /* The following structure defines the format of the Location and    */
   /* Speed characteristic.  Flag variable value is a comnination of 1  */
   /* or more LNS_LOCATION_AND_SPEED_FLAG_CONTENT_MASK_XXX.             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagLNS_Location_Speed_t
{
   NonAlignedWord_t Flags;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ LNS_Location_Speed_t;

#define LNS_LOCATION_SPEED_SIZE(_x)                            (BTPS_STRUCTURE_OFFSET(LNS_Location_Speed_t, Variable_Data) + _x)

   /* The following MACRO is a utility MACRO that describe the minimum  */
   /* size of a Location and speed value.                               */
#define LNS_LOCATION_SPEED_LENGTH(_x)                          (LNS_LOCATION_SPEED_SIZE(0)                                          + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_INSTANTANEOUS_SPEED_PRESENT)                   ? NON_ALIGNED_WORD_SIZE                  :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_TOTAL_DISTANCE_PRESENT)                        ? NON_ALIGNED_BYTE_SIZE*3                :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_LOCATION_PRESENT)                              ? NON_ALIGNED_DWORD_SIZE*2               :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_ELEVATION_PRESENT)                             ? NON_ALIGNED_BYTE_SIZE*3                :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_HEADING_PRESENT)                               ? NON_ALIGNED_WORD_SIZE                  :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_ROLLING_TIME_PRESENT)                          ? NON_ALIGNED_BYTE_SIZE                  :0) + \
   (((_x) & LNS_LOCATION_AND_SPEED_FLAG_UTC_TIME_PRESENT)                              ? GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE:0))

   /* The following structure defines the format of the Position        */
   /* quality characteristic.  Flag variable value is a comnination of  */
   /* 1 or more LNS_POSITION_QUALITY_FLAG_XXX.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagLNS_Position_Quality_t
{
   NonAlignedWord_t Flags;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ LNS_Position_Quality_t;

#define LNS_POSITION_QUALITY_SIZE(_x)                          (BTPS_STRUCTURE_OFFSET(LNS_Position_Quality_t, Variable_Data) + _x)

   /* The following MACRO is a utility MACRO that describe the minimum  */
   /* size of a LNS_Position Quality value.                             */
#define LNS_POSITION_QUALITY_MINIMUM_LENGTH                    (LNS_POSITION_QUALITY_SIZE(0))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of a Position Quality data based of Flags  */
   /* value.  The only parameter to this MACRO is Position Quality      */
   /* Flags.                                                            */
#define LNS_POSITION_QUALITY_LENGTH(_x)                        (LNS_POSITION_QUALITY_SIZE(0) +             \
   (((_x) & LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT) ? NON_ALIGNED_BYTE_SIZE :0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_NUMBER_OF_BEACONS_IN_VIEW_PRESENT)     ? NON_ALIGNED_BYTE_SIZE :0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_TIME_TO_FIRST_FIX_PRESENT)             ? NON_ALIGNED_WORD_SIZE :0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_EHPE_PRESENT)                          ? NON_ALIGNED_DWORD_SIZE:0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_EVPE_PRESENT)                          ? NON_ALIGNED_DWORD_SIZE:0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_HDOP_PRESENT)                          ? NON_ALIGNED_BYTE_SIZE :0) + \
   (((_x) & LNS_POSITION_QUALITY_FLAG_VDOP_PRESENT)                          ? NON_ALIGNED_BYTE_SIZE :0))

   /* The following structure defines the format of the                 */
   /* LN_Control_Point structure.  This structure will be used for both */
   /* Control Point request and response purpose.                       */
typedef __PACKED_STRUCT_BEGIN__ struct _tagLNS_LN_Control_Point_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ LNS_LN_Control_Point_t;

#define LNS_LN_CONTROL_POINT_SIZE(_x)                          (BTPS_STRUCTURE_OFFSET(LNS_LN_Control_Point_t, Variable_Data) + _x)

   /* The following structure contains Navigation data that is passed to*/
   /* the function that builds the Navigation packet.  Flag variable    */
   /* value is a comnination of 1 or more LNS_NAVIGATION_FLAG_XXX.      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagLNS_Navigation_t
{
   NonAlignedWord_t Flags;
   NonAlignedWord_t Bearing;
   NonAlignedWord_t Heading;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ LNS_Navigation_t;

#define LNS_NAVIGATION_SIZE(_x)                                (BTPS_STRUCTURE_OFFSET(LNS_Navigation_t, Variable_Data) + _x)

   /* The following MACRO is a utility MACRO that describe the minimum  */
   /* size of a LNS_Mavigation value.                                   */
#define LNS_NAVIGATION_MINIMUM_LENGTH(_x)                      (LNS_NAVIGATION_SIZE(0) + (3 * NON_ALIGNED_WORD_SIZE))

#define LNS_ATT_MTU_SIZE                                       28

   /* The following defines the LNS GATT Service Flags MASK that        */
   /* should be passed into GATT_Register_Service when the LN Service   */
   /* is registered.                                                    */
#define LNS_SERVICE_FLAGS                                      (GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
