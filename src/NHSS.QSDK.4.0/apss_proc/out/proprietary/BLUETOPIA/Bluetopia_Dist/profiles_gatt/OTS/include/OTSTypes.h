/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*****< otstypes.h >***********************************************************/
/*      Copyright 2016 Qualcomm Technologies, Inc.                            */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  OTSTypes - Qualcomm Technologies Bluetooth Object Transfer Service Types. */
/*                                                                            */
/*  Author:  Ryan McCord                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/11/16  R. McCord      Initial creation.                               */
/******************************************************************************/
#ifndef __OTSTYPESH__
#define __OTSTYPESH__

#include "BTTypes.h"            /* Bluetooth Type Definitions.                */
#include "GATTType.h"           /* Bluetooth GATT Type Definitions.           */

   /* The following defines the defined OTS Error Codes that may be     */
   /* sent in a GATT Error Response.                                    */
#define OTS_ERROR_CODE_SUCCESS                                    0x00
#define OTS_ERROR_CODE_WRITE_REQUEST_REJECTED                     0x80
#define OTS_ERROR_CODE_OBJECT_NOT_SELECTED                        0x81
#define OTS_ERROR_CODE_CONCURRENCY_LIMIT_EXCEEDED                 0x82
#define OTS_ERROR_CODE_OBJECT_NAME_ALREADY_EXISTS                 0x83

   /* The following defines common attribute protocol error codes use by*/
   /* this service that may be needed by the Application.               */
#define OTS_ERROR_CODE_INSUFFICIENT_AUTHENTICATION                (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION)
#define OTS_ERROR_CODE_INSUFFICIENT_AUTHORIZATION                 (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION)
#define OTS_ERROR_CODE_INSUFFICIENT_ENCRYPTION                    (ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION)
#define OTS_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED                 (ATT_PROTOCOL_ERROR_CODE_CCCD_IMPROPERLY_CONFIGURED)
#define OTS_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS              (ATT_PROTOCOL_ERROR_CODE_PROCEDURE_ALREADY_IN_PROGRESS)
#define OTS_ERROR_CODE_UNLIKELY_ERROR                             (ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR)

   /* The following MACRO is a utility MACRO that assigns the Object    */
   /* Transfer Service 16 bit UUID to the specified UUID_16_t variable. */
   /* This MACRO accepts one parameter which is a pointer to a UUID_16_t*/
   /* variable that is to receive the OTS UUID Constant value.          */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OTS_SERVICE_UUID_16(_x)                        ASSIGN_BLUETOOTH_UUID_16(*((UUID_16_t *)(_x)), 0x18, 0x25)

   /* The following MACRO is a utility MACRO that assigns the OTS       */
   /* Service 16 bit UUID to the specified UUID_16_t variable.  This    */
   /* MACRO accepts one parameter which is a UUID_16_t variable that is */
   /* to receive the OTS UUID Constant value.                           */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Big-Endian format.                                       */
#define OTS_ASSIGN_OTS_SERVICE_SDP_UUID_16(_x)                    ASSIGN_SDP_UUID_16((_x), 0x18, 0x25)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Service UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Service UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS Service UUID.                               */
#define OTS_COMPARE_OTS_SERVICE_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x18, 0x25)

   /* The following defines the Object Transfer Service UUID that is    */
   /* used when building the OTS Service Table.                         */
#define OTS_SERVICE_BLUETOOTH_UUID_CONSTANT                       { 0x25, 0x18 }

   /* The following MACRO is a utility MACRO that assigns the OTS       */
   /* Feature Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Feature UUID Constant value.  */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OTS_FEATURE_UUID16(_x)                         ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBD)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Feature UUID in UUID16 form.  This     */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Feature UUID (MACRO returns boolean result) NOT less          */
   /* than/greater than.  The first parameteris the UUID_16_t variable  */
   /* to compare to the OTS Feature UUID.                               */
#define OTS_COMPARE_OTS_FEATURE_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBD)

   /* The following defines the OTS Feature Characteristic UUID that is */
   /* used when building the OTS Service Table.                         */
#define OTS_FEATURE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT        { 0xBD, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Name Characteristic 16 bit UUID to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Object Name UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_NAME_UUID16(_x)                         ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBE)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Name UUID in UUID16 form.  This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Object Name UUID (MACRO returns boolean result) NOT less      */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS Object Name UUID.                           */
#define OTS_COMPARE_OBJECT_NAME_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBE)

   /* The following defines the OTS Object Name Characteristic UUID that*/
   /* is used when building the OTS Service Table.                      */
