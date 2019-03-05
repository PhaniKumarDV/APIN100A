/*****< cpsapi.h >*************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CPSAPI - Stonestreet One Bluetooth Cycling Power Servie (GATT based) API  */
/*           Type Definitions, Constants, and Prototypes.                     */
/*                                                                            */
/*  Author:  Zahid Khan                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/25/13  Z. Khan        Initial creation.                               */
/******************************************************************************/
#ifndef __CPSAPIH__
#define __CPSAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API Prototypes/Constants. */
#include "CPSTypes.h"       /* CPS Service Types/Constants.                   */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define CPS_ERROR_INVALID_PARAMETER                            (-1000)
#define CPS_ERROR_INVALID_BLUETOOTH_STACK_ID                   (-1001)
#define CPS_ERROR_INSUFFICIENT_RESOURCES                       (-1002)
#define CPS_ERROR_INSUFFICIENT_BUFFER_SPACE                    (-1003)
#define CPS_ERROR_SERVICE_ALREADY_REGISTERED                   (-1004)
#define CPS_ERROR_INVALID_INSTANCE_ID                          (-1005)
#define CPS_ERROR_MALFORMATTED_DATA                            (-1006)
#define CPS_ERROR_INDICATION_OUTSTANDING                       (-1007)
#define CPS_ERROR_NO_AUTHENTICATION                            (-1008)
#define CPS_ERROR_UNKNOWN_ERROR                                (-1009)

   /* The following structure contains the Handles that will need to be */
   /* cached by a CPS client in order to only do service discovery      */
   /* once.                                                             */
typedef struct _tagCPS_Client_Information_t
{
   Word_t CP_Measurement;
   Word_t CP_Measurement_Client_Configuration;
   Word_t CP_Measurement_Server_Configuration;
   Word_t CP_Feature;
   Word_t Sensor_Location;
   Word_t CP_Vector;
   Word_t CP_Vector_Client_Configuration;
   Word_t CP_Control_Point;
   Word_t CP_Control_Point_Client_Configuration;
} CPS_Client_Information_t;

#define CPS_CLIENT_INFORMATION_DATA_SIZE                 (sizeof(CPS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a CPS Server.                           */
typedef struct _tagCPS_Server_Information_t
{
   Word_t CP_Measurement_Client_Configuration;
   Word_t CP_Measurement_Server_Configuration;
   Word_t CP_Control_Point_Client_Configuration;
   Word_t CP_Vector_Client_Configuration;
} CPS_Server_Information_t;

#define CPS_SERVER_INFORMATION_DATA_SIZE                 (sizeof(CPS_Server_Information_t))

   /* The following structure defines the format of the optional Wheel  */
   /* Revolution Data field of the Cycling Power Measurement            */
   /* characteristic.  The Cumulative Wheel Revolutions field represents*/
   /* the number of times wheel was rotated.  The Last Wheel Event Time */
   /* is free-running-count of 1/2048 second units and it represents the*/
   /* time when the wheel revolution was detected by the wheel rotation */
   /* sensor.                                                           */
typedef struct _tagCPS_Wheel_Revolution_Data_t
{
   DWord_t CumulativeWheelRevolutions;
   Word_t  LastWheelEventTime;
} CPS_Wheel_Revolution_Data_t;

#define CPS_WHEEL_REVOLUTIION_DATA_SIZE                  (sizeof(CPS_Wheel_Revolution_Data_t))

   /* The following structure defines the format of the optional Crank  */
   /* Revolution Data field of the Cycling Power Measurement            */
   /* characteristic & Cycling Power Vector characteristic.  The        */
   /* Cumulative Crank Revolutions field represents the number of times */
   /* crank was rotated.  The Last Crank Event Time is                  */
   /* free-running-count of 1/1024 second units and it represents the   */
   /* time when the last crank revolution was detected by the crank     */
   /* rotation sensor.                                                  */
typedef struct _tagCPS_Crank_Revolution_Data_t
{
   Word_t CumulativeCrankRevolutions;
   Word_t LastCrankEventTime;
} CPS_Crank_Revolution_Data_t;

#define CPS_CRANK_REVOLUTIION_DATA_SIZE                  (sizeof(CPS_Crank_Revolution_Data_t))

   /* The following structure defines the format of the optional Extreme*/
   /* Force Magnitudes Data field of the Cycling Power Measurement      */
   /* characteristic.  The Maximum Force Magnitude field represents the */
   /* maximum force value measured in a single crank revolution;        */
   /* respectively the Minimum Force Magnitude field represents the     */
   /* minimum force value measured in a single crank revolution.        */
typedef struct _tagCPS_Extreme_Force_Magnitudes_Data_t
{
   SWord_t MaximumForceMagnitude;
   SWord_t MinimumForceMagnitude;
} CPS_Extreme_Force_Magnitudes_Data_t;

