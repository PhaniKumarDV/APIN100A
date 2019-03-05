/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< udstypes.h >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  UDSTypes - Qualcomm Technologies Bluetooth Stack User Data Service        */
/*             Types.                                                         */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __UDSTYPEH__
#define __UDSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.          */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.     */

   /* The following defines the attribute protocol Application error    */
   /* codes.                                                            */
   /* * NOTE * HPS_ERROR_CODE_SUCCESS should be used for all response   */
   /*          API's to indicate a successful response.                 */
#define UDS_ERROR_CODE_SUCCESS                                 0x00
#define UDS_ERROR_CODE_USER_DATA_ACCESS_NOT_PERMITTED          0x80

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define UDS_ERROR_CODE_INSUFFICIENT_AUTHENTICATION            (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)
#define UDS_ERROR_CODE_INSUFFICIENT_AUTHORIZATION             (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION)
#define UDS_ERROR_CODE_INSUFFICIENT_ENCRYPTION                (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION)
#define UDS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED             (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define UDS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS          (ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)
#define UDS_ERROR_CODE_UNLIKELY_ERROR                         (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the UDS 16 bit*/
   /* UUID to the specified UUID_16_t variable.  This MACRO accepts one */
   /* parameter which is a pointer to a UUID_16_t variable that is to   */
   /* receive the UDS UUID Constant value.                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_UDS_SERVICE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x1C)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the UDS Service UUID.                               */
#define UDS_COMPARE_UDS_SERVICE_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x1C)

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the UDS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define UDS_ASSIGN_UDS_SERVICE_SDP_UUID_16(_x)                 ASSIGN_SDP_UUID_16((_x), 0x18, 0x1C)

   /* The following defines the UDS Parameter Service UUID that is      */
   /* used when building the UDS Service Table.                         */
#define UDS_SERVICE_BLUETOOTH_UUID_CONSTANT                    { 0x1C, 0x18 }

   /*********************************************************************/
   /* Start UDS Characteristics.                                        */
   /*********************************************************************/

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic First Name 16 bit UUID to the specified UUID_16_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic First Name UUID*/
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_FIRST_NAME_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8A)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS First Name UUID in UUID16 form.  This  */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS First Name UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the UDS First Name UUID.                            */
#define UDS_COMPARE_FIRST_NAME_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8A)

   /* The following defines the UDS Characteristic First Name UUID that */
   /* is used when building the UDS Service Table.                      */
#define UDS_CHARACTERISTIC_FIRST_NAME_UUID_CONSTANT            { 0x8A, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Last Name 16 bit UUID to the specified UUID_16_t   */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Last Name UUID */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_LAST_NAME_UUID_16(_x)                       ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x90)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Last Name UUID in UUID16 form.  This   */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS Last Name UUID (MACRO returns boolean result) NOT less        */
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Last Name UUID.              */
#define UDS_COMPARE_LAST_NAME_UUID_TO_UUID_16(_x)              COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x90)

   /* The following defines the UDS Characteristic Last Name UUID that  */
   /* is used when building the UDS Service Table.                      */
#define UDS_CHARACTERISTIC_LAST_NAME_UUID_CONSTANT             { 0x90, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Email Address 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Email*/
   /* Address UUID Constant value.                                      */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_EMAIL_ADDRESS_UUID_16(_x)                   ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x87)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Email Address UUID in UUID16 form.     */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Email Address UUID (MACRO returns boolean result) NOT less*/
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Email Address UUID.          */
#define UDS_COMPARE_EMAIL_ADDRESS_UUID_TO_UUID_16(_x)          COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x87)

   /* The following defines the UDS Characteristic Email Address UUID   */
   /* that is used when building the UDS Service Table.                 */
#define UDS_CHARACTERISTIC_EMAIL_ADDRESS_UUID_CONSTANT         { 0x87, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Age 16 bit UUID to the specified UUID_16_t         */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Age UUID       */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_AGE_UUID_16(_x)                             ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x80)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Age UUID in UUID16 form.  This MACRO   */
   /* only returns whether the UUID_16_t variable is equal to the UDS   */
   /* Age UUID (MACRO returns boolean result) NOT less than/greater     */
   /* than.  The last parameter is the UUID_16_t variable to compare to */
   /* the UDS Characteristic Age UUID.                                  */
