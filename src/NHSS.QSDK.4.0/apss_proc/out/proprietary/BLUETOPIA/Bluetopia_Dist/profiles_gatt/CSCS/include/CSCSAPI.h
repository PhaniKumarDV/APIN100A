/*****< cscsapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CSCSAPI - Stonestreet One Bluetooth Cycling Speed and Cadence Service     */
/*            (GATT based) API Type Definitions, Constants, and Prototypes.   */
/*                                                                            */
/*  Author:  Zahid Khan                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/22/13  Z. Khan        Initial creation.                               */
/******************************************************************************/
#ifndef __CSCSAPIH__
#define __CSCSAPIH__

#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API Prototypes/Constants. */
#include "CSCSType.h"       /* CSCS Service Types/Constants.                  */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define CSCS_ERROR_INVALID_PARAMETER                       (-1000)
#define CSCS_ERROR_INVALID_BLUETOOTH_STACK_ID              (-1001)
#define CSCS_ERROR_INSUFFICIENT_RESOURCES                  (-1002)
#define CSCS_ERROR_INSUFFICIENT_BUFFER_SPACE               (-1003)
#define CSCS_ERROR_SERVICE_ALREADY_REGISTERED              (-1004)
#define CSCS_ERROR_INVALID_INSTANCE_ID                     (-1005)
#define CSCS_ERROR_MALFORMATTED_DATA                       (-1006)
#define CSCS_ERROR_INDICATION_OUTSTANDING                  (-1007)
#define CSCS_ERROR_NO_AUTHENTICATION                       (-1008)
#define CSCS_ERROR_UNKNOWN_ERROR                           (-1009)

   /* The following structure contains the Handles that will need to be */
   /* cached by a CSCS client in order to only do service discovery     */
   /* once.                                                             */
typedef struct _tagCSCS_Client_Information_t
{
   Word_t CSC_Measurement;
   Word_t CSC_Measurement_Client_Configuration;
   Word_t CSC_Feature;
   Word_t Sensor_Location;
   Word_t SC_Control_Point;
   Word_t SC_Control_Point_Client_Configuration;
} CSCS_Client_Information_t;

#define CSCS_CLIENT_INFORMATION_DATA_SIZE                (sizeof(CSCS_Client_Information_t))

   /* The following structure contains all of the per Client data that  */
   /* will need to be stored by a CSCS Server.                          */
typedef struct _tagCSCS_Server_Information_t
{
   Word_t CSC_Measurement_Client_Configuration;
   Word_t SC_Control_Point_Client_Configuration;
} CSCS_Server_Information_t;

#define CSCS_SERVER_INFORMATION_DATA_SIZE                (sizeof(CSCS_Server_Information_t))

   /* The following structure defines the format of the optional Wheel  */
   /* Revolution Data field of the CSC Mesaurement characteristic.  The */
   /* Cumulative Wheel Revolutions field represents the number of times */
   /* wheel was rotated. The Las Wheel Event Time is free-running-count */
   /* of 1/1024 second units and it represents the time when the last   */
   /* wheel revolution was detected by the wheel rotation sensor.       */
typedef struct _tagCSCS_Wheel_Revolution_Data_t
{
   DWord_t CumulativeWheelRevolutions;
   Word_t  LastWheelEventTime;
} CSCS_Wheel_Revolution_Data_t;

#define CSCS_WHEEL_REVOLUTIION_DATA_SIZE                 (sizeof(CSCS_Wheel_Revolution_Data_t))

   /* The following structure defines the format of the optional Crank  */
   /* Revolution Data field of the CSC Mesaurement characteristic.  The */
   /* Cumulative Crank Revolutions field represents the number of times */
   /* crank was rotated. The Las Crank Event Time is free-running-count */
   /* of 1/1024 second units and it represents the time when the last   */
   /* crank revolution was detected by the crank rotation sensor.       */
typedef struct _tagCSCS_Crank_Revolution_Data_t
{
   Word_t CumulativeCrankRevolutions;
   Word_t LastCrankEventTime;
} CSCS_Crank_Revolution_Data_t;