#define CPS_EXTREME_FORCE_MAGNITUDES_DATA_SIZE           (sizeof(CPS_Extreme_Force_Magnitudes_Data_t))

   /* The following structure defines the format of the optional Extreme*/
   /* Torque Magnitudes Data field of the Cycling Power Measurement     */
   /* characteristic.  The Maximum Torque Magnitude field represents the*/
   /* maximum torque value measured in a single crank revolution;       */
   /* respectively the Minimum Torque Magnitude field represents the    */
   /* minimum torque value measured in a single crank revolution.       */
typedef struct _tagCPS_Extreme_Torque_Magnitudes_Data_t
{
   SWord_t MaximumTorqueMagnitude;
   SWord_t MinimumTorqueMagnitude;
} CPS_Extreme_Torque_Magnitudes_Data_t;

#define CPS_EXTREME_TORQUE_MAGNITUDES_DATA_SIZE          (sizeof(CPS_Extreme_Torque_Magnitudes_Data_t))

   /* The following structure defines the format of the optional Extreme*/
   /* Angles Data field of the Cycling Power Measurement characteristic.*/
   /* The Maximum Angle field represents the angle of the crank when the*/
   /* maximum value is measured in a single crank revolution.  Similarly*/
   /* the Minimum Angle field represents the angle of the crank when the*/
   /* minimum value is measured in the same crank revolution.           */
   /* * NOTE * The actual size of Minimum Angle and Maximum Angle data  */
   /*          fields are 12bits, So Most Significant 4bits of these    */
   /*          fields will be unused and should be set to zero.         */
typedef struct _tagCPS_Extreme_Angles_Data_t
{
   Word_t MaximumAngle;
   Word_t MinimumAngle;
} CPS_Extreme_Angles_Data_t;

#define CPS_EXTREMA_ANGLES_DATA_SIZE                     (sizeof(CPS_Extreme_Angles_Data_t))

   /* The following defines the structure of the Cycling Power          */
   /* Measurement Data that is passed to the function that builds the   */
   /* Cycling Power Measurement packet.                                 */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_PEDAL_POWER_BALANCE_PRESENT Flag*/
   /*          is set, then a valid value must be entered for Pedal     */
   /*          Power Balance.                                           */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_ACCUMULATED_TORQUE_PRESENT Flag */
   /*          is set, then a valid value must be entered for           */
   /*          Accumulated Torque.                                      */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT   */
   /*          Flag is set, then a valid value must be entered for Wheel*/
   /*          Revolution Data.                                         */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT   */
   /*          Flag is set, then a valid value must be entered for Crank*/
   /*          Revolution Data.                                         */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_EXTREME_FORCE_MAGNITUDES_PRESENT*/
   /*          Flag is set, then a valid value must be entered for      */
   /*          Extreme Force Magnitudes.                                */
   /* * NOTE * If                                                       */
   /*          CPS_MEASUREMENT_FLAGS_EXTREME_TORQUE_MAGNITUDES_PRESENT  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          Extreme Torque Magnitudes.                               */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_EXTREME_ANGLES_PRESENT Flag is  */
   /*          set, then a valid value must be entered for Extreme      */
   /*          Angles.                                                  */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_TOP_DEAD_SPOT_ANGLE_PRESENT Flag*/
   /*          is set, then a valid value must be entered for Top Dead  */
   /*          Spot Angle.                                              */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT  */
   /*          Flag is set, then a valid value must be entered for      */
   /*          Bottom Dead Spot Angle.                                  */
   /* * NOTE * If CPS_MEASUREMENT_FLAGS_ACCUMULATED_ENERGY_PRESENT Flag */
   /*          is set, then a valid value must be entered for           */
   /*          Accumulated Energy.                                      */
typedef struct _tagCPS_Measurement_Data_t
{
   Word_t                               Flags;
   SWord_t                              InstantaneousPower;
   Byte_t                               PedalPowerBalance;
   Word_t                               AccumulatedTorque;
   CPS_Wheel_Revolution_Data_t          WheelRevolutionData;
   CPS_Crank_Revolution_Data_t          CrankRevolutionData;
   CPS_Extreme_Force_Magnitudes_Data_t  ExtremeForceMagnitudes;
   CPS_Extreme_Torque_Magnitudes_Data_t ExtremeTorqueMagnitudes;
   CPS_Extreme_Angles_Data_t            ExtremeAngles;
   Word_t                               TopDeadSpotAngle;
   Word_t                               BottomDeadSpotAngle;
   Word_t                               AccumulatedEnergy;
} CPS_Measurement_Data_t;

