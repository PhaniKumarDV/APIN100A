/*****< cscmmsg.h >************************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CSCMMSG - Defined Interprocess Communication Messages for the Cycling     */
/*            Speed and Cadence Profile (CSC) Manager for Stonestreet One     */
/*            Bluetopia Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/07/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __CSCMMSGH__
#define __CSCMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTCSCM.h"           /* CSC Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "CSCMType.h"            /* BTPM CSC Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Cycling Speed   */
   /* and Cadence Profile (CSC) Manager.                                */
#define BTPM_MESSAGE_GROUP_CYCLING_SPEED_CADENCE_MANAGER       0x0000110E

#define CSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS        0x00001001
#define CSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS     0x00001002

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Cycling Speed and*/
   /* Cadence (CSC) Manager.                                            */

   /* Cycling Speed and Cadence Profile (CSC) Manager Commands.         */
#define CSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS          0x00001101
#define CSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR          0x00001102
#define CSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR       0x00001103
#define CSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO        0x00001104
#define CSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION              0x00001105
#define CSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE          0x00001106
#define CSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION           0x00001107

   /* Cycling Speed and Cadence Profile (CSC) Manager Asynchronous      */
   /* Events.                                                           */
#define CSCM_MESSAGE_FUNCTION_CONNECTED                        0x00010001
#define CSCM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010002
#define CSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED     0x00010003

#define CSCM_MESSAGE_FUNCTION_MEASUREMENT                      0x00011001
#define CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE         0x00011002
#define CSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED         0x00011003
#define CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED          0x00011004
#define CSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE               0x00011005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Cycling Speed and       */
   /* Cadence Profile (CSC) Manager.                                    */

   /* Cycling Speed and Cadence Profile (CSC) Manager Command/Response  */
   /* Message Formats.                                                  */

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to register for collector events (Request).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} CSCM_Register_Collector_Events_Request_t;

#define CSCM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(CSCM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to register for collector events (Response).  */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          CallbackID;
} CSCM_Register_Collector_Events_Response_t;

#define CSCM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE           (sizeof(CSCM_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to un-register for collector events (Request).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} CSCM_Un_Register_Collector_Events_Request_t;

#define CSCM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(CSCM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message to un-register for collector events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Un_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Un_Register_Collector_Events_Response_t;

#define CSCM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE        (sizeof(CSCM_Un_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to query connected sensors (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Query_Connected_Sensors_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          MaximumRemoteDeviceListEntries;
} CSCM_Query_Connected_Sensors_Request_t;

#define CSCM_QUERY_CONNECTED_SENSORS_REQUEST_SIZE              (sizeof(CSCM_Query_Connected_Sensors_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to query connected sensors (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_QUERY_CONNECTED_SENSORS         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Query_Connected_Sensors_Response_t
{
   BTPM_Message_Header_t   MessageHeader;
   int                     Status;
   unsigned int            TotalNumberConnectedDevices;
   unsigned int            NumberDevices;
   CSCM_Connected_Sensor_t ConnectedSensors[1];
} CSCM_Query_Connected_Sensors_Response_t;
   
   /* The following macro calculates the total memory required to hold a*/
   /* CSCM_Query_Connected_Sensors_Response message given the number of */
   /* connected sensors specified by the NumberDevices member.          */
#define CSCM_QUERY_CONNECTED_SENSORS_RESPONSE_SIZE(_x)         (sizeof(CSCM_Query_Connected_Sensors_Response_t) + ((_x)*sizeof(CSCM_Connected_Sensor_t)))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to configure a remote sensor (Request).       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Configure_Remote_Sensor_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned long         Flags;
} CSCM_Configure_Remote_Sensor_Request_t;

#define CSCM_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE              (sizeof(CSCM_Configure_Remote_Sensor_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to configure a remote sensor (Response).      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_CONFIGURE_REMOTE_SENSOR         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Configure_Remote_Sensor_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Configure_Remote_Sensor_Response_t;

#define CSCM_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE             (sizeof(CSCM_Configure_Remote_Sensor_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to un-configure a remote sensor (Request).    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Un_Configure_Remote_Sensor_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} CSCM_Un_Configure_Remote_Sensor_Request_t;

#define CSCM_UN_CONFIGURE_REMOTE_SENSOR_REQUEST_SIZE           (sizeof(CSCM_Un_Configure_Remote_Sensor_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to un-configure a remote sensor (Response).   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UN_CONFIGURE_REMOTE_SENSOR      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Un_Configure_Remote_Sensor_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Un_Configure_Remote_Sensor_Response_t;

#define CSCM_UN_CONFIGURE_REMOTE_SENSOR_RESPONSE_SIZE          (sizeof(CSCM_Un_Configure_Remote_Sensor_Response_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message to get info about a connected sensor        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Get_Connected_Sensor_Info_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} CSCM_Get_Connected_Sensor_Info_Request_t;

#define CSCM_GET_CONNECTED_SENSOR_INFO_REQUEST_SIZE            (sizeof(CSCM_Get_Connected_Sensor_Info_Request_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message to get info about a connected sensor        */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_GET_CONNECTED_SENSOR_INFO       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Get_Connected_Sensor_Info_Response_t
{
   BTPM_Message_Header_t   MessageHeader;
   int                     Status;
   CSCM_Connected_Sensor_t ConnectedSensorInfo;
} CSCM_Get_Connected_Sensor_Info_Response_t;

