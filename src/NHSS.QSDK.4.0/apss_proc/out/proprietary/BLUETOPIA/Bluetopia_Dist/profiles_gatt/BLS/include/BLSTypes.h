/*****< blstypes.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLSTypes - Stonestreet One Bluetooth Stack Blood Presure Service Types    */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/11/12  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __BLSTYPESH__
#define __BLSTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following MACRO is a utility MACRO that assigns the BLS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable. This     */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the BLS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BLS_ASSIGN_BLS_SERVICE_UUID_16(_x)               ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x10)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined BLS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* BLS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the BLS Service UUID.                               */
#define BLS_COMPARE_BLS_SERVICE_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x10)

   /* The following defines the BLS Service UUID that is used when      */
   /* building the BLS Service Table.                                   */
#define BLS_SERVICE_BLUETOOTH_UUID_CONSTANT              { 0x10, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the Blood     */
   /* Pressure Measurement Characteristic 16 bit UUID to the specified  */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the Blood Pressure          */
   /* Measurement UUID Constant value.                                  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BLS_ASSIGN_BLOOD_PRESSURE_MEASUREMENT_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x35)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Blood Pressure Measurement UUID in UUID16  */
   /* form. This MACRO only returns whether the UUID_16_t variable is   */
   /* equal to the Blood Pressure Measurement UUID (MACRO returns       */
   /* boolean result) NOT less than/greater than. The first parameter   */
   /* is the UUID_16_t variable to compare to the Blood Pressure        */
   /* Measurement UUID.                                                 */
#define BLS_COMPARE_BLOOD_PRESSURE_MEASUREMENT_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x35)

   /* The following defines the Blood Pressure Measurement              */
   /* Characteristic UUID that is used when building the BLS Service    */
   /* Table.                                                            */
#define BLS_BLOOD_PRESSURE_MEASUREMENT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x35, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the           */
   /* Intermediate Cuff Pressure Characteristic 16 bit UUID to the      */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the            */
   /* Intermediate Cuff Pressure UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BLS_ASSIGN_INTERMEDIATE_CUFF_PRESSURE_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x36)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Intermediate Cuff Pressure UUID in UUID16  */
   /* form. This MACRO only returns whether the UUID_16_t variable is   */
   /* equal to the Intermediate Cuff Pressure UUID (MACRO returns       */
   /* boolean result) NOT less than/greater than. The first parameter   */
   /* is the UUID_16_t variable to compare to the Intermediate Cuff     */
   /* Pressure UUID.                                                    */
#define BLS_COMPARE_INTERMEDIATE_CUFF_PRESSURE_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x36)

   /* The following defines the Intermediate Cuff Pressure              */
   /* Characteristic UUID that is used when building the BLS Service    */
   /* Table.                                                            */
#define BLS_INTERMEDIATE_CUFF_PRESSURE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x36, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the Blood     */
   /* Pressure Feature Characteristic 16 bit UUID to the specified      */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is    */
   /* the UUID_16_t variable that is to receive the Blood Pressure      */
   /* Feature UUID Constant value.                                      */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define BLS_ASSIGN_BLOOD_PRESSURE_FEATURE_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x49)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined Blood Pressure Feature UUID in UUID16 form.*/
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the Blood Pressure Feature UUID (MACRO returns boolean result) NOT*/
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the Blood Pressure Feature UUID.           */
#define BLS_COMPARE_BLOOD_PRESSURE_FEATURE_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x49)

   /* The following defines the Blood Pressure Feature Characteristic   */
   /* UUID that is used when building the BLS Service Table.            */
#define BLS_BLOOD_PRESSURE_FEATURE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x49, 0x2A }

   /* The following defines the valid Blood Pressure Measurement Flags  */
   /* bit that may be set in the Flags field of a Blood Pressure        */
   /* Measurement characteristic.                                       */
#define BLS_MEASUREMENT_FLAGS_BLOOD_PRESSURE_UNITS_IN_KPA             0x01
#define BLS_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT                      0x02
#define BLS_MEASUREMENT_FLAGS_PULSE_RATE_PRESENT                      0x04
#define BLS_MEASUREMENT_FLAGS_USER_ID_PRESENT                         0x08
#define BLS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT              0x10

   /* The following defines the valid Blood Pressure Measurement  Status*/
   /* values that may be set in the Measurement Status  flag field of a */
   /* Blood Presure Measurement characteristic.                         */