#define CPS_MEASUREMENT_DATA_SIZE                        (sizeof(CPS_Measurement_Data_t))

   /* The following defines the structure of the Cycling Power Vector   */
   /* Data that is passed to the function that builds the Cycling Power */
   /* Vector packet.                                                    */
   /* * NOTE * If CPS_VECTOR_FLAGS_CRANK_REVOLUTION_DATA_PRESENT Flag is*/
   /*          set, then a valid value must be entered for Crank        */
   /*          Revolution Data.                                         */
   /* * NOTE * If CPS_VECTOR_FLAGS_FIRST_CRANK_MEASUREMENT_ANGLE_PRESENT*/
   /*          Flag is set, then a valid value must be entered for First*/
   /*          Crank Measurement Angle.                                 */
   /* * NOTE * If                                                       */
   /*       CPS_VECTOR_FLAGS_INSTANTANEOUS_FORCE_MAGNITUDE_ARRAY_PRESENT*/
   /*          Flag is set, then a valid value must be entered for the  */
   /*          Instantaneous Magnitude Array, and the length of the     */
   /*          array will be represented by the MagnitudeDataLength     */
   /*          member.                                                  */
   /* * NOTE * If                                                       */
   /*      CPS_VECTOR_FLAGS_INSTANTANEOUS_TORQUE_MAGNITUDE_ARRAY_PRESENT*/
   /*          Flag is set, then a valid value must be entered for the  */
   /*          Instantaneous Magnitude Array, and the length of the     */
   /*          array will be represented by the MagnitudeDataLength     */
   /*          member.                                                  */
   /* * NOTE * The Instantaneous Force Magnitude array and the          */
   /*          Instantaneous Torque Magnitude array are mutually        */
   /*          exclusive. Only one can be supported at a time. The      */
   /*          supported array should correspond with the Sensor        */
   /*          Measurement Context flag in the Cycling Power Feature    */
   /*          value.                                                   */
typedef struct _tagCPS_Vector_Data_t
{
   Byte_t                       Flags;
   CPS_Crank_Revolution_Data_t  CrankRevolutionData;
   Word_t                       FirstCrankMeasurementAngle;
   Byte_t                       MagnitudeDataLength;
   SWord_t                     *InstantaneousMagnitude;
} CPS_Vector_Data_t;

#define CPS_VECTOR_DATA_SIZE                             (sizeof(CPS_Vector_Data_t))

#define CPS_MAXIMUM_SUPPORTED_SENSOR_LOCATIONS                 (17)

   /* The following defines the format of a Supported Sensor Locations  */
   /* Values that will be used to respond to Supported Sensor Locations */
   /* request made by the remote device.  The first member represents   */
   /* the Number of Sensor Locations available.  The second member      */
   /* represents the Byte pointer pointing to Sensor Locations.         */
typedef struct _tagCPS_Supported_Sensor_Locations_t
{
   Byte_t  NumberOfSensorLocations;
   Byte_t *SensorLocations;
} CPS_Supported_Sensor_Locations_t;

#define CPS_SUPPORTED_SENSOR_LOCATIONS_SIZE              (sizeof(CPS_Supported_Sensor_Locations_t))

   /* The following structure represents the format of a CPS Date/Time  */
   /* value.  This is used to represent the Date-Time which contains the*/
   /* Day/Month/Year and Hours:Minutes:Second data.                     */
   /* * NOTE * A value of 0 for the year, month or day fields shall not */
   /*          be used.                                                 */
typedef struct _tagCPS_Date_Time_Data_t
{
   Word_t Year;
   Byte_t Month;
   Byte_t Day;
   Byte_t Hours;
   Byte_t Minutes;
   Byte_t Seconds;
} CPS_Date_Time_Data_t;

#define CPS_DATE_TIME_DATA_SIZE                          (sizeof(CPS_Date_Time_Data_t))

   /* The following MACRO is a utility MACRO that exists to valid that a*/
   /* specified Date Time is valid.  The only parameter to this function*/
   /* is the CPS_Date_Time_Data_t structure to valid.  This MACRO       */
   /* returns TRUE if the Date Time is valid or FALSE otherwise.        */
#define CPS_DATE_TIME_VALID(_x)                          ((GATT_DATE_TIME_VALID_YEAR(((_x)).Year)) && (GATT_DATE_TIME_VALID_MONTH(((_x)).Month)) && (GATT_DATE_TIME_VALID_DAY(((_x)).Day)) && (GATT_DATE_TIME_VALID_HOURS(((_x)).Hours)) && (GATT_DATE_TIME_VALID_MINUTES(((_x)).Minutes)) && (GATT_DATE_TIME_VALID_SECONDS(((_x)).Seconds)))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of Cycling Power Control Point         */
   /* characteristic.                                                   */
