/*****< htpmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMMSG - Defined Interprocess Communication Messages for the Health      */
/*            Thermometer Profile (HTP) Manager for Stonestreet One Bluetopia */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HTPMMSGH__
#define __HTPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHTPM.h"           /* HTP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HTPMType.h"            /* BTPM HTP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Health          */
   /* Thermometer Profile (HTP) Manager.                                */
#define BTPM_MESSAGE_GROUP_HEALTH_THERMOMETER_MANAGER                         0x0000110A

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Health           */
   /* Thermometer Profile (HTP) Manager.                                */

   /* Health Thermometer Profile (HTP) Manager Commands.                */
#define HTPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS                       0x00001001
#define HTPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS                    0x00001002

#define HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE                            0x00001101
#define HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL                        0x00001102
#define HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL                        0x00001103
#define HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE            0x00001104

   /* Health Thermometer Profile (HTP) Manager Asynchronous Events.     */
#define HTPM_MESSAGE_FUNCTION_CONNECTED                                       0x00010001
#define HTPM_MESSAGE_FUNCTION_DISCONNECTED                                    0x00010002

#define HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE_RESPONSE                   0x00011001
#define HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_RESPONSE               0x00011002
#define HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL_RESPONSE               0x00011003
#define HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE   0x00011004

#define HTPM_MESSAGE_FUNCTION_TEMPERATURE_MEASUREMENT                         0x00012001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Health Thermometer      */
   /* Profile (HTP) Manager.                                            */

   /* Health Thermometer Profile (HTP) Manager Manager Command/Response */
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to register for HTP Collector events          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HTPM_Register_Collector_Events_Request_t;

#define HTPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(HTPM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to register for HTP Collector events          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          HTPCollectorEventsHandlerID;
} HTPM_Register_Collector_Events_Response_t;

#define HTPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE           (sizeof(HTPM_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to un-register for HTP Collector events       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HTPCollectorEventsHandlerID;
} HTPM_Un_Register_Collector_Events_Request_t;

#define HTPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(HTPM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to un-register for HTP Collector events       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Un_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HTPM_Un_Register_Collector_Events_Response_t;

#define HTPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE        (sizeof(HTPM_Un_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Temperature Type message      */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Temperature_Type_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HTPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HTPM_Get_Temperature_Type_Request_t;

#define HTPM_GET_TEMPERATURE_TYPE_REQUEST_SIZE                 (sizeof(HTPM_Get_Temperature_Type_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Temperature Type message      */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE            */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Temperature_Type_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HTPM_Get_Temperature_Type_Response_t;

#define HTPM_GET_TEMPERATURE_TYPE_RESPONSE_SIZE                (sizeof(HTPM_Get_Temperature_Type_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Measurement Interval message  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HTPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HTPM_Get_Measurement_Interval_Request_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_REQUEST_SIZE                 (sizeof(HTPM_Get_Measurement_Interval_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Measurement Interval message  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HTPM_Get_Measurement_Interval_Response_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_SIZE                (sizeof(HTPM_Get_Measurement_Interval_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Set Measurement Interval message  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Set_Measurement_Interval_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HTPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          MeasurementInterval;
} HTPM_Set_Measurement_Interval_Request_t;

#define HTPM_SET_MEASUREMENT_INTERVAL_REQUEST_SIZE                 (sizeof(HTPM_Set_Measurement_Interval_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Set Measurement Interval message  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Set_Measurement_Interval_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HTPM_Set_Measurement_Interval_Response_t;

#define HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_SIZE                (sizeof(HTPM_Set_Measurement_Interval_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Measurement Interval Valid    */
   /* Range message (Request).                                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Valid_Range_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          HTPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
} HTPM_Get_Measurement_Interval_Valid_Range_Request_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_REQUEST_SIZE    (sizeof(HTPM_Get_Measurement_Interval_Valid_Range_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message to send the Get Measurement Interval Valid    */
   /* Range message (Response).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*       HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_VALID_RANGE  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Valid_Range_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          TransactionID;
} HTPM_Get_Measurement_Interval_Valid_Range_Response_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_SIZE   (sizeof(HTPM_Get_Measurement_Interval_Valid_Range_Response_t))

   /* HTP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a HTP device has */
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HTPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} HTPM_Connected_Message_t;

#define HTPM_CONNECTED_MESSAGE_SIZE                            (sizeof(HTPM_Connected_Message_t))

   /* The following constants are used with the ConnectedFlags member of*/
   /* the HTPM_Connected_Message_t to denote various connection options.*/
