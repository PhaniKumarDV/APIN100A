/*****< cscmapi.h >************************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  CSCMAPI - Cycling Speed and Cadence Profile (CSC) Manager API for         */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/07/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __CSCMAPIH__
#define __CSCMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "CSCMMSG.h"             /* BTPM CSC Manager Message Formats.         */

   /* The following enumerated type represents the CSC Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of CSC Manager Changes.                                           */
typedef enum
{
   cetCSCConnected,
   cetCSCDisconnected,
   cetCSCConfigurationStatusChanged,
   cetCSCMeasurement,
   cetCSCSensorLocationResponse,
   cetCSCCumulativeValueUpdated,
   cetCSCSensorLocationUpdated,
   cetCSCProcedureComplete
} CSCM_Event_Type_t;

   /* The following structure is a container for the information        */
   /* returned in a cetCSCConnected event. The RemoteDeviceAddress      */
   /* is the Bluetooth Address of the remote device.  The               */
   /* SupportedOptionalCharacteristics is a bitmask indicates which     */
   /* characteristics the remote device supports that are not mandatory.*/
   /* The Configured field will be TRUE if the remote device is bonded  */
   /* and was configured previously.                                    */
   /* * NOTE * This event is dispatched when a device is connected with */
   /*          its services known, and its services contain CSCS.       */
typedef struct _tagCSCM_Connected_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   Boolean_t     Configured;
   unsigned long SupportedOptionalCharateristics;
} CSCM_Connected_Event_Data_t;

#define CSCM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(CSCM_Connected_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCDisconnected event. The RemoteDeviceAddress is*/
   /* the Bluetooth Address of the remote device.                       */
   /* * NOTE * This event is dispatched when a device which declares    */
   /*          support for CSCS is disconnected.                        */
typedef struct _tagCSCM_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} CSCM_Disconnected_Event_Data_t;

#define CSCM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(CSCM_Disconnected_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a etCSCConfigurationStatus event. The                 */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The Configured member indicates the new configuration status.  The*/
   /* Status indicates the status of a configuration attempt.           */
   /* * NOTE * If the Status field indicates a failure, than the        */
   /*          Configured field will be unchanged.                      */
typedef struct _tagCSCM_Configuration_Status_Changed_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   Boolean_t    Configured;
   unsigned int Status;
} CSCM_Configuration_Status_Changed_Event_Data_t;

#define CSCM_CONFIGURATION_STATUS_CHANGED_EVENT_DATA_SIZE      (sizeof(CSCM_Configuration_Status_Changed_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCMeasurement event. The RemoteDeviceAddress is */
   /* the Bluetooth Address of the remote device.  The Flags indicate   */
   /* which of the data structure are present in the measurement        */
   /* event. The Wheel and Crank Revolution Data represent the          */
   /* measurement data for their respectives measurements if the Flags  */
   /* members indicate they are present.                                */
typedef struct _tagCSCM_Measurement_Event_Data_t
{
   BD_ADDR_t                    RemoteDeviceAddress;
   unsigned long                Flags;
   CSCM_Wheel_Revolution_Data_t WheelRevolutionData;
   CSCM_Crank_Revolution_Data_t CrankRevolutionData;
} CSCM_Measurement_Event_Data_t;

#define CSCM_MEASUREMENT_EVENT_DATA_SIZE                       (sizeof(CSCM_Measurement_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCSensorLocationResponse event. The             */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The SensorLocation is the location of the sensor.                 */
typedef struct _tagCSCM_Sensor_Location_Response_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           Status;
   CSCM_Sensor_Location_t SensorLocation;
} CSCM_Sensor_Location_Response_Event_Data_t;

#define CSCM_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE          (sizeof(CSCM_Sensor_Location_Response_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCCumulativeValueUpdated event. The             */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* * NOTE * This is dispatched when a CSCM_Update_Cumulative_Value() */
   /*          procedure successfully completes.                        */
typedef struct _tagCSCM_Cumulative_Value_Updated_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   DWord_t   CumulativeValue;
} CSCM_Cumulative_Value_Updated_Event_Data_t;

#define CSCM_CUMULATIVE_VALUE_UPDATED_EVENT_DATA_SIZE          (sizeof(CSCM_Cumulative_Value_Updated_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCSensorLocationUpdated event. The              */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The sensor location is the new location.                          */
   /* * NOTE * This event is NOT dispatched in responsent to            */
   /*          CSCM_Get_Sensor_Location(). It is dispatched when a      */
   /*          CSCM_Update_Sensor_Location() procedure successfully     */
   /*          completes.                                               */
typedef struct _tagCSCM_Sensor_Location_Updated_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   CSCM_Sensor_Location_t SensorLocation;
} CSCM_Sensor_Location_Updated_Event_Data_t;

#define CSCM_SENSOR_LOCATION_UPDATED_EVENT_DATA_SIZE           (sizeof(CSCM_Sensor_Location_Updated_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a cetCSCProcedureComplete event. The                  */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The Procedure ID indicates the procedure which completed. The     */
   /* Status member indicates the status of the request. If the Status  */
   /* member indicates a ERROR_RESPONSE code, then the ResponseErrorCode*/
   /* will contain the control point response code from the remote      */
   /* device.                                                           */
   /* * NOTE * When the application receives this event, it is free to  */
   /*          start a new procedure (Set Location or Update Cumulative)*/