#define CSCS_CRANK_REVOLUTIION_DATA_SIZE                 (sizeof(CSCS_Crank_Revolution_Data_t))

   /* The following defines the structure of the CSC Measurement Data   */
   /* that is passed to the function that builds the CSC Measurement    */
   /* packet.                                                           */
   /* * NOTE * If the                                                   */
   /*          CSCS_CSC_MEASUREMENT_FLAGS_WHEEL_REVOLUTION_DATA_PRESENT */
   /*          Flag is set, then a valid value must be entered for Wheel*/
   /*          Revolution Data.                                         */
   /*          If the                                                   */
   /*          CSCS_CSC_MEASUREMENT_FLAGS_CRANK_REVOLUTION_DATA_PRESENT */
   /*          Flag is set, then a valid value must be entered for Crank*/
   /*          Revolution Data.                                         */
typedef struct _tagCSCS_CSC_Measurement_Data_t
{
   Byte_t                       Flags;
   CSCS_Wheel_Revolution_Data_t WheelRevolutionData;
   CSCS_Crank_Revolution_Data_t CrankRevolutionData;
} CSCS_CSC_Measurement_Data_t;

#define CSCS_CSC_MEASUREMENT_DATA_SIZE                   (sizeof(CSCS_CSC_Measurement_Data_t))

#define MAXIMUM_SUPPORTED_SENSOR_LOCATIONS               (17)

   /* The following defines the format of a Supported Sensor Location   */
   /* Values that will be used to respond to Supported Sensor Locations */
   /* request made by remote device. The first member represents the    */
   /* Byte array of multiple Sensor Locations. The second member        */
   /* represents Number of Sensor Locations available in the Byte array */
typedef struct _tagCSCS_SCCP_Supported_Sensor_Locations_t
{
   Byte_t SensorLocations[MAXIMUM_SUPPORTED_SENSOR_LOCATIONS];
   Byte_t NumberOfSensorLocations;
} CSCS_SCCP_Supported_Sensor_Locations_t;

#define CSCS_SCCP_SUPPORTED_SENSOR_LOCATIONS_SIZE        (sizeof(CSCS_SCCP_Supported_Sensor_Locations_t))

   /* The following defines the format of a SC Control Point Response   */
   /* Data. This structure will hold the SC Control Point response data */
   /* received from remote CSCS Server. The first member                */
   /* RequestOpCode must be of the form CSCS_SC_CONTROL_POINT_OPCODE_XXX*/
   /* And the second member ResponseCodeValue must be of the form       */
   /* CSCS_SC_CONTROL_POINT_RESPONSE_CODE_XXX. The third member is      */
   /* Supported Sensor Locations, this field must be filled with valid  */
   /* values when client has made                                       */
   /* CSCS_SC_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS   */
   /* request via SC Control Point and when Multiple sensor locations   */
   /* are available in CSCS Server                                      */
typedef struct _tagCSCS_SC_Control_Point_Response_Data_t
{
   Byte_t                                 RequestOpCode;
   Byte_t                                 ResponseCodeValue;
   CSCS_SCCP_Supported_Sensor_Locations_t SupportedSensorLocations;
} CSCS_SC_Control_Point_Response_Data_t;

#define CSCS_SC_CONTROL_POINT_RESPONSE_DATA_SIZE         (sizeof(CSCS_SC_Control_Point_Response_Data_t))

   /* The following define the valid Read/Write Client Configuration    */
   /* Request types that a server may receive in a                      */
   /* etCSCS_Server_Read_Client_Configuration_Request or                */
   /* etCSCS_Server_Client_Configuration_Update event.  This type is    */
   /* also used by Notify/Indicate APIs to denote the characteristic    */
   /* value to notify or indicate.                                      */
   /* * NOTE * For each event it is up to the application to return (or */
   /*          write) the correct Client Configuration descriptor based */
   /*          on this value.                                           */
