/*****< CPPMAPI.h >************************************************************/
/*      Copyright (c) 2016 Qualcomm Technologies, Inc.                        */
/*      All Rights Reserved                                                   */
/*                                                                            */
/*  CPPMAPI - Cycling Power Client Application Programming Interface          */
/*                                                                            */
/*  Author:  Glenn Steenrod                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/06/15  G. Steenrod    Initial creation                                */
/******************************************************************************/
#ifndef __CPPMAPIH__
#define __CPPMAPIH__

   /* CPPM_Event_Type_t is an enum used to identify cycling power       */
   /* events in the EventType field of the CPPM_Event_Data_t struct.    */ 
typedef enum
{
   cetConnectedCPP,
   cetDisconnectedCPP,
   cetMeasurementsSetCPP,
   cetVectorsSetCPP,
   cetProceduresSetCPP,
   cetBroadcastsSetCPP,
   cetProcedureBegunCPP,
   cetMeasurementCPP,
   cetVectorCPP,
   cetControlPointCPP,
   cetSensorFeaturesCPP,
   cetSensorLocationCPP
} CPPM_Event_Type_t;

typedef struct _tagCPPM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   CPPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
   unsigned int           NumberOfInstances;
} CPPM_Connected_Event_Data_t;

#define CPPM_CONNECTED_EVENT_DATA_SIZE   (sizeof(CPPM_Connected_Event_Data_t))

typedef struct _tagCPPM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   CPPM_Connection_Type_t ConnectionType;
} CPPM_Disconnected_Event_Data_t;

#define CPPM_DISCONNECTED_EVENT_DATA_SIZE   (sizeof(CPPM_Disconnected_Event_Data_t))

   /* The structure CPPM_Write_Response_Event_Data_t contains data from */
   /* sensor responses to GATM writes. GATM writes are attempted when   */
   /* measurement and vector notifications, control point indications,  */
   /* and broadcasts are enabled, and when procedures are initiated.    */
   /* The TransactionID field holds the transaction ID from the GATM    */
   /* write and the status contains an error code if an error has       */
   /* occurred.                                                         */
typedef struct _tagCPPM_Write_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int TransactionID;
   int          Status;
} CPPM_Write_Response_Event_Data_t;

#define CPPM_WRITE_RESPONSE_EVENT_DATA_SIZE   (sizeof(CPPM_Write_Response_Event_Data_t))

   /* Data from cycling power measurement notifications, delivered in   */
   /* cetMeasurementCPP type events, is contained in                    */
   /* CPPM_Measurement_Event_Data_t type structures. See CPPMType.h for */
   /* the definition of the CPPM_Measurement_Data_t struct.             */
typedef struct _tagCPPM_Measurement_Event_Data_t
{
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            InstanceID;
   CPPM_Measurement_Data_t Measurement;
} CPPM_Measurement_Event_Data_t;

#define CPPM_MEASUREMENT_EVENT_DATA_SIZE   (sizeof(CPPM_Measurement_Event_Data_t))

   /* Data from cycling power vector notifications, delivered in        */
   /* cetVectorCPP type events, is contained in                         */
   /* CPPM_Vector_Event_Data_t type structures. See CPPMType.h for the  */
   /* the definition of the CPPM_Vector_Data_t struct.                  */
typedef struct _tagCPPM_Vector_Event_Data_t
{
   BD_ADDR_t          RemoteDeviceAddress;
   unsigned int       InstanceID;
   CPPM_Vector_Data_t Vector;
} CPPM_Vector_Event_Data_t;

#define CPPM_VECTOR_EVENT_DATA_SIZE   (sizeof(CPPM_Vector_Event_Data_t))

   /* Data from cycling power control point indications, delivered in   */
   /* cetControlPointCPP type events, is contained in                   */
   /* CPPM_Control_Point_Event_Data_t type structures. Control point    */
   /* indications package the results from procedures. If the procedure */
   /* has timed out then the Timeout field will be true. See CPPMType.h */
   /* the definition of the CPPM_Control_Point_Data_t struct.           */
