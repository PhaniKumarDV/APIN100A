/*****< glpmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMMSG - Defined Interprocess Communication Messages for the Glucose     */
/*            Profile (GLPM) Manager for Stonestreet One Bluetopia Protocol   */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GLPMMSGH__
#define __GLPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTGLPM.h"           /* GLPM Framework Prototypes/Constants.      */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "GLPMType.h"            /* BTPM GLP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */
   /* Platform Manager Message Group that specifies the Gluocse Profile */
   /* (GLPM) Manager.                                                   */
#define BTPM_MESSAGE_GROUP_GLUCOSE_MANAGER                     0x0000110B

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Glucose Profile  */
   /* (GLPM) Manager.                                                   */

   /* Glucose Profile (GLPM) Manager Commands.                          */
#define GLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS        0x00001001
#define GLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS     0x00001002

#define GLPM_MESSAGE_FUNCTION_START_PROCEDURE                  0x00002001
#define GLPM_MESSAGE_FUNCTION_STOP_PROCEDURE                   0x00002002

   /* Glucose Profile (GLPM) Manager Asynchronous Events.               */
#define GLPM_MESSAGE_FUNCTION_CONNECTED                        0x00010002
#define GLPM_MESSAGE_FUNCTION_DISCONNECTED                     0x00010003

#define GLPM_MESSAGE_FUNCTION_PROCEDURE_STARTED_EVENT          0x00011001
#define GLPM_MESSAGE_FUNCTION_PROCEDURE_STOPPED_EVENT          0x00011002

#define GLPM_MESSAGE_FUNCTION_GLUCOSE_MEASUREMENT_EVENT        0x00020001

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Glucose Profile (GLPM)  */
   /* Manager.                                                          */

   /* Gluocse Profile (GLPM) Manager Manager Command/Response Message   */
   /* Formats.                                                          */

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to register for GLP Manager events (Request). */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} GLPM_Register_Collector_Events_Request_t;

#define GLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(GLPM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to register for GLP Manager events (Response).*/
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          GLPEventsHandlerID;
} GLPM_Register_Collector_Events_Response_t;

#define GLPM_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE           (sizeof(GLPM_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to un-register for GLP Manager events         */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GLPEventsHandlerID;
} GLPM_Un_Register_Collector_Events_Request_t;

#define GLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(GLPM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to un-register for GLP Manager events         */
   /* (Response).                                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Un_Register_Collector_Events_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GLPM_Un_Register_Collector_Events_Response_t;

#define GLPM_UN_REGISTER_COLLECTOR_EVENTS_RESPONSE_SIZE        (sizeof(GLPM_Un_Register_Collector_Events_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to start a Glucose Procedure to a remote      */
   /* Glucose Device (Request).                                         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_START_PROCEDURE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Start_Procedure_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GLPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   GLPM_Procedure_Data_t ProcedureData;
} GLPM_Start_Procedure_Request_t;

#define GLPM_START_PROCEDURE_REQUEST_SIZE                      (sizeof(GLPM_Start_Procedure_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to start a Glucose Procedure to a remote      */
   /* Glucose Device (Response).                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_START_PROCEDURE                 */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Start_Procedure_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
   unsigned int          ProcedureID;
} GLPM_Start_Procedure_Response_t;

#define GLPM_START_PROCEDURE_RESPONSE_SIZE                     (sizeof(GLPM_Start_Procedure_Response_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to Stop a Glucose Procedure to a remote       */
   /* Glucose Device that is current in progress (Request).             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_STOP_PROCEDURE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Stop_Procedure_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          GLPCollectorEventsHandlerID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ProcedureID;
} GLPM_Stop_Procedure_Request_t;

#define GLPM_STOP_PROCEDURE_REQUEST_SIZE                       (sizeof(GLPM_Stop_Procedure_Request_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message to Stop a Glucose Procedure to a remote       */
   /* Glucose Device that is current in progress (Response).            */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_STOP_PROCEDURE                  */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Stop_Procedure_Response_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} GLPM_Stop_Procedure_Response_t;

#define GLPM_STOP_PROCEDURE_RESPONSE_SIZE                      (sizeof(GLPM_Stop_Procedure_Response_t))

   /* GLP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message that informs the client that a Glucose device */
   /* is currently connected (asynchronously).                          */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   GLPM_Connection_Type_t ConnectionType;
   unsigned long          SupportedFeatures;
} GLPM_Connected_Message_t;

#define GLPM_CONNECTED_MESSAGE_SIZE                            (sizeof(GLPM_Connected_Message_t))

   /* The following define the valid bits that may be set in the        */
   /* SupportedFeatures member of the GLPM_Connected_Message_t          */
   /* structure.                                                        */