typedef enum
{
   ctCSCMeasurement,
   ctSCControlPoint
} CSCS_Characteristic_Type_t;

   /* The following enumeration covers all the events generated by the  */
   /* CSCS Service.  These are used to determine the type of each event */
   /* generated, and to ensure the proper union element is accessed for */
   /* the CSCS_Event_Data_t structure.                                  */
typedef enum
{
   etCSCS_Read_Client_Configuration_Request,
   etCSCS_Client_Configuration_Update,
   etCSCS_Control_Point_Command,
   etCSCS_Confirmation_Data
} CSCS_Event_Type_t;

   /* The following is dispatched to a CSCS Server when a CSCS Client   */
   /* is attempting to read the Client Configuration descriptor.  The   */
   /* ConnectionID, and RemoteDevice identifies the Client that is      */
   /* making the request.  The TransactionID specifies the TransactionID*/
   /* of the request, this can be used when responding to the request   */
   /* using the CSCS_Client_Configuration_Read_Response() API function. */
typedef struct _tagCSCS_Read_Client_Configuration_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   unsigned int               TransactionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   CSCS_Characteristic_Type_t ClientConfigurationType;
} CSCS_Read_Client_Configuration_Data_t;

#define CSCS_READ_CLIENT_CONFIGURATION_DATA_SIZE         (sizeof(CSCS_Read_Client_Configuration_Data_t))

   /* The following is dispatched to a CSCS Server when a CSCS Client   */
   /* attempts to write to a Client Configuration descriptor.  The      */
   /* ConnectionID and RemoteDevice identify the Client that is making  */
   /* the update request.  The ClientConfiguration value specifies the  */
   /* new Client Configuration value.                                   */
typedef struct _tagCSCS_Client_Configuration_Update_Data_t
{
   unsigned int               InstanceID;
   unsigned int               ConnectionID;
   GATT_Connection_Type_t     ConnectionType;
   BD_ADDR_t                  RemoteDevice;
   CSCS_Characteristic_Type_t ClientConfigurationType;
   Word_t                     ClientConfiguration;
} CSCS_Client_Configuration_Update_Data_t;

#define CSCS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE       (sizeof(CSCS_Client_Configuration_Update_Data_t))

   /* The following enumerates the valid values that may be set as the  */
   /* value for the OpCode field of SC Control Point characteristic.    */
typedef enum
{
   cpcSetCumulativeValueRequest        = CSCS_SC_CONTROL_POINT_OPCODE_SET_CUMULATIVE_VALUE,
   cpcStartSensorCalibrationRequest    = CSCS_SC_CONTROL_POINT_OPCODE_START_SENSOR_CALIBRATION,
   cpcUpdateSensorLocationRequest      = CSCS_SC_CONTROL_POINT_OPCODE_UPDATE_SENSOR_LOCATION,
   cpcSupportedSensorLocationsRequest  = CSCS_SC_CONTROL_POINT_OPCODE_REQUEST_SUPPORTED_SENSOR_LOCATIONS
} CSCS_SCCP_Command_Type_t;

   /* The following structure defines the format of the SC Control      */
   /* Point Command Request Data. This structure is passed as a         */
   /* parameter to CSCS_Format_Control_Point_Command API                */
typedef struct _tagCSCS_SC_Control_Point_Format_Data_t
{
   CSCS_SCCP_Command_Type_t CommandType;
   union
   {
      DWord_t CumulativeValue;
      Byte_t  SensorLocation;
   } CmdParameter;
} CSCS_SC_Control_Point_Format_Data_t;

#define CSCS_SC_CONTROL_POINT_FORMAT_DATA_SIZE           (sizeof(CSCS_SC_Control_Point_Format_Data_t))

   /* The following is dispatched to a CSCS Server in response to the   */
   /* reception of request from a Client to write to the CS Control     */
   /* Point.                                                            */
