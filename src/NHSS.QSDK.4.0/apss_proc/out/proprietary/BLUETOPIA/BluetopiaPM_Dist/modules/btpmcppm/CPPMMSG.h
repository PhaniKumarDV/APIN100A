/*****< CPPMMSG.h >************************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CPPMMSG - Cycling Power profile manager IPC messages                      */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#ifndef __CPPMMSGH__
#define __CPPMMSGH__

#include "BTPMMSGT.h"      /* BTPM Message Type Definitions/Constants.  */

   /* Cycling Power platform manager message group                      */
#define BTPM_MESSAGE_GROUP_CYCLING_POWER_MANAGER                0x0000110D

   /* Cycling Power platform manager message function ID                */

   /*********************************************************************/
   /* Cycling Power profile platform manager commands                   */
   /*********************************************************************/
#define CPPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS         0x00001001
#define CPPM_MESSAGE_FUNCTION_UNREGISTER_COLLECTOR_EVENTS       0x00001002

#define CPPM_MESSAGE_FUNCTION_REGISTER_MEASUREMENTS             0x00001103
#define CPPM_MESSAGE_FUNCTION_UNREGISTER_MEASUREMENTS           0x00001104

#define CPPM_MESSAGE_FUNCTION_REGISTER_VECTORS                  0x00001105
#define CPPM_MESSAGE_FUNCTION_UNREGISTER_VECTORS                0x00001106

#define CPPM_MESSAGE_FUNCTION_REGISTER_PROCEDURES               0x00001107
#define CPPM_MESSAGE_FUNCTION_UNREGISTER_PROCEDURES             0x00001108

#define CPPM_MESSAGE_FUNCTION_ENABLE_BROADCASTS                 0x00001109
#define CPPM_MESSAGE_FUNCTION_DISABLE_BROADCASTS                0x00001110

#define CPPM_MESSAGE_FUNCTION_READ_SENSOR_FEATURES              0x00001111
#define CPPM_MESSAGE_FUNCTION_READ_SENSOR_LOCATION              0x00001112

#define CPPM_MESSAGE_FUNCTION_WRITE_SENSOR_CONTROL_POINT        0x00001113

#define CPPM_MESSAGE_FUNCTION_QUERY_SENSORS                     0x00001114

#define CPPM_MESSAGE_FUNCTION_QUERY_SENSOR_INSTANCES            0x00001115

   /*********************************************************************/
   /* Cycling Power profile platform manager asynchronous Events        */
   /*********************************************************************/
#define CPPM_MESSAGE_FUNCTION_CONNECTED                         0x00010001
#define CPPM_MESSAGE_FUNCTION_DISCONNECTED                      0x00010002

#define CPPM_MESSAGE_FUNCTION_WRITE_RESPONSE                    0x00011003
#define CPPM_MESSAGE_FUNCTION_MEASUREMENT                       0x00011004
#define CPPM_MESSAGE_FUNCTION_VECTOR                            0x00011005
#define CPPM_MESSAGE_FUNCTION_CONTROL_POINT                     0x00011006
#define CPPM_MESSAGE_FUNCTION_SENSOR_FEATURES                   0x00011007
#define CPPM_MESSAGE_FUNCTION_SENSOR_LOCATION                   0x00011008


   /*********************************************************************/
   /* Cycling Power (CPPM) Manager Request-Response Messages            */
   /*********************************************************************/

   /* Message format for the                                            */
   /*                                                                   */
   /*          CPPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS          */
   /*                                                                   */
   /* message request function                                          */
typedef struct _tagCPPM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} CPPM_Register_Collector_Events_Request_t;

#define CPPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE   (sizeof(CPPM_Register_Collector_Events_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_COLLECTOR_EVENTS             */
   /*                                                                   */
   /* request message                                                   */
typedef struct _tagCPPM_Unregister_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} CPPM_Unregister_Collector_Events_Request_t;

#define CPPM_UNREGISTER_COLLECTOR_EVENTS_REQUEST_SIZE   (sizeof(CPPM_Unregister_Collector_Events_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_MEASUREMENTS                   */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_VECTORS                        */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_PROCEDURES                     */
   /*                                                                   */
   /* request messages                                                  */
typedef struct _tagCPPM_Register_Unsolicited_Updates_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   Boolean_t             EnableUnsolicitedUpdates;
} CPPM_Register_Unsolicited_Updates_Request_t;