#define UDS_COMPARE_AGE_UUID_TO_UUID_16(_x)                    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x80)

   /* The following defines the UDS Characteristic Age UUID that is used*/
   /* when building the UDS Service Table.                              */
#define UDS_CHARACTERISTIC_AGE_UUID_CONSTANT                   { 0x80, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Date of Birth 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Date */
   /* of Birth UUID Constant value.                                     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_DATE_OF_BIRTH_UUID_16(_x)                   ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x85)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Date of Birth UUID in UUID16 form.     */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Date of Birth UUID (MACRO returns boolean result) NOT less*/
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Date of Birth UUID.          */
#define UDS_COMPARE_DATE_OF_BIRTH_UUID_TO_UUID_16(_x)          COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x85)

   /* The following defines the UDS Characteristic Date of Birth UUID   */
   /* that is used when building the UDS Service Table.                 */
#define UDS_CHARACTERISTIC_DATE_OF_BIRTH_UUID_CONSTANT         { 0x85, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Gender 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Gender UUID    */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_GENDER_UUID_16(_x)                          ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8C)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Gender UUID in UUID16 form.  This MACRO*/
   /* only returns whether the UUID_16_t variable is equal to the UDS   */
   /* Gender UUID (MACRO returns boolean result) NOT less than/greater  */
   /* than.  The last parameter is the UUID_16_t variable to compare to */
   /* the UDS Characteristic Gender UUID.                               */
#define UDS_COMPARE_GENDER_UUID_TO_UUID_16(_x)                 COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8C)

   /* The following defines the UDS Characteristic Gender UUID that is  */
   /* used when building the UDS Service Table.                         */
#define UDS_CHARACTERISTIC_GENDER_UUID_CONSTANT                { 0x8C, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Weight 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Weight UUID    */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_WEIGHT_UUID_16(_x)                          ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x98)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Weight UUID in UUID16 form.  This MACRO*/
   /* only returns whether the UUID_16_t variable is equal to the UDS   */
   /* Weight UUID (MACRO returns boolean result) NOT less than/greater  */
   /* than.  The last parameter is the UUID_16_t variable to compare to */
   /* the UDS Characteristic Weight UUID.                               */
#define UDS_COMPARE_WEIGHT_UUID_TO_UUID_16(_x)                 COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x98)

   /* The following defines the UDS Characteristic Weight UUID that is  */
   /* used when building the UDS Service Table.                         */
#define UDS_CHARACTERISTIC_WEIGHT_UUID_CONSTANT                { 0x98, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Height 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Height UUID    */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_HEIGHT_UUID_16(_x)                          ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Height UUID in UUID16 form.  This MACRO*/
   /* only returns whether the UUID_16_t variable is equal to the UDS   */
   /* Height UUID (MACRO returns boolean result) NOT less than/greater  */
   /* than.  The last parameter is the UUID_16_t variable to compare to */
   /* the UDS Characteristic Height UUID.                               */
#define UDS_COMPARE_HEIGHT_UUID_TO_UUID_16(_x)                 COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8E)

   /* The following defines the UDS Characteristic Height UUID that is  */
   /* used when building the UDS Service Table.                         */
#define UDS_CHARACTERISTIC_HEIGHT_UUID_CONSTANT                { 0x8E, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Vo2 max 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Vo2 max UUID   */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_VO2_MAX_UUID_16(_x)                         ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x96)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Vo2 max UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS Vo2 max UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Vo2 max UUID.                */
#define UDS_COMPARE_VO2_MAX_UUID_TO_UUID_16(_x)                COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x96)

   /* The following defines the UDS Characteristic Vo2 max UUID that is */
   /* used when building the UDS Service Table.                         */
#define UDS_CHARACTERISTIC_VO2_MAX_UUID_CONSTANT               { 0x96, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Heart rate max 16 bit UUID to the specified        */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Heart*/
   /* rate max UUID Constant value.                                     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_HEART_RATE_MAX_UUID_16(_x)                  ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8D)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Heart rate max UUID in UUID16 form.    */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Heart rate max UUID (MACRO returns boolean result) NOT    */
   /* less than/greater than.  The last parameter is the UUID_16_t      */
   /* variable to compare to the UDS Characteristic Heart rate max UUID.*/