typedef struct _tagCSCS_SC_Control_Point_Command_Data_t
{
   unsigned int                        InstanceID;
   unsigned int                        ConnectionID;
   unsigned int                        TransactionID;
   GATT_Connection_Type_t              ConnectionType;
   BD_ADDR_t                           RemoteDevice;
   CSCS_SC_Control_Point_Format_Data_t FormatData;
} CSCS_SC_Control_Point_Command_Data_t;

#define CSCS_SC_CONTROL_POINT_COMMAND_DATA_SIZE          (sizeof(CSCS_SC_Control_Point_Command_Data_t))

   /* The following CSCS Service Event is dispatched to a CSCS Server   */
   /* when a CSCS Client has sent a confirmation to a previously sent   */
   /* Indication.  The InstanceID specifies the Unique Service Instance.*/
   /* The ConnectionID, ConnectionType, and RemoteDevice specifies the  */
   /* Client that is sending the confirmation. The final parameter      */
   /* specifies the status of the Indication.                           */
   /* * NOTE * The Status member is set to one of the following values: */
   /*                GATT_CONFIRMATION_STATUS_SUCCESS                   */
   /*                GATT_CONFIRMATION_STATUS_TIMEOUT                   */
typedef struct _tagCSCS_Confirmation_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 Status;
} CSCS_Confirmation_Data_t;

#define CSCS_CONFIRMATION_DATA_SIZE                      (sizeof(CSCS_Confirmation_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all CSCS Service Event Data.  This structure is received  */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagCSCS_Event_Data_t
{
   CSCS_Event_Type_t Event_Data_Type;
   Word_t            Event_Data_Size;
   union
   {
      CSCS_Read_Client_Configuration_Data_t   *CSCS_Read_Client_Configuration_Data;
      CSCS_Client_Configuration_Update_Data_t *CSCS_Client_Configuration_Update_Data;
      CSCS_SC_Control_Point_Command_Data_t    *CSCS_Control_Point_Command_Data;
      CSCS_Confirmation_Data_t                *CSCS_Confirmation_Data;
   } Event_Data;
} CSCS_Event_Data_t;

#define CSCS_EVENT_DATA_SIZE                             (sizeof(CSCS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a CSCS Service Event Receive Data Callback.  This function will   */
   /* be called whenever an CSCS Service Event occurs that is           */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the CSCS Event Data  */
   /* that occurred and the CSCS Service Event Callback Parameter that  */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the CSCS Service Event Data ONLY in   */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer This function is guaranteed NOT to  */
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* re-entrant).  It needs to be noted however, that if the same      */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another CSCS Service Event will not be processed while this       */
   /* function call is outstanding).                                    */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving CSCS Service Event  */
   /*            Packets.  A Deadlock WILL occur because NO CSCS Event  */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *CSCS_Event_Callback_t)(unsigned int BluetoothStackID, CSCS_Event_Data_t *CSCS_Event_Data, unsigned long CallbackParameter);

   /* CSCS Server API.                                                  */

   /* The following function is responsible for opening a CSCS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered CSCS       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 CSCS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Initialize_Service(unsigned int BluetoothStackID, CSCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Initialize_Service_t)(unsigned int BluetoothStackID, CSCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a CSCS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered CSCS       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 CSCS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, CSCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, CSCS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened CSCS Server.  The first parameter is the Bluetooth Stack   */
   /* ID on which to close the server.  The second parameter is the     */
   /* InstanceID that was returned from a successful call to            */
   /* CSCS_Initialize_Service().  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
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
BTPSAPI_DECLARATION unsigned long BTPSAPI CSCS_Suspend(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned long (BTPSAPI *PFN_CSCS_Suspend_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is used to perform a resume of the         */
   /* Bluetooth stack after a successful suspend has been performed (see*/
   /* CSCS_Suspend()).  This function accepts as input the Bluetooth    */
   /* Stack ID of the Bluetooth Stack that the Device is associated     */
   /* with.  The final two parameters are the buffer size and buffer    */
   /* that contains the memory that was used to collapse Bluetopia      */
   /* context into with a successfully call to CSCS_Suspend().  This    */
   /* function returns ZERO on success or a negative error code.        */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Resume(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Resume_t)(unsigned int BluetoothStackID, unsigned long BufferSize, void *Buffer);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the CSCS Service that is         */
   /* registered with a call to CSCS_Initialize_Service().  This        */
   /* function returns the non-zero number of attributes that are       */
   /* contained in a CSCS Server or zero on failure.                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI CSCS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_CSCS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for responding to a CSCS    */
   /* Read Client Configuration Request.  The first parameter is the    */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* CSCS_Initialize_Server().  The third is the Transaction ID of the */
   /* request.  The final parameter contains the Client Configuration   */
   /* to send to the remote device.  This function returns a zero if    */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Read_Client_Configuration_Response_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration);