typedef struct _tagCSCM_Procedure_Complete_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ProcedureID;
   unsigned int Status;
   unsigned int ResponseErrorCode;
} CSCM_Procedure_Complete_Event_Data_t;

#define CSCM_PROCEDURE_COMPLETE_EVENT_DATA_SIZE                (sizeof(CSCM_Procedure_Complete_Event_Data_t))

   /* The following structure is a container that holds the Cycling     */
   /* Speed and Cadence Profile (CSC) Manager Event Data of a CSC       */
   /* Manager Event.                                                    */
typedef struct _tagCSCM_Event_Data_t
{
   CSCM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      CSCM_Connected_Event_Data_t ConnectedEventData;
      CSCM_Disconnected_Event_Data_t DisconnectedEventData;
      CSCM_Configuration_Status_Changed_Event_Data_t ConfigurationStatusChangedEventData;
      CSCM_Measurement_Event_Data_t MeasurementEventData;
      CSCM_Sensor_Location_Response_Event_Data_t SensorLocationResponseEventData;
      CSCM_Cumulative_Value_Updated_Event_Data_t CumulativeValueUpdatedEventData;
      CSCM_Sensor_Location_Updated_Event_Data_t SensorLocationUpdatedEventData;
      CSCM_Procedure_Complete_Event_Data_t ProcedureCompleteEventData;
   } EventData;
} CSCM_Event_Data_t;

#define CSCM_EVENT_DATA_SIZE                                   (sizeof(CSCM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the CSC */
   /* Profile (CSC) Manager dispatches an event (and the client has     */
   /* registered for events).  This function passes to the caller the   */
   /* CSC Manager Event and the Callback Parameter that was specified   */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *CSCM_Event_Callback_t)(CSCM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager CSC Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI CSCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI CSCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the CSC    */
   /* Manager Service.  This Callback will be dispatched by the CSC     */
   /* Manager when various CSC Manager Collector Events occur.  This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a CSC Manager Collector Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          CSCM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Register_Collector_Event_Callback(CSCM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Register_Collector_Event_Callback_t)(CSCM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered CSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* CSCM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the Collector Event Callback ID (return value    */
   /* from CSCM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI CSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_CSCM_Un_Register_Collector_Event_Callback_t)(unsigned int CollectorCallbackID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected Time    */
   /* Profile devices.  This function accepts the buffer information    */
   /* to receive any currently connected devices.  The first parameter  */
   /* specifies the maximum number of entries that the buffer will      */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the total   */
   /* number of connected devices if the function is successful.  The   */
   /* final parameter can be used to retrieve the total number of       */
   /* connected devices (regardless of the size of the list specified by*/
   /* the first two parameters).  This function returns a non-negative  */
   /* value if successful which represents the number of connected      */
   /* devices that were copied into the specified input buffer.  This   */
   /* function returns a negative return error code if there was an     */
   /* error.                                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, CSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Query_Connected_Sensors_t)(unsigned int MaximumRemoteDeviceListEntries, CSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function will attempt to configure a remote device  */
   /* which supports the CSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etCSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Configure_Remote_Sensor_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);
#endif

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Un_Configure_Remote_Sensor_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, CSCM_Connected_Sensor_t *DeviceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Get_Connected_Sensor_Info_t)(BD_ADDR_t RemoteDeviceAddress, CSCM_Connected_Sensor_t *DeviceInfo);
#endif

   /* The following function queries the current sensor location from a */
   /* remote sensor. The RemoteDeviceAddress parameter is the address of*/
   /* the remote sensor. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
   /* * NOTE * If this function is succesful, the status of the request */
   /*          will be returned in a cetCSCSensorLocationResponse event.*/
BTPSAPI_DECLARATION int BTPSAPI CSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Get_Sensor_Location_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function sends a control point opcode to update     */
   /* the cumulative value on a remote sensor. The RemoteDeviceAddress  */
   /* parameter is the address of the remote sensor. The CumulativeValue*/
   /* parameter is the value to set on the remote sensor. If successful,*/
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Update_Cumulative_Value_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);
#endif

   /* The following function sends a control point opcode to update     */
   /* the sensor location on a remote sensor. The RemoteDeviceAddress   */
   /* parameter is the address of the remote sensor. The SensorLocation */
   /* parameter is the new location to set.  If successful, this        */
   /* function returns a postive integer which represents the Procedure */
   /* ID associated with this procedure. If there is an error, this     */
   /* function returns a negative error code.                           */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * Only one procedure can be outstanding at a time. If this */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          cetCSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          cetCSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
BTPSAPI_DECLARATION int BTPSAPI CSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, CSCM_Sensor_Location_t SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_CSCM_Update_Sensor_Location_t)(BD_ADDR_t RemoteDeviceAddress, CSCM_Sensor_Location_t SensorLocation);
#endif

#endif