#define GLPM_SUPPORTED_FEATURES_LOW_BATTERY_DETECTION_DURING_MEASUREMENT   0x00000001
#define GLPM_SUPPORTED_FEATURES_SENSOR_MALFUNCTION_DETECTION               0x00000002
#define GLPM_SUPPORTED_FEATURES_SENSOR_SAMPLE_SIZE                         0x00000004
#define GLPM_SUPPORTED_FEATURES_SENSOR_STRIP_INSERTION_ERROR_DETECTION     0x00000008
#define GLPM_SUPPORTED_FEATURES_SENSOR_STRIP_TYPE_ERROR_DETECTION          0x00000010
#define GLPM_SUPPORTED_FEATURES_SENSOR_RESULT_HIGH_LOW_DETECTION           0x00000020
#define GLPM_SUPPORTED_FEATURES_SENSOR_TEMPERATURE_HIGH_LOW_DETECTION      0x00000040
#define GLPM_SUPPORTED_FEATURES_SENSOR_READ_INTERRUPT_DETECTION            0x00000080
#define GLPM_SUPPORTED_FEATURES_GENERAL_DEVICE_FAULT                       0x00000100
#define GLPM_SUPPORTED_FEATURES_TIME_FAULT                                 0x00000200
#define GLPM_SUPPORTED_FEATURES_MULTIPLE_BOND_SUPPORT                      0x00000400
#define GLPM_SUPPORTED_FEATURES_GLUCOSE_MEASUREMENT_CONTEXT                0x00010000

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message that informs the client that a Glucose device */
   /* is now disconnected (asynchronously).                             */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   BD_ADDR_t              RemoteDeviceAddress;
   GLPM_Connection_Type_t ConnectionType;
} GLPM_Disconnected_Message_t;

#define GLPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(GLPM_Disconnected_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message that informs the local device that a Glucose  */
   /* Procedure has been started (asynchronously).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_PROCEDURE_STARTED_EVENT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Procedure_Started_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          ProcedureID;
   Boolean_t             Success;
   Byte_t                AttributeProtocolErrorCode;
} GLPM_Procedure_Started_Message_t;

#define GLPM_PROCEDURE_STARTED_MESSAGE_SIZE                    (sizeof(GLPM_Procedure_Started_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message that informs the local device that a Glucose  */
   /* Procedure has been stopped (asynchronously).                      */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_PROCEDURE_STOPPED_EVENT         */
   /*                                                                   */
   /*          Message Function ID.                                     */
   /* * NOTE * The NumberStoredRecords member is only valid when the    */
   /*          ProcedureType is set to gptReportNumberStoredRecords and */
   /*          ResponseCode is set to grcSuccess.                       */
typedef struct _tagGLPM_Procedure_Stopped_Message_t
{
   BTPM_Message_Header_t               MessageHeader;
   BD_ADDR_t                           RemoteDeviceAddress;
   unsigned int                        ProcedureID;
   GLPM_Procedure_Type_t               ProcedureType;
   GLPM_Procedure_Response_Code_Type_t ResponseCode;
   unsigned int                        NumberStoredRecords;
} GLPM_Procedure_Stopped_Message_t;

#define GLPM_PROCEDURE_STOPPED_MESSAGE_SIZE                    (sizeof(GLPM_Procedure_Stopped_Message_t))

   /* The following structure represents the Message definition for a   */
   /* GLP Manager Message that informs the local device that a Glucose  */
   /* measurement has been received (asynchronously).                   */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             GLPM_MESSAGE_FUNCTION_GLUCOSE_MEASUREMENT_EVENT       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagGLPM_Glucose_Measurement_Message_t
{
   BTPM_Message_Header_t                   MessageHeader;
   BD_ADDR_t                               RemoteDeviceAddress;
   unsigned int                            ProcedureID;
   GLPM_Measurement_Error_Type_t           MeasurementErrorType;
   unsigned long                           MeasurementFlags;
   GLPM_Glucose_Measurement_Data_t         GlucoseMeasurementData;
   GLPM_Glucose_Measurement_Context_Data_t GlucoseMeasurementContextData;
} GLPM_Glucose_Measurement_Message_t;

#define GLPM_GLUCOSE_MEASUREMENT_MESSAGE_SIZE                  (sizeof(GLPM_Glucose_Measurement_Message_t))

   /* The following define all of the valid bits that may be set in the */
   /* MeasurementFlags member of the GLPM_Glucose_Measurement_Message_t */
   /* structure.                                                        */
#define GLPM_MEASUREMENT_FLAGS_GLUCOSE_MEASUREMENT_DATA_PRESENT            0x00000001
#define GLPM_MEASUREMENT_FLAGS_GLUCOSE_MEASUREMENT_CONTEXT_DATA_PRESENT    0x00000002

#endif

