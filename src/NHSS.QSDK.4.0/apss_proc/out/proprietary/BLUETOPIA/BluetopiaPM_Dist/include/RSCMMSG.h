/*****< rscmmsg.h >************************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  RSCMMSG - Defined Interprocess Communication Messages for the Running     */
/*            Speed and Cadence Profile (RSC) Manager for Stonestreet One     */
/*            Bluetopia Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/18/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __RSCMMSGH__
#define __RSCMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTRSCM.h"           /* RSC Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "RSCMType.h"            /* BTPM RSC Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Running Speed   */
   /* and Cadence Profile (RSC) Manager.                                */
#define BTPM_MESSAGE_GROUP_RUNNING_SPEED_CADENCE_MANAGER       0x0000110F

#define RSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS        0x00001001
#define RSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS     0x00001002

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Running Speed and*/
   /* Cadence (RSC) Manager.                                            */

   /* Running Speed and Cadence Profile (RSC) Manager Commands.         */
#define RSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS          0x00001101
#define RSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR          0x00001102
#define RSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR       0x00001103
#define RSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO        0x00001104
#define RSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION              0x00001105
#define RSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE          0x00001106
#define RSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION           0x00001107
#define RSCM_MESSAGE_FUNCTION_START_SENSOR_CALIBRATION         0x00001108

   /* Running Speed and Cadence Profile (RSC) Manager Asynchronous      */
   /* Events.                                                           */
#define RSCM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define RSCM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002
#define RSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED     0x00010003

#define RSCM_MESSAGE_FUNCTION_MEASUREMENT                      0x00011001
#define RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE         0x00011002
#define RSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED         0x00011003
#define RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED          0x00011004
#define RSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE               0x00011005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Running Speed and       */
   /* Cadence Profile (RSC) Manager.                                    */

   /* Running Speed and Cadence Profile (RSC) Manager Command/Response  */
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to register for collector events (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} RSCM_Register_Collector_Events_Request_t;

#define RSCM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(RSCM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to register for collector events (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          CallbackID;
} RSCM_Register_Collector_Events_Response_t;

#define RSCM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE           (sizeof(RSCM_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to un-register for collector events (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} RSCM_Un_Register_Collector_Events_Request_t;

#define RSCM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(RSCM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message to un-register for collector events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Un_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Un_Register_Collector_Events_Response_t;

#define RSCM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE        (sizeof(RSCM_Un_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to query connected sensors (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Query_Connected_Sensors_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MaximumRemoteDeviceListEntries;
} RSCM_Query_Connected_Sensors_Request_t;

#define RSCM_QUERY_CONNECTED_SENSORS_REQUEST_SIZE              (sizeof(RSCM_Query_Connected_Sensors_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to query connected sensors (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Query_Connected_Sensors_Response_t
{
   BTPM_Message_Header_t   MessageHeader;
   int                     Status;
   unsigned int            TotalNumberConnectedDevices;
   unsigned int            NumberDevices;
   RSCM_Connected_Sensor_t ConnectedSensors[1];
} RSCM_Query_Connected_Sensors_Response_t;
   
   /* The following macro calculates the total memory required to hold a*/
   /* RSCM_Query_Connected_Sensors_Response message given the number of */
   /* connected sensors specified by the NumberDevices member.          */
#define RSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(_x)         (sizeof(RSCM_Query_Connected_Sensors_Response_t) + ((_x)*sizeof(RSCM_Connected_Sensor_t)))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to configure a remote sensor (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Configure_Remote_Sensor_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         Flags;
} RSCM_Configure_Remote_Sensor_Request_t;

#define RSCM_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE              (sizeof(RSCM_Configure_Remote_Sensor_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to configure a remote sensor (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Configure_Remote_Sensor_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Configure_Remote_Sensor_Response_t;

#define RSCM_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE             (sizeof(RSCM_Configure_Remote_Sensor_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to un-configure a remote sensor (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Un_Configure_Remote_Sensor_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} RSCM_Un_Configure_Remote_Sensor_Request_t;

#define RSCM_UN_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE           (sizeof(RSCM_Un_Configure_Remote_Sensor_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to un-configure a remote sensor (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Un_Configure_Remote_Sensor_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Un_Configure_Remote_Sensor_Response_t;

#define RSCM_UN_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE          (sizeof(RSCM_Un_Configure_Remote_Sensor_Response_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message to get info about a connected sensor        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Get_Connected_Sensor_Info_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} RSCM_Get_Connected_Sensor_Info_Request_t;

#define RSCM_GET_CONNECTED_SENSOR_INFO_REQUEST_SIZE            (sizeof(RSCM_Get_Connected_Sensor_Info_Request_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message to get info about a connected sensor        */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Get_Connected_Sensor_Info_Response_t
{
   BTPM_Message_Header_t   MessageHeader;
   int                     Status;
   RSCM_Connected_Sensor_t ConnectedSensorInfo;
} RSCM_Get_Connected_Sensor_Info_Response_t;

#define RSCM_GET_CONNECTED_SENSOR_INFO_RESPONSE_SIZE           (sizeof(RSCM_Get_Connected_Sensor_Info_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to get the sensor location (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Get_Sensor_Location_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          CallbackID;
} RSCM_Get_Sensor_Location_Request_t;