#define UDS_COMPARE_HEART_RATE_MAX_UUID_TO_UUID_16(_x)         COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8D)

   /* The following defines the UDS Characteristic Heart rate max UUID  */
   /* that is used when building the UDS Service Table.                 */
#define UDS_CHARACTERISTIC_HEART_RATE_MAX_UUID_CONSTANT               { 0x8D, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Resting heart rate 16 bit UUID to the specified    */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic      */
   /* Resting heart rate UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_RESTING_HEART_RATE_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x92)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Resting heart rate UUID in UUID16 form.*/
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Resting heart rate UUID (MACRO returns boolean result) NOT*/
   /* less than/greater than.  The last parameter is the UUID_16_t      */
   /* variable to compare to the UDS Characteristic Resting heart rate  */
   /* UUID.                                                             */
#define UDS_COMPARE_RESTING_HEART_RATE_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x92)

   /* The following defines the UDS Characteristic Resting heart rate   */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_RESTING_HEART_RATE_UUID_CONSTANT    { 0x92, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Maximum recommended heart rate 16 bit UUID to the  */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Maximum recommended heart rate UUID Constant value.*/
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_MAXIMUM_RECOMMENDED_HEART_RATE_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x91)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Maximum recommended heart rate UUID in */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Maximum recommended heart rate UUID  */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Maximum recommended heart rate UUID.               */
#define UDS_COMPARE_MAXIMUM_RECOMMENDED_HEART_RATE_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x91)

   /* The following defines the UDS Characteristic Maximum recommended  */
   /* heart rate UUID that is used when building the UDS Service Table. */
#define UDS_CHARACTERISTIC_MAXIMUM_RECOMMENDED_HEART_RATE_UUID_CONSTANT  { 0x91, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Aerobic threshold 16 bit UUID to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic      */
   /* Aerobic threshold UUID Constant value.                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_AEROBIC_THRESHOLD_UUID_16(_x)               ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x7F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Aerobic threshold UUID in UUID16 form. */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Aerobic threshold UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The last parameter is the UUID_16_t      */
   /* variable to compare to the UDS Characteristic Aerobic threshold   */
   /* UUID.                                                             */
