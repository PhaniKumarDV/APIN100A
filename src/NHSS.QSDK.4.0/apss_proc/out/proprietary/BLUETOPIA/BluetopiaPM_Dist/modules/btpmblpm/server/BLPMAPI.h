/*****< blpmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BLPMAPI - Blood Pressure Profile (BLP) Manager API for Stonestreet        */
/*            One Bluetooth Protocol Stack Platform Manager.                  */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/03/13  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BLPMAPIH__
#define __BLPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "BLPMMSG.h"             /* BTPM BLP Manager Message Formats.         */

   /* The following enumerated type represents the BLP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of BLP Manager Changes.                                           */
typedef enum
{
   betBLPConnected,
   betBLPDisconnected,
   betBLPBloodPressureMeasurement,
   betBLPIntermediateCuffPressure,
   betBLPBloodPressureFeatureResponse,
} BLPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBLPConnected event.          */
typedef struct _tagBLPM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   BLPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} BLPM_Connected_Event_Data_t;

#define BLPM_CONNECTED_EVENT_DATA_SIZE   (sizeof(BLPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBLPDisconnected event.       */
typedef struct _tagBLPM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   BLPM_Connection_Type_t ConnectionType;
   unsigned long          DisconnectedFlags;
} BLPM_Disconnected_Event_Data_t;

#define BLPM_DISCONNECTED_EVENT_DATA_SIZE   (sizeof(BLPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBLPBloodPressureMeasurement  */
   /* event.                                                            */
typedef struct _tagBLPM_Blood_Pressure_Measurement_Event_Data_t
{
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                MeasurementFlags;
   Word_t                SystolicPressure;
   Word_t                DiastolicPressure;
   Word_t                MeanArterialPressure;
   BLPM_Date_Time_Data_t TimeStamp;
   Word_t                PulseRate;
   Byte_t                UserID;
   Word_t                MeasurementStatus;
} BLPM_Blood_Pressure_Measurement_Event_Data_t;

#define BLPM_BLOOD_PRESSURE_MEASUREMENT_EVENT_DATA_SIZE   (sizeof(BLPM_Blood_Pressure_Measurement_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBLPIntermediateCuffPressure  */
   /* event.                                                            */
typedef struct _tagBLPM_Intermediate_Cuff_Pressure_Event_Data_t
{
   BD_ADDR_t             RemoteDeviceAddress;
   Byte_t                Flags;
   Word_t                IntermediateCuffPressure;
   BLPM_Date_Time_Data_t TimeStamp;
   Word_t                PulseRate;
   Byte_t                UserID;
   Word_t                MeasurementStatus;
} BLPM_Intermediate_Cuff_Pressure_Event_Data_t;

#define BLPM_INTERMEDIATE_CUFF_PRESSURE_EVENT_DATA_SIZE   (sizeof(BLPM_Intermediate_Cuff_Pressure_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* betBLPBloodPressureFeatureResponse event.                         */
typedef struct _tagBLPM_Blood_Pressure_Feature_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int TransactionID;
   int          Status;
   Word_t       Feature;
} BLPM_Blood_Pressure_Feature_Response_Event_Data_t;

#define BLPM_BLOOD_PRESSURE_FEATURE_RESPONSE_EVENT_DATA_SIZE   (sizeof(BLPM_Blood_Pressure_Feature_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Blood Pressure Profile (BLP) Manager Event (and Event Data) of a  */
   /* BLP Manager Event.                                                */
typedef struct _tagBLPM_Event_Data_t
{
   BLPM_Event_Type_t EventType;
   unsigned int      EventLength;
   unsigned int      EventCallbackID;
   union
   {
      BLPM_Connected_Event_Data_t                       ConnectedEventData;
      BLPM_Disconnected_Event_Data_t                    DisconnectedEventData;
      BLPM_Blood_Pressure_Measurement_Event_Data_t      BloodPressureMeasurementEventData;
      BLPM_Intermediate_Cuff_Pressure_Event_Data_t      IntermediateCuffPressureEventData;
      BLPM_Blood_Pressure_Feature_Response_Event_Data_t BloodPressureFeatureResponseEventData;
   } EventData;
} BLPM_Event_Data_t;

#define BLPM_EVENT_DATA_SIZE                                   (sizeof(BLPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Blood Pressure Profile (BLP) Manager dispatches an event (and the */
   /* client has registered for events).  This function passes to the   */
   /* caller the BLP Manager Event and the Callback Parameter that was  */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the Event Data ONLY in the context of this    */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e. this function DOES NOT have be reentrant).  Because of      */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another Message will */
   /* not be processed while this function call is outstanding).        */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *BLPM_Event_Callback_t)(BLPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager BLP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI BLPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI BLPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Blood Pressure   */
   /* (BLP) Manager Service.  This Callback will be dispatched by the   */
   /* BLP Manager when various BLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Register_Collector_Event_Callback(BLPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Register_Collector_Event_Callback_t)(BLPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the BLP Manager Event Callback ID (return value  */
   /* from BLPM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI BLPM_Un_Register_Collector_Event_Callback(unsigned int CallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BLPM_Un_Register_Collector_Event_Callback_t)(unsigned int CallbackID);
#endif

   /* The following function is provided to allow a mechanism to enable */
   /* indications for Blood Pressure Measurements on a specified Blood  */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Enable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Enable_Blood_Pressure_Indications_t)(unsigned int CallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to disable*/
   /* indications for Blood Pressure Measurements on a specified Blood  */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable indications on.  */
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Disable_Blood_Pressure_Indications(unsigned int CallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Disable_Blood_Pressure_Indications_t)(unsigned int CallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to enable */
   /* notifications for Intermediate Cuff Pressure on a specified Blood */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Enable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Enable_Intermediate_Cuff_Pressure_Notifications_t)(unsigned int CallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications for Intermediate Cuff Pressure on a specified Blood */
   /* Pressure Sensor.  This function accepts as input the Callback ID  */
   /* (return value from BLPM_Register_Collector_Event_Callback()       */
   /* function) as the first parameter.  The second parameter is the    */
   /* Bluetooth Address of the remote device to enable notifications on.*/
   /* This function returns zero on success; otherwise, a negative error*/
   /* value is returned.                                                */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Disable_Intermediate_Cuff_Pressure_Notifications(unsigned int CallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Disable_Intermediate_Cuff_Pressure_Notifications_t)(unsigned int CallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Blood Pressure Feature Request to a remote sensor.  This    */
   /* function accepts as input the Callback ID (return value from      */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Blood Pressure Feature from.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Get_Blood_Pressure_Feature(unsigned int CallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Get_Blood_Pressure_Feature_t)(unsigned int CallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BLPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
BTPSAPI_DECLARATION int BTPSAPI BLPM_Cancel_Transaction(unsigned int CallbackID, unsigned int TransactionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BLPM_Cancel_Transaction_t)(unsigned int CallbackID, unsigned int TransactionID);
#endif

#endif
