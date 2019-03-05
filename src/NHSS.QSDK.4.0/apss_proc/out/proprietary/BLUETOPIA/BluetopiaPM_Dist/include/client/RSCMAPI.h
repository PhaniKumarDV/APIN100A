/*****< rscmapi.h >************************************************************/
/*      Copyright 2012 - 2015 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  RSCMAPI - Running Speed and Cadence Profile (RSC) Manager API for         */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/18/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __RSCMAPIH__
#define __RSCMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "RSCMMSG.h"             /* BTPM RSC Manager Message Formats.         */

   /* The following enumerated type represents the RSC Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of RSC Manager Changes.                                           */
typedef enum
{
   retRSCConnected,
   retRSCDisconnected,
   retRSCConfigurationStatusChanged,
   retRSCMeasurement,
   retRSCSensorLocationResponse,
   retRSCCumulativeValueUpdated,
   retRSCSensorLocationUpdated,
   retRSCProcedureComplete
} RSCM_Event_Type_t;

   /* The following structure is a container for the information        */
   /* returned in a retRSCConnected event. The RemoteDeviceAddress      */
   /* is the Bluetooth Address of the remote device.  The               */
   /* SupportedOptionalCharacteristics is a bitmask indicates which     */
   /* characteristics the remote device supports that are not mandatory.*/
   /* The Configured field will be TRUE if the remote device is bonded  */
   /* and was configured previously.                                    */
   /* * NOTE * This event is dispatched when a device is connected with */
   /*          its services known, and its services contain RSCS.       */
typedef struct _tagRSCM_Connected_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   Boolean_t     Configured;
   unsigned long SupportedOptionalCharateristics;
} RSCM_Connected_Event_Data_t;

#define RSCM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(RSCM_Connected_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCDisconnected event. The RemoteDeviceAddress is*/
   /* the Bluetooth Address of the remote device.                       */
   /* * NOTE * This event is dispatched when a device which declares    */
   /*          support for RSCS is disconnected.                        */
typedef struct _tagRSCM_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} RSCM_Disconnected_Event_Data_t;

#define RSCM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(RSCM_Disconnected_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a etRSCConfigurationStatus event. The                 */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The Configured member indicates the new configuration status.  The*/
   /* Status indicates the status of a configuration attempt.           */
typedef struct _tagRSCM_Configuration_Status_Changed_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   Boolean_t    Configured;
   unsigned int Status;
} RSCM_Configuration_Status_Changed_Event_Data_t;

#define RSCM_CONFIGURATION_STATUS_CHANGED_EVENT_DATA_SIZE      (sizeof(RSCM_Configuration_Status_Changed_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCMeasurement event. The RemoteDeviceAddress is */
   /* the Bluetooth Address of the remote device.  The MeasurementData  */
   /* is the information contained in the measurement notification.     */
typedef struct _tagRSCM_Measurement_Event_Data_t
{
   BD_ADDR_t                    RemoteDeviceAddress;
   RSCM_Measurement_Data_t      MeasurementData;
} RSCM_Measurement_Event_Data_t;

#define RSCM_MEASUREMENT_EVENT_DATA_SIZE                       (sizeof(RSCM_Measurement_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCSensorLocationResponse event. The             */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The SensorLocation is the location of the sensor.                 */
typedef struct _tagRSCM_Sensor_Location_Response_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           Status;
   RSCM_Sensor_Location_t SensorLocation;
} RSCM_Sensor_Location_Response_Event_Data_t;

#define RSCM_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE          (sizeof(RSCM_Sensor_Location_Response_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCCumulativeValueUpdated event. The             */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* * NOTE * This is dispatched when a RSCM_Update_Cumulative_Value() */
   /*          procedure successfully completes.                        */
typedef struct _tagRSCM_Cumulative_Value_Updated_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   DWord_t   CumulativeValue;
} RSCM_Cumulative_Value_Updated_Event_Data_t;

#define RSCM_CUMULATIVE_VALUE_UPDATED_EVENT_DATA_SIZE          (sizeof(RSCM_Cumulative_Value_Updated_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCSensorLocationUpdated event. The              */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The sensor location is the new location.                          */
   /* * NOTE * This event is NOT dispatched in responsent to            */
   /*          RSCM_Get_Sensor_Location(). It is dispatched when a      */
   /*          RSCM_Update_Sensor_Location() procedure successfully     */
   /*          completes.                                               */
typedef struct _tagRSCM_Sensor_Location_Updated_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   RSCM_Sensor_Location_t SensorLocation;
} RSCM_Sensor_Location_Updated_Event_Data_t;

#define RSCM_SENSOR_LOCATION_UPDATED_EVENT_DATA_SIZE           (sizeof(RSCM_Sensor_Location_Updated_Event_Data_t))

   /* The following structure is a container for the information        */
   /* returned in a retRSCProcedureComplete event. The                  */
   /* RemoteDeviceAddress is the Bluetooth Address of the remote device.*/
   /* The Procedure ID indicates the procedure which completed. The     */
   /* Status member indicates the status of the request. If the Status  */
   /* member indicates a ERROR_RESPONSE code, then the ResponseErrorCode*/
   /* will contain the control point response code from the remote      */
   /* device.                                                           */
   /* * NOTE * When the application receives this event, it is free to  */
   /*          start a new procedure (Set Location or Update Cumulative)*/
typedef struct _tagRSCM_Procedure_Complete_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ProcedureID;
   unsigned int Status;
   unsigned int ResponseErrorCode;
} RSCM_Procedure_Complete_Event_Data_t;

#define RSCM_PROCEDURE_COMPLETE_EVENT_DATA_SIZE                (sizeof(RSCM_Procedure_Complete_Event_Data_t))

   /* The following structure is a container that holds the Running     */
   /* Speed and Cadence Profile (RSC) Manager Event Data of a RSC       */
   /* Manager Event.                                                    */