#define UDS_COMPARE_AEROBIC_THRESHOLD_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x7F)

   /* The following defines the UDS Characteristic Aerobic threshold    */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_AEROBIC_THRESHOLD_UUID_CONSTANT     { 0x7F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Anaerobic threshold 16 bit UUID to the specified   */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic      */
   /* Anaerobic threshold UUID Constant value.                          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_ANAEROBIC_THRESHOLD_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x83)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Anaerobic threshold UUID in UUID16     */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the UDS Anaerobic threshold UUID (MACRO returns boolean  */
   /* result) NOT less than/greater than.  The last parameter is the    */
   /* UUID_16_t variable to compare to the UDS Characteristic Anaerobic */
   /* threshold UUID.                                                   */
#define UDS_COMPARE_ANAEROBIC_THRESHOLD_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x83)

   /* The following defines the UDS Characteristic Anaerobic threshold  */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_ANAEROBIC_THRESHOLD_UUID_CONSTANT   { 0x83, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Sport type 16 bit UUID to the specified UUID_16_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Sport type UUID*/
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_SPORT_TYPE_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x93)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Sport type UUID in UUID16 form.  This  */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS Sport type UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Sport type UUID.             */
#define UDS_COMPARE_SPORT_TYPE_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x93)

   /* The following defines the UDS Characteristic Sport type UUID that */
   /* is used when building the UDS Service Table.                      */
#define UDS_CHARACTERISTIC_SPORT_TYPE_UUID_CONSTANT            { 0x93, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Date of threshold 16 bit UUID to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Date */
   /* of threshold UUID Constant value.                                 */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_DATE_OF_THRESHOLD_UUID_16(_x)               ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x86)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Date of threshold UUID in UUID16 form. */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Date of threshold UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The last parameter is the UUID_16_t      */
   /* variable to compare to the UDS Characteristic Date of threshold   */
   /* UUID.                                                             */
#define UDS_COMPARE_DATE_OF_THRESHOLD_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x86)

   /* The following defines the UDS Characteristic Date of threshold    */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_DATE_OF_THRESHOLD_UUID_CONSTANT     { 0x86, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Waist circumference 16 bit UUID to the specified   */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Waist*/
   /* circumference UUID Constant value.                                */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_WAIST_CIRCUMFERENCE_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x97)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Waist circumference UUID in UUID16     */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the UDS Waist circumference UUID (MACRO returns boolean  */
   /* result) NOT less than/greater than.  The last parameter is the    */
   /* UUID_16_t variable to compare to the UDS Characteristic Waist     */
   /* circumference UUID.                                               */
#define UDS_COMPARE_WAIST_CIRCUMFERENCE_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x97)

   /* The following defines the UDS Characteristic Waist circumference  */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_WAIST_CIRCUMFERENCE_UUID_CONSTANT   { 0x97, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Hip circumference 16 bit UUID to the specified     */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the UDS Characteristic Hip  */
   /* circumference UUID Constant value.                                */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_HIP_CIRCUMFERENCE_UUID_16(_x)               ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Hip circumference UUID in UUID16 form. */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the UDS Hip circumference UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The last parameter is the UUID_16_t      */
   /* variable to compare to the UDS Characteristic Hip circumference   */
   /* UUID.                                                             */
#define UDS_COMPARE_HIP_CIRCUMFERENCE_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8F)

   /* The following defines the UDS Characteristic Hip circumference    */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_CHARACTERISTIC_HIP_CIRCUMFERENCE_UUID_CONSTANT     { 0x8F, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Fat burn heart rate lower limit 16 bit UUID to the */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Fat burn heart rate lower limit UUID Constant      */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_FAT_BURN_HEART_RATE_LOWER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x88)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Fat burn heart rate lower limit UUID in*/
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Fat burn heart rate lower limit UUID */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Fat burn heart rate lower limit UUID.              */
#define UDS_COMPARE_FAT_BURN_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x88)

   /* The following defines the UDS Characteristic Fat burn heart rate  */
   /* lower limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_FAT_BURN_HEART_RATE_LOWER_LIMIT_UUID_CONSTANT  { 0x88, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Fat burn heart rate upper limit 16 bit UUID to the */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Fat burn heart rate upper limit UUID Constant      */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_FAT_BURN_HEART_RATE_UPPER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x89

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Fat burn heart rate upper limit UUID in*/
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Fat burn heart rate upper limit UUID */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Fat burn heart rate upper limit UUID.              */
#define UDS_COMPARE_FAT_BURN_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x89)

   /* The following defines the UDS Characteristic Fat burn heart rate  */
   /* upper limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_FAT_BURN_HEART_RATE_UPPER_LIMIT_UUID_CONSTANT  { 0x89, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Aerobic heart rate lower limit 16 bit UUID to the  */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Aerobic heart rate lower limit UUID Constant value.*/
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_AEROBIC_HEART_RATE_LOWER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x7E)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Aerobic heart rate lower limit UUID in */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Aerobic heart rate lower limit UUID  */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Aerobic heart rate lower limit UUID.               */
#define UDS_COMPARE_AEROBIC_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x7E)

   /* The following defines the UDS Characteristic Aerobic heart rate   */
   /* lower limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_AEROBIC_HEART_RATE_LOWER_LIMIT_UUID_CONSTANT  { 0x7E, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Aerobic heart rate upper limit 16 bit UUID to the  */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Aerobic heart rate upper limit UUID Constant value.*/
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_AEROBIC_HEART_RATE_UPPER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x84)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Aerobic heart rate upper limit UUID in */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Aerobic heart rate upper limit UUID  */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Aerobic heart rate upper limit UUID.               */
#define UDS_COMPARE_AEROBIC_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x84)

   /* The following defines the UDS Characteristic Aerobic heart rate   */
   /* upper limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_AEROBIC_HEART_RATE_UPPER_LIMIT_UUID_CONSTANT  { 0x84, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Anaerobic heart rate lower limit 16 bit UUID to the*/
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Anaerobic heart rate lower limit UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_ANAEROBIC_HEART_RATE_LOWER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x81)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Anaerobic heart rate lower limit UUID  */
   /* in UUID16 form.  This MACRO only returns whether the UUID_16_t    */
   /* variable is equal to the UDS Anaerobic heart rate lower limit UUID*/
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Anaerobic heart rate lower limit UUID.             */
#define UDS_COMPARE_ANAEROBIC_HEART_RATE_LOWER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x81)

   /* The following defines the UDS Characteristic Anaerobic heart rate */
   /* lower limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_ANAEROBIC_HEART_RATE_LOWER_LIMIT_UUID_CONSTANT  { 0x81, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Anaerobic heart rate upper limit 16 bit UUID to the*/
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Anaerobic heart rate upper limit UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_ANAEROBIC_HEART_RATE_UPPER_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x82)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Anaerobic heart rate upper limit UUID  */
   /* in UUID16 form.  This MACRO only returns whether the UUID_16_t    */
   /* variable is equal to the UDS Anaerobic heart rate upper limit UUID*/
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Anaerobic heart rate upper limit UUID.             */
#define UDS_COMPARE_ANAEROBIC_HEART_RATE_UPPER_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x82)

   /* The following defines the UDS Characteristic Anaerobic heart rate */
   /* upper limit UUID that is used when building the UDS Service Table.*/
