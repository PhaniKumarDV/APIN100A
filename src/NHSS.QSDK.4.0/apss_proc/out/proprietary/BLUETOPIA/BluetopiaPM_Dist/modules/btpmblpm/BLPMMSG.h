/*****< blpmmsg.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMMSG - Defined Interprocess Communication Messages for the Blood       */
/*            Pressure Profile (BLP) Manager for Stonestreet One Bluetopia    */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BLPMMSGH__
#define __BLPMMSGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

#include "SS1BTBLPM.h"           /* BLP Framework Prototypes/Constants.       */

#include "BTPMMSGT.h"            /* BTPM Message Type Definitions/Constants.  */

#include "BLPMType.h"            /* BTPM BLP Manager Type Definitions.        */

   /* The following Message Group constant represents the Bluetopia     */

   /* Platform Manager Message Group that specifies the Blood Pressure  */
   /* Profile (BLP) Manager.                                            */
#define BTPM_MESSAGE_GROUP_BLOOD_PRESSURE_MANAGER                  0x00001108

   /* The following constants represent the defined Bluetopia Platform  */
   /* Manager Message Functions that are valid for the Blood Pressure   */
   /* (BLP) Manager.                                                    */

   /* Blood Pressure Profile (BLP) Manager Commands.                    */
#define BLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS            0x00001001
#define BLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS         0x00001002

#define BLPM_MESSAGE_FUNCTION_ENABLE_BPM_INDICATIONS               0x00001101
#define BLPM_MESSAGE_FUNCTION_DISABLE_BPM_INDICATIONS              0x00001102
#define BLPM_MESSAGE_FUNCTION_ENABLE_ICP_NOTIFICATIONS             0x00001103
#define BLPM_MESSAGE_FUNCTION_DISABLE_ICP_NOTIFICATIONS            0x00001104
#define BLPM_MESSAGE_FUNCTION_GET_BLOOD_PRESSURE_FEATURE           0x00001105
#define BLPM_MESSAGE_FUNCTION_CANCEL_TRANSACTION                   0x00001106

   /* Blood Pressure Profile (BLP) Manager Asynchronous Events.         */
#define BLPM_MESSAGE_FUNCTION_CONNECTED                            0x00010001
#define BLPM_MESSAGE_FUNCTION_DISCONNECTED                         0x00010002

#define BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_MEASUREMENT           0x00011003
#define BLPM_MESSAGE_FUNCTION_INTERMEDIATE_CUFF_PRESSURE           0x00011004
#define BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_FEATURE_RESPONSE      0x00011005

   /* The following constants and/or definitions define the specific    */
   /* Message structures that are valid for the Blood Pressure Profile  */
   /* (BLP) Manager.                                                    */

   /* Blood Pressure Profile (BLP) Manager Manager Command/Response     */
   /* Message Formats.                                                  */


   /* The following structure represents the Message definition for a   */
   /* common device BLP Manager Message Request.                        */
   /* * NOTE * This is the message format for the following Message     */
   /*          Function ID's:                                           */
   /*          BLPM_MESSAGE_FUNCTION_ENABLE_BPM_INDICATIONS             */
   /*          BLPM_MESSAGE_FUNCTION_DISABLE_BPM_INDICATIONS            */
   /*          BLPM_MESSAGE_FUNCTION_ENABLE_ICP_NOTIFICATIONS           */
   /*          BLPM_MESSAGE_FUNCTION_DISABLE_ICP_NOTIFICATIONS          */
   /*          BLPM_MESSAGE_FUNCTION_GET_BLOOD_PRESSURE_FEATURE         */
typedef struct _tagBLPM_Device_Request_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
} BLPM_Device_Request_Message_t;

#define BLPM_DEVICE_REQUEST_MESSAGE_SIZE                 (sizeof(BLPM_Device_Request_Message_t))

   /* The following structure represents the Message definition for a   */
   /* common BLP Manager Message Response.                              */
   /* * NOTE * This is the message format for the following Message     */
   /*          Function ID's:                                           */
   /*          BLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS          */
   /*          BLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS       */
   /*          BLPM_MESSAGE_FUNCTION_ENABLE_BPM_INDICATIONS             */
   /*          BLPM_MESSAGE_FUNCTION_DISABLE_BPM_INDICATIONS            */
   /*          BLPM_MESSAGE_FUNCTION_ENABLE_ICP_NOTIFICATIONS           */
   /*          BLPM_MESSAGE_FUNCTION_DISABLE_ICP_NOTIFICATIONS          */
   /*          BLPM_MESSAGE_FUNCTION_GET_BLOOD_PRESSURE_FEATURE         */
   /*          BLPM_MESSAGE_FUNCTION_CANCEL_TRANSACTION                 */
