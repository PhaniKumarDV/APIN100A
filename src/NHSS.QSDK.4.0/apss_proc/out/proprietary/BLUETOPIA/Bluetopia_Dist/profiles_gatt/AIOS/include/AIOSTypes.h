/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< aiostypes.h >**********************************************************/
/*      Copyright 2015 - 2016 Qualcomm Technologies, Inc.                     */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AIOSTypes - Qualcomm Technologies Bluetooth Automation IO Service Types.  */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/14/15  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __AIOSTYPEH__
#define __AIOSTYPEH__

#include "SS1BTGAT.h"     /* Bluetooth Stack GATT API Prototypes/Constants.   */
#include "BTPSKRNL.h"     /* BTPS Kernel Prototypes/Constants.                */

   /* The following define the defined AIOS Error Codes that may be sent*/
   /* in a GATT Error Response.                                         */
#define AIOS_ERROR_CODE_SUCCESS                                         0x00
#define AIOS_ERROR_CODE_TRIGGER_CONDITION_VALUE_NOT_SUPPORTED           0x80

   /* The following MACRO is a utility MACRO that assigns the Automation*/
   /* IO Service 16 bit UUID to the specified UUID_16_t variable.  This */
   /* MACRO accepts one parameter which is a pointer to a UUID_16_t     */
   /* variable that is to receive the AIOS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_AIOS_SERVICE_UUID_16(_x)                            ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x15)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* AIOS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the AIOS Service UUID.                              */
#define AIOS_COMPARE_AIOS_SERVICE_UUID_TO_UUID_16(_x)                   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x15)

   /* The following defines the AIOS UUID that is used when building the*/
   /* AIOS Service Table.                                               */
#define AIOS_SERVICE_BLUETOOTH_UUID_CONSTANT                            { 0x15, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the AIOS      */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the AIOS UUID Constant value.                          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define AIOS_ASSIGN_AIOS_SERVICE_SDP_UUID_16(_x)                        ASSIGN_SDP_UUID_16((_x), 0x18, 0x15)

   /* The following MACRO is a utility MACRO that assigns the AIOS      */
   /* Digital Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the AIOS Digital Characteristic UUID  */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_DIGITAL_CHARACTERISTIC_UUID_16(_x)                  ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x56)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Digital Characteristic UUID in UUID16 */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Digital Characteristic UUID (MACRO returns boolean   */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the AIOS Digital Characteristic  */
   /* UUID.                                                             */
#define AIOS_COMPARE_DIGITAL_CHARACTERISTIC_UUID_TO_UUID_16(_x)         COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x56)

   /* The following defines the AIOS Digital Characteristic             */
   /* Characteristic UUID that is used when building the AIOS Service   */
   /* Table.                                                            */
#define AIOS_DIGITAL_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT             { 0x56, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the AIOS      */
   /* Analog Characteristic 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the AIOS Analog Characteristic UUID   */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_ANALOG_CHARACTERISTIC_UUID_16(_x)                   ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x58)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Analog Characteristic UUID in UUID16  */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Analog Characteristic UUID (MACRO returns boolean    */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the AIOS Analog Characteristic   */
   /* UUID.                                                             */
#define AIOS_COMPARE_ANALOG_CHARACTERISTIC_UUID_TO_UUID_16(_x)          COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x58)

   /* The following defines the AIOS Analog Characteristic              */
   /* Characteristic UUID that is used when building the AIOS Service   */
   /* Table.                                                            */
#define AIOS_ANALOG_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT              { 0x58, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the AIOS      */
   /* Aggregate Characteristic 16 bit UUID to the specified UUID_16_t   */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the AIOS Aggregate Characteristic UUID*/
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_AGGREGATE_CHARACTERISTIC_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x5A)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Aggregate Characteristic UUID in      */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the Aggregate Characteristic UUID (MACRO     */
   /* returns boolean result) NOT less than/greater than.  The first    */
   /* parameter is the UUID_16_t variable to compare to the AIOS        */
   /* Aggregate Characteristic UUID.                                    */
#define AIOS_COMPARE_AGGREGATE_CHARACTERISTIC_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x5A)

   /* The following defines the AIOS Aggregate Characteristic UUID that */
   /* is used when building the AIOS Service Table.                     */
#define AIOS_AGGREGATE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT           { 0x5A, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the AIOS Value*/
   /* Trigger Setting Characteristic Descriptor 16 bit UUID to the      */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the AIOS Value */
   /* Trigger Setting Characteristic Descriptor UUID Constant value.    */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_VALUE_TRIGGER_SETTING_CD_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x29, 0x0A)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Value Trigger Setting Characteristic  */
   /* Descriptor UUID in UUID16 form.  This MACRO only returns whether  */
   /* the UUID_16_t variable is equal to the Value Trigger Setting      */
   /* Descriptor UUID (MACRO returns boolean result) NOT less           */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the AIOS Value Trigger Setting Characteristic       */
   /* Descriptor UUID.                                                  */