#define CPPM_REGISTER_UNSOLICITED_UPDATES_REQUEST_SIZE (sizeof(CPPM_Register_Unsolicited_Updates_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_WRITE_SENSOR_CONTROL_POINT              */
   /*                                                                   */
   /* request messages                                                  */
typedef struct _tagCPPM_Write_Sensor_Control_Point_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
   CPPM_Procedure_Data_t ProcedureData;
} CPPM_Write_Sensor_Control_Point_Request_t;

#define CPPM_WRITE_SENSOR_CONTROL_POINT_REQUEST_SIZE (sizeof(CPPM_Write_Sensor_Control_Point_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_QUERY_SENSORS                           */
   /*                                                                   */
   /* request message                                                   */
typedef struct _tagCPPM_Query_Sensors_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   unsigned int           NumberOfSensors;
} CPPM_Query_Sensors_Request_t;

#define CPPM_QUERY_SENSORS_REQUEST_SIZE (sizeof(CPPM_Query_Sensors_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_QUERY_SENSORS                           */
   /*                                                                   */
   /* response message                                                  */
typedef struct _tagCPPM_Query_Sensors_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   int                    Status;
   unsigned int           NumberOfSensors;
   BD_ADDR_t              Sensors[1];
} CPPM_Query_Sensors_Response_t;

#define CPPM_QUERY_SENSORS_RESPONSE_SIZE(_x) (STRUCTURE_OFFSET(CPPM_Query_Sensors_Response_t, Sensors) + (unsigned int)(sizeof(BD_ADDR_t) * ((unsigned int)(_x))))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_QUERY_SENSOR_INSTANCES                  */
   /*                                                                   */
   /* request message                                                   */
typedef struct _tagCPPM_Query_Sensor_Instances_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              Sensor;
   unsigned int           NumberOfInstances;
} CPPM_Query_Sensor_Instances_Request_t;

#define CPPM_QUERY_SENSOR_INSTANCES_REQUEST_SIZE (sizeof(CPPM_Query_Sensor_Instances_Request_t))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_QUERY_SENSOR_INSTANCES                  */
   /*                                                                   */
   /* response message                                                  */
typedef struct _tagCPPM_Query_Sensor_Instances_Response_t
{
   BTPM_Message_Header_t  MessageHeader;
   int                    Status;
   unsigned int           NumberOfInstances;
   Instance_Record_t      Instances[1];
} CPPM_Query_Sensor_Instances_Response_t;

#define CPPM_QUERY_SENSOR_INSTANCES_RESPONSE_SIZE(_x) (STRUCTURE_OFFSET(CPPM_Query_Sensor_Instances_Response_t, Instances) + (unsigned int)(sizeof(Instance_Record_t) * ((unsigned int)(_x))))

   /* Message format for the                                            */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_MEASUREMENTS                 */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_VECTORS                      */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_PROCEDURES                   */
   /*     CPPM_MESSAGE_FUNCTION_ENABLE_BROADCASTS                       */
   /*     CPPM_MESSAGE_FUNCTION_DISABLE_BROADCASTS                      */
   /*     CPPM_MESSAGE_FUNCTION_READ_SENSOR_FEATURES                    */
   /*     CPPM_MESSAGE_FUNCTION_READ_SENSOR_LOCATION                    */
   /*                                                                   */
   /* request messages                                                  */
typedef struct _tagCPPM_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          InstanceID;
} CPPM_Request_t;

#define CPPM_REQUEST_SIZE   (sizeof(CPPM_Request_t))

   /* Message format of the response to the                             */
   /*                                                                   */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS               */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_COLLECTOR_EVENTS             */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_MEASUREMENTS                   */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_MEASUREMENTS                 */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_VECTORS                        */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_VECTORS                      */
   /*     CPPM_MESSAGE_FUNCTION_REGISTER_PROCEDURES                     */
   /*     CPPM_MESSAGE_FUNCTION_UNREGISTER_PROCEDURES                   */
   /*     CPPM_MESSAGE_FUNCTION_ENABLE_BROADCASTS                       */
   /*     CPPM_MESSAGE_FUNCTION_DISABLE_BROADCASTS                      */
   /*     CPPM_MESSAGE_FUNCTION_READ_SENSOR_FEATURES                    */
   /*     CPPM_MESSAGE_FUNCTION_READ_SENSOR_LOCATION                    */
   /*                                                                   */
   /* request messages                                                  */
typedef struct _tagCPPM_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CPPM_Response_t;

