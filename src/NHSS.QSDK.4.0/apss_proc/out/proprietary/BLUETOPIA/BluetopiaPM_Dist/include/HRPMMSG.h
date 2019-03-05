/*****< hrpmmsg.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMMSG - Defined Interprocess Communication Messages for the Heart Rate  */
/*            Profile (HRP) Manager for Stonestreet One Bluetopia Protocol    */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __HRPMMSGH__
#define __HRPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTHRPM.h"           /* HRP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "HRPMType.h"            /* BTPM HRP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */

   /* Platform Manager Message Group that specifies the Heart Rate      */
   /* Profile (HRP) Manager.                                            */
#define BTPM_MESSAGE_GROUP_HEART_RATE_MANAGER                      0x00001102

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Heart Rate (HRP) */
   /* Manager.                                                          */

   /* Heart Rate Profile (HRP) Manager Commands.                        */
#define HRPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS            0x00001001
#define HRPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS         0x00001002

#define HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION             0x00001101
#define HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED                0x00001102

   /* Heart Rate Profile (HRP) Manager Asynchronous Events.             */
#define HRPM_MESSAGE_FUNCTION_CONNECTED                            0x00010001
#define HRPM_MESSAGE_FUNCTION_DISCONNECTED                         0x00010002

#define HRPM_MESSAGE_FUNCTION_HEART_RATE_MEASUREMENT               0x00011003
#define HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION_RESPONSE    0x00011004
#define HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED_RESPONSE       0x00011005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Heart Rate Profile (HRP)*/
   /* Manager.                                                          */

   /* Heart Rate Profile (HRP) Manager Manager Command/Response Message */
   /* Formats.                                                          */

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to register for HRP Collector events          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HRPM_Register_Collector_Events_Request_t;

#define HRPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(HRPM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to register for HRP Collector events          */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HRPM_Register_Collector_Events_Response_t;

#define HRPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE           (sizeof(HRPM_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to un-register for HRP Collector events       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} HRPM_Un_Register_Collector_Events_Request_t;

#define HRPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(HRPM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to un-register for HRP Collector events       */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Un_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HRPM_Un_Register_Collector_Events_Response_t;

#define HRPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE        (sizeof(HRPM_Un_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to send the Get Body Sensor Location message  */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Get_Body_Sensor_Location_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HRPM_Get_Body_Sensor_Location_Request_t;

#define HRPM_GET_BODY_SENSOR_LOCATION_REQUEST_SIZE             (sizeof(HRPM_Get_Body_Sensor_Location_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to send the Get Body Sensor Location message  */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Get_Body_Sensor_Location_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HRPM_Get_Body_Sensor_Location_Response_t;

#define HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_SIZE            (sizeof(HRPM_Get_Body_Sensor_Location_Response_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to send Reset Energy Expended message         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Reset_Energy_Expended_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} HRPM_Reset_Energy_Expended_Request_t;

#define HRPM_RESET_ENERGY_EXPENDED_REQUEST_SIZE                (sizeof(HRPM_Reset_Energy_Expended_Request_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message to send Reset Energy Expended message         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED           */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct HRPM_Reset_Energy_Expended_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} HRPM_Reset_Energy_Expended_Response_t;

#define HRPM_RESET_ENERGY_EXPENDED_RESPONSE_SIZE               (sizeof(HRPM_Reset_Energy_Expended_Response_t))

   /* HRP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message that informs the client that a HRP device has */
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HRPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} HRPM_Connected_Message_t;

#define HRPM_CONNECTED_MESSAGE_SIZE                            (sizeof(HRPM_Connected_Message_t))

   /* The following constants are used with the ConnectedFlags member of*/
   /* the HRPM_Connected_Message_t to denote various connection options.*/
#define HRPM_CONNECTED_FLAGS_BODY_SENSOR_LOCATION_SUPPORTED    0x00000001
#define HRPM_CONNECTED_FLAGS_RESET_ENERGY_EXPENDED_SUPPORTED   0x00000002

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message that informs the client that a HRP device has */
   /* disconnected (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   HRPM_Connection_Type_t ConnectionType;
} HRPM_Disconnected_Message_t;

#define HRPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(HRPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message that informs the client that a Heart Rate     */
   /* Measurement has been sent by a sensor (asynchronously).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_HEART_RATE_MEASUREMENT          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Heart_Rate_Measurement_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         MeasurementFlags;
   Word_t                HeartRate;
   Word_t                EnergyExpended;
   Word_t                NumberOfRRIntervals;
   Word_t                RRIntervals[1];
} HRPM_Heart_Rate_Measurement_Message_t;

   /* The following MACRO is provided to allow the programmer a very    */
   /* simple means of quickly determining the total number of Bytes that*/
   /* will be required to hold a an entire Heart Rate Profile Manager   */
   /* Heart Rate Measurement Message given the number of Intervals that */
   /* are present in the list (NOT the number of bytes occupied by the  */
   /* list).  This function accepts as it's input the total number      */
   /* individual Intervals that are present starting from the           */
   /* RRIntervals member of the HRPM_Heart_Rate_Measurement_Message_t   */
   /* structure and returns the total number of bytes required to hold  */
   /* the entire message.                                               */
#define HRPM_HEART_RATE_MEASUREMENT_MESSAGE_SIZE(_x)           (STRUCTURE_OFFSET(HRPM_Heart_Rate_Measurement_Message_t, RRIntervals) + (((unsigned int)(_x)) * sizeof(Word_t)))

   /* The following constants represent the bit-mask values that are    */
   /* possible for the MeasurementFlags member of the                   */
   /* HRPM_Heart_Rate_Measurement_Message_t message.                    */
#define HRPM_HEART_RATE_MEASUREMENT_FLAGS_HEART_RATE_IS_WORD               0x00000001
#define HRPM_HEART_RATE_MEASUREMENT_FLAGS_SENSOR_CONTACT_STATUS_DETECTED   0x00000002
#define HRPM_HEART_RATE_MEASUREMENT_FLAGS_SENSOR_CONTACT_STATUS_SUPPORTED  0x00000004
#define HRPM_HEART_RATE_MEASUREMENT_FLAGS_ENERGY_EXPENDED_PRESENT          0x00000008
#define HRPM_HEART_RATE_MEASUREMENT_FLAGS_RR_INTERVAL_PRESENT              0x00000010

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message that informs the client that a Get Body Sensor*/
   /* Location Response has been sent by a sensor (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*           HRPM_MESSAGE_FUNCTION_GET_BODY_SENSOR_LOCATION_RESPONSE */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Get_Body_Sensor_Location_Response_Message_t
{
   BTPM_Message_Header_t       MessageHeader;
   BD_ADDR_t                   RemoteDeviceAddress;
   int                         Status;
   HRPM_Body_Sensor_Location_t Location;
} HRPM_Get_Body_Sensor_Location_Response_Message_t;

#define HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE    (sizeof(HRPM_Get_Body_Sensor_Location_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* HRP Manager Message that informs the client that a Reset Energy   */
   /* Expended Response has been sent by a sensor (asynchronously).     */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             HRPM_MESSAGE_FUNCTION_RESET_ENERGY_EXPENDED_RESPONSE  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagHRPM_Reset_Energy_Expended_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   int                   Status;
} HRPM_Reset_Energy_Expended_Response_Message_t;

#define HRPM_RESET_ENERGY_EXPENDED_RESPONSE_MESSAGE_SIZE       (sizeof(HRPM_Reset_Energy_Expended_Response_Message_t))

#endif
