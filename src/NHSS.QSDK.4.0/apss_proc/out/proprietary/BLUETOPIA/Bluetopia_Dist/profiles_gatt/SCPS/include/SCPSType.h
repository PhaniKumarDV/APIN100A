/*****< scpstypes.h >**********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SCPSType - Stonestreet One Bluetooth Stack Scan Parameters Service        */
/*             Types.                                                         */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/09/12  A. Parashar    Initial creation.                               */
/******************************************************************************/
#ifndef __SCPSTYPEH__
#define __SCPSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following MACRO is a utility MACRO that assigns the Scan      */
   /* Parameter Service 16 bit UUID to the specified UUID_16_t          */
   /* variable.  This MACRO accepts one parameter which is a pointer to */
   /* a UUID_16_t variable that is to receive the SCPS UUID Constant    */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define SCPS_ASSIGN_SCPS_SERVICE_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x13)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* SCPS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the SCPS Service UUID.                              */
#define SCPS_COMPARE_SCPS_SERVICE_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x13)

   /* The following defines the Scan Parameter Service UUID that is     */
   /* used when building the SCPS Service Table.                        */
#define SCPS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x13, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the SCPS      */
   /* Scan Interval Window Characteristic 16 bit UUID to the            */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the SCPS       */
   /* Scan Parameter UUID Constant value.                               */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define SCPS_ASSIGN_SCAN_INTERVAL_WINDOW_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x4F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Scan Interval Window UUID in UUID16   */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Scan Interval Window UUID (MACRO returns boolean     */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the SCPS Scan Inerval Window UUID*/
#define SCPS_COMPARE_SCAN_INTERVAL_WINDOW_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x4F)

   /* The following defines the SCPS Scan Interval Window               */
   /* Characteristic UUID that is used when building the SCPS Service   */
   /* Table.                                                            */
#define SCPS_SCAN_INTERVAL_WINDOW_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x4F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the SCPS      */
   /* Scan_Refresh information Characteristic 16 bit UUID to the        */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the SCPS       */
   /* Scan Refresh information UUID Constant value.                     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define SCPS_ASSIGN_SCAN_REFRESH_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x31)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined SCPS Scan Refresh Information UUID in      */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the Scan Refresh Information UUID            */
   /* (MACRO returns boolean result) NOT less than/greater than.        */
   /* The first parameter is the UUID_16_t variable to compare to       */
   /* the SCPS Scan Refresh  UUID                                       */
#define SCPS_COMPARE_SCAN_REFRESH_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x31)

  /* The following defines the SCPS Scan Refresh information            */
  /* Characteristic UUID that is used when building the SCPS Service    */
  /* Table.                                                             */
#define SCPS_SCAN_REFRESH_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0x31, 0x2A }

  /* The following macro defines the valid Scan Refresh value that may  */
  /* be set as a value for the Scan Refresh Value field of Scan         */
  /*  Parameter Characteristic.                                         */
#define SCPS_SCAN_REFRESH_VALUE_SERVER_REQUIRES_REFRESH  0x00

  /* The followng defines the format of _tagSCPS_Scan_Interval_Window_t */
  /* The first member specifies information on LE_Scan_Interval.        */
  /* The second member specifies LE_Scan_Window    .                    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagSCPS_Scan_Interval_Window_t
{
   NonAlignedWord_t  LE_Scan_Interval;
   NonAlignedWord_t  LE_Scan_Window;
}__PACKED_STRUCT_END__ SCPS_Scan_Interval_Window_t;

#define SCPS_SCAN_INTERVAL_WINDOW_SIZE    (sizeof(SCPS_Scan_Interval_Window_t))

  /* The following defines the SCPS GATT Service Flags MASK that should */
  /* be passed into GATT_Register_Service when the SCPS Service is      */
  /* registered.                                                        */
#define SCPS_SERVICE_FLAGS                (GATT_SERVICE_FLAGS_LE_SERVICE)

#endif