#define OTS_OBJECT_NAME_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT    { 0xBE, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Type Characteristic 16 bit UUID to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Object Type UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_TYPE_UUID16(_x)                         ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xBF)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Type UUID in UUID16 form.  This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Object Type UUID (MACRO returns boolean result) NOT less      */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS Object Type UUID.                           */
#define OTS_COMPARE_OBJECT_TYPE_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xBF)

   /* The following defines the OTS Object Type Characteristic UUID that*/
   /* is used when building the OTS Service Table.                      */
#define OTS_OBJECT_TYPE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT    { 0xBF, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS       */
   /* Directory Listing Object Type Characteristic 16 bit UUID to the   */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the OTS        */
   /* Directory Listing Object Type UUID Constant value.                */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_DIRECTORY_LISTING_OBJECT_TYPE_UUID16(_x)           ASSIGN_BLUETOOTH_UUID_16((_x), 0x7F, 0xB1)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Unspecified Object Type UUID in UUID16 */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Unspecified Object Type UUID (MACRO returns      */
   /* boolean result) NOT less than/greater than.  The first parameter  */
   /* is the UUID_16_t variable to compare to the OTS Unspecified Object*/
   /* Type UUID.                                                        */
#define OTS_COMPARE_DIRECTORY_LISTING_OBJECT_TYPE_UUID_TO_UUID_16(_x) COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x7F, 0xB1)

   /* The following MACRO is a utility MACRO that assigns the OTS       */
   /* Unspecified Object Type Characteristic 16 bit UUID to the         */
   /* specified UUID_16_t variable.  This MACRO accepts one parameter   */
   /* which is the UUID_16_t variable that is to receive the OTS        */
   /* Unspecified Object Type UUID Constant value.                      */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_UNSPECIFIED_OBJECT_TYPE_UUID16(_x)             ASSIGN_BLUETOOTH_UUID_16((_x), 0x7F, 0xB0)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Unspecified Object Type UUID in UUID16 */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Unspecified Object Type UUID (MACRO returns      */
   /* boolean result) NOT less than/greater than.  The first parameter  */
   /* is the UUID_16_t variable to compare to the OTS Unspecified       */
   /* Object Type UUID.                                                 */
#define OTS_COMPARE_UNSPECIFIED_OBJECT_TYPE_UUID_TO_UUID_16(_x)   COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x7F, 0xB0)

   /* The following MACRO is a utility MACRO that assigns the OTS       */
   /* Firmware Object Type Characteristic 16 bit UUID to the specified  */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Firmware Object Type*/
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_FIRMWARE_OBJECT_TYPE_UUID16(_x)                ASSIGN_BLUETOOTH_UUID_16((_x), 0x7F, 0xB1)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Firmware Object Type UUID in UUID16    */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Firmware Object Type UUID (MACRO returns boolean */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the OTS Firmware Object Type     */
   /* UUID.                                                             */
#define OTS_COMPARE_FIRMWARE_OBJECT_TYPE_UUID_TO_UUID_16(_x)      COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x7F, 0xB1)

   /* The following MACRO is a utility MACRO that assigns the OTS Route */
   /* GPS Object Type Characteristic 16 bit UUID to the specified       */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Route GPX Object    */
   /* Type UUID Constant value.                                         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_ROUTE_GPX_OBJECT_TYPE_UUID16(_x)               ASSIGN_BLUETOOTH_UUID_16((_x), 0x7F, 0xB1)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Route GPX Object Type UUID in UUID16   */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Route GPX Object Type UUID (MACRO returns boolean*/
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the OTS Route GPX Object Type    */
   /* UUID.                                                             */
#define OTS_COMPARE_ROUTE_GPX_OBJECT_TYPE_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x7F, 0xB2)

   /* The following MACRO is a utility MACRO that assigns the OTS Track */
   /* GPS Object Type Characteristic 16 bit UUID to the specified       */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Track GPX Object    */
   /* Type UUID Constant value.                                         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_TRACK_GPX_OBJECT_TYPE_UUID16(_x)               ASSIGN_BLUETOOTH_UUID_16((_x), 0x7F, 0xB3)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Track GPX Object Type UUID in UUID16   */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Track GPX Object Type UUID (MACRO returns boolean*/
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the OTS Track GPX Object Type    */
   /* UUID.                                                             */
#define OTS_COMPARE_TRACK_GPX_OBJECT_TYPE_UUID_TO_UUID_16(_x)     COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x7F, 0xB3)

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Size Characteristic 16 bit UUID to the specified UUID_16_t        */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Object Size UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_SIZE_UUID16(_x)                         ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC0)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Size UUID in UUID16 form.  This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Object Size UUID (MACRO returns boolean result) NOT less      */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS Object Allocated Size UUID.                 */
#define OTS_COMPARE_OBJECT_SIZE_UUID_TO_UUID_16(_x)               COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC0)

   /* The following defines the OTS Object Size Characteristic UUID that*/
   /* is used when building the OTS Service Table.                      */
#define OTS_OBJECT_SIZE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT    { 0xC0, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* First Created Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Object First Created*/
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_FIRST_CREATED_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC1)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object First Created UUID in UUID16    */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Object First Created UUID (MACRO returns boolean */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the OTS Object First Created     */
   /* UUID.                                                             */
#define OTS_COMPARE_OBJECT_FIRST_CREATED_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC1)

   /* The following defines the OTS Object First Created Characteristic */
   /* UUID that is used when building the OTS Service Table.            */
#define OTS_OBJECT_FIRST_CREATED_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC1, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Last Modified Characteristic 16 bit UUID to the specified         */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Object Last Modified*/
   /* UUID Constant value.                                              */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_LAST_MODIFIED_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC2)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Last Modified UUID in UUID16    */
   /* form.  This MACRO only returns whether the UUID_16_t variable is  */
   /* equal to the OTS Object Last Modified UUID (MACRO returns boolean */
   /* result) NOT less than/greater than.  The first parameter is the   */
   /* UUID_16_t variable to compare to the OTS Object Last Modified     */
   /* UUID.                                                             */
#define OTS_COMPARE_OBJECT_LAST_MODIFIED_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC2)

   /* The following defines the OTS Object Last Modified Characteristic */
   /* UUID that is used when building the OTS Service Table.            */
#define OTS_OBJECT_LAST_MODIFIED_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC2, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* ID Characteristic 16 bit UUID to the specified UUID_16_t variable.*/
   /* This MACRO accepts one parameter which is the UUID_16_t variable  */
   /* that is to receive the OTS Object ID UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_ID_UUID16(_x)                           ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC3)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object ID UUID in UUID16 form.  This   */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS Object ID UUID (MACRO returns boolean result) NOT less        */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS Object ID UUID.                             */
#define OTS_COMPARE_OBJECT_ID_UUID_TO_UUID_16(_x)                 COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC3)

   /* The following defines the OTS Object ID Characteristic UUID that  */
   /* is used when building the OTS Service Table.                      */
#define OTS_OBJECT_ID_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT      { 0xC3, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Properties Characteristic 16 bit UUID to the specified UUID_16_t  */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Object Properties UUID        */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_PROPERTIES_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC4)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Properties UUID in UUID16 form. */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the OTS Object Properties UUID (MACRO returns boolean result) NOT */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the OTS Object Properties UUID.            */
#define OTS_COMPARE_OBJECT_PROPERTIES_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC4)

   /* The following defines the OTS Object Properties Characteristic    */
   /* UUID that is used when building the OTS Service Table.            */
#define OTS_OBJECT_PROPERTIES_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC4, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Action Control Point Characteristic 16 bit UUID to the specified  */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Object Action       */
   /* Control Point UUID Constant value.                                */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_ACTION_CONTROL_POINT_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC5)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Action Control Point UUID in    */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the OTS Object Action Control Point UUID     */
   /* (MACRO returns boolean result) NOT less than/greater than.  The   */
   /* first parameter is the UUID_16_t variable to compare to the OTS   */
   /* Object Action Control Point UUID.                                 */
#define OTS_COMPARE_OBJECT_ACTION_CONTROL_POINT_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC5)

   /* The following defines the OTS Object Action Control Point         */
   /* Characteristic UUID that is used when building the OTS Service    */
   /* Table.                                                            */
#define OTS_OBJECT_ACTION_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC5, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* List Control Point Characteristic 16 bit UUID to the specified    */
   /* UUID_16_t variable.  This MACRO accepts one parameter which is the*/
   /* UUID_16_t variable that is to receive the OTS Object List Control */
   /* Point UUID Constant value.                                        */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_LIST_CONTROL_POINT_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC6)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object List Control Point UUID in      */
   /* UUID16 form.  This MACRO only returns whether the UUID_16_t       */
   /* variable is equal to the OTS Object List Control Point UUID (MACRO*/
   /* returns boolean result) NOT less than/greater than.  The first    */
   /* parameter is the UUID_16_t variable to compare to the OTS Object  */
   /* List Control Point UUID.                                          */
#define OTS_COMPARE_OBJECT_LIST_CONTROL_POINT_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC6)

   /* The following defines the OTS Object List Control Point           */
   /* Characteristic UUID that is used when building the OTS Service    */
   /* Table.                                                            */
#define OTS_OBJECT_LIST_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC6, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS List  */
   /* Filter Characteristic 16 bit UUID to the specified UUID_16_t      */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS List Filter UUID Constant     */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_LIST_FILTER_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC7)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS List Filter UUID in UUID16 form.  This */
   /* MACRO only returns whether the UUID_16_t variable is equal to the */
   /* OTS List Filter UUID (MACRO returns boolean result) NOT less      */
   /* than/greater than.  The first parameter is the UUID_16_t variable */
   /* to compare to the OTS List Filter UUID.                           */
