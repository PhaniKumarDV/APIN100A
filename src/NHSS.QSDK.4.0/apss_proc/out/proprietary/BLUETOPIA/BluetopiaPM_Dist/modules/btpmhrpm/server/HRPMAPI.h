/*****< hrpmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HRPMAPI - Heart Rate Profile (HRP) Manager API for Stonestreet            */
/*            One Bluetooth Protocol Stack Platform Manager.                  */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/12/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __HRPMAPIH__
#define __HRPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HRPMMSG.h"             /* BTPM HRP Manager Message Formats.         */

   /* The following enumerated type represents the HRP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of HRP Manager Changes.                                           */
typedef enum
{
   hetHRPConnected,
   hetHRPDisconnected,
   hetHRPHeartRateMeasurement,
   hetHRPGetBodySensorLocationResponse,
   hetHRPResetEnergyExpendedResponse
} HRPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHRPConnected event.          */
typedef struct _tagHRPM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   HRPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} HRPM_Connected_Event_Data_t;

#define HRPM_CONNECTED_EVENT_DATA_SIZE   (sizeof(HRPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHRPDisconnected event.       */
typedef struct _tagHRPM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   HRPM_Connection_Type_t ConnectionType;
   unsigned long          DisconnectedFlags;
} HRPM_Disconnected_Event_Data_t;

#define HRPM_DISCONNECTED_EVENT_DATA_SIZE   (sizeof(HRPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHRPHeartRateMeasurement      */
   /* event.                                                            */
typedef struct _tagHRPM_Heart_Rate_Measurement_Event_Data_t
{
   BD_ADDR_t      RemoteDeviceAddress;
   unsigned long  MeasurementFlags;
   Word_t         HeartRate;
   Word_t         EnergyExpended;
   Word_t         NumberOfRRIntervals;
   Word_t        *RRIntervals;
} HRPM_Heart_Rate_Measurement_Event_Data_t;

#define HRPM_HEART_RATE_MEASUREMENT_EVENT_DATA_SIZE   (sizeof(HRPM_Heart_Rate_Measurement_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHRPGetBodySensorLocationResponse event.                        */
typedef struct _tagHRPM_Get_Body_Sensor_Location_Response_Event_Data_t
{
   BD_ADDR_t                   RemoteDeviceAddress;
   int                         Status;
   HRPM_Body_Sensor_Location_t Location;
} HRPM_Get_Body_Sensor_Location_Response_Event_Data_t;

#define HRPM_GET_BODY_SENSOR_LOCATION_RESPONSE_EVENT_DATA_SIZE   (sizeof(HRPM_Get_Body_Sensor_Location_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHRPResetEnergyExpendedResponse event.                          */
typedef struct _tagHRPM_Reset_Energy_Expended_Response_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   int       Status;
} HRPM_Reset_Energy_Expended_Response_Event_Data_t;

#define HRPM_RESET_ENERGY_EXPENDED_RESPONSE_EVENT_DATA_SIZE   (sizeof(HRPM_Reset_Energy_Expended_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Heart Rate Profile (HRP) Manager Event (and Event Data) of a HRP  */
   /* Manager Event.                                                    */
typedef struct _tagHRPM_Event_Data_t
{
   HRPM_Event_Type_t EventType;
   unsigned int      EventLength;
   unsigned int      EventCallbackID;
   union
   {
      HRPM_Connected_Event_Data_t                         ConnectedEventData;
      HRPM_Disconnected_Event_Data_t                      DisconnectedEventData;
      HRPM_Heart_Rate_Measurement_Event_Data_t            HeartRateMeasurementEventData;
      HRPM_Get_Body_Sensor_Location_Response_Event_Data_t BodySensorLocationResponse;
      HRPM_Reset_Energy_Expended_Response_Event_Data_t    ResetEnergyExpendedResponse;
   } EventData;
} HRPM_Event_Data_t;

#define HRPM_EVENT_DATA_SIZE                                   (sizeof(HRPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Heart Rate Profile (HRP) Manager dispatches an event (and the     */
   /* client has registered for events).  This function passes to the   */
   /* caller the HRP Manager Event and the Callback Parameter that was  */
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
typedef void (BTPSAPI *HRPM_Event_Callback_t)(HRPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HRP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HRPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HRPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Heart Rate (HRP) */
   /* Manager Service.  This Callback will be dispatched by the HRP     */
   /* Manager when various HRP Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a HRP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HRPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HRPM_Register_Collector_Event_Callback(HRPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HRPM_Register_Collector_Event_Callback_t)(HRPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HRP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HRPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the HRP Manager Event Callback ID (return value  */
   /* from HRPM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI HRPM_Un_Register_Collector_Event_Callback(unsigned int CollectorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HRPM_Un_Register_Collector_Event_Callback_t)(unsigned int CollectorCallbackID);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Body Sensor Location Request to a remote sensor.  This      */
   /* function accepts as input the Callback ID (return value from      */
   /* HRPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Body Sensor Location from.  This     */
   /* function returns zero on success; otherwise, a negative error     */
   /* value is returned.                                                */
BTPSAPI_DECLARATION int BTPSAPI HRPM_Get_Body_Sensor_Location(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HRPM_Get_Body_Sensor_Location_t)(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Reset Energy Expended Request to a remote sensor.  This function*/
   /* accepts as input the Callback ID (return value from               */
   /* HRPM_Register_Collector_Event_Callback() function) as the first   */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the execution of the Reset Energy        */
   /* Expended command.  This function returns zero on success;         */
   /* otherwise, a negative error value is returned.                    */
BTPSAPI_DECLARATION int BTPSAPI HRPM_Reset_Energy_Expended(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HRPM_Reset_Energy_Expended_t)(unsigned int CollectorCallbackID, BD_ADDR_t RemoteSensor);
#endif

#endif