#define UDS_CHARACTERISTIC_ANAEROBIC_HEART_RATE_UPPER_LIMIT_UUID_CONSTANT  { 0x82, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Five zone heart rate limits 16 bit UUID to the     */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Five zone heart rate limits UUID Constant value.   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_FIVE_ZONE_HEART_RATE_LIMITS_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x8B)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Five zone heart rate limits UUID in    */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Five zone heart rate limits UUID     */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Five zone heart rate limits UUID.                  */
#define UDS_COMPARE_FIVE_ZONE_HEART_RATE_LIMITS_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x8B)

   /* The following defines the UDS Characteristic Five zone heart rate */
   /* limits UUID that is used when building the UDS Service Table.     */
#define UDS_CHARACTERISTIC_FIVE_ZONE_HEART_RATE_LIMITS_UUID_CONSTANT  { 0x8B, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Three zone heart rate limits 16 bit UUID to the    */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Three zone heart rate limits UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_THREE_ZONE_HEART_RATE_LIMITS_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x94)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Three zone heart rate limits UUID in   */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Three zone heart rate limits UUID    */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* last parameter is the UUID_16_t variable to compare to the UDS    */
   /* Characteristic Three zone heart rate limits UUID.                 */
#define UDS_COMPARE_THREE_ZONE_HEART_RATE_LIMITS_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x94)

   /* The following defines the UDS Characteristic Three zone heart rate*/
   /* limits UUID that is used when building the UDS Service Table.     */