#define OTS_COMPARE_OBJECT_LIST_FILTER_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC7)

   /* The following defines the OTS List Filter Characteristic UUID that*/
   /* is used when building the OTS Service Table.                      */
#define OTS_OBJECT_LIST_FILTER_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC7, 0x2A }

   /* The following MACRO is a utility MACRO that assigns the OTS Object*/
   /* Changed Characteristic 16 bit UUID to the specified UUID_16_t     */
   /* variable.  This MACRO accepts one parameter which is the UUID_16_t*/
   /* variable that is to receive the OTS Object Changed UUID Constant  */
   /* value.                                                            */
   /* * NOTE * The UUID will be assigned into the UUID_16_t variable in */
   /*          Little-Endian format.                                    */
#define OTS_ASSIGN_OBJECT_CHANGED_UUID16(_x)                      ASSIGN_BLUETOOTH_UUID_16((_x), 0x2A, 0xC8)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined OTS Object Changed UUID in UUID16 form.    */
   /* This MACRO only returns whether the UUID_16_t variable is equal to*/
   /* the OTS Object Changed UUID (MACRO returns boolean result) NOT    */
   /* less than/greater than.  The first parameter is the UUID_16_t     */
   /* variable to compare to the OTS Object Changed UUID.               */
#define OTS_COMPARE_OBJECT_CHANGED_UUID_TO_UUID_16(_x)            COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0x2A, 0xC8)

   /* The following defines the OTS Object Changed Characteristic UUID  */
   /* that is used when building the OTS Service Table.                 */