#define HTPM_CONNECTED_FLAGS_GET_TEMPERATURE_TYPE_SUPPORTED       0x00000001
#define HTPM_CONNECTED_FLAGS_GET_MEASUREMENT_INTERVAL_SUPPORTED   0x00000002
#define HTPM_CONNECTED_FLAGS_SET_MEASUREMENT_INTERVAL_SUPPORTED   0x00000004
#define HTPM_CONNECTED_FLAGS_INTERMEDIATE_TEMPERATURE_SUPPORTED   0x00000008

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a HTP device has */
   /* disconnected (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HTPM_Connection_Type_t ConnectionType;
} HTPM_Disconnected_Message_t;

#define HTPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(HTPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a Get Temperature*/
   /* Type Response has been sent by a sensor (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_TEMPERATURE_TYPE_RESPONSE   */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Temperature_Type_Response_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            TransactionID;
   Boolean_t               Success;
   Byte_t                  AttributeErrorCode;
   HTPM_Temperature_Type_t TemperatureType;
} HTPM_Get_Temperature_Type_Response_Message_t;

#define HTPM_GET_TEMPERATURE_TYPE_RESPONSE_MESSAGE_SIZE        (sizeof(HTPM_Get_Temperature_Type_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a Get Measurement*/
   /* Interval Response has been sent by a sensor (asynchronously).     */
   /* * NOTE * The TransactionID member may be set to ZERO to indicate  */
   /*          that this event was generated asynchronously by the HTP  */
   /*          Sensor.                                                  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Response_Message_t
{
   BTPM_Message_Header_t               MessageHeader;
   BD_ADDR_t                           RemoteDeviceAddress;
   unsigned int                        TransactionID;
   Boolean_t                           Success;
   Byte_t                              AttributeErrorCode;
   unsigned int                        MeasurementInterval;
} HTPM_Get_Measurement_Interval_Response_Message_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE    (sizeof(HTPM_Get_Measurement_Interval_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a Set Measurement*/
   /* Interval Response has been sent by a sensor (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           HTPM_MESSAGE_FUNCTION_SET_MEASUREMENT_INTERVAL_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Set_Measurement_Interval_Response_Message_t
{
   BTPM_Message_Header_t               MessageHeader;
   BD_ADDR_t                           RemoteDeviceAddress;
   unsigned int                        TransactionID;
   Boolean_t                           Success;
   Byte_t                              AttributeErrorCode;
} HTPM_Set_Measurement_Interval_Response_Message_t;

#define HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_MESSAGE_SIZE    (sizeof(HTPM_Set_Measurement_Interval_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a Get Measurement*/
   /* Interval Response has been sent by a sensor (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_GET_MEASUREMENT_INTERVAL_       */
   /*             VALID_RANGE_RESPONSE                                  */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t
{
   BTPM_Message_Header_t               MessageHeader;
   BD_ADDR_t                           RemoteDeviceAddress;
   unsigned int                        TransactionID;
   Boolean_t                           Success;
   Byte_t                              AttributeErrorCode;
   unsigned int                        LowerBounds;
   unsigned int                        UpperBounds;
} HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_MESSAGE_SIZE    (sizeof(HTPM_Get_Measurement_Interval_Valid_Range_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HTP Manager Message that informs the client that a Health         */
   /* Thermometer Measurement has been sent by a sensor                 */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HTPM_MESSAGE_FUNCTION_TEMPERATURE_MEASUREMENT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHTPM_Temperature_Measurement_Message_t
{
   BTPM_Message_Header_t               MessageHeader;
   BD_ADDR_t                           RemoteDeviceAddress;
   HTPM_Temperature_Measurement_Type_t MeasurementType;
   unsigned long                       MeasurementFlags;
   HTPM_Temperature_Type_t             TemperatureType;
   HTPM_Time_Stamp_Data_t              TimeStamp;
   long                                TemperatureMantissa;
   int                                 TemperatureExponent;
} HTPM_Temperature_Measurement_Message_t;

#define HTPM_TEMPERATURE_MEASUREMENT_MESSAGE_SIZE              (sizeof(HTPM_Temperature_Measurement_Message_t))

   /* The following constants are used with the MeasurementFlags member */
   /* of the HTPM_Temperature_Measurement_Message_t to denote various   */
   /* connection options.                                               */
#define HTPM_MEASUREMENT_FLAGS_TEMPERATURE_FAHRENHEIT          0x00000001
#define HTPM_MEASUREMENT_FLAGS_TIME_STAMP_VALID                0x00000002
#define HTPM_MEASUREMENT_FLAGS_TEMPERATURE_TYPE_VALID          0x00000004
#define HTPM_MEASUREMENT_FLAGS_TEMPERATURE_VALUE_NAN           0x00000008

#endif