#define AIOS_COMPARE_VALUE_TRIGGER_SETTING_CD_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x29, 0x0A)

   /* The following defines the AIOS Value Trigger Setting              */
   /* Characteristic Descriptor UUID that is used when building the AIOS*/
   /* Service Table.                                                    */
#define AIOS_VALUE_TRIGGER_SETTING_CD_BLUETOOTH_UUID_CONSTANT           { 0x0A, 0x29 }

   /* The following MACRO is a utility MACRO that assigns the AIOS Time */
   /* Trigger Setting Characteristic Descriptor 16 bit UUID to the      */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the AIOS Time  */
   /* Trigger Setting Characteristic Descriptor UUID Constant time.     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_TIME_TRIGGER_SETTING_CD_UUID_16(_x)                 ASSIGN_BLUETOOTH_UUID_16((_x), 0x29, 0x0E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Time Trigger Setting Characteristic   */
   /* Descriptor UUID in UUID16 form.  This MACRO only returns whether  */
   /* the UUID_16_t variable is equal to the Time Trigger Setting       */
   /* Descriptor UUID (MACRO returns boolean result) NOT less           */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the AIOS Time Trigger Setting Characteristic        */
   /* Descriptor UUID.                                                  */
#define AIOS_COMPARE_TIME_TRIGGER_SETTING_CD_UUID_TO_UUID_16(_x)        COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x29, 0x0E)

   /* The following defines the AIOS Valid Range Characteristic         */
   /* Descriptor UUID that is used when building the AIOS Service Table.*/
#define AIOS_TIME_TRIGGER_SETTING_CD_BLUETOOTH_UUID_CONSTANT            { 0x0E, 0x29 }

   /* The following MACRO is a utility MACRO that assigns the AIOS      */
   /* Number Of Digitals Characteristic Descriptor 16 bit UUID to the   */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the AIOS Number*/
   /* Of Digitals Characteristic Descriptor UUID Constant time.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_NUMBER_OF_DIGITALS_CD_UUID_16(_x)                   ASSIGN_BLUETOOTH_UUID_16((_x), 0x29, 0x09)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Number Of Digitals Characteristic     */
   /* Descriptor UUID in UUID16 form.  This MACRO only returns whether  */
   /* the UUID_16_t variable is equal to the Number Of Digitals         */
   /* Descriptor UUID (MACRO returns boolean result) NOT less           */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the AIOS Number Of Digitals Characteristic          */
   /* Descriptor UUID.                                                  */
#define AIOS_COMPARE_NUMBER_OF_DIGITALS_CD_UUID_TO_UUID_16(_x)          COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x29, 0x09)

   /* The following defines the AIOS Number Of Digitals Characteristic  */
   /* Descriptor UUID that is used when building the AIOS Service Table.*/
#define AIOS_NUMBER_OF_DIGITALS_CD_BLUETOOTH_UUID_CONSTANT              { 0x09, 0x29 }

   /* The following MACRO is a utility MACRO that assigns the AIOS Valid*/
   /* Range Characteristic Descriptor 16 bit UUID to the specified      */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the AIOS Valid Range        */
   /* Characteristic Descriptor UUID Constant time.                     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define AIOS_ASSIGN_VALID_RANGE_CD_UUID_16(_x)                          ASSIGN_BLUETOOTH_UUID_16((_x), 0x29, 0x06)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined AIOS Valid Range Characteristic Descriptor */
   /* UUID in UUID16 form.  This MACRO only returns whether the         */
   /* UUID_16_t variable is equal to the Valid Range Descriptor UUID    */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the AIOS  */
   /* Valid Range Characteristic Descriptor UUID.                       */