#define OTS_OBJECT_CHANGED_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0xC8, 0x2A }

   /* The following defines the valid values for the Object Action      */
   /* Control Point (OACP) Features field of the OTS Feature            */
   /* Characteristic.                                                   */
#define OTS_FEATURE_OACP_CREATE_OP_CODE_SUPPORTED              0x0001
#define OTS_FEATURE_OACP_DELETE_OP_CODE_SUPPORTED              0x0002
#define OTS_FEATURE_OACP_CALCULATE_CHECKSUM_OP_CODE_SUPPORTED  0x0004
#define OTS_FEATURE_OACP_EXECUTE_OP_CODE_SUPPORTED             0x0008
#define OTS_FEATURE_OACP_READ_OP_CODE_SUPPORTED                0x0010
#define OTS_FEATURE_OACP_WRITE_OP_CODE_SUPPORTED               0x0020
#define OTS_FEATURE_OACP_APPENDING_SUPPORTED                   0x0040
#define OTS_FEATURE_OACP_TRUNCATION_SUPPORTED                  0x0080
#define OTS_FEATURE_OACP_PATCHING_SUPPORTED                    0x0100
#define OTS_FEATURE_OACP_ABORT_OP_CODE_SUPPORTED               0x0200

   /* The following defines the valid values for the Object List Control*/
   /* Point (OLCP) Features field of the OTS Feature Characteristic.    */
#define OTS_FEATURE_OLCP_GO_TO_OP_CODE_SUPPORTED                      0x0001
#define OTS_FEATURE_OLCP_ORDER_OP_CODE_SUPPORTED                      0x0002
#define OTS_FEATURE_OLCP_REQUEST_NUMBER_OF_OBJECTS_OP_CODE_SUPPORTED  0x0004
#define OTS_FEATURE_OLCP_CLEAR_MARKING_OP_CODE_SUPPORTED              0x0008

   /* The following defines the valid values that may be set for the OTS*/
   /* Object Properties Characteristic.                                 */