typedef struct _tagBLPM_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   int                   Status;
} BLPM_Response_Message_t;

#define BLPM_RESPONSE_MESSAGE_SIZE                 (sizeof(BLPM_Response_Message_t))

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message to register for BLP Collector events          */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_REGISTER_COLLECTOR_EVENTS       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
} BLPM_Register_Collector_Events_Request_t;

#define BLPM_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE            (sizeof(BLPM_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message to un-register for BLP Collector events       */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_UN_REGISTER_COLLECTOR_EVENTS    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Un_Register_Collector_Events_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
} BLPM_Un_Register_Collector_Events_Request_t;

#define BLPM_UN_REGISTER_COLLECTOR_EVENTS_REQUEST_SIZE         (sizeof(BLPM_Un_Register_Collector_Events_Request_t))

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message to send the Cancel Transaction message        */
   /* (Request).                                                        */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_CANCEL_TRANSACTION              */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Cancel_Transaction_Request_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   unsigned int          TransactionID;
} BLPM_Cancel_Transaction_Request_t;

#define BLPM_CANCEL_TRANSACTION_REQUEST_SIZE        (sizeof(BLPM_Cancel_Transaction_Request_t))

   /* BLP Manager Asynchronous Message Formats.                         */

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message that informs the client that a BLP device has */
   /* connected (asynchronously).                                       */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_CONNECTED                       */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Connected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   BLPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} BLPM_Connected_Message_t;

#define BLPM_CONNECTED_MESSAGE_SIZE                            (sizeof(BLPM_Connected_Message_t))

   /* The following constants are used with the ConnectedFlags member of*/
   /* the BLPM_Connected_Message_t to denote various connection options.*/
#define BLPM_CONNECTED_FLAGS_INTERMEDIATE_CUFF_PRESSURE_SUPPORTED 0x00000001

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message that informs the client that a BLP device has */
   /* disconnected (asynchronously).                                    */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_DISCONNECTED                    */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Disconnected_Message_t
{
   BTPM_Message_Header_t  MessageHeader;
   unsigned int           CallbackID;
   BD_ADDR_t              RemoteDeviceAddress;
   BLPM_Connection_Type_t ConnectionType;
} BLPM_Disconnected_Message_t;

#define BLPM_DISCONNECTED_MESSAGE_SIZE                         (sizeof(BLPM_Disconnected_Message_t))

   /* The following structure defines the structure of the Time Stamp   */
   /* field that may be included in the Blood Pressure Measurement and  */
   /* Intermediate Cuff Pressure events.                                */
typedef struct _tagBLPM_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} BLPM_Date_Time_Data_t;

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message that informs the client that a Blood Pressure */
   /* Measurement has been sent by a sensor (asynchronously).           */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_MEASUREMENT      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Blood_Pressure_Measurement_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                MeasurementFlags;
   Word_t                SystolicPressure;
   Word_t                DiastolicPressure;
   Word_t                MeanArterialPressure;
   BLPM_Date_Time_Data_t TimeStamp;
   Word_t                PulseRate;
   Byte_t                UserID;
   Word_t                MeasurementStatus;
} BLPM_Blood_Pressure_Measurement_Message_t;

#define BLPM_BLOOD_PRESSURE_MEASUREMENT_MESSAGE_SIZE          (sizeof(BLPM_Blood_Pressure_Measurement_Message_t))

   /* The following constants represent the bit-mask values that are    */
   /* possible for the MeasurementFlags member of the                   */
   /* BLPM_Blood_Pressure_Measurement_Message_t message.                */
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_FLAGS_UNITS_KPA                          0x00000001
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_FLAGS_TIME_STAMP_PRESENT                 0x00000002
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_FLAGS_PULSE_RATE_PRESENT                 0x00000004
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_FLAGS_USER_ID_PRESENT                    0x00000008
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_FLAGS_MEASUREMENT_STATUS_PRESENT         0x00000010