typedef struct _tagRSCM_Event_Data_t
{
   RSCM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      RSCM_Connected_Event_Data_t                    ConnectedEventData;
      RSCM_Disconnected_Event_Data_t                 DisconnectedEventData;
      RSCM_Configuration_Status_Changed_Event_Data_t ConfigurationStatusChangedEventData;
      RSCM_Measurement_Event_Data_t                  MeasurementEventData;
      RSCM_Sensor_Location_Response_Event_Data_t     SensorLocationResponseEventData;
      RSCM_Cumulative_Value_Updated_Event_Data_t     CumulativeValueUpdatedEventData;
      RSCM_Sensor_Location_Updated_Event_Data_t      SensorLocationUpdatedEventData;
      RSCM_Procedure_Complete_Event_Data_t           ProcedureCompleteEventData;
   } EventData;
} RSCM_Event_Data_t;

#define RSCM_EVENT_DATA_SIZE                                   (sizeof(RSCM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the RSC */
   /* Profile (RSC) Manager dispatches an event (and the client has     */
   /* registered for events).  This function passes to the caller the   */
   /* RSC Manager Event and the Callback Parameter that was specified   */
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
typedef void (BTPSAPI *RSCM_Event_Callback_t)(RSCM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager RSC Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI RSCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI RSCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Collector callback function with the RSC    */
   /* Manager Service.  This Callback will be dispatched by the RSC     */
   /* Manager when various RSC Manager Collector Events occur.  This    */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a RSC Manager Collector Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          RSCM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Register_Collector_Event_Callback(RSCM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Register_Collector_Event_Callback_t)(RSCM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered RSC Manager Collector         */
   /* Event Callback (registered via a successful call to the           */
   /* RSCM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the Collector Event Callback ID (return value    */
   /* from RSCM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI RSCM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_RSCM_Un_Register_Collector_Event_Callback_t)(unsigned int CollectorCallbackID);
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
BTPSAPI_DECLARATION int BTPSAPI RSCM_Query_Connected_Sensors(unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Query_Connected_Sensors_t)(unsigned int MaximumRemoteDeviceListEntries, RSCM_Connected_Sensor_t *ConnectedDeviceList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function will attempt to configure a remote device  */
   /* which supports the RSC sensor role. It will register measurement  */
   /* and control point notifications and set the device up for         */
   /* usage. The RemoteDeviceAddress is the address of the remote       */
   /* sensor. This function returns zero if successful and a negative   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return from this call does not mean the     */
   /*          device has been configured. An                           */
   /*          etRSCConfigurationStatusChanged event will indicate the  */
   /*          status of the attempt to configure.                      */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Configure_Remote_Sensor_t)(BD_ADDR_t RemoteDeviceAddress, unsigned long Flags);
#endif

   /* The following function will un-configure a remote sensor which was*/
   /* previously configured. All notifications will be disabled. The    */
   /* RemoteDeviceAddress is the address of the remote sensor. This     */
   /* function returns zero if success and a negative return code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Un_Configure_Remote_Sensor(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Un_Configure_Remote_Sensor_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function queries information about a known remote   */
   /* sensor. The RemoteDeviceAddress parameter is the address of       */
   /* the remote sensor. The DeviceInfo parameter is a pointer to       */
   /* the Connected Sensor structure in which the data should be        */
   /* populated. This function returns zero if successful and a negative*/
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Get_Connected_Sensor_Info(BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Get_Connected_Sensor_Info_t)(BD_ADDR_t RemoteDeviceAddress, RSCM_Connected_Sensor_t *DeviceInfo);
#endif

   /* The following function queries the current sensor location from a */
   /* remote sensor. The RemoteDeviceAddress parameter is the address of*/
   /* the remote sensor. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
   /* * NOTE * If this function is succesful, the status of the request */
   /*          will be returned in a retRSCSensorLocationResponse event.*/
BTPSAPI_DECLARATION int BTPSAPI RSCM_Get_Sensor_Location(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Get_Sensor_Location_t)(BD_ADDR_t RemoteDeviceAddress);
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
   /*          retRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          retRSCCumulativeValueUpdated event will notify all       */
   /*          registered callbacks that the value has changed.         */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Update_Cumulative_Value(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Update_Cumulative_Value_t)(BD_ADDR_t RemoteDeviceAddress, DWord_t CumulativeValue);
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
   /*          retRSCProcedureComplete then attempt to resend.          */
   /* * NOTE * If this procedure completes succcessfully, a             */
   /*          retRSCSensorLocationUpdated event will notify all        */
   /*          registered callbacks that the location has changed.      */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Update_Sensor_Location(BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t SensorLocation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Update_Sensor_Location_t)(BD_ADDR_t RemoteDeviceAddress, RSCM_Sensor_Location_t SensorLocation);
#endif

   /* The following function sends a control point opcode to start      */
   /* sensor calibration on a remote sensor. The RemoteDeviceAddress    */
   /* parameter is the address of the remote sensor.  If successful,    */
   /* this function returns a postive integer which represents the      */
   /* Procedure ID associated with this procedure. If there is an error,*/
   /* this function returns a negative error code.                      */
   /* * NOTE * This function submits a control point procedure. Only one*/
   /*          procedure can be outstanding at a time. If this          */
   /*          function returns an error code indicating an             */
   /*          outstanding procedure, the caller can wait for an        */
   /*          retRSCProcedureComplete then attempt to resend.          */
BTPSAPI_DECLARATION int BTPSAPI RSCM_Start_Sensor_Calibration(BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_RSCM_Start_Sensor_Calibration_t)(BD_ADDR_t RemoteDeviceAddress);
#endif

#endif