#define OTS_OBJECT_PROPERTIES_DELETE                           0x00000001
#define OTS_OBJECT_PROPERTIES_EXECUTE                          0x00000002
#define OTS_OBJECT_PROPERTIES_READ                             0x00000004
#define OTS_OBJECT_PROPERTIES_WRITE                            0x00000008
#define OTS_OBJECT_PROPERTIES_APPEND                           0x00000010
#define OTS_OBJECT_PROPERTIES_TRUNCATE                         0x00000020
#define OTS_OBJECT_PROPERTIES_PATCH                            0x00000040
#define OTS_OBJECT_PROPERTIES_MARK                             0x00000080

   /* The following defines the valid values for the Op Code field of   */
   /* the Object Action Control Point (OACP) Characteristic.            */
#define OTS_OACP_OP_CODE_CREATE                                0x01
#define OTS_OACP_OP_CODE_DELETE                                0x02
#define OTS_OACP_OP_CODE_CALCULATE_CHECKSUM                    0x03
#define OTS_OACP_OP_CODE_EXECUTE                               0x04
#define OTS_OACP_OP_CODE_READ                                  0x05
#define OTS_OACP_OP_CODE_WRITE                                 0x06
#define OTS_OACP_OP_CODE_ABORT                                 0x07
#define OTS_OACP_OP_CODE_RESPONSE_CODE                         0x60

   /* The following defines the valid values that may be set for the    */
   /* Mode parameter if the OTS_OACP_OPCODE_WRITE is set for the Op Code*/
   /* field of the OTS Object Action Control Point (OACP)               */
   /* Characteristic.                                                   */
   /* * NOTE * The OTS Object's properties MUST support truncation or   */
   /*          the OTS Server will reject the request if truncation is  */
   /*          requested for the OTS Object.                            */
#define OTS_OACP_WRITE_MODE_NONE                               0x00
#define OTS_OACP_WRITE_MODE_TRUNCATE                           0x01

   /* The following defines the valid values that may be set for the    */
   /* Result Code field of Object Action Control Point (OACP)           */
   /* Characteristic Response Value.                                    */
#define OTS_OACP_RESULT_CODE_SUCCESS                           0x01
#define OTS_OACP_RESULT_CODE_OPCODE_NOT_SUPPORTED              0x02
#define OTS_OACP_RESULT_CODE_INVALID_PARAMETER                 0x03
#define OTS_OACP_RESULT_CODE_INSUFFICIENT_RESOURCES            0x04
#define OTS_OACP_RESULT_CODE_INVALID_OBJECT                    0x05
#define OTS_OACP_RESULT_CODE_CHANNEL_UNAVAILABLE               0x06
#define OTS_OACP_RESULT_CODE_UNSUPPORTED_TYPE                  0x07
#define OTS_OACP_RESULT_CODE_PROCEDURE_NOT_PERMITTED           0x08
#define OTS_OACP_RESULT_CODE_OBJECT_LOCKED                     0x09
#define OTS_OACP_RESULT_CODE_OPERATION_FAILED                  0x0A

   /* The following defines the valid values for the Op Code field of   */
   /* the Object List Control Point (OLCP) Characteristic.              */
#define OTS_OLCP_OP_CODE_FIRST                                 0x01
#define OTS_OLCP_OP_CODE_LAST                                  0x02
#define OTS_OLCP_OP_CODE_PREVIOUS                              0x03
#define OTS_OLCP_OP_CODE_NEXT                                  0x04
#define OTS_OLCP_OP_CODE_GO_TO                                 0x05
#define OTS_OLCP_OP_CODE_ORDER                                 0x06
#define OTS_OLCP_OP_CODE_REQUEST_NUMBER_OF_OBJECTS             0x07
#define OTS_OLCP_OP_CODE_CLEAR_MARKING                         0x08
#define OTS_OLCP_OP_CODE_RESPONSE_CODE                         0x70

   /* The following defines the valid values that may be set for the    */
   /* List Sort Order parameter if the OTS_OLCP_OPCODE_ORDER is set for */
   /* the Op Code field of the OTS Object List Control Point (OLCP)     */
   /* Characteristic.                                                   */
   /* * NOTE * Values between ascending and descending are reserved for */
   /*          future use.                                              */
