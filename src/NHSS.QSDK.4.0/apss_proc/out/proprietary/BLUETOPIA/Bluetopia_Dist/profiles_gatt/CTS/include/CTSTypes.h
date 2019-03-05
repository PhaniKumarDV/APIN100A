/*****< ctstype.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CTSType - Stonestreet One Bluetooth Stack Current Time Service Types.     */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/25/12  A. Parashar    Initial creation.                               */
/*   11/29/12  T. Cook        Fixed naming conventions and comments.          */
/******************************************************************************/
#ifndef __CTSTYPEH__
#define __CTSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.           */

   /* The following define the defined CTS Error Codes that may be sent */
   /* in a GATT Error Response.                                         */
#define CTS_ERROR_CODE_SUCCESS                           0x00
#define CTS_ERROR_CODE_DATA_FIELD_IGNORED                0x80

   /* The following MACRO is a utility MACRO that assigns the Current   */
   /* Time Service 16 bit UUID to the specified UUID_16_t variable.     */
   /* This MACRO accepts one parameter which is a UUID_16_t variable    */
   /* that is to receive the CTS UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CTS_ASSIGN_CTS_SERVICE_UUID_16(_x)               ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x05)

   /* The following MACRO is a utility MACRO that assigns the CTS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the CTS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define CTS_ASSIGN_CTS_SERVICE_SDP_UUID_16(_x)           ASSIGN_SDP_UUID_16((_x), 0x18, 0x05)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CTS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* CTS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CTS Service UUID.                               */
#define CTS_COMPARE_CTS_SERVICE_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x05)

   /* The following defines the Current Time Service UUID that is used  */
   /* when building the CTS Service Table.                              */
#define CTS_SERVICE_BLUETOOTH_UUID_CONSTANT              { 0x05, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the CTS       */
   /* Current Time Characteristic 16 bit UUID to the                    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the CTS        */
   /* Current Time UUID Constant value.                                 */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CTS_ASSIGN_CURRENT_TIME_UUID_16(_x)                        ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x2B)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CTS Current Time UUID in UUID16 form.  This*/
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* Current Time UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the CTS Current Time UUID                           */
#define CTS_COMPARE_CTS_CURRENT_TIME_UUID_TO_UUID_16(_x)           COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x2B)

   /* The following defines the CTS Current Time                        */
   /* Characteristic UUID that is used when building the CTS Service    */
   /* Table.                                                            */