typedef enum
{
   cpcSetCumulativeValue              = CPS_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE,
   cpcUpdateSensorLocation            = CPS_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION,
   cpcRequestSupportedSensorLocations = CPS_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS,
   cpcSetCrankLength                  = CPS_CONTROL_POINT_OPCODE_SET_CRANK_LENGTH,
   cpcRequestCrankLength              = CPS_CONTROL_POINT_OPCODE_REQUEST_CRANK_LENGTH,
   cpcSetChainLength                  = CPS_CONTROL_POINT_OPCODE_SET_CHAIN_LENGTH,
   cpcRequestChainLength              = CPS_CONTROL_POINT_OPCODE_REQUEST_CHAIN_LENGTH,
   cpcSetChainWeight                  = CPS_CONTROL_POINT_OPCODE_SET_CHAIN_WEIGHT,
   cpcRequestChainWeight              = CPS_CONTROL_POINT_OPCODE_REQUEST_CHAIN_WEIGHT,
   cpcSetSpanLength                   = CPS_CONTROL_POINT_OPCODE_SET_SPAN_LENGTH,
   cpcRequestSpanLength               = CPS_CONTROL_POINT_OPCODE_REQUEST_SPAN_LENGTH,
   cpcStartOffsetCompensation         = CPS_CONTROL_POINT_OPCODE_START_OFFSET_COMPENSATION,
   cpcMaskCyclingPowerMeasurement     = CPS_CONTROL_POINT_OPCODE_MASK_MEASUREMENT_CHARACTERISTIC_CONTENT,
   cpcRequestSamplingRate             = CPS_CONTROL_POINT_OPCODE_REQUEST_SAMPLING_RATE,
   cpcRequestFactoryCalibrationDate   = CPS_CONTROL_POINT_OPCODE_REQUEST_FACTORY_CALIBRATION_DATE
} CPS_Control_Point_Command_Type_t;

   /* The following defines the format of possible Response Parameter   */
   /* Values that will be used to respond to Cycling Power Control Point*/
   /* request made by the remote device.                                */
typedef union _tagCPS_Control_Point_Response_Parameter_t
{
   CPS_Supported_Sensor_Locations_t SupportedSensorLocations;
   CPS_Date_Time_Data_t             FactoryCalibrationDate;
   Word_t                           CrankLength;
   Word_t                           ChainLength;
   Word_t                           ChainWeight;
   Word_t                           SpanLength;
   SWord_t                          OffsetCompensation;
   Byte_t                           SamplingRate;
} CPS_Control_Point_Response_Parameter_t;

   /* The following defines the format of possible Response Parameter   */
   /* Values that will be used to respond to Cycling Power Control Point*/
   /* request using the CPS_Indicate_Control_Point_Result_With_Data()   */
   /* API.  The first member represents the requested Command Type and  */
   /* it must be of the form CPS_CONTROL_POINT_OPCODE_XXX.  The second  */
   /* member is a union of possible responses to send to the remote     */
   /* device.                                                           */
typedef struct _tagCPS_Control_Point_Indication_Data_t
{
   Byte_t                                 CommandType;
   CPS_Control_Point_Response_Parameter_t ResponseParameter;
}CPS_Control_Point_Indication_Data_t;

#define CPS_CONTROL_POINT_INDICATION_DATA_SIZE           (sizeof(CPS_Control_Point_Indication_Data_t))

   /* The following defines the format of a Cycling Power Control Point */
   /* Response Data.  This structure will hold the Cycling Power Control*/
   /* Point response data received from remote CPS Server.  This        */
   /* structure will be used by CPS_Decode_Control_Point_Response API.  */
   /* The first member RequestOpCode must be of the form                */
   /* CPS_CONTROL_POINT_OPCODE_XXX and the second member                */
   /* ResponseCodeValue must be of the form                             */
   /* CPS_CONTROL_POINT_RESPONSE_CODE_XXX.  The third member is a       */
   /* Response Parameter for a specific RequestOpCode.                  */
typedef struct _tagCPS_Control_Point_Response_Data_t
{
   Byte_t                                 RequestOpCode;
   Byte_t                                 ResponseCodeValue;
   CPS_Control_Point_Response_Parameter_t ResponseParameter;
} CPS_Control_Point_Response_Data_t;

#define CPS_CONTROL_POINT_RESPONSE_DATA_SIZE             (sizeof(CPS_Control_Point_Response_Data_t))

   /* The following define the valid Read/Write Client Configuration    */
   /* Request types that a server may receive in a                      */
   /* etCPS_Server_Read_Client_Configuration_Request or                 */
   /* etCPS_Server_Client_Configuration_Update event.  This type is also*/
   /* used by Notify/Indicate APIs to denote the characteristic value to*/
   /* notify or indicate.                                               */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctCyclingPowerMeasurement,
   ctCyclingPowerControlPoint,
   ctCyclingPowerVector
} CPS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* CPS Service.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the CPS_Event_Data_t structure.                                   */
typedef enum
{
   etCPS_Read_Client_Configuration_Request,
   etCPS_Client_Configuration_Update,
   etCPS_Read_CP_Measurement_Server_Configuration_Request,
   etCPS_CP_Measurement_Server_Configuration_Update,
   etCPS_Control_Point_Command,
   etCPS_Confirmation_Data
} CPS_Event_Type_t;

   /* The following is dispatched to a CPS Server when a CPS Client is  */
   /* attempting to read the Client Configuration descriptor.  The      */
   /* ConnectionID, and RemoteDevice identifies the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the CPS_Read_Client_Configuration_Response() API function.  */
typedef struct _tagCPS_Read_Client_Configuration_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   CPS_Characteristic_Type_t ClientConfigurationType;
} CPS_Read_Client_Configuration_Data_t;

