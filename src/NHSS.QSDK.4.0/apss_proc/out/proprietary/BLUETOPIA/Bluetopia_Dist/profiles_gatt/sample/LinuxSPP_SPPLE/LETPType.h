/*****< letptype.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LETPTYPE - Embedded Bluetooth Service for measuring LE Throughput Types   */
/*             File.                                                          */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/22/15  Tim Thomas     Initial creation.                               */
/******************************************************************************/
#ifndef __LETPTYPEH__
#define __LETPTYPEH__

#include "SS1BTPS.h"

   /* The following defines the LETP Service UUID that is used when     */
   /* building the LETP Service Table.                                  */
#define LETP_SERVICE_UUID_CONSTANT                             { 0x10, 0x00, 0x4C, 0xB5, 0xBD, 0x01, 0x90, 0x01, 0x48, 0x01, 0xB4, 0x03, 0x10, 0x09, 0x7E, 0x87 }

   /* The following MACRO is a utility MACRO that assigns the LETP      */
   /* Service 16 bit UUID to the specified UUID_128_t variable.  This   */
   /* MACRO accepts one parameter which is a pointer to a UUID_128_t    */
   /* variable that is to receive the LETP UUID Constant value.         */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define LETP_ASSIGN_LETP_SERVICE_UUID_128(_x)                  ASSIGN_BLUETOOTH_UUID_128((_x), 0x87, 0x7E, 0x09, 0x10, 0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD, 0xB5, 0x4C, 0x00, 0x10)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LETP Service UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* LETP Service UUID (MACRO returns boolean result) NOT less         */
   /* than/(greater 4.1) than.  The first parameter is the UUID_128_t   */
   /* variable to compare to the LETP Service UUID.                     */
#define LETP_COMPARE_LETP_SERVICE_UUID_TO_UUID_128(_x)         COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x87, 0x7E, 0x09, 0x10, 0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD, 0xB5, 0x4C, 0x00, 0x10)

   /* The following define the LETP RX Bulk Characteristic UUID that is */
   /* used when performing throughput testin.                           */
#define LETP_TX_BULK_CHARACTERISTIC_UUID_CONSTANT              { 0xBD, 0x01, 0x90, 0x01, 0x48, 0x01, 0xB4, 0x03, 0x10, 0x01, 0x7B, 0x6F, 0xBD, 0x01, 0x90, 0x01 }

   /* The following MACRO is a utility MACRO that assigns the LETP      */
   /* TX BULK Characteristic 16 bit UUID to the specified UUID_128_t    */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the LETP TX BULK UUID      */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define LETP_ASSIGN_TX_BULK_UUID_128(_x)                       ASSIGN_BLUETOOTH_UUID_128((_x), 0x01, 0x90, 0x01, 0xBD, 0x6F, 0x7B, 0x01, 0x10, 0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LETP TX BULK UUID in UUID16 form.  This    */
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* TX BULK UUID (MACRO returns boolean result) NOT less than/(greater*/
   /* 4.1) than.  The first parameter is the UUID_128_t variable to     */
   /* compare to the LETP TX BULK UUID.                                 */
#define LETP_COMPARE_LETP_TX_BULK_UUID_TO_UUID_128(_x)         COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x01, 0x90, 0x01, 0xBD, 0x6F, 0x7B, 0x01, 0x10, 0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD)

   /* The following define the LETP TX Interval Characteristic UUID that*/
   /* is used when performing throughput testin.                        */
#define LETP_TX_INTERVAL_CHARACTERISTIC_UUID_CONSTANT          { 0x6C, 0x3A, 0x92, 0x03, 0x10, 0x81, 0x2D, 0x84, 0xD0, 0x99, 0x11, 0x90, 0x9B, 0xC0, 0x82, 0x69 }

   /* The following MACRO is a utility MACRO that assigns the LETP TX   */
   /* Interval Characteristic 16 bit UUID to the specified UUID_128_t   */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the LETP TX Interval UUID  */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define LETP_ASSIGN_TX_INTERVAL_UUID_128(_x)                   ASSIGN_BLUETOOTH_UUID_128((_x), 0x69, 0x82, 0xC0, 0x9B, 0x90, 0x11, 0x99, 0xD0, 0x84, 0x2D, 0x81, 0x10, 0x03, 0x92, 0x3A, 0x6C)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined LETP TX Interval UUID in UUID16 form.  This*/
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* TX Interval UUID (MACRO returns boolean result) NOT less          */
   /* than/(greater 4.1) than.  The first parameter is the UUID_128_t   */
   /* variable to compare to the LETP TX Interval UUID.                 */
#define LETP_COMPARE_LETP_TX_INTERVAL_UUID_TO_UUID_128(_x)     COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x69, 0x82, 0xC0, 0x9B, 0x90, 0x11, 0x99, 0xD0, 0x84, 0x2D, 0x81, 0x10, 0x03, 0x92, 0x3A, 0x6C)

   /* The following defines the structure that holds all of the LETP    */
   /* Characteristic Handles that need to be cached by a LETP Client.   */
typedef struct _tagLETP_Client_Info_t
{
   Word_t Tx_Interval_Characteristic;
   Word_t Tx_Bulk_Characteristic;
   Word_t Tx_Bulk_Client_Configuration_Descriptor;
} LETP_Client_Info_t;

#define LETP_CLIENT_INFO_DATA_SIZE                      (sizeof(LETP_Client_Info_t))

#define LETP_CLIENT_INFORMATION_VALID(_x)               (((_x).Tx_Interval_Characteristic) && ((_x).Tx_Bulk_Characteristic) && ((_x).Tx_Bulk_Client_Configuration_Descriptor))

   /* The following defines the length of the Client Characteristic     */
   /* Configuration Descriptor.                                         */
#define LETP_CLIENT_CHARACTERISTIC_CONFIGURATION_VALUE_LENGTH (WORD_SIZE)

   /* The following defines the structure that holds the information    */
   /* that needs to be cached by a LETP Server for EACH paired LETP     */
   /* Client.                                                           */
typedef struct _tagLETP_Server_Info_t
{
   Word_t  Tx_Interval_Value;
   DWord_t Tx_Bulk_Value;
   Word_t  Tx_Bulk_Client_Configuration;
} LETP_Server_Info_t;

#define LETP_SERVER_INFO_DATA_SIZE                      (sizeof(LETP_Server_Info_t))

   /* The following defines the LETP GATT Service Flags MASK that should*/
   /* be passed into GATT_Register_Service when the HRS Service is      */
   /* registered.                                                       */
#define LETP_SERVICE_FLAGS                              (GATT_SERVICE_FLAGS_LE_SERVICE)

#endif