#endif

   /* The following function is responsible for sending an CSC          */
   /* Measurement notification to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CSCS_Initialize_Server().  The third parameter is the          */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the CSC Measurement Data strcuture that    */
   /* contains all of the required and optional data for the            */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Notify_CSC_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_CSC_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Notify_CSC_Measurement_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_CSC_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for setting the supported   */
   /* CSC features on the specified CSCS Instance.  The first parameter */
   /* is the Bluetooth Stack ID of the Bluetooth Device. The second     */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CSCS_Initialize_Server().  The final parameter is a bitmask of    */
   /* the supported features to set for the specified CSCS Instance.    */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The SupportedFeatures bitmask parameter should be 1 or   */
   /* more combination of CSCS_CSC_FEATURE_XXX bitmask values           */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Set_CSC_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Set_CSC_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures);
#endif

   /* The following function is responsible for querying the current    */
   /* CSC Features on the specified CSCS Instance.  The first parameter */
   /* is the Bluetooth Stack ID of the Bluetooth Device.  The second    */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CSCS_Initialize_Server().  The final parameter is a pointer to    */
   /* return the current CSC Features for the specified CSCS Instance.  */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Query_CSC_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Query_CSC_Feature_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures);
#endif

   /* The following function is responsible for setting the Sensor      */
   /* Location on the specified CSCS Instance.  The first parameter is  */
   /* the Bluetooth Stack ID of the Bluetooth Device. The second        */
   /* parameter is the InstanceID returned from a successful call to    */
   /* CSCS_Initialize_Server().  The final parameter is the Sensor      */
   /* Location to set for the specified CSCS Instance.  This function   */
   /* returns a zero if successful or a negative return error code if   */
   /* an error occurs.                                                  */
   /* * NOTE * The SensorLocation parameter should be in a range between*/
   /* CSCS_SENSOR_LOCATION_OTHER to CSCS_SENSOR_LOCATION_REAR_HUB       */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Set_Sensor_Location(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Set_Sensor_Location_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t SensorLocation);
#endif

   /* The following function is responsible for querying the current    */
   /* Sensor Location on the specified CSCS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CSCS_Initialize_Server().  The final parameter is a pointer to */
   /* return the current Sensor Location for the specified CSCS         */
   /* Instance. This function returns a zero if successful or a         */
   /* negative return error code if an error occurs.                    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Query_Sensor_Location(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Query_Sensor_Location_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *SensorLocation);
#endif

   /* The following function is responsible for responding to a SC      */
   /* Control Point Command received from a remote device.  The first   */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.      */
   /* The second is the TransactionID that was received in the SC       */
   /* Control Point event.  The final parameter is an error code that   */
   /* is used to determine if the Request is being accepted by the      */
   /* server or if an error response should be issued instead.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject SC Control Point commands when the Client is      */
   /*          already in progress or SC Control Point is not properly  */
   /*          configured for indications.  All other reasons should    */
   /*          return ZERO for the ErrorCode and then send SC Control   */
   /*          Point Result indication to indicate any other errors.    */
   /*          For Example: If the Op Code in the Request is not        */
   /*          supported by the Server, this API should be called with  */
   /*          ErrorCode set to ZERO and then the                       */
   /*          CSCS_Indicate_SC_Control_Point_Result() should be called */
   /*          with the ResponseCode set to                             */
   /*          CSCS_SC_CONTROL_POINT_RESPONSE_CODE_OPCODE_NOT_SUPPORTED.*/