#define CPS_READ_CLIENT_CONFIGURATION_DATA_SIZE          (sizeof(CPS_Read_Client_Configuration_Data_t))

   /* The following is dispatched to a CPS Server when a CPS Client     */
   /* attempts to write to a Client Configuration descriptor.  The      */
   /* ConnectionID and RemoteDevice identify the Client that is making  */
   /* the update request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the CPS_Vector_Client_Configuration_Update_Response() API   */
   /* function.  The ClientConfiguration value specifies the new Client */
   /* Configuration value to update.                                    */
typedef struct _tagCPS_Client_Configuration_Update_Data_t
{
   unsigned int              InstanceID;
   unsigned int              ConnectionID;
   unsigned int              TransactionID;
   GATT_Connection_Type_t    ConnectionType;
   BD_ADDR_t                 RemoteDevice;
   CPS_Characteristic_Type_t ClientConfigurationType;
   Word_t                    ClientConfiguration;
} CPS_Client_Configuration_Update_Data_t;

#define CPS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE        (sizeof(CPS_Client_Configuration_Update_Data_t))

   /* The following is dispatched to a CPS Server when a CPS Client is  */
   /* attempting to read the Cycling Power Measurement Server           */
   /* Characteristic Configuration descriptor.  The ConnectionID, and   */
   /* RemoteDevice identifies the Client that is making the request.    */
   /* The TransactionID specifies the TransactionID of the request, this*/
   /* can be used when responding to the request using the              */
   /* CPS_CP_Measurement_Server_Configuration_Read_Response() API       */
   /* function.                                                         */
typedef struct _tagCPS_Read_CP_Measurement_Server_Configuration_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   unsigned int           TransactionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
} CPS_Read_CP_Measurement_Server_Configuration_Data_t;

#define CPS_READ_CP_MEASUREMENT_SERVER_CONFIGURATION_DATA_SIZE   (sizeof(CPS_Read_CP_Measurement_Server_Configuration_Data_t))

   /* The following is dispatched to a CPS Server when a CPS Client     */
   /* attempts to write to a Cycling Power Measurement Server           */
   /* Configuration descriptor.  The ConnectionID and RemoteDevice      */
   /* identifies the Client that is making the update request.  The     */
   /* ServerConfiguration value specifies the new Server Configuration  */
   /* value to update.                                                  */
typedef struct _tagCPS_CP_Measurement_Server_Configuration_Update_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Word_t                 ServerConfiguration;
} CPS_CP_Measurement_Server_Configuration_Update_Data_t;

#define CPS_CP_MEASUREMENT_SERVER_CONFIGURATION_UPDATE_DATA_SIZE   (sizeof(CPS_CP_Measurement_Server_Configuration_Update_Data_t))

   /* The following structure defines the format of the Cycling Power   */
   /* Control Point Command Request Data.  This structure is passed as a*/
   /* parameter to CPS_Format_Control_Point_Command API.                */
typedef struct _tagCPS_Control_Point_Format_Data_t
{
   CPS_Control_Point_Command_Type_t CommandType;
   union
   {
      DWord_t CumulativeValue;
      Byte_t  SensorLocation;
      Word_t  CrankLength;
      Word_t  ChainLength;
      Word_t  ChainWeight;
      Word_t  SpanLength;
      Word_t  ContentMask;
   } CommandParameter;
} CPS_Control_Point_Format_Data_t;

#define CPS_CONTROL_POINT_FORMAT_DATA_SIZE               (sizeof(CPS_Control_Point_Format_Data_t))

   /* The following is dispatched to a CPS Server in response to the    */
   /* reception of request from a Client to write to the Cycling Power  */
   /* Control Point.                                                    */
typedef struct _tagCPS_Control_Point_Command_Data_t
{
   unsigned int                    InstanceID;
   unsigned int                    ConnectionID;
   unsigned int                    TransactionID;
   GATT_Connection_Type_t          ConnectionType;
   BD_ADDR_t                       RemoteDevice;
   CPS_Control_Point_Format_Data_t FormatData;
} CPS_Control_Point_Command_Data_t;

#define CPS_CONTROL_POINT_COMMAND_DATA_SIZE              (sizeof(CPS_Control_Point_Command_Data_t))

   /* The following CPS Service Event is dispatched to a CPS Server when*/
   /* a CPS Client has sent a confirmation to a previously sent         */
   /* Indication.  The InstanceID specifies the Unique Service Instance.*/
   /* The ConnectionID, ConnectionType, and RemoteDevice specifies the  */
   /* Client that is sending the confirmation.  The final parameter     */
   /* specifies the status of the Indication.                           */
   /* * NOTE * The Status member is set to one of the following values: */
   /*                GATT_CONFIRMATION_STATUS_SUCCESS                   */
   /*                GATT_CONFIRMATION_STATUS_TIMEOUT                   */
typedef struct _tagCPS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
} CPS_Confirmation_Data_t;

