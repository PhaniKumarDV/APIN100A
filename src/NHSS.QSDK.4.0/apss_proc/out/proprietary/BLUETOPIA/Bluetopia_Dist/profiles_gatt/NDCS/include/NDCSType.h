/*****< ndcstype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  NDCSType - Stonestreet One Bluetooth Stack Next DST Change Service Type   */
/*             Definitions/Constants.                                         */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/25/12  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __NDCSTYPESH__
#define __NDCSTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.           */

   /* The following MACRO is a utility MACRO that assigns the Current   */
   /* Time Service 16 bit UUID to the specified UUID_16_t variable.     */
   /* This MACRO accepts one parameter which is a pointer to a UUID_16_t*/
   /* variable that is to receive the NDCS UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define NDCS_ASSIGN_NDCS_SERVICE_UUID_16(_x)                ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x07)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined NDCS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* NDCS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the NDCS Service UUID.                              */
#define NDCS_COMPARE_NDCS_SERVICE_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x07)

   /* The following defines the Next DST Change Service UUID that is    */
   /* used when building the NDCS Service Table.                        */
#define NDCS_SERVICE_BLUETOOTH_UUID_CONSTANT                { 0x07, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the NDCS Time */
   /* with DST Characteristic 16 bit UUID to the specified UUID_16_t    */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the NDCS Time with DST UUID Constant  */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define NDCS_ASSIGN_TIME_WITH_DST_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x11)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined NDCS Time with DST UUID in UUID16 form.    */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the Time With DST UUID (MACRO returns boolean result) NOT less    */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the NDCS Time With DST UUID                         */
#define NDCS_COMPARE_NDCS_TIME_WITH_DST_UUID_TO_UUID_16(_x)        COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x11)

   /* The following defines the NDCS Time with DST Characteristic UUID  */
   /* that is used when building the NDCS Service Table.                */
#define NDCS_TIME_WITH_DST_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x11, 0x2A }

   /* The following define the valid Daylight Savings offsets that may  */
   /* be used.                                                          */
#define NDCS_DST_OFFSET_STANDARD_TIME                       0x00
#define NDCS_DST_OFFSET_HALF_HOUR_DAYLIGHT_TIME             0x02
#define NDCS_DST_OFFSET_DAYLIGHT_TIME                       0x04
#define NDCS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME                0x08
#define NDCS_DST_OFFSET_NOT_KNOWN                           0xFF

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Date Time is valid.  The only parameter to this function*/
   /* is the NDCS_Date_Time_t structure to valid.  This MACRO returns   */
   /* TRUE if the Date Time is valid or FALSE otherwise.                */
#define NDCS_DATE_TIME_VALID(_x)                            ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following structure defines the format of                     */
   /* _tagNDCS_Time_With_Dst_t This is used to represent Time with Dst  */
   /* The first member specifies date time The Second member specifies  */
   /* dst_offset                                                        */
typedef __PACKED_STRUCT_BEGIN__ struct _tagNDCS_Time_With_Dst_t
{
   GATT_Date_Time_Characteristic_t Date_Time;
   NonAlignedByte_t                Dst_Offset;
}__PACKED_STRUCT_END__  NDCS_Time_With_Dst_t;

#define NDCS_TIME_WITH_DST_SIZE                             (sizeof(NDCS_Time_With_Dst_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified DST Offset is valid.  The only parameter to this        */
   /* function is the DSTOFFSET structure to valid.  This MACRO returns */
   /* TRUE if the DST Offset is valid or FALSE otherwise.               */
#define NDCS_DST_OFFSET_VALID(_x)                           ((((Byte_t)(_x)) >= NDCS_DST_OFFSET_STANDARD_TIME) && (((Byte_t)(_x)) <= NDCS_DST_OFFSET_DOUBLE_DAYLIGHT_TIME))
#define NDCS_DST_OFFSET_DATA_SIZE                           (NON_ALIGNED_BYTE_SIZE)

   /* The following defines the CTS GATT Service Flags MASK that should */
   /* be passed into GATT_Register_Service when the CTS Service is      */
   /* registered.                                                       */
#define NDCS_SERVICE_FLAGS                                  (GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