BTPSAPI_DECLARATION int BTPSAPI CSCS_SC_Control_Point_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_SC_Control_Point_Response_t)(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode);
#endif

   /* The following function is responsible for sending the SC Control  */
   /* Point indication to a specified remote device.  The first         */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to CSCS_Initialize_Server().  The third parameter the ConnectionID*/
   /* of the remote device to send the indication to.  The fourth       */
   /* parameter is the Request data to indicate.  The last parameter is */
   /* response code .This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 SC Control Point Request indication may be        */
   /*          outstanding per CSCS Instance.                           */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Indicate_SC_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_SCCP_Command_Type_t CommandType, Byte_t ResponseCode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Indicate_SC_Control_Point_Result_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_SCCP_Command_Type_t CommandType, Byte_t ResponseCode);
#endif

   /* The following function is responsible for sending the SC Control  */
   /* Point indication for Supported Sensor Locations request to a      */
   /* specified remote device.  The first parameter is the Bluetooth    */
   /* Stack ID of the Bluetooth Device.  The second parameter is the    */
   /* InstanceID returned from a successful call to                     */
   /* CSCS_Initialize_Server().  The third parameter the ConnectionID   */
   /* of the remote device to send the indication to.  The fourth       */
   /* parameter is the Supported Sensor Locations to indicate.          */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * Only 1 SC Control Point Request indication may be        */
   /*          outstanding per CSCS Instance.                           */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Indicate_Supported_Sensor_Locations(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_SCCP_Supported_Sensor_Locations_t *SupportedSensorLocations);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Indicate_Supported_Sensor_Locations_t)(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, CSCS_SCCP_Supported_Sensor_Locations_t *SupportedSensorLocations);
#endif

   /* CSCS Client API.                                                  */

   /* The following function is responsible for parsing a value received*/
   /* from a remote CSCS Server interpreting it as a CSC Measurement    */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote CSCS Server.  The second parameter is a    */
   /* pointer to the data returned by the remote CSCS Server. The final */
   /* parameter is a pointer to store the parsed CSC Measurement value. */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Decode_CSC_Measurement(unsigned int ValueLength, Byte_t *Value, CSCS_CSC_Measurement_Data_t *MeasurementData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Decode_CSC_Measurement_t)(unsigned int ValueLength, Byte_t *Value, CSCS_CSC_Measurement_Data_t *MeasurementData);
#endif

   /* The following function is responsible for parsing a value received*/
   /* from a remote CSCS Server interpreting it as a response code of   */
   /* SC Control Point. The first parameter is the length of the value  */
   /* returned by the remote CSCS Server.The second parameter is a      */
   /* pointer to the data returned by the remote CSCS Server.The final  */
   /* parameter is a pointer to store the parsed SC Control Point       */
   /* Response data value.This function returns a zero if successful or */
   /* a negative return error code if an error occurs.                  */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Decode_SC_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, CSCS_SC_Control_Point_Response_Data_t *SCControlPointResponse);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Decode_SC_Control_Point_Response_t)(unsigned int ValueLength, Byte_t *Value, CSCS_SC_Control_Point_Response_Data_t *SCControlPointResponse);
#endif

   /* The following function is responsible for formatting a SC Control */
   /* Point Command into a user specified buffer.  The first parameter  */
   /* is the input SC Control Point Command to format.  The second      */
   /* parameter is size of the input SC Control Poin Request Data.      */
   /* The final parameter is the output that will contain data in the   */
   /* Buffer after formatting.  This function returns a zero if         */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The second parameter BufferLength is the size of input   */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
BTPSAPI_DECLARATION int BTPSAPI CSCS_Format_SC_Control_Point_Command(CSCS_SC_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCS_Format_SC_Control_Point_Command_t)(CSCS_SC_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer);
#endif

#endif