#define BLS_MEASUREMENT_STATUS_BODY_MOVEMENT_DURING_MEASUREMENT      0x0001
#define BLS_MEASUREMENT_STATUS_CUFF_TOO_LOOSE                        0x0002
#define BLS_MEASUREMENT_STATUS_IRREGULAR_PULSE_DETECTED              0x0004
#define BLS_MEASUREMENT_STATUS_PULSE_RATE_EXCEEDS_UPPER_LIMIT        0x0008
#define BLS_MEASUREMENT_STATUS_PULSE_RATE_IS_LESS_THAN_LOWER_LIMIT   0x0010
#define BLS_MEASUREMENT_STATUS_IMPROPER_MEASUREMENT_POSITION         0x0020

   /* The following defines the valid values that may be used as the    */
   /* Flag value of a BLS Feature Characteristic                        */
#define BLS_FEATURE_BODY_MOVEMENT_DETECTION_FEATURE_SUPPORTED        0x0001
#define BLS_FEATURE_CUFF_FIT_DETECTION_FEATURE_SUPPORTED             0x0002
#define BLS_FEATURE_IRREGULAR_PULSE_DETECTION_FEATURE_SUPPORTED      0x0004
#define BLS_FEATURE_PULSE_RATE_RANGE_DETECTION_FEATURE_SUPPORTED     0x0008
#define BLS_FEATURE_MEASUREMENT_POSITION_DETECTION_FEATURE_SUPPORTED 0x0010
#define BLS_FEATURE_MULTIPLE_BONDS_SUPPORTED                         0x0020

   /* The following defines the structure of the Blood Pressure         */
   /* Measurement Compound Value included in the Blood Pressure         */
   /* Measurement characteristic.                                       */
   /* * NOTE * If a value for Systolic, Diastolic or MAP subfields is   */
   /*          unavailable , the special short float value NaN will be  */
   /*          filled in each of the unavailable subfields.             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBLS_Compound_Value_t
{
   NonAlignedWord_t Systolic;
   NonAlignedWord_t Diastolic;
   NonAlignedWord_t Mean_Arterial_Pressure;
}__PACKED_STRUCT_END__ BLS_Compound_Value_t;

#define BLS_COMPOUND_VALUE_SIZE                             (sizeof(BLS_Compound_Value_t))

   /* The following defines the structure of the Blood Pressure         */
   /* Measurement Data that is passed to the function that builds the   */
   /* Blood Pressure Measurement packet.                                */
   /* Variable_Data field represents none or more combination of below  */
   /* optional data fields                                              */
   /* GATT_Date_Time_Characteristic_t TimeStamp                         */
   /* NonAlignedWord_t PulseRate                                        */
   /* NonAlignedByte_t UserID                                           */
   /* NonAlignedWord_t Measurement_Status                               */
typedef __PACKED_STRUCT_BEGIN__ struct _tagBLS_Blood_Pressure_Measurement_t
{
   NonAlignedByte_t        Flags;
   BLS_Compound_Value_t    Compound_Value;
   NonAlignedByte_t        Variable_Data[1];
} __PACKED_STRUCT_END__ BLS_Blood_Pressure_Measurement_t;

#define BLS_BLOOD_PRESSURE_MEASUREMENT_SIZE(_x)             (BTPS_STRUCTURE_OFFSET(BLS_Blood_Pressure_Measurement_t, Variable_Data) + (_x))

#define BLS_BLOOD_PRESSURE_MEASUREMENT_LENGTH(_x)           (BLS_BLOOD_PRESSURE_MEASUREMENT_SIZE(0) + ( (( (_x) & BLS_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT)?GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE :0  ) + (( (_x) & BLS_MEASUREMENT_FLAGS_PULSE_RATE_PRESENT)?NON_ALIGNED_WORD_SIZE :0  ) + (( (_x) & BLS_MEASUREMENT_FLAGS_USER_ID_PRESENT)?NON_ALIGNED_BYTE_SIZE :0  ) + (( (_x) & BLS_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT)?NON_ALIGNED_WORD_SIZE :0  )) )

   /* The following defines the BLS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the BLS Service is      */
   /* registered.                                                       */
#define BLS_SERVICE_FLAGS                                   (GATT_SERVICE_FLAGS_LE_SERVICE)

#define BLS_SFLOAT_NOT_A_NUMBER            (0x07FF)
#define BLS_SFLOAT_NOT_AT_THIS_RESOLUTION  (0x0800)
#define BLS_SFLOAT_POSITIVE_INFINITY       (0x07FE)
#define BLS_SFLOAT_NEGATIVE_INFINITY       (0x0802)

#endif