#define UDS_CHARACTERISTIC_THREE_ZONE_HEART_RATE_LIMITS_UUID_CONSTANT  { 0x94, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Two zone heart rate limit 16 bit UUID to the       */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the UDS        */
   /* Characteristic Two zone heart rate limit UUID Constant value.     */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_TWO_ZONE_HEART_RATE_LIMIT_UUID_16(_x)            ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x95)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Two zone heart rate limit UUID in      */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Two zone heart rate limit UUID (MACRO*/
   /* returns boolean result) NOT less than/greater than.  The last     */
   /* parameter is the UUID_16_t variable to compare to the UDS         */
   /* Characteristic Two zone heart rate limit UUID.                    */
#define UDS_COMPARE_TWO_ZONE_HEART_RATE_LIMIT_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x95)

   /* The following defines the UDS Characteristic Five zone heart rate */
   /* limits UUID that is used when building the UDS Service Table.     */
#define UDS_CHARACTERISTIC_TWO_ZONE_HEART_RATE_LIMIT_UUID_CONSTANT  { 0x95, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Characteristic Language 16 bit UUID to the specified UUID_16_t    */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Characteristic Language UUID  */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_LANGUAGE_UUID_16(_x)                        ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xA2)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Language UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS Language UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The last parameter is the UUID_16_t variable  */
   /* to compare to the UDS Characteristic Language UUID.               */
#define UDS_COMPARE_LANGUAGE_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xA2)

   /* The following defines the UDS Characteristic Five zone heart rate */
   /* limits UUID that is used when building the UDS Service Table.     */
#define UDS_CHARACTERISTIC_LANGUAGE_UUID_CONSTANT              { 0xA2, 0x2A }

   /*********************************************************************/
   /* End UDS Characteristics.                                          */
   /*********************************************************************/

   /* The following MACRO is a utility MACRO that assigns the UDS       */
   /* Database Change Increment 16 bit UUID to the specified UUID_16_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the UDS Database Change Increment UUID*/
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_DATABASE_CHANGE_INCREMENT_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x99)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS Database Change Increment UUID in      */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the UDS Database Change Increment UUID (MACRO*/
   /* returns boolean result) NOT less than/greater than.  The first    */
   /* parameter is the UUID_16_t variable to compare to the UDS Database*/
   /* Change Increment UUID.                                            */
#define UDS_COMPARE_DATABASE_CHANGE_INCREMENT_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x99)

   /* The following defines the UDS Database Change Increment           */
   /* Characteristic UUID that is used when building the UDS Service    */
   /* Table.                                                            */
#define UDS_DATABASE_CHANGE_INCREMENT_CHARACTERISTIC_UUID_CONSTANT    { 0x99, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS User  */
   /* Index 16 bit UUID to the specified UUID_16_t variable.  This MACRO*/
   /* accepts one parameter which is the UUID_16_t variable that is to  */
   /* receive the UDS User Index UUID Constant value.                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_USER_INDEX_UUID_16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x9A)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS User Index UUID in UUID16 form.  This  */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS User Index UUID (MACRO returns boolean result) NOT less       */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the UDS User Index UUID.                            */
#define UDS_COMPARE_USER_INDEX_UUID_TO_UUID_16(_x)             COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x9A)

   /* The following defines the UDS User Index Characteristic UUID that */
   /* is used when building the UDS Service Table.                      */
#define UDS_USER_INDEX_CHARACTERISTIC_UUID_CONSTANT            { 0x9A, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the UDS User  */
   /* Control Point 16 bit UUID to the specified UUID_16_t variable.    */
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the UDS User Control Point UUID Constant value.*/
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define UDS_ASSIGN_USER_CONTROL_POINT_UUID_16(_x)              ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x9F)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined UDS User Index UUID in UUID16 form.  This  */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* UDS User Control Point UUID (MACRO returns boolean result) NOT    */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the UDS User Control Point UUID.           */
#define UDS_COMPARE_USER_CONTROL_POINT_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x9F)

   /* The following defines the UDS User Control Point Characteristic   */
   /* UUID that is used when building the UDS Service Table.            */
#define UDS_USER_CONTROL_POINT_CHARACTERISTIC_UUID_CONSTANT    { 0x9F, 0x2A }

   /* The following defines the values for the user control point.      */
#define UDS_USER_CONTROL_POINT_OP_CODE_REGISTER_NEW_USER              0x01
#define UDS_USER_CONTROL_POINT_OP_CODE_CONSENT                        0x02
#define UDS_USER_CONTROL_POINT_OP_CODE_DELETE_USER_DATA               0x03
#define UDS_USER_CONTROL_POINT_OP_CODE_RESPONSE_CODE                  0x20

   /* The following defines the response values for the user control    */
   /* point.                                                            */
