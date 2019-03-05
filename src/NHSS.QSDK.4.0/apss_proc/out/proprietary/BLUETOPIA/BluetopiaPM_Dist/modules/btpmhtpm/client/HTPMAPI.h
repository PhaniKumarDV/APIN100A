/*****< htpmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HTPMAPI - Health Thermometer Profile (HTP) Manager API for Stonestreet    */
/*            One Bluetooth Protocol Stack Platform Manager.                  */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/12/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HTPMAPIH__
#define __HTPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HTPMMSG.h"             /* BTPM HTP Manager Message Formats.         */

   /* The following enumerated type represents the HTP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of HTP Manager Changes.                                           */
typedef enum
{
   hetHTPConnected,
   hetHTPDisconnected,
   hetHTPGetTemperatureTypeResponse,
   hetHTPGetMeasurementIntervalResponse,
   hetHTPSetMeasurementIntervalResponse,
   hetHTPGetMeasurementIntervalValidRangeResponse,
   hetHTPTemperatureMeasurement
} HTPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHTPConnected event.          */
typedef struct _tagHTPM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   HTPM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
} HTPM_Connected_Event_Data_t;

#define HTPM_CONNECTED_EVENT_DATA_SIZE   (sizeof(HTPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHTPDisconnected event.       */
typedef struct _tagHTPM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   HTPM_Connection_Type_t ConnectionType;
} HTPM_Disconnected_Event_Data_t;

#define HTPM_DISCONNECTED_EVENT_DATA_SIZE   (sizeof(HTPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHTPGetTemperatureTypeResponse*/
   /* event.                                                            */
typedef struct _tagHTPM_Get_Temperature_Type_Response_Event_Data_t
{
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned int            TransactionID;
   Boolean_t               Success;
   Byte_t                  AttributeErrorCode;
   HTPM_Temperature_Type_t TemperatureType;
} HTPM_Get_Temperature_Type_Response_Event_Data_t;

#define HTPM_GET_TEMPERATURE_TYPE_RESPONSE_EVENT_DATA_SIZE   (sizeof(HTPM_Get_Temperature_Type_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHTPGetMeasurementIntervalResponse event.                       */
   /* * NOTE * The TransactionID member may be set to ZERO to indicate  */
   /*          that this event was generated asynchronously by the HTP  */
   /*          Sensor.                                                  */
typedef struct _tagHTPM_Get_Measurement_Interval_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int TransactionID;
   Boolean_t    Success;
   Byte_t       AttributeErrorCode;
   unsigned int MeasurementInterval;
} HTPM_Get_Measurement_Interval_Response_Event_Data_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE   (sizeof(HTPM_Get_Measurement_Interval_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHTPSetMeasurementIntervalResponse event.                       */
typedef struct _tagHTPM_Set_Measurement_Interval_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int TransactionID;
   Boolean_t    Success;
   Byte_t       AttributeErrorCode;
} HTPM_Set_Measurement_Interval_Response_Event_Data_t;

#define HTPM_SET_MEASUREMENT_INTERVAL_RESPONSE_EVENT_DATA_SIZE   (sizeof(HTPM_Set_Measurement_Interval_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHTPGetMeasurementIntervalValidRangeResponse event.             */
typedef struct _tagHTPM_Get_Measurement_Interval_Valid_Range_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int TransactionID;
   Boolean_t    Success;
   Byte_t       AttributeErrorCode;
   unsigned int LowerBounds;
   unsigned int UpperBounds;
} HTPM_Get_Measurement_Interval_Valid_Range_Response_Event_Data_t;

#define HTPM_GET_MEASUREMENT_INTERVAL_VALID_RANGE_RESPONSE_EVENT_DATA_SIZE   (sizeof(HTPM_Get_Measurement_Interval_Valid_Range_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHTPTemperatureMeasurement    */
   /* event.                                                            */
typedef struct _tagHTPM_Temperature_Measurement_Event_Data_t
{
   BD_ADDR_t                           RemoteDeviceAddress;
   HTPM_Temperature_Measurement_Type_t MeasurementType;
   unsigned long                       MeasurementFlags;
   HTPM_Temperature_Type_t             TemperatureType;
   HTPM_Time_Stamp_Data_t              TimeStamp;
   long                                TemperatureMantissa;
   int                                 TemperatureExponent;
} HTPM_Temperature_Measurement_Event_Data_t;

#define HTPM_TEMPERATURE_MEASUREMENT_EVENT_DATA_SIZE           (sizeof(HTPM_Temperature_Measurement_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Health Thermometer Profile (HTP) Manager Event (and Event Data) of*/
   /* a HTP Manager Event.                                              */