#define AIOS_COMPARE_VALID_RANGE_CD_UUID_TO_UUID_16(_x)                 COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x29, 0x06)

   /* The following defines the AIOS Valid Range Characteristic         */
   /* Descriptor UUID that is used when building the AIOS Service Table.*/
#define AIOS_VALID_RANGE_CD_BLUETOOTH_UUID_CONSTANT                     { 0x06, 0x29 }

  /* The following defines the possible digital states for a 2-bit      */
  /* digital signal.                                                    */
#define AIOS_DIGITAL_CHARACTERISTIC_STATE_INACTIVE                             0x00
#define AIOS_DIGITAL_CHARACTERISTIC_STATE_ACTIVE                               0x01
#define AIOS_DIGITAL_CHARACTERISTIC_STATE_TRI_STATE                            0x02
#define AIOS_DIGITAL_CHARACTERISTIC_STATE_UNKNOWN                              0x03

  /* The following defines the valid values for the Condition field of  */
  /* the Value Trigger Setting characteristic descriptor.               */
  /* * NOTE * AIOS_VALUE_TRIGGER_SETTING_CONDITION_FLAG_STATE_CHANGED is*/
  /*          the default value if the Value Trigger Descriptor is      */
  /*          present.                                                  */
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_STATE_CHANGED                     0x00
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_CROSSED_BOUNDARY_ANALOG_VALUE     0x01
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_ON_BOUNDARY_ANALOG_VALUE          0x02
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_STATE_CHANGED_ANALOG_VALUE        0x03
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_DIGITAL_STATE_CHANGED_BIT_MASK    0x04
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_CROSSED_BOUNDARY_ANALOG_INTERVAL  0x05
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_ON_BOUNDARY_ANALOG_INTERVAL       0x06
#define AIOS_VALUE_TRIGGER_SETTING_CONDITION_NO_VALUE_TRIGGER                  0x07

  /* The following defines the valid values for the Condition field of  */
  /* the Time Trigger Setting characteristic descriptor.                */
  /* * NOTE * AIOS_TIME_TRIGGER_SETTING_CONDITION_NO_TIME_BASED_TRIGGERI*/
  /*          is the default value if the Time Trigger Descriptor is    */
  /*          present.                                                  */
#define AIOS_TIME_TRIGGER_SETTING_CONDITION_NO_TIME_BASED_TRIGGERING_USED       0x00
#define AIOS_TIME_TRIGGER_SETTING_CONDITION_TIME_INTERVAL_IGNORE_VALUE_TRIGGER  0x01
#define AIOS_TIME_TRIGGER_SETTING_CONDITION_TIME_INTERVAL_CHECK_VALUE_TRIGGER   0x02
#define AIOS_TIME_TRIGGER_SETTING_CONDITION_COUNT_CHANGED_MORE_OFTEN_THAN       0x03

    /* The following defines valid values for the Format field of the   */
    /* AIOS_Presentation_Format_t (optional characteristic descriptor)  */
    /* for Digital characteristics.                                     */
    /* * NOTE * These values are in base 10.                            */
#define AIOS_DIGITAL_PRESENTATION_FORMAT_STRUCT                         (27)

  /* The following defines valid values for the Format field of the     */
  /* AIOS_Presentation_Format_t (optional characteristic descriptor) for*/
  /* Analog characteristics.                                            */
  /* * NOTE * These values are in base 10.                              */
#define AIOS_ANALOG_PRESENTATION_FORMAT_UINT8                           (4)
#define AIOS_ANALOG_PRESENTATION_FORMAT_UINT12                          (5)
#define AIOS_ANALOG_PRESENTATION_FORMAT_UINT16                          (6)
#define AIOS_ANALOG_PRESENTATION_FORMAT_SINT8                           (12)
#define AIOS_ANALOG_PRESENTATION_FORMAT_SINT12                          (13)
#define AIOS_ANALOG_PRESENTATION_FORMAT_SINT16                          (14)
#define AIOS_ANALOG_PRESENTATION_FORMAT_SFLOAT_16                       (22)

  /* Defines the BT Sig name space for the NameSpace field of the       */
  /* presentation format.                                               */
  /* * NOTE * These values are in base 10.                              */