#define OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_NAME              0x01
#define OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_TYPE              0x02
#define OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_CURRENT_SIZE      0x03
#define OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_FIRST_CREATED     0x04
#define OTS_LIST_SORT_ORDER_ASCENDING_OBJECT_LAST_MODIFIED     0x05
#define OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_NAME             0x11
#define OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_TYPE             0x12
#define OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_CURRENT_SIZE     0x13
#define OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_FIRST_CREATED    0x14
#define OTS_LIST_SORT_ORDER_DESCENDING_OBJECT_LAST_MODIFIED    0x15

   /* The following defines the valid values that may be set for the    */
   /* Result Code field of Object List Control Point (OLCP)             */
   /* Characteristic Response Value.                                    */
#define OTS_OLCP_RESULT_CODE_SUCCESS                           0x01
#define OTS_OLCP_RESULT_CODE_OPCODE_NOT_SUPPORTED              0x02
#define OTS_OLCP_RESULT_CODE_INVALID_PARAMETER                 0x03
#define OTS_OLCP_RESULT_CODE_OPERATION_FAILED                  0x04
#define OTS_OLCP_RESULT_CODE_OUT_OF_BOUNDS                     0x05
#define OTS_OLCP_RESULT_CODE_TOO_MANY_OBJECTS                  0x06
#define OTS_OLCP_RESULT_CODE_NO_OBJECT                         0x07
#define OTS_OLCP_RESULT_CODE_OBJECT_ID_NOT_FOUND               0x08

   /* The following defines the valid values that may be set for the    */
   /* Filter field of the OTS Object List Filter Characteristic.        */
#define OTS_OBJECT_LIST_FILTER_NO_FILTER                       0x00
#define OTS_OBJECT_LIST_FILTER_NAME_STARTS_WITH                0x01
#define OTS_OBJECT_LIST_FILTER_NAME_ENDS_WITH                  0x02
#define OTS_OBJECT_LIST_FILTER_NAME_CONTAINS                   0x03
#define OTS_OBJECT_LIST_FILTER_NAME_IS_EXACTLY                 0x04
#define OTS_OBJECT_LIST_FILTER_OBJECT_TYPE                     0x05
#define OTS_OBJECT_LIST_FILTER_CREATED_BETWEEN                 0x06
#define OTS_OBJECT_LIST_FILTER_MODIFIED_BETWEEN                0x07
#define OTS_OBJECT_LIST_FILTER_CURRENT_SIZE_BETWEEN            0x08
#define OTS_OBJECT_LIST_FILTER_ALLOCATED_SIZE_BETWEEN          0x09
#define OTS_OBJECT_LIST_FILTER_MARKED_OBJECTS                  0x0A

   /* The following structure defines format for the OTS Feature        */
   /* Characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Feature_t
{
   NonAlignedDWord_t  OACP_Features;
   NonAlignedDWord_t  OLCP_Features;
} __PACKED_STRUCT_END__ OTS_Feature_t;

#define OTS_FEATURE_SIZE                                 (sizeof(OTS_Feature_t))

   /* The following defines the maximum length for the OTS Object Name  */
   /* Characteristic.                                                   */
#define OTS_MAXIMUM_OBJECT_NAME_LENGTH                   (120)

   /* The following structure defines the format for the OTS Object Size*/
   /* Characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tag_OTS_Object_Size_t
{
   NonAlignedDWord_t Current_Size;
   NonAlignedDWord_t Allocated_Size;
} __PACKED_STRUCT_END__ OTS_Object_Size_t;

#define OTS_OBJECT_SIZE_SIZE                             (sizeof(OTS_Object_Size_t))

   /* The following structure defines the structure for the UINT48 data */
   /* type.                                                             */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_UINT48_t
{
   NonAlignedDWord_t Lower;
   NonAlignedWord_t  Upper;
} __PACKED_STRUCT_END__ OTS_UINT48_t;