typedef struct _tagHTPM_Event_Data_t
{
   HTPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HTPM_Connected_Event_Data_t                                     ConnectedEventData;
      HTPM_Disconnected_Event_Data_t                                  DisconnectedEventData;
      HTPM_Get_Temperature_Type_Response_Event_Data_t                 GetTemperatureTypeResponseEventData;
      HTPM_Get_Measurement_Interval_Response_Event_Data_t             GetMeasurementIntervalResponseEventData;
      HTPM_Set_Measurement_Interval_Response_Event_Data_t             SetMeasurementIntervalResponseEventData;
      HTPM_Get_Measurement_Interval_Valid_Range_Response_Event_Data_t GetMeasurementIntervalValidRangeResponseEventData;
      HTPM_Temperature_Measurement_Event_Data_t                       TemperatureMeasurementEventData;
   } EventData;
} HTPM_Event_Data_t;

#define HTPM_EVENT_DATA_SIZE                                   (sizeof(HTPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Health Thermometer Profile (HTP) Manager dispatches an event (and */
   /* the client has registered for events).  This function passes to   */
   /* the caller the HTP Manager Event and the Callback Parameter that  */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the Event Data ONLY in the context of */
   /* this callback.  If the caller requires the Data for a longer      */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).        */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* Message will not be processed while this function call is         */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *HTPM_Event_Callback_t)(HTPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HTP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HTPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HTPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Health           */
   /* Thermometer (HTP) Manager Service.  This Callback will be         */
   /* dispatched by the HTP Manager when various HTP Manager Events     */
   /* occur.  This function accepts the Callback Function and Callback  */
   /* Parameter (respectively) to call when a HTP Manager Event needs to*/
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HTPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HTPM_Register_Collector_Event_Callback(HTPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HTPM_Register_Collector_Event_Callback_t)(HTPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HTP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HTPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the HTP Manager Event Callback ID (return value  */
   /* from HTPM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI HTPM_Un_Register_Collector_Event_Callback(unsigned int HTPMCollectorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HTPM_Un_Register_Collector_Event_Callback_t)(unsigned int HTPMCollectorCallbackID);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Temperature Type procedure to a remote HTP   */
   /* Sensor.  This function accepts as input the HTP Collector Callback*/
   /* ID (return value from HTPM_Register_Collector_Event_Callback()    */
   /* function), and the BD_ADDR of the remote HTP Sensor.  This        */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHTPGetTemperatureTypeResponse event will be       */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Temperature Type Request.                                */
BTPSAPI_DECLARATION int BTPSAPI HTPM_Get_Temperature_Type_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HTPM_Get_Temperature_Type_Request_t)(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Get */
   /*          Measurement Interval Request.                            */
BTPSAPI_DECLARATION int BTPSAPI HTPM_Get_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HTPM_Get_Measurement_Interval_Request_t)(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Set Measurement Interval procedure to a remote   */
   /* HTP Sensor.  This function accepts as input the HTP Collector     */
   /* Callback ID (return value from                                    */
   /* HTPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote HTP Sensor, and the Measurement Interval to attempt to */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request on success or a negative error code.               */
   /* * NOTE * The hetHTPSetMeasurementIntervalResponse event will be   */
   /*          generated when the remote HTP Sensor responds to the Set */
   /*          Measurement Interval Request.                            */
BTPSAPI_DECLARATION int BTPSAPI HTPM_Set_Measurement_Interval_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HTPM_Set_Measurement_Interval_Request_t)(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int MeasurementInterval);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HTP Get Measurement Interval Valid Range procedure to*/
   /* a remote HTP Sensor.  This function accepts as input the HTP      */
   /* Collector Callback ID (return value from                          */
   /* HTPM_Register_Collector_Event_Callback() function), and the       */
   /* BD_ADDR of the remote HTP Sensor.  This function returns the      */
   /* positive, non-zero, Transaction ID of the request on success or a */
   /* negative error code.                                              */
   /* * NOTE * The hetHTPGetMeasurementIntervalValidRangeResponse event */
   /*          will be generated when the remote HTP Sensor responds to */
   /*          the Get Measurement Interval Request.                    */
BTPSAPI_DECLARATION int BTPSAPI HTPM_Get_Measurement_Interval_Valid_Range_Request(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HTPM_Get_Measurement_Interval_Valid_Range_Request_t)(unsigned int HTPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

#endif