#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_BODY_MOVEMENT_DETECTED            0x00000001
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_CUFF_FIT_TOO_LOOSE                0x00000002
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_IRREGULAR_PULSE_DETECTED          0x00000004

#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_PULSE_RATE_MASK                   0x00000018
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_PULSE_RATE_WITHIN_RANGE           0x00000000
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_PULSE_RATE_ABOVE_UPPER_LIMIT      0x00000008
#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_PULSE_RATE_BELOW_LOWER_LIMIT      0x00000010

#define BLPM_BLOOD_PRESSURE_MEASUREMENT_STATUS_IMPROPER_MEASUREMENT_POSITION     0x00000020

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message that informs the client that an Intermediate  */
   /* Cuff Pressure has been sent by a sensor (asynchronously).         */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*             BLPM_MESSAGE_FUNCTION_INTERMEDIATE_CUFF_PRESSURE      */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Intermediate_Cuff_Pressure_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                Flags;
   Word_t                IntermediateCuffPressure;
   BLPM_Date_Time_Data_t TimeStamp;
   Word_t                PulseRate;
   Byte_t                UserID;
   Word_t                MeasurementStatus;
} BLPM_Intermediate_Cuff_Pressure_Message_t;

#define BLPM_INTERMEDIATE_CUFF_PRESSURE_MESSAGE_SIZE          (sizeof(BLPM_Intermediate_Cuff_Pressure_Message_t))

   /* The following constants represent the bit-mask values that are    */
   /* possible for the Flags member of the                              */
   /* BLPM_Intermediate_Cuff_Pressure_Message_t message.                */
#define BLPM_INTERMEDIATE_CUFF_PRESSURE_FLAGS_UNITS_KPA                          0x00000001
#define BLPM_INTERMEDIATE_CUFF_PRESSURE_FLAGS_TIME_STAMP_PRESENT                 0x00000002
#define BLPM_INTERMEDIATE_CUFF_PRESSURE_FLAGS_PULSE_RATE_PRESENT                 0x00000004
#define BLPM_INTERMEDIATE_CUFF_PRESSURE_FLAGS_USER_ID_PRESENT                    0x00000008
#define BLPM_INTERMEDIATE_CUFF_PRESSURE_FLAGS_MEASUREMENT_STATUS_PRESENT         0x00000010

   /* The following structure represents the Message definition for a   */
   /* BLP Manager Message that informs the client that a Get Blood      */
   /* Pressure FeatureResponse has been sent by a sensor                */
   /* (asynchronously).                                                 */
   /* * NOTE * This is the message format for the                       */
   /*                                                                   */
   /*         BLPM_MESSAGE_FUNCTION_BLOOD_PRESSURE_FEATURE_RESPONSE     */
   /*                                                                   */
   /*          Message Function ID.                                     */
typedef struct _tagBLPM_Blood_Pressure_Feature_Response_Message_t
{
   BTPM_Message_Header_t MessageHeader;
   unsigned int          CallbackID;
   BD_ADDR_t             RemoteDeviceAddress;
   unsigned int          TransactionID;
   int                   Status;
   Word_t                Feature;
} BLPM_Blood_Pressure_Feature_Response_Message_t;

#define BLPM_BLOOD_PRESSURE_FEATURE_RESPONSE_MESSAGE_SIZE  (sizeof(BLPM_Blood_Pressure_Feature_Response_Message_t))

#define BLPM_BLOOD_PRESSURE_FEATURE_BODY_MOVEMENT_DETECTION_SUPPORTED            0x00000001
#define BLPM_BLOOD_PRESSURE_FEATURE_CUFF_FIT_DETECTION_SUPPORTED                 0x00000002
#define BLPM_BLOOD_PRESSURE_FEATURE_IRREGULAR_PULSE_DETECTION_SUPPORTED          0x00000004
#define BLPM_BLOOD_PRESSURE_FEATURE_PULSE_RATE_RANGE_DETECTION_SUPPORTED         0x00000008
#define BLPM_BLOOD_PRESSURE_FEATURE_MEASUREMENT_POSITION_DETECTION_SUPPORTED     0x00000010
#define BLPM_BLOOD_PRESSURE_FEATURE_MULTIPLE_BOND_SUPPORTED                      0x00000020

#endif