#define CPPM_RESPONSE_SIZE   (sizeof(CPPM_Response_t))


   /*********************************************************************/
   /* CPP Manager Asynchronous Message Formats.                         */
   /*********************************************************************/

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   CPPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
   unsigned int           NumberOfInstances;
} CPPM_Connected_Message_t;

#define CPPM_CONNECTED_MESSAGE_SIZE   (sizeof(CPPM_Connected_Message_t))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   CPPM_Connection_Type_t ConnectionType;
} CPPM_Disconnected_Message_t;

#define CPPM_DISCONNECTED_MESSAGE_SIZE   (sizeof(CPPM_Disconnected_Message_t))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_WRITE_RESPONSE                  */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Write_Response_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           TransactionID;
   CPPM_Event_Type_t      EventType;
   int                    Status;
} CPPM_Write_Response_Message_t;

#define CPPM_WRITE_RESPONSE_MESSAGE_SIZE   (sizeof(CPPM_Write_Response_Message_t))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_MEASUREMENT                     */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Measurement_Message_t
{  
   BTPM_Message_Header_t   MessageHeader;
   unsigned int            CallbackID;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            InstanceID;
   CPPM_Measurement_Data_t Measurement;
} CPPM_Measurement_Message_t;

#define CPPM_MEASUREMENT_MESSAGE_SIZE   (sizeof(CPPM_Measurement_Message_t))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_VECTOR                          */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Vector_Message_t
{  
   BTPM_Message_Header_t         MessageHeader;
   unsigned int                  CallbackID;
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  InstanceID;
   Byte_t                        VectorFlags;
   CPPM_Crank_Revolution_Data_t  CrankRevolutionData;
   Word_t                        FirstCrankMeasurementAngle;
   Byte_t                        MagnitudeDataLength;
   SWord_t                       InstantaneousMagnitude[1];
} CPPM_Vector_Message_t;

#define CPPM_VECTOR_MESSAGE_SIZE(_x)   ((STRUCTURE_OFFSET(CPPM_Vector_Message_t, InstantaneousMagnitude)) + (unsigned int)(sizeof(SWord_t) * ((Byte_t)(_x))))

typedef struct _tagCPPM_Supported_Sensor_Locations_Message_t
{
   Byte_t                  NumberOfSensorLocations;
   CPPM_Sensor_Location_t  SensorLocations[1];
} CPPM_Supported_Sensor_Locations_Message_t;

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_CONTROL_POINT                   */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Control_Point_Message_t
{  
   BTPM_Message_Header_t           MessageHeader;
   unsigned int                    CallbackID;
   BD_ADDR_t                       RemoteDeviceAddress;
   unsigned int                    InstanceID;
   Boolean_t                       Timeout;
   CPPM_Procedure_Opcode_t         Opcode;
   CPPM_Procedure_Response_Code_t  ResponseCode;
   unsigned int                    ParameterLength;
   union
   {
      CPPM_Supported_Sensor_Locations_Message_t SupportedSensorLocations;
      CPPM_Date_Time_Data_t                     FactoryCalibrationDate;
      Word_t                                    CrankLength;
      Word_t                                    ChainLength;
      Word_t                                    ChainWeight;
      Word_t                                    SpanLength;
      SWord_t                                   OffsetCompensation;
      Byte_t                                    SamplingRate;
   } Parameter;
} CPPM_Control_Point_Message_t;

#define CPPM_CONTROL_POINT_MESSAGE_SIZE(_x)   ((STRUCTURE_OFFSET(CPPM_Control_Point_Message_t, Parameter)) + (_x))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_SENSOR_FEATURES                 */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Sensor_Features_Message_t
{  
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           TransactionID;
   unsigned long          Features;
   int                    Status;
} CPPM_Sensor_Features_Message_t;

#define CPPM_SENSOR_FEATURES_MESSAGE_SIZE   (sizeof(CPPM_Sensor_Features_Message_t))

   /* Message format of the                                             */
   /*                                                                   */
   /*             CPPM_MESSAGE_FUNCTION_SENSOR_LOCATION                 */
   /*                                                                   */
   /* event message                                                     */
typedef struct _tagCPPM_Sensor_Location_Message_t
{
   BTPM_Message_Header_t   MessageHeader;
   unsigned int            CallbackID;
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            InstanceID;
   unsigned int            TransactionID;
   CPPM_Sensor_Location_t  Location;
   int                     Status;
} CPPM_Sensor_Location_Message_t;

#define CPPM_SENSOR_LOCATION_MESSAGE_SIZE   (sizeof(CPPM_Sensor_Location_Message_t))

#endif
