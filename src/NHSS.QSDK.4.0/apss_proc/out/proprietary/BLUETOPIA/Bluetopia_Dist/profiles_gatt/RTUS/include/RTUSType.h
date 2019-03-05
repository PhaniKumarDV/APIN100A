/*****< rtustypes.h >**********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  RTUSTypes - Stonestreet One Bluetooth Stack Reference Time Update Service */
/*              Types.                                                        */
/*                                                                            */
/*  Author:  Zahid Khan                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/11/12  Z. Khan        Initial creation.                               */
/******************************************************************************/
#ifndef __RTUSTYPEH__
#define __RTUSTYPEH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */

   /* The following MACRO is a utility MACRO that assigns the Reference */
   /* Time Update Service 16 bit UUID to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is a pointer to */
   /* a UUID_16_t variable that is to receive the RTUS UUID Constant    */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define RTUS_ASSIGN_RTUS_SERVICE_UUID_16(_x)             ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x06)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined RTUS Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* RTUS Service UUID (MACRO returns boolean result) NOT less         */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the RTUS Service UUID.                              */
#define RTUS_COMPARE_RTUS_SERVICE_UUID_TO_UUID_16(_x)    COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x06)

   /* The following defines the Reference Time Update Service UUID that */
   /* is used when building the RTUS Service Table.                     */
#define RTUS_SERVICE_BLUETOOTH_UUID_CONSTANT             { 0x06, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the RTUS      */
   /* Time Update State Characteristic 16 bit UUID to the               */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the RTUS       */
   /* Time Update State UUID Constant value.                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define RTUS_ASSIGN_TIME_UPDATE_STATE_UUID_16(_x)                     ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x17)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined RTUS Time Update State UUID in UUID16      */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the Time Update State UUID (MACRO returns boolean        */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the RTUS Time Update State UUID  */
#define RTUS_COMPARE_RTUS_TIME_UPDATE_STATE_UUID_TO_UUID_16(_x)       COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x17)

   /* The following defines the RTUS Time Update State                  */
   /* Characteristic UUID that is used when building the rtus Service   */
   /* Table.                                                            */
#define RTUS_TIME_UPDATE_STATE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x17, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the RTUS Time */
   /* Update Control Point Characteristic 16 bit UUID to the specified  */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the RTUS Time Update Control*/
   /* Point UUID Constant value.                                        */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define RTUS_ASSIGN_TIME_UPDATE_CONTROL_POINT_UUID_16(_x) ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0x16)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID16 to the defined RTUS Time Update Control Point UUID in      */
   /* UUID16 form. This MACRO only returns whether the UUID_16_t        */
   /* variable is equal to the RTUS Time Update Control Point UUID      */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the RTUS  */
   /* Time Update Control Point UUID.                                   */
#define RTUS_COMPARE_RTUS_TIME_UPDATE_CONTROL_POINT_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0x16)

   /* The following defines the RTUS Time Update Control Point          */
   /* Characteristic UUID that is used when building the RTUS Service   */
   /* Table.                                                            */
#define RTUS_TIME_UPDATE_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x16, 0x2A }

   /* The following define the valid Time Update Control Point commands */
   /* that may be written to the Time Update Control Point              */
   /* characteristic value.                                             */
#define RTUS_TIME_UPDATE_CONTROL_POINT_GET_REFERENCE_UPDATE             0x01
#define RTUS_TIME_UPDATE_CONTROL_POINT_CANCEL_REFERENCE_UPDATE          0x02

   /* The following defines the valid Current State that may be read    */
   /* from the Time Update State characteristic.                        */
#define RTUS_CURRENT_STATE_IDLE                                         0x00
#define RTUS_CURRENT_STATE_UPDATE_PENDING                               0X01

   /* The following defines the valid Result that may be read from the  */
   /* Time Update State characteristic.                                 */
#define RTUS_RESULT_SUCCESSFUL                                          0x00
#define RTUS_RESULT_CANCELED                                            0x01
#define RTUS_RESULT_NO_CONNECTION_TO_REFERENCE                          0x02
#define RTUS_RESULT_REFERENCE_RESPONDED_WITH_AN_ERROR                   0x03
#define RTUS_RESULT_TIMEOUT                                             0x04
#define RTUS_RESULT_UPDATE_NOT_ATTEMPTED_AFTER_RESET                    0x05

   /* The following MACRO is a utility MACRO that exists to determine   */
   /* if the value written to the Time Update Control Point is a valid  */
   /* command.                                                          */
#define RTUS_TIME_UPDATE_CONTROL_POINT_VALID_COMMAND(_x)  ((((Byte_t)(_x)) >= RTUS_TIME_UPDATE_CONTROL_POINT_GET_REFERENCE_UPDATE) &&   \
                                                          (((Byte_t)(_x)) <= RTUS_TIME_UPDATE_CONTROL_POINT_CANCEL_REFERENCE_UPDATE))

   /* The following MACRO is a utility MACRO that exists to validate    */
   /* the specified Time Update State. The parameters to this           */
   /* function is CurrentState and Result field of                      */
   /* RTUS_Time_Update_State_Data_t structure. This MACRO returns TRUE  */
   /* if the Time Update State is valid or FALSE otherwise.             */
#define RTUS_TIME_UPDATE_STATE_VALID(_x,_y)     (((((Byte_t)(_x)) >= RTUS_CURRENT_STATE_IDLE) && (((Byte_t)(_x)) <= RTUS_CURRENT_STATE_UPDATE_PENDING)) && \
                                                ((((Byte_t)(_y)) >= RTUS_RESULT_SUCCESSFUL) && (((Byte_t)(_y)) <= RTUS_RESULT_UPDATE_NOT_ATTEMPTED_AFTER_RESET)))

   /* The following defines the length of the Reference Time Update     */
   /* Control Point characteristic value.                               */
#define RTUS_TIME_UPDATE_CONTROL_POINT_VALUE_LENGTH               (BYTE_SIZE)

  /* The following defines the RTUS GATT Service Flags MASK that should */
  /* be passed into GATT_Register_Service when the RTUS Service is      */
  /* registered.                                                        */
#define RTUS_SERVICE_FLAGS                                        (GATT_SERVICE_FLAGS_LE_SERVICE)

   /* The followng defines the format of a Time Update State.           */
   /* The first member specifies information of Current State.          */
   /* The second member specifies Result of Time Update State.          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagRTUS_Time_Update_State_t
{
   NonAlignedByte_t CurrentState;
   NonAlignedByte_t Result;
} __PACKED_STRUCT_END__ RTUS_Time_Update_State_t;

#define RTUS_TIME_UPDATE_STATE_SIZE                       (sizeof(RTUS_Time_Update_State_t))

#endif