typedef struct _tagCPPM_Control_Point_Event_Data_t
{
   BD_ADDR_t                 RemoteDeviceAddress;
   unsigned int              InstanceID;
   Boolean_t                 Timeout;
   CPPM_Control_Point_Data_t ControlPoint;
} CPPM_Control_Point_Event_Data_t;

#define CPPM_CONTROL_POINT_EVENT_DATA_SIZE   (sizeof(CPPM_Control_Point_Event_Data_t))

   /* Response data from an attempt to read the supported features of a */
   /* cycling power sensor is packaged in a                             */
   /* CPPM_Sensor_Features_Event_Data_t type structure. TransactionID   */
   /* is set to the transaction ID of the GATM read. Features is a mask */
   /* of supported feature bits. See CPPMType.h for a set of #define    */
   /* values that can be used to test for particular features. The      */
   /* Status field is set to an error code if an error occurred and     */
   /* zero otherwise.                                                   */ 
typedef struct _tagCPPM_Sensor_Features_Event_Data_t
{  
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  InstanceID;
   unsigned int  TransactionID;
   unsigned long Features;
   int           Status;
} CPPM_Sensor_Features_Event_Data_t;

#define CPPM_SENSOR_FEATURES_EVENT_DATA_SIZE   (sizeof(CPPM_Sensor_Features_Event_Data_t))

   /* Response data from an attempt to read the mount location of a     */
   /* cycling power sensor is packaged in a                             */
   /* CPPM_Sensor_Location_Event_Data_t type structure. TransactionID   */
   /* is set to the transaction ID of the GATM read. Location is the    */
   /* mount location enum value. See CPPMType.h for the definition of   */
   /* the CPPM_Sensor_Location_t enum. The Status field is set to an    */
   /* error code if an error occurred and zero otherwise.               */ 
typedef struct _tagCPPM_Sensor_Location_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned int           InstanceID;
   unsigned int           TransactionID;
   CPPM_Sensor_Location_t Location;
   int                    Status;
} CPPM_Sensor_Location_Event_Data_t;

#define CPPM_SENSOR_LOCATION_EVENT_DATA_SIZE   (sizeof(CPPM_Sensor_Location_Event_Data_t))

   /* Cycling power event data is delivered in a CPPM_Event_Data_t type */
   /* structure. The EventType field identifies the event. EventLength  */
   /* equals the length in bytes. EventCallbackID equals the ID         */
   /* returned in CPPM_Register_Collector_Event_Callback. EventData is  */
   /* a union of event data structures which package the event data.    */
typedef struct _tagCPPM_Event_Data_t
{
   CPPM_Event_Type_t EventType;
   unsigned int      EventLength;
   unsigned int      EventCallbackID;
   union
   {
      CPPM_Connected_Event_Data_t        ConnectedEventData;
      CPPM_Disconnected_Event_Data_t     DisconnectedEventData;
      CPPM_Write_Response_Event_Data_t   WriteResponseEventData;
      CPPM_Measurement_Event_Data_t      MeasurementEventData;
      CPPM_Vector_Event_Data_t           VectorEventData;
      CPPM_Control_Point_Event_Data_t    ControlPointEventData;
      CPPM_Sensor_Features_Event_Data_t  SensorFeaturesEventData;
      CPPM_Sensor_Location_Event_Data_t  SensorLocationEventData;
   } EventData;
} CPPM_Event_Data_t;

#define CPPM_EVENT_DATA_SIZE   (sizeof(CPPM_Event_Data_t))

typedef void (BTPSAPI *CPPM_Event_Callback_t)(CPPM_Event_Data_t *EventData, void *CallbackParameter);

   /* CPPM_InitializationHandlerFunction is included in the MODC        */
   /* ModuleHandlerList array and is called to initalize the module.    */