#define OTS_UINT48_SIZE                                  (sizeof(OTS_UINT48_t))

   /* The following defines the format for the Object Action Control    */
   /* Point (OACP) Characteristic.                                      */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Object_Action_Control_Point_t
{
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ OTS_Object_Action_Control_Point_t;

#define OTS_OBJECT_ACTION_CONTROL_POINT_SIZE(_x)         (BTPS_STRUCTURE_OFFSET(OTS_Object_Action_Control_Point_t, Variable_Data) + _x)

   /* The following defines the OACP Response Code Op Code.             */
#define OTS_OACP_RESPONSE_CODE_OP_CODE                   (0x60)

   /* The following structure defines the format of the Object Action   */
   /* Control Point (OACP) Characteristic Response Value.               */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_OACP_Response_Value_t
{
   NonAlignedByte_t Response_Code_Op_Code;
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Result_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ OTS_OACP_Response_Value_t;

#define OTS_OACP_RESPONSE_VALUE_SIZE(_x)                 (BTPS_STRUCTURE_OFFSET(OTS_OACP_Response_Value_t, Variable_Data) + _x)

   /* The following defines the format for the Object List Control Point*/
   /* (OLCP) Characteristic.                                            */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Object_List_Control_Point_t
{
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ OTS_Object_List_Control_Point_t;

#define OTS_OBJECT_LIST_CONTROL_POINT_SIZE(_x)           (BTPS_STRUCTURE_OFFSET(OTS_Object_List_Control_Point_t, Variable_Data) + (_x))

   /* The following defines the OLCP Response Code Op Code.             */
#define OTS_OLCP_RESPONSE_CODE_OP_CODE                   (0x70)

   /* The following structure defines the format of the Object List     */
   /* Control Point (OLCP) Characteristic Response Value.               */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_OLCP_Response_Value_t
{
   NonAlignedByte_t Response_Code_Op_Code;
   NonAlignedByte_t Request_Op_Code;
   NonAlignedByte_t Result_Code;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ OTS_OLCP_Response_Value_t;

#define OTS_OLCP_RESPONSE_VALUE_SIZE(_x)                 (BTPS_STRUCTURE_OFFSET(OTS_OLCP_Response_Value_t, Variable_Data) + (_x))

   /* The following structure defines the OTS Date Time range data type.*/
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Date_Time_Range_t
{
   GATT_Date_Time_Characteristic_t Minimum;
   GATT_Date_Time_Characteristic_t Maximum;
} __PACKED_STRUCT_END__ OTS_Date_Time_Range_t;

#define OTS_DATE_TIME_RANGE_SIZE                         (sizeof(OTS_Date_Time_Range_t))

   /* The following structure defines the OTS Size range data type.     */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Size_Range_t
{
   NonAlignedDWord_t Minimum;
   NonAlignedDWord_t Maximum;
} __PACKED_STRUCT_END__ OTS_Size_Range_t;

#define OTS_SIZE_RANGE_SIZE                              (sizeof(OTS_Size_Range_t))

   /* The following structure defines the OTS Object List Filter        */
   /* Characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Object_List_Filter_t
{
   NonAlignedByte_t Filter;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__ OTS_Object_List_Filter_t;

#define OTS_OBJECT_LIST_FILTER_SIZE(_x)                  (BTPS_STRUCTURE_OFFSET(OTS_Object_List_Filter_t, Variable_Data) + _x)

   /* The following defines the valid values that may be set for the    */
   /* Flags field of the OTS Object Changed Characteristic.             */
   /* * NOTE * If OTS_OBJECT_CHANGED_FLAGS_SOURCE_OF_CHANGE_CLIENT is   */
   /*          not present then this indicates that the OTS Object was  */
   /*          changed by the OTS Server.                               */
#define OTS_OBJECT_CHANGED_FLAGS_SOURCE_OF_CHANGE_CLIENT       0x01
#define OTS_OBJECT_CHANGED_FLAGS_OBJECT_CONTENTS_CHANGED       0x02
#define OTS_OBJECT_CHANGED_FLAGS_OBJECT_METADATA_CHANGED       0x04
#define OTS_OBJECT_CHANGED_FLAGS_OBJECT_CREATION               0x08
#define OTS_OBJECT_CHANGED_FLAGS_OBJECT_DELETION               0x10

   /* The following structure defines the OTS Object Changed            */
   /* Characteristic.                                                   */
typedef __PACKED_STRUCT_BEGIN__ struct _tagOTS_Object_Changed_t
{
   NonAlignedByte_t Flags;
   OTS_UINT48_t     Object_ID;
} __PACKED_STRUCT_END__ OTS_Object_Changed_t;

#define OTS_OBJECT_CHANGED_SIZE                          (sizeof(OTS_Object_Changed_t))

   /* The following defines the valid values that may be set for the    */
   /* Flags field of an OTS Object Record.                              */
   /* * NOTE * The OTS_OBJECT_RECORD_FLAGS_TYPE_UUID_SIZE_128 flag will */
   /*          indicate the size of mandatory Object Type field.  This  */
   /*          flag indicates that it is a 128-bit UUID, while not      */
   /*          present indicates that it is 16-bit UUID.                */
#define OTS_OBJECT_RECORD_FLAGS_TYPE_UUID_SIZE_128             0x01
#define OTS_OBJECT_RECORD_FLAGS_CURRENT_SIZE_PRESENT           0x02
#define OTS_OBJECT_RECORD_FLAGS_ALLOCATED_SIZE_PRESENT         0x04
#define OTS_OBJECT_RECORD_FLAGS_FIRST_CREATED_PRESENT          0x08
#define OTS_OBJECT_RECORD_FLAGS_LAST_MODIFIED_PRESENT          0x10
#define OTS_OBJECT_RECORD_FLAGS_PROPERTIES_PRESENT             0x20

   /* The following structure defines an OTS Object Record that is      */
   /* included in the OTS Directory Listing Object.                     */
   /* ** NOTE ** This structure only contains the mandatory fixed length*/
   /*            fields of an OTS Object Record.  This is because the   */
   /*            Object Name field is variable and needs to be stored   */
   /*            before other mandatory fields.                         */
   /* ** NOTE ** The MACRO below may be used to help determine the      */
   /*            Record_Length field for the OTS Object Record.         */
typedef struct __PACKED_STRUCT_BEGIN__ _tagOTS_Object_Record_t
{
   NonAlignedWord_t Record_Length;
   OTS_UINT48_t     Object_ID;
   NonAlignedByte_t Name_Length;
   NonAlignedByte_t Variable_Data[1];
} __PACKED_STRUCT_END__   OTS_Object_Record_t;

#define OTS_OBJECT_RECORD_SIZE(_x)                       (BTPS_STRUCTURE_OFFSET(OTS_Object_Record_t, Variable_Data) + (_x))

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the OTS Object Record length based on the OTS Object  */
   /* Record Flags.  The first parameter to this MACRO is the OTS Object*/
   /* Record Flags.  This parameter will allow us to determine the size */
   /* of the optional fields included in the OTS Object Record.  The    */
   /* second parameter is the OTS Object Record Name Length.            */
   /* * NOTE * Since the OTS Object Record Name Length is a mandatory   */
   /*          field of the OTS Object Record and it is variable, it    */
   /*          will need to be passed as a parameter so we can determine*/
   /*          the correct size of the OTS Object Record.               */
#define OTS_OBJECT_RECORD_LENGTH(_x, _y)                 (OTS_OBJECT_RECORD_SIZE(0)                                                                              + \
                                                         (_y)                                                                                                    + \
                                                         (NON_ALIGNED_BYTE_SIZE)                                                                                 + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_TYPE_UUID_SIZE_128)     ? UUID_128_SIZE : UUID_16_SIZE)                + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_CURRENT_SIZE_PRESENT)   ? NON_ALIGNED_DWORD_SIZE : 0)                  + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_ALLOCATED_SIZE_PRESENT) ? NON_ALIGNED_DWORD_SIZE : 0)                  + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_FIRST_CREATED_PRESENT)  ? GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE : 0) + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_LAST_MODIFIED_PRESENT)  ? GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE : 0) + \
                                                         (((_x) & OTS_OBJECT_RECORD_FLAGS_PROPERTIES_PRESENT)     ? NON_ALIGNED_DWORD_SIZE : 0))

   /* The following defines the maximum number of OTS Object List Filter*/
   /* instances that may be supported by the OTS Server.                */