#define CTS_CURRENT_TIME_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT    { 0x2B, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CTS       */
   /* Local Time information Characteristic 16 bit UUID to the          */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the CTS        */
   /* Local Time information UUID Constant value.                       */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CTS_ASSIGN_LOCAL_TIME_INFORMATION_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x0F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CTS Local Time Information UUID in UUID16  */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Local Time Information UUID (MACRO returns boolean   */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the CTS Local Time  UUID         */
#define CTS_COMPARE_CTS_LOCAL_TIME_INFORMATION_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x0F)

  /* The following defines the CTS Local Time information               */
  /* Characteristic UUID that is used when building the CTS Service     */
  /* Table.                                                             */
#define CTS_LOCAL_TIME_INFORMATION_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x0F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the CTS       */
   /* Reference Time information Characteristic 16 bit UUID to the      */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the CTS        */
   /* Reference Time information UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define CTS_ASSIGN_REFERENCE_TIME_INFORMATION_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x14)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined CTS Reference Time Information UUID in     */
   /* UUID16 form.This MACRO only returns whether the UUID_16_t variable*/
   /* is equal to the Reference Time Information UUID (MACRO returns    */
   /* boolean result) NOT less than/greater than.The first parameter is */
   /* the UUID_16_t variable to compare to the CTS Reference Time  UUID */
#define CTS_COMPARE_CTS_REFERENCE_TIME_INFORMATION_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x14)

    /* The following defines the CTS Local Time information             */
    /* Characteristic UUID that is used when building the CTS Service   */
    /* Table.                                                           */
#define CTS_REFERENCE_TIME_INFORMATION_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x14, 0x2A }

   /* The following defines the valid adjust reason of the current time.*/
#define CTS_CURRENT_TIME_ADJUST_REASON_MANUAL_TIME_UPDATE             0x01
#define CTS_CURRENT_TIME_ADJUST_REASON_EXTERNAL_REFERENCE_TIME_UPDATE 0x02
#define CTS_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_TIMEZONE             0x04
#define CTS_CURRENT_TIME_ADJUST_REASON_CHANGE_OF_DST                  0x08

   /* The following defines the valid values that may be used as the Day*/
   /* of the Week value of a Current Time characteristic.               */
#define CTS_DAY_OF_WEEK_UNKNOWN                          0x00
#define CTS_DAY_OF_WEEK_MONDAY                           0x01
#define CTS_DAY_OF_WEEK_TUESDAY                          0x02
#define CTS_DAY_OF_WEEK_WEDNESDAY                        0x03
#define CTS_DAY_OF_WEEK_THURSDAY                         0x04
#define CTS_DAY_OF_WEEK_FRIDAY                           0x05
#define CTS_DAY_OF_WEEK_SATURDAY                         0x06
#define CTS_DAY_OF_WEEK_SUNDAY                           0x07

   /* The following defines the valid values that may be used as the    */
   /* Month Of Year value of a Current TIme characteristic.             */
#define CTS_MONTH_OF_YEAR_UNKNOWN                        0x00
#define CTS_MONTH_OF_YEAR_JANUARY                        0x01
#define CTS_MONTH_OF_YEAR_FEBRUARY                       0x02
#define CTS_MONTH_OF_YEAR_MARCH                          0x03
#define CTS_MONTH_OF_YEAR_APRIL                          0x04
#define CTS_MONTH_OF_YEAR_MAY                            0x05
#define CTS_MONTH_OF_YEAR_JUNE                           0x06
#define CTS_MONTH_OF_YEAR_JULY                           0x07
#define CTS_MONTH_OF_YEAR_AUGUST                         0x08
#define CTS_MONTH_OF_YEAR_SEPTEMBER                      0x09
#define CTS_MONTH_OF_YEAR_OCTOBER                        0x0A
#define CTS_MONTH_OF_YEAR_NOVEMBER                       0x0B
#define CTS_MONTH_OF_YEAR_DECEMBER                       0x0C

   /* The following defines the valid values that may be used as the    */
   /* Time Zone value of a Local TIme Information characteristic.       */
#define CTS_TIME_ZONE_UTC_OFFSET_UNKNOWN                 -128
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_12_00             -48
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_11_00             -44
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_10_00             -40
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_9_30              -38
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_9_00              -36
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_8_00              -32
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_7_00              -28
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_6_00              -24
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_5_00              -20
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_4_30              -18
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_4_00              -16
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_3_30              -14
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_3_00              -12
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_2_00              -8
#define CTS_TIME_ZONE_UTC_OFFSET_MINUS_1_00              -4
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_0_00                0
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_1_00               +4
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_2_00               +8
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_3_00               +12
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_3_30               +14
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_4_00               +16
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_4_30               +18
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_00               +20
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_30               +22
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_5_45               +23
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_6_00               +24
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_6_30               +26
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_7_00               +28
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_8_00               +32
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_8_45               +35
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_9_00               +36
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_9_30               +38
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_10_00              +40
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_10_30              +42
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_11_00              +44
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_11_30              +46
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_12_00              +48
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_12_45              +51
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_13_00              +52
#define CTS_TIME_ZONE_UTC_OFFSET_PLUS_14_00              +56

   /* The following defines the valid values that may be used as the    */
   /* DST OFFSET value of a Local TIme Information characteristic.      */
#define CTS_DST_OFFSET_UNKNOWN                           255
#define CTS_DST_OFFSET_STANDARD_TIME                     0
#define CTS_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME        2
#define CTS_DST_OFFSET_DAYLIGHT_TIME                     4
#define CTS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME              8

  /* The following defines the valid Time Source values that may be     */
  /* set in the Time Source field of Reference Time Information         */
  /* characteristic.                                                    */
#define CTS_TIME_SOURCE_UNKNOWN                          0x00
#define CTS_TIME_SOURCE_NETWORK_TIME_PROTOCOL            0x01
#define CTS_TIME_SOURCE_GPS                              0x02
#define CTS_TIME_SOURCE_RADIO_TIME_SIGNAL                0x03
#define CTS_TIME_SOURCE_MANUAL                           0x04
#define CTS_TIME_SOURCE_ATOMIC_CLOCK                     0x05
#define CTS_TIME_SOURCE_CELLULAR_NETWORK                 0x06

   /* The following define the special defined Accuracy values that may */
   /* be set in the Accuracy field of the Reference Time Information    */
   /* Characteristic to denote some special cases.                      */
#define CTS_ACCURACY_MINIMUM_VALUE                       0
#define CTS_ACCURACY_MAXIMUM_VALUE                       253
#define CTS_ACCURACY_OUT_OF_RANGE                        254
#define CTS_ACCURACY_UNKOWN                              255

  /* The followng defines the format of a _tagCTS_Day_Date_Time_t       */
  /* The first member specifies information on Date_Time.               */
  /* The second member specifies Day_Of_Week     .                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCTS_Day_Date_Time_t
{
   GATT_Date_Time_Characteristic_t Date_Time;
   NonAlignedByte_t                Day_Of_Week;
}__PACKED_STRUCT_END__ CTS_Day_Date_Time_t;

#define CTS_DAY_DATE_TIME_SIZE                           (sizeof(CTS_Day_Date_Time_t))

  /* The followng defines the format of a _tagCTS_Exact_Time_t          */
  /* The first member specifies information on Day_Date_Timee.          */
  /* The second member specifies Fractions256   .                       */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCTS_Exact_Time_t
{
   CTS_Day_Date_Time_t Day_Date_Time;
   NonAlignedByte_t    Fractions256;
}__PACKED_STRUCT_END__ CTS_Exact_Time_t;

#define CTS_EXACT_TIME_SIZE                              (sizeof(CTS_Exact_Time_t))

   /* The followng defines the format of a Current Time.                */
   /* The first member specifies information on exact Time.             */
   /* The second member specifies Adjust Reason.                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCTS_Current_Time_t
{
   CTS_Exact_Time_t Exact_Time;
   NonAlignedByte_t Adjust_Reason_Mask;
} __PACKED_STRUCT_END__ CTS_Current_Time_t;

#define CTS_CURRENT_TIME_SIZE                            (sizeof(CTS_Current_Time_t))

   /* The followng defines the format of a Local Time information.      */
   /* The first member specifies information on Time Zone.              */
   /* The second member specifies Daylight Saving Time.                 */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCTS_Local_Time_Information_t
{
   NonAlignedSByte_t Time_Zone;
   NonAlignedByte_t  Daylight_Saving_Time;
} __PACKED_STRUCT_END__ CTS_Local_Time_Information_t;

#define CTS_LOCAL_TIME_INFORMATION_SIZE                  (sizeof(CTS_Local_Time_Information_t))

   /* The followng defines the format of a reference Time Information.  */
   /* The first member specifies information on Source   .              */
   /* The second member specifies Accuracy.                             */
   /* The Third  member specifies Days_Since_Update.                    */
   /* The second member specifies Hours_Since_Update.                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagCTS_Reference_Time_Information_t
{
   NonAlignedByte_t Source;
   NonAlignedByte_t Accuracy;
   NonAlignedByte_t Days_Since_Update;
   NonAlignedByte_t Hours_Since_Update;
} __PACKED_STRUCT_END__ CTS_Reference_Time_Information_t;

#define CTS_REFERENCE_TIME_INFORMATION_SIZE              (sizeof(CTS_Reference_Time_Information_t))

  /* The following defines the CTS GATT Service Flags MASK that should  */
  /* be passed into GATT_Register_Service when the CTS Service is       */
  /* registered.                                                        */
#define CTS_SERVICE_FLAGS_LE                             (GATT_SERVICE_FLAGS_LE_SERVICE)
#define CTS_SERVICE_FLAGS_BR_EDR                         (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define CTS_SERVICE_FLAGS_DUAL_MODE                      (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