#define RSCM_GET_SENSOR_LOCATION_REQUEST_SIZE                  (sizeof(RSCM_Get_Sensor_Location_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to get the sensor location (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Get_Sensor_Location_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Get_Sensor_Location_Response_t;

#define RSCM_GET_SENSOR_LOCATION_RESPONSE_SIZE                 (sizeof(RSCM_Get_Sensor_Location_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to update the cumulative value on a sensor    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Update_Cumulative_Value_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               CumulativeValue;
} RSCM_Update_Cumulative_Value_Request_t;

#define RSCM_UPDATE_CUMULATIVE_VALUE_REQUEST_SIZE              (sizeof(RSCM_Update_Cumulative_Value_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to update the cumulative value on a sensor    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Update_Cumulative_Value_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Update_Cumulative_Value_Response_t;

#define RSCM_UPDATE_CUMULATIVE_VALUE_RESPONSE_SIZE             (sizeof(RSCM_Update_Cumulative_Value_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to update the sensor location on a remote     */
   /* sensor (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Update_Sensor_Location_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   RSCM_Sensor_Location_t SensorLocation;
} RSCM_Update_Sensor_Location_Request_t;

#define RSCM_UPDATE_SENSOR_LOCATION_REQUEST_SIZE               (sizeof(RSCM_Update_Sensor_Location_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to update the sensor location on a remote     */
   /* sensor (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Update_Sensor_Location_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Update_Sensor_Location_Response_t;

#define RSCM_UPDATE_SENSOR_LOCATION_RESPONSE_SIZE              (sizeof(RSCM_Update_Sensor_Location_Response_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to start sensor calibration on a remote sensor*/
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_START_SENSOR_CALIBRATION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Start_Sensor_Calibration_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
} RSCM_Start_Sensor_Calibration_Request_t;

#define RSCM_START_SENSOR_CALIBRATION_REQUEST_SIZE             (sizeof(RSCM_Start_Sensor_Calibration_Request_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message to start sensor calibration on a remote sensor*/
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_START_SENSOR_CALIBRATION        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Start_Sensor_Calibration_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} RSCM_Start_Sensor_Calibration_Response_t;

#define RSCM_START_SENSOR_CALIBRATION_RESPONSE_SIZE            (sizeof(RSCM_Start_Sensor_Calibration_Response_t))

   /* RSC Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message that informs the client that remote sensor has*/
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Configured;
   unsigned long         SupportedOptionalCharateristics;
} RSCM_Connected_Message_t;

#define RSCM_CONNECTED_MESSAGE_SIZE                            (sizeof(RSCM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message that informs the client that a remote sensor  */
   /* has disconnected (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} RSCM_Disconnected_Message_t;

#define RSCM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(RSCM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message that informs the client that the configuration*/
   /* status of a remote sensor has changed (asynchronously).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Configuration_Status_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Configured;
   unsigned int          Status;
} RSCM_Configuration_Status_Changed_Message_t;

#define RSCM_CONFIGURATION_STATUS_CHANGED_MESSAGE_SIZE         (sizeof(RSCM_Configuration_Status_Changed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message that informs the client that a measurement    */
   /* notification was received (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_MEASUREMENT                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Measurement_Message_t
{
   BTPM_Message_Header_t        MessageHeader;
   BD_ADDR_t                    RemoteDeviceAddress;
   RSCM_Measurement_Data_t      MeasurementData;
} RSCM_Measurement_Message_t;

#define RSCM_MEASUREMENT_MESSAGE_SIZE                          (sizeof(RSCM_Measurement_Message_t))

   /* The following structure represents the Message definition for a   */
   /* RSC Manager Message that informs the client that a request for the*/
   /* sensor location has completed (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Sensor_Location_Response_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           TransactionID;
   unsigned int           Status;
   RSCM_Sensor_Location_t SensorLocation;
} RSCM_Sensor_Location_Response_Message_t;

#define RSCM_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE             (sizeof(RSCM_Sensor_Location_Response_Message_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message that informs the client that an update      */
   /* cumulative value control point procedure succesfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Cumulative_Value_Updated_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               CumulativeValue;
} RSCM_Cumulative_Value_Updated_Message_t;

#define RSCM_CUMULATIVE_VALUE_UPDATED_MESSAGE_SIZE             (sizeof(RSCM_Cumulative_Value_Updated_Message_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message that informs the client that an update      */
   /* sensor location control point procedure successfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Sensor_Location_Updated_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   RSCM_Sensor_Location_t SensorLocation;
} RSCM_Sensor_Location_Updated_Message_t;

#define RSCM_SENSOR_LOCATION_UPDATED_MESSAGE_SIZE              (sizeof(RSCM_Sensor_Location_Updated_Message_t))

   /* The following structure represents the Message definition for     */
   /* a RSC Manager Message that informs the client that an update      */
   /* sensor location control point procedure successfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             RSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagRSCM_Procedure_Complete_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ProcedureID;
   unsigned int          Status;
   unsigned int          ResponseErrorCode;
} RSCM_Procedure_Complete_Message_t;

#define RSCM_PROCEDURE_COMPLETE_MESSAGE_SIZE                   (sizeof(RSCM_Procedure_Complete_Message_t))

#endif