#define AIOS_PRESENTATION_FORMAT_NAMESPACE_BT_SIG                       (1)

   /* The Characteristic Presentation Format descriptor defines the     */
   /* format of the Characteristic Value.                               */
   /* * NOTE * The Format field determines how a single value contained */
   /*          in the Characteristic Value is formatted.                */
   /* * NOTE * The Exponent field is used with interger data types to   */
   /*          determine how the Characteristic Value is further        */
   /*          formatted.  The actual value = Characteristic Value *    */
   /*          10^Exponent.                                             */
   /* * NOTE * Unit specifies Unit of this attribute UUID               */
   /* * NOTE * The Name Space field is used to indentify the            */
   /*          organization (BT SIG 0x01) that is responsible for       */
   /*          defining the enumerations for the description field.     */
   /* * NOTE * The Description is an enumerated value from the          */
   /*          organization identified by the Name Space field.         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagAIOS_Presentation_Format_t
{
  NonAlignedByte_t  Format;
  NonAlignedByte_t  Exponent;
  NonAlignedWord_t  Unit;
  NonAlignedByte_t  NameSpace;
  NonAlignedWord_t  Description;
} __PACKED_STRUCT_END__ AIOS_Presentation_Format_t;

#define AIOS_PRESENTATION_FORMAT_SIZE                                   (sizeof(AIOS_Presentation_Format_t))

  /* The following structure defines the Value Trigger Setting          */
  /* Characteristic Descriptor.                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagAIOS_Value_Trigger_t
{
  NonAlignedByte_t  Condition;
  NonAlignedByte_t  Variable_Data[1];
} __PACKED_STRUCT_END__ AIOS_Value_Trigger_t;

#define AIOS_VALUE_TRIGGER_SIZE(_x)                                     (BTPS_STRUCTURE_OFFSET(AIOS_Value_Trigger_t, Variable_Data) + (_x))

  /* The following structure defines the UINT24 type needed for the Time*/
  /* Interval field of the Time Trigger.                                */
typedef __PACKED_STRUCT_BEGIN__ struct _tagAIOS_UINT24_t
{
  NonAlignedWord_t  Lower;
  NonAlignedByte_t  Upper;
} __PACKED_STRUCT_END__ AIOS_UINT24_t;

#define AIOS_UINT24_SIZE                                                (sizeof(AIOS_UINT24_t))

  /* The following structure defines the Time Trigger Setting           */
  /* Characteristic Descriptor.                                         */
typedef __PACKED_STRUCT_BEGIN__ struct _tagAIOS_Time_Trigger_t
{
  NonAlignedByte_t  Condition;
  NonAlignedByte_t  Variable_Data[1];
} __PACKED_STRUCT_END__ AIOS_Time_Trigger_t;

#define AIOS_TIME_TRIGGER_SIZE(_x)                                      (BTPS_STRUCTURE_OFFSET(AIOS_Time_Trigger_t, Variable_Data) + (_x))

  /* The following structure defines the Valid Range Characteristic     */
  /* Descriptor.                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagAIOS_Valid_Range_t
{
  NonAlignedWord_t  Lower;
  NonAlignedWord_t  Upper;
} __PACKED_STRUCT_END__ AIOS_Valid_Range_t;

#define AIOS_VALID_RANGE_SIZE                                           (sizeof(AIOS_Valid_Range_t))

  /* The following defines the valid values (bit mask) that may be set  */
  /* for client characteristic configuration descriptors (CCCD).        */
  /* * NOTE * Both cannot be set simultaneously.                        */
#define AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE          (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define AIOS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE        (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

  /* The following defines the valid values (bit mask) that may be set  */
  /* for the extended properties.                                       */
#define AIOS_EXTENDED_PROPERTIES_WRITE_AUXILARIES                       (GATT_CHARACTERISTIC_EXTENDED_PROPERTIES_WRITABLE_AUXILARIES)

   /* The following defines the AIOS GATT Service Flags MASK that should*/
   /* be passed into GATT_Register_Service when the AIOS Service is     */
   /* registered.                                                       */
#define AIOS_SERVICE_FLAGS_LE                                           (GATT_SERVICE_FLAGS_LE_SERVICE)
#define AIOS_SERVICE_FLAGS_BR_EDR                                       (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define AIOS_SERVICE_FLAGS_DUAL_MODE                                    (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