#define UDS_USER_CONTROL_POINT_RESPONSE_VALUE_SUCCESS                 0x01
#define UDS_USER_CONTROL_POINT_RESPONSE_VALUE_OP_CODE_NOT_SUPPORTED   0x02
#define UDS_USER_CONTROL_POINT_RESPONSE_VALUE_INVALID_PARAMETER       0x03
#define UDS_USER_CONTROL_POINT_RESPONSE_VALUE_OPERATION_FAILED        0x04
#define UDS_USER_CONTROL_POINT_RESPONSE_VALUE_USER_NOT_AUTHORIZED     0x05

   /* The following structure defines the format of the Date Of Birth   */
   /* UDS Characteristic data.                                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUDS_Date_t
{
   NonAlignedWord_t Year;
   NonAlignedByte_t Month;
   NonAlignedByte_t Day;
} __PACKED_STRUCT_END__ UDS_Date_t;

#define UDS_DATE_SIZE                                          (sizeof(UDS_Date_t))

   /* The following structure defines the format of the Five Zone Heart */
   /* Rate Limits UDS Characteristic data.                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUDS_Five_Zone_Heart_Rate_Limits_t
{
   NonAlignedByte_t Light_VeryLight_Limit;
   NonAlignedByte_t Light_Moderate_Limit;
   NonAlignedByte_t Moderate_Hard_Limit;
   NonAlignedByte_t Hard_Maximum_Limit;
} __PACKED_STRUCT_END__ UDS_Five_Zone_Heart_Rate_Limits_t;

#define UDS_FIVE_ZONE_HEART_RATE_LIMITS_SIZE                   (sizeof(UDS_Five_Zone_Heart_Rate_Limits_t))

   /* The following structure defines the format of the Three Zone Heart*/
   /* Rate Limits UDS Characteristic data.                              */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUDS_Three_Zone_Heart_Rate_Limits_t
{
   NonAlignedByte_t Light_Moderate_Limit;
   NonAlignedByte_t Moderate_Hard_Limit;
} __PACKED_STRUCT_END__ UDS_Three_Zone_Heart_Rate_Limits_t;

#define UDS_THREE_ZONE_HEART_RATE_LIMITS_SIZE                  (sizeof(UDS_Three_Zone_Heart_Rate_Limits_t))

  /* The following structure defines the format of the User Control     */
  /* Point Request.                                                     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUDS_User_Control_Point_Request_t
{
   NonAlignedByte_t Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ UDS_User_Control_Point_Request_t;

#define UDS_USER_CONTROL_POINT_REQUEST_SIZE(_x)                (BTPS_STRUCTURE_OFFSET(UDS_User_Control_Point_Request_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the minimum size of a UDS User Control Point request  */
   /* based on the User Coontrol Point request Op Code.  The only       */
   /* parameter to this MACRO is the request Op Code.                   */
#define UDS_USER_CONTROL_POINT_REQUEST_MINIMUM_LENGTH(_x)      (UDS_USER_CONTROL_POINT_REQUEST_SIZE(0)                                                                     + \
                                                               (((_x) == UDS_USER_CONTROL_POINT_OP_CODE_REGISTER_NEW_USER) ? NON_ALIGNED_WORD_SIZE                         : 0) + \
                                                               (((_x) == UDS_USER_CONTROL_POINT_OP_CODE_CONSENT)           ? NON_ALIGNED_WORD_SIZE + NON_ALIGNED_BYTE_SIZE : 0))


  /* The following structure defines the format of the User Control     */
  /* Point Response.                                                    */
typedef __PACKED_STRUCT_BEGIN__ struct _tagUDS_User_Control_Point_Response_t
{
   NonAlignedByte_t Response_Code_Op_Code;
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Response_Value;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ UDS_User_Control_Point_Response_t;

#define UDS_USER_CONTROL_POINT_RESPONSE_SIZE(_x)               (BTPS_STRUCTURE_OFFSET(UDS_User_Control_Point_Response_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* Consent Code to see if it is valid.  This MACRO only returns      */
   /* whether the UDS Consent Code variable is valid.  The first        */
   /* parameter is the UDS Consent Code to validate.                    */
#define UDS_CONSENT_CODE_VALID(_x)                             ((((_x) >= 0) && ((_x) <= 9999)))

   /* The following defines the UDS User Index value for an unknown     */
   /* user.                                                             */
#define UDS_USER_INDEX_UNKNOWN_USER                            0xFF

   /* The following defines the valid values that may be set Client     */
   /* Characteristic Configuration Descriptor (CCCD).                   */
   /* * NOTE * Only one value should be set for any CCCD.  Some CCCD's  */
   /*          will only allow notifications or indications to be       */
   /*          enabled.  Disabled is always allowed.                    */
#define UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_DISABLED         (0)
#define UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define UDS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the UDS Service Flags MASK that should be   */
   /* passed into the GATT_Register_Service() function to configure the */
   /* transport the service will use when it is registered with GATT.   */
#define UDS_SERVICE_FLAGS_LE                                   (GATT_SERVICE_FLAGS_LE_SERVICE)
#define UDS_SERVICE_FLAGS_BR_EDR                               (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define UDS_SERVICE_FLAGS_DUAL_MODE                            (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