void BTPSAPI CPPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* CPPM_DeviceManagerHandlerFunction is included in the MODC         */
   /* ModuleHandlerList array. It is the DEVM callback function for the */
   /* module.                                                           */
void BTPSAPI CPPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* CPPM_Register_Collector_Event_Callback registers a callback       */
   /* function that will be run when a cycling power event occurs. If   */
   /* If successful, the callback ID to be used in other API functions  */
   /* is returned.                                                      */
int BTPSAPI CPPM_Register_Collector_Event_Callback(CPPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

   /* After a successful call to                                        */
   /* CPPM_Unregister_Collector_Event_Callback the previously           */
   /* registered callback function will no longer be run when a cycling */
   /* power event occurs.                                               */
void BTPSAPI CPPM_Unregister_Collector_Event_Callback(unsigned int CallbackID);

  /* CPPM_Register_Measurements registers for measurement notifications */
  /* from the specified sensor instance. If EnableUpdates is TRUE then  */
  /* the client configuration descriptor will be written to enable the  */
  /* notifications and the transaction ID from a successful GATM write  */
  /* will be returned. If EnableUpdates is false then zero indicates    */
  /* success. If the descriptor is written then the response comes in a */
  /* CPPM_Write_Response_Event_Data_t type struct, delivered in a       */
  /* cetMeasurementsSetCPP type event.                                  */
int BTPSAPI CPPM_Register_Measurements(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates);

   /* After a successful call to CPPM_Unregister_Measurements, the      */
   /* caller will no longer receive measurement notifications from the  */
   /* specified sensor. If there are no other registered applications,  */
   /* then the notifications are disabled by writing the configuration  */
   /* descriptor. If the descriptor is written then the response data   */
   /* comes in a CPPM_Write_Response_Event_Data_t type struct delivered */
   /* in a cetMeasurementsSetCPP type event.                            */
int BTPSAPI CPPM_Unregister_Measurements(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

  /* CPPM_Register_Vectors registers for vector notifications from the  */
  /* specified sensor instance. If EnableUpdates is TRUE then the       */
  /* client configuration descriptor will be written to enable the      */
  /* notifications and the transaction ID from a successful GATM write  */
  /* will be returned. If there is such a write then the response is    */
  /* delivered via a cetVectorsSetCPP type event. It delivers the data  */
  /* in a CPPM_Write_Response_Event_Data_t type struct. If              */
  /* EnableUpdates is false then zero indicates success.                */
int BTPSAPI CPPM_Register_Vectors(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates);

   /* After a successful call to CPPM_Unregister_Vectors, the caller    */
   /* will no longer receive vector notifications from the specified    */
   /* sensor. If no other  applications have registered to receive      */
   /* them, then the notifications are disabled by writing the          */
   /* configuration descriptor. If the descriptor is written then the   */
   /* response is delivered in a CPPM_Write_Response_Event_Data_t       */
   /* struct by a cetVectorsSetCPP type event.                          */ 
int BTPSAPI CPPM_Unregister_Vectors(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

  /* CPPM_Register_Procedures registers for control point indications   */
  /* from the specified sensor instance. If EnableUpdates is TRUE then  */
  /* the client configuration descriptor will be written to enable the  */
  /* indications and the transaction ID from a successful GATM write    */
  /* will be returned. If the descriptor is written the write response  */
  /* comes in a cetProceduresSetCPP type event. It delivers the         */
  /* response data in a CPPM_Write_Response_Event_Data_t type struct.   */
  /* If EnableUpdates is false then zero indicates success. Control     */
  /* point indications complete procedures. Procedures are initiated by */
  /* writing an opcode to the control point characteristic.             */
  /* See CPPM_Write_Sensor_Control_Point.                               */
int BTPSAPI CPPM_Register_Procedures(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, Boolean_t EnableUpdates);

   /* After a successful call to CPPM_Unregister_Procedures, the caller */
   /* will no longer receive procedure results via control point        */
   /* indications from the specified sensor. If there are no other      */
   /* registered applications, then the indications are disabled by     */
   /* writing the configuration descriptor. If such a write is          */
   /* triggered then the response is delivered via a                    */
   /* cetProceduresSetCPP type event. The write response data is in a   */
   /* CPPM_Write_Response_Event_Data_t type struct.                     */
int BTPSAPI CPPM_Unregister_Procedures(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

   /* CPPM_Enable_Broadcasts enables measurement broadcasts by writing  */
   /* the server configuration descriptor of the measurement            */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The write response  */
   /* triggers a cetBroadcastsSetCPP type event which delivers the      */
   /* response in a CPPM_Write_Response_Event_Data_t type struct.       */
int BTPSAPI CPPM_Enable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

   /* CPPM_Enable_Broadcasts enables measurement broadcasts by writing  */
   /* the server configuration descriptor of the measurement            */
   /* characteristic of the specified sensor. If successful, the        */
   /* transaction ID of the GATM write is returned. The write response  */
   /* triggers a cetBroadcastsSetCPP type event which delivers the      */
   /* response in a CPPM_Write_Response_Event_Data_t type struct.       */
int BTPSAPI CPPM_Disable_Broadcasts(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

   /* CPPM_Read_Sensor_Features reads the supported features of a       */
   /* sensor. If successful it returns the GATM transaction ID from the */
   /* read. The response is delivered in a cetSensorFeaturesCPP type    */
   /* event with a CPPM_Sensor_Features_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Features(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

   /* CPPM_Read_Sensor_Location reads the mount location of the sensor. */
   /* If successful, it returns the GATM transaction ID from the read.  */
   /* The read response is delivered in a cetSensorLocationCPP type     */
   /* event with a CPPM_Sensor_Location_Event_Data_t type struct.       */
int BTPSAPI CPPM_Read_Sensor_Location(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID);

   /* CPPM_Write_Sensor_Control_Point writes an opcode and, depending   */
   /* on the procedure type, may also write additional procedure data   */
   /* to the specified sensor instance's control point characteristic   */
   /* thereby initiating a procedure. The write response triggers a     */
   /* cetProcedureBegunCPP type event. The response data is packaged in */
   /* a CPPM_Write_Response_Event_Data_t type struct. A                 */
   /* cetControlPointCPP type event indicates the procedure has         */
   /* completed. The control point indication data is packaged in a     */
   /* CPPM_Control_Point_Event_Data_t type struct.                      */
int BTPSAPI CPPM_Write_Sensor_Control_Point(unsigned int CallbackID, BD_ADDR_t RemoteSensor, unsigned int InstanceID, CPPM_Procedure_Data_t ProcedureData);

   /* CPPM_Query_Sensors lists the bluetooth address of sensors. The    */
   /* function returns the total number of sensors in the device list.  */
   /* If the caller allocates the RemoteSensors buffer, then the        */
   /* NumberOfSensors parameter should point to a value equaling the    */
   /* number of BD_ADDR_t type addresses that the buffer can hold.      */
   /* The value pointed to by NumberOfSensors will be changed to the    */
   /* number of sensor addresses copied to the buffer.                  */
int BTPSAPI CPPM_Query_Sensors(unsigned int CallbackID, unsigned int *NumberOfSensors, BD_ADDR_t *RemoteSensors);

   /* CPPM_Query_Sensor_Instances lists the sensor service instance     */
   /* records of the specified sensor. The function returns the total   */
   /* number of service instances. If the caller allocates the          */
   /* Instances buffer, then the NumberOfInstances parameter should     */
   /* point to a value equaling the number of instance records that the */
   /* buffer can hold. The value pointed to by NumberOfInstances will   */
   /* be changed to the number of records copied to the buffer.         */
int BTPSAPI CPPM_Query_Sensor_Instances(unsigned int CallbackID, BD_ADDR_t Sensor, unsigned int *NumberOfInstances, Instance_Record_t *Instances);

#endif