#define CPS_CONFIRMATION_DATA_SIZE                       (sizeof(CPS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all CPS Service Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagCPS_Event_Data_t
{
   CPS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      CPS_Read_Client_Configuration_Data_t                  *CPS_Read_Client_Configuration_Data;
      CPS_Client_Configuration_Update_Data_t                *CPS_Client_Configuration_Update_Data;
      CPS_Read_CP_Measurement_Server_Configuration_Data_t   *CPS_Read_CP_Measurement_Server_Configuration_Data;
      CPS_CP_Measurement_Server_Configuration_Update_Data_t *CPS_CP_Measurement_Server_Configuration_Update_Data;
      CPS_Control_Point_Command_Data_t                      *CPS_Control_Point_Command_Data;
      CPS_Confirmation_Data_t                               *CPS_Confirmation_Data;
   } Event_Data;
} CPS_Event_Data_t;

#define CPS_EVENT_DATA_SIZE                              (sizeof(CPS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a CPS Service Event Receive Data Callback.  This function will be */
   /* called whenever a CPS Service Event occurs that is associated with*/
   /* the specified Bluetooth Stack ID.  This function passes to the    */
   /* caller the Bluetooth Stack ID, the CPS Event Data that occurred   */
   /* and the CPS Service Event Callback Parameter that was specified   */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the CPS Service Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another CPS Service  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving CPS Service Event   */
   /*            Packets.  A Deadlock WILL occur because NO CPS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *CPS_Event_Callback_t)(unsigned int BluetoothStackID, CPS_Event_Data_t *CPS_Event_Data, unsigned long CallbackParameter);

   /* CPS Server API.                                                   */

   /* The following function is responsible for opening a CPS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered CPS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 CPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI CPS_Initialize_Service(unsigned int BluetoothStackID, CPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Initialize_Service_t)(unsigned int BluetoothStackID, CPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a CPS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered CPS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 CPS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI CPS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, CPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, CPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened CPS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successful call to            */
   /* CPS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is used to perform a suspend of the        */
   /* Bluetooth stack.  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that Bluetopia is to use to collapse it's state information into. */
   /* This function can be called with BufferSize and Buffer set to 0   */
   /* and NULL, respectively.  In this case this function will return   */
   /* the number of bytes that must be passed to this function in order */
   /* to successfully perform a suspend (or 0 if an error occurred, or  */
   /* this functionality is not supported).  If the BufferSize and      */
   /* Buffer parameters are NOT 0 and NULL, this function will attempt  */
   /* to perform a suspend of the stack.  In this case, this function   */
   /* will return the amount of memory that was used from the provided  */
   /* buffers for the suspend (or zero otherwise).                      */
BTPSAPI_DECLARATION unsigned long BTPSAPI CPS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_CPS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* CPS_Suspend()).  This function accepts as input the Bluetooth     */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to CPS_Suspend().  This     */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI CPS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the CPS Service that is          */
   /* registered with a call to CPS_Initialize_Service() or             */
   /* CPS_Initialize_Service_Handle_Range().  This function returns the */
   /* non-zero number of attributes that are contained in a CPS Server  */
   /* or zero on failure.                                               */
BTPSAPI_DECLARATION unsigned int BTPSAPI CPS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_CPS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a CPS Read*/
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* CPS_Initialize_Service().  The third is the Transaction ID of the */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for responding to a CPS     */
   /* Write Client Configuration Request when an error occurred.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second parameter is the Transaction ID of the request.  The   */
   /* final parameter is an error code that is used to determine if the */
   /* Request is being accepted by the server or if an error response   */
   /* should be issued instead.  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Cycling Power Vector Client Configuration commands*/
   /*          when the client has rejected new connection parameters   */
   /*          update request made by CPS Server.  The only valid Error */
   /*          Code for this condition is                               */
   /*          CPS_ERROR_CODE_INAPPROPRIATE_CONNECTION_PARAMETERS.      */
BTPSAPI_DECLARATION int BTPSAPI CPS_Vector_Client_Configuration_Update_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Vector_Client_Configuration_Update_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for responding to a CPS Read*/
   /* Cycling Power Measurement Server Configuration Request.  The first*/
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CPS_Initialize_Service().  The third is the Transaction ID of  */
   /* the request.  The final parameter contains the Server             */
   /* Configuration to send to the remote device.  This function returns*/
   /* a zero if successful or a negative return error code if an error  */
   /* occurs.                                                           */
BTPSAPI_DECLARATION int BTPSAPI CPS_Read_CP_Measurement_Server_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ServerConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Read_CP_Measurement_Server_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ServerConfiguration);
#endif

   /* The following function is responsible for setting the supported   */
   /* CPS features on the specified CPS Instance.  The first parameter  */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CPS_Initialize_Service().  The final parameter is a bitmask of the*/
   /* supported features to set for the specified CPS Instance.  This   */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The SupportedFeatures parameter is a bitmask which is    */
   /*          made up of bits of the form:                             */
   /*                CPS_FEATURE_XXX                                    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Set_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Set_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the current CPS*/
   /* Features on the specified CPS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CPS_Initialize_Service().  The final parameter is a pointer to    */
   /* return the current CPS Features for the specified CPS Instance.   */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Query_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Query_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, DWord_t *SupportedFeatures);