#define OTS_MAXIMUM_SUPPORTED_OBJECT_LIST_FILTERS        (3)

   /* The following defines the OTS Protocol Service Multiplexer (PSM)  */
   /* value.                                                            */
#define OTS_PROTOCOL_SERVICE_MULTIPLEXER                 (0x0025)

   /* The following defines the valid values that may be set Client     */
   /* Characteristic Configuration Descriptor (CCCD).                   */
   /* * NOTE * Only one value should be set for any CCCD.  This service */
   /*          only allows indications to be enabled, however we include*/
   /*          all for reference.                                       */
#define OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_DISABLED         (0)
#define OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFY_ENABLE    (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
#define OTS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATE_ENABLE  (GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE)

   /* The following defines the valid values that may be set for the OTS*/
   /* Service Flags.                                                    */
#define OTS_SERVICE_FLAGS_LE                             (GATT_SERVICE_FLAGS_LE_SERVICE)
#define OTS_SERVICE_FLAGS_BR_EDR                         (GATT_SERVICE_FLAGS_BR_EDR_SERVICE)
#define OTS_SERVICE_FLAGS_DUAL_MODE                      (GATT_SERVICE_FLAGS_LE_SERVICE | GATT_SERVICE_FLAGS_BR_EDR_SERVICE)

#endif