#define CSCM_GET_CONNECTED_SENSOR_INFO_RESPONSE_SIZE           (sizeof(CSCM_Get_Connected_Sensor_Info_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to get the sensor location (Request).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Get_Sensor_Location_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          CallbackID;
} CSCM_Get_Sensor_Location_Request_t;

#define CSCM_GET_SENSOR_LOCATION_REQUEST_SIZE                  (sizeof(CSCM_Get_Sensor_Location_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to get the sensor location (Response).        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_GET_SENSOR_LOCATION             */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Get_Sensor_Location_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Get_Sensor_Location_Response_t;

#define CSCM_GET_SENSOR_LOCATION_RESPONSE_SIZE                 (sizeof(CSCM_Get_Sensor_Location_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to update the cumulative value on a sensor    */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Update_Cumulative_Value_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               CumulativeValue;
} CSCM_Update_Cumulative_Value_Request_t;

#define CSCM_UPDATE_CUMULATIVE_VALUE_REQUEST_SIZE              (sizeof(CSCM_Update_Cumulative_Value_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to update the cumulative value on a sensor    */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UPDATE_CUMULATIVE_VALUE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Update_Cumulative_Value_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Update_Cumulative_Value_Response_t;

#define CSCM_UPDATE_CUMULATIVE_VALUE_RESPONSE_SIZE             (sizeof(CSCM_Update_Cumulative_Value_Response_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to update the sensor location on a remote     */
   /* sensor (Request).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Update_Sensor_Location_Request_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   CSCM_Sensor_Location_t SensorLocation;
} CSCM_Update_Sensor_Location_Request_t;

#define CSCM_UPDATE_SENSOR_LOCATION_REQUEST_SIZE               (sizeof(CSCM_Update_Sensor_Location_Request_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message to update the sensor location on a remote     */
   /* sensor (Response).                                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_UPDATE_SENSOR_LOCATION          */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Update_Sensor_Location_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} CSCM_Update_Sensor_Location_Response_t;

#define CSCM_UPDATE_SENSOR_LOCATION_RESPONSE_SIZE              (sizeof(CSCM_Update_Sensor_Location_Response_t))

   /* CSC Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message that informs the client that remote sensor has*/
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Connected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Configured;
   unsigned long         SupportedOptionalCharateristics;
} CSCM_Connected_Message_t;

#define CSCM_CONNECTED_MESSAGE_SIZE                            (sizeof(CSCM_Connected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message that informs the client that a remote sensor  */
   /* has disconnected (asynchronously).                                */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Disconnected_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
} CSCM_Disconnected_Message_t;

#define CSCM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(CSCM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message that informs the client that the configuration*/
   /* status of a remote sensor has changed (asynchronously).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_CONFIGURATION_STATUS_CHANGED    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Configuration_Status_Changed_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   Boolean_t             Configured;
   unsigned int          Status;
} CSCM_Configuration_Status_Changed_Message_t;

#define CSCM_CONFIGURATION_STATUS_CHANGED_MESSAGE_SIZE         (sizeof(CSCM_Configuration_Status_Changed_Message_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message that informs the client that a measurement    */
   /* notification was received (asynchronously).                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_MEASUREMENT                     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Measurement_Message_t
{
   BTPM_Message_Header_t        MessageHeader;
   BD_ADDR_t                    RemoteDeviceAddress;
   unsigned long                Flags;
   CSCM_Wheel_Revolution_Data_t WheelRevolutionData;
   CSCM_Crank_Revolution_Data_t CrankRevolutionData;
} CSCM_Measurement_Message_t;

#define CSCM_MEASUREMENT_MESSAGE_SIZE                          (sizeof(CSCM_Measurement_Message_t))

   /* The following structure represents the Message definition for a   */
   /* CSC Manager Message that informs the client that a request for the*/
   /* sensor location has completed (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_RESPONSE        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Sensor_Location_Response_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           TransactionID;
   unsigned int           Status;
   CSCM_Sensor_Location_t SensorLocation;
} CSCM_Sensor_Location_Response_Message_t;

#define CSCM_SENSOR_LOCATION_RESPONSE_MESSAGE_SIZE             (sizeof(CSCM_Sensor_Location_Response_Message_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message that informs the client that an update      */
   /* cumulative value control point procedure succesfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_CUMULATIVE_VALUE_UPDATED        */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Cumulative_Value_Updated_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   DWord_t               CumulativeValue;
} CSCM_Cumulative_Value_Updated_Message_t;

#define CSCM_CUMULATIVE_VALUE_UPDATED_MESSAGE_SIZE             (sizeof(CSCM_Cumulative_Value_Updated_Message_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message that informs the client that an update      */
   /* sensor location control point procedure successfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_SENSOR_LOCATION_UPDATED         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Sensor_Location_Updated_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   CSCM_Sensor_Location_t SensorLocation;
} CSCM_Sensor_Location_Updated_Message_t;

#define CSCM_SENSOR_LOCATION_UPDATED_MESSAGE_SIZE              (sizeof(CSCM_Sensor_Location_Updated_Message_t))

   /* The following structure represents the Message definition for     */
   /* a CSC Manager Message that informs the client that an update      */
   /* sensor location control point procedure successfully completed    */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             CSCM_MESSAGE_FUNCTION_PROCEDURE_COMPLETE         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagCSCM_Procedure_Complete_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ProcedureID;
   unsigned int          Status;
   unsigned int          ResponseErrorCode;
} CSCM_Procedure_Complete_Message_t;

#define CSCM_PROCEDURE_COMPLETE_MESSAGE_SIZE                   (sizeof(CSCM_Procedure_Complete_Message_t))

#endif