#endif

   /* The following function is responsible for sending a CPS           */
   /* Measurement notification to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CPS_Initialize_Service().  The third parameter is the          */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the Cycling Power Measurement Data         */
   /* structure that contains all of the required and optional data for */
   /* the notification.  This function returns a zero if successful or a*/
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Notify_CP_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Notify_CP_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for setting the Sensor      */
   /* Location on the specified CPS Instance.  The first parameter is   */
   /* the Bluetooth Stack ID of the Bluetooth Device.  The second       */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CPS_Initialize_Service().  The final parameter is the Sensor      */
   /* Location to set for the specified CPS Instance.  This function    */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The SensorLocation parameter should be of the form       */
   /*                CPS_SENSOR_LOCATION_XXX                            */
BTPSAPI_DECLARATION int BTPSAPI CPS_Set_Sensor_Location(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Set_Sensor_Location_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t SensorLocation);
#endif

   /* The following function is responsible for querying the current    */
   /* Sensor Location on the specified CPS Instance.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CPS_Initialize_Service().  The final parameter is a pointer to */
   /* return the current Sensor Location for the specified CPS Instance.*/
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Query_Sensor_Location(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Query_Sensor_Location_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *SensorLocation);
#endif

   /* The following function is responsible for sending a CPS Cycling   */
   /* Power Vector notification to a specified remote device.  The first*/
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CPS_Initialize_Service().  The third parameter is the          */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the Cycling Power Measurement Data         */
   /* structure that contains all of the required and optional data for */
   /* the notification.  This function returns a zero if successful or a*/
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Notify_CP_Vector(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Vector_Data_t *VectorData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Notify_CP_Vector_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Vector_Data_t *VectorData);
#endif

   /* The following function is responsible for responding to a Cycling */
   /* Power Control Point Command received from a remote device.  The   */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second is the TransactionID that was received in the Cycling  */
   /* Power Control Point event.  The final parameter is an error code  */
   /* that is used to determine if the Request is being accepted by the */
   /* server or if an error response should be issued instead.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero then an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Cycling Power Control Point commands when the     */
   /*          Client is already in progress or Cycling PowerControl    */
   /*          Point is not properly configured for indications.  All   */
   /*          the other reasons should return ZERO for the ErrorCode   */
   /*          and then send Cycling Power Control Point Result         */
   /*          indication to indicate any other errors.  For Example: If*/
   /*          the Op Code in the Request is not supported by the       */
   /*          Server, this API should be called with ErrorCode set to  */
   /*          ZERO and then the CPS_Indicate_Control_Point_Result()    */
   /*          should be called with the ResponseCode set to            */
   /*          CPS_CONTROL_POINT_RESPONSE_CODE_OPCODE_NOT_SUPPORTED.    */
BTPSAPI_DECLARATION int BTPSAPI CPS_Control_Point_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending the Cycling     */
   /* Power Control Point indication to a specified remote device that  */
   /* requires no parameter in the response.  The first parameter is the*/
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* CPS_Initialize_Service().  The third parameter the ConnectionID of*/
   /* the remote device to send the indication to.  The fourth parameter*/
   /* is the Request data to indicate.  The last parameter is response  */
   /* code.  This function returns a zero if successful or a negative   */
   /* return error code if an error occurs.                             */
   /* * NOTE * Only 1 Cycling Power Control Point Request indication may*/
   /*          be outstanding per CPS Instance.                         */
BTPSAPI_DECLARATION int BTPSAPI CPS_Indicate_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Control_Point_Command_Type_t CommandType, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Indicate_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Control_Point_Command_Type_t CommandType, Byte_t ResponseCode);
#endif

   /* The following function is responsible for sending the Cycling     */
   /* Power Control Point indication to a specified remote device that  */
   /* requires response parameter in the response.                      */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device.  The second parameter is the InstanceID returned from a   */
   /* successful call to CPS_Initialize_Service().  The third parameter */
   /* the ConnectionID of the remote device to send the indication to.  */
   /* The last parameter is the response data to indicate.  This        */
   /* function returns zero if successful or a negative return error    */
   /* code if an error occurs.                                          */
   /* * NOTE * Only the following set of commands are valid for this API*/
   /*             cpcRequestSupportedSensorLocations                    */
   /*             cpcRequestFactoryCalibrationDate                      */
   /*             cpcRequestCrankLength                                 */
   /*             cpcRequestChainLength                                 */
   /*             cpcRequestChainWeight                                 */
   /*             cpcRequestSpanLength                                  */
   /*             cpcStartOffsetCompensation                            */
   /*             cpcRequestSamplingRate                                */
   /* * NOTE * If any error response need to be sent for the specified  */
   /*          set of commands then CPS_Indicate_Control_Point_Result   */
   /*          API should be used.                                      */
   /* * NOTE * If the response data is for supported sensor locations   */
   /*          and the SensorLocations data in SupportedSensorLocations */
   /*          field in Response data is not NULL then it should be     */
   /*          freed by calling                                         */
   /*          CPS_Free_Supported_Sensor_Locations_Data() when the      */
   /*          decoded Response data is no longer needed.               */
   /* * NOTE * Only 1 Cycling Power Control Point Request indication    */
   /*          may be outstanding per CPS Instance.                     */
BTPSAPI_DECLARATION int BTPSAPI CPS_Indicate_Control_Point_Result_With_Data(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Control_Point_Indication_Data_t *IndicationData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Indicate_Control_Point_Result_With_Data_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CPS_Control_Point_Indication_Data_t *IndicationData);
#endif

   /* CPS Client API.                                                   */

   /* The following function is responsible for formatting a Cycling    */
   /* Power Control Measurement Data into a user specified buffer.  The */
   /* first parameter is the input Cycling Power Measurement data to    */
   /* format.  The second parameter is the size of Cycling Power Power  */
   /* Measurement Data.  The final parameter is the output that will    */
   /* contain data in the Buffer after formatting.  This function       */
   /* returns a zero if successful or a negative return error code if an*/
   /* error occurs.                                                     */
   /* * NOTE * The second parameter BufferLength is the size of input   */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
BTPSAPI_DECLARATION int BTPSAPI CPS_Format_CP_Measurement(CPS_Measurement_Data_t *MeasurementData, Word_t *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Format_CP_Measurement_t)(CPS_Measurement_Data_t *MeasurementData, Word_t *BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CPS Server interpreting it as a Cycling Power       */
   /* Measurement characteristic.  The first parameter is the length of */
   /* the value returned by the remote CPS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote CPS Server.  The  */
   /* final parameter is a pointer to store the parsed Cycling Power    */
   /* Measurement value.  This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
BTPSAPI_DECLARATION int BTPSAPI CPS_Decode_CP_Measurement(unsigned int BufferLength, Byte_t *Buffer, CPS_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Decode_CP_Measurement_t)(unsigned int BufferLength, Byte_t *Buffer, CPS_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CPS Server interpreting it as a Cycling Power Vector*/
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote CPS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote CPS Server.  This      */
   /* function returns a pointer to decoded CP Vector data or NULL if an*/
   /* error occurred.                                                   */
   /* * NOTE * The return value from this function MUST be freed by     */
   /*          calling CPS_Free_CP_Vector_Data() when the decoded CP    */
   /*          Vector data is no longer needed.                         */
BTPSAPI_DECLARATION CPS_Vector_Data_t *BTPSAPI CPS_Decode_CP_Vector(unsigned int BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef CPS_Vector_Data_t* (BTPSAPI *PFN_CPS_Decode_CP_Vector_t)(unsigned int BufferLength, Byte_t *Buffer);
#endif

   /* The following function is responsible for freeing the decoded CP  */
   /* Vector Data that was returned by a successful call to             */
   /* CPS_Decode_CP_Vector.  The only parameter to this function is the */
   /* CP Vector Data returned by the call to CPS_Decode_CP_Vector.      */
BTPSAPI_DECLARATION void BTPSAPI CPS_Free_CP_Vector_Data(CPS_Vector_Data_t *VectorData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_CPS_Free_CP_Vector_Data_t)(CPS_Vector_Data_t *VectorData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CPS Server interpreting it as a response code of    */
   /* Cycling Power Control Point.  The first parameter is the length of*/
   /* the value returned by the remote CPS Server.  The second parameter*/
   /* is a pointer to the data returned by the remote CPS Server.  The  */
   /* final parameter is a pointer to store the parsed Cycling Power    */
   /* Control Point Response data value.  This function returns a zero  */
   /* if successful or a negative return error code if an error occurs. */
BTPSAPI_DECLARATION int BTPSAPI CPS_Decode_Control_Point_Response(unsigned int BufferLength, Byte_t *Buffer, CPS_Control_Point_Response_Data_t *ResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Decode_Control_Point_Response_t)(unsigned int BufferLength, Byte_t *Buffer, CPS_Control_Point_Response_Data_t *ResponseData);
#endif

   /* The following function is responsible for freeing the             */
   /* SensorLocations data that is a member of                          */
   /* CPS_Supported_Sensor_Locations_t data that was returned by        */
   /* successful call to CPS_Decode_Control_Point_Response.  The only   */
   /* parameter to this function is the SensorLocations Byte pointer.   */
BTPSAPI_DECLARATION void BTPSAPI CPS_Free_Supported_Sensor_Locations_Data(Byte_t *SensorLocations);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_CPS_Free_Supported_Sensor_Locations_Data_t)(Byte_t *SensorLocations);
#endif

   /* The following function is responsible for formatting a Cycling    */
   /* Power Control Point Command into a user specified buffer.  The    */
   /* first parameter is the input Cycling Power Control Point Command  */
   /* to format.  The second parameter is the size of the input Cycling */
   /* Power Control Point Request Data.  The final parameter is the     */
   /* output that will contain data in the Buffer after formatting.     */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The second parameter BufferLength is the size of input   */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
BTPSAPI_DECLARATION int BTPSAPI CPS_Format_Control_Point_Command(CPS_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CPS_Format_Control_Point_Command_t)(CPS_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);
#endif

#endif
