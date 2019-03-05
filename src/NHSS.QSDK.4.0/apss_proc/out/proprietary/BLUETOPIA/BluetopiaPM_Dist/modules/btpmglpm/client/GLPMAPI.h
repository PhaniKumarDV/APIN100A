/*****< glpmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GLPMAPI - Glucose Profile (GLPM) Manager API for Stonestreet One          */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   03/15/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GLPMAPIH__
#define __GLPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "GLPMMSG.h"             /* BTPM GLPM Manager Message Formats.        */

   /* The following enumerated type represents the GLP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of GLP Manager Changes.                                           */
typedef enum
{
   getGLPMConnected,
   getGLPMDisconnected,
   getGLPMProcedureStarted,
   getGLPMProcedureStopped,
   getGLPMGlucoseMeasurement
} GLPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGLPMConnected event.         */
typedef struct _tagGLPM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   GLPM_Connection_Type_t ConnectionType;
   unsigned long          SupportedFeatures;
} GLPM_Connected_Event_Data_t;

#define GLPM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(GLPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGLPMDisconnected event.      */
typedef struct _tagGLPM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   GLPM_Connection_Type_t ConnectionType;
} GLPM_Disconnected_Event_Data_t;

#define GLPM_DISCONNECTED_EVENT_DATA_SIZE                      (sizeof(GLPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGLPMProcedureStarted event.  */
typedef struct _tagGLPM_Procedure_Started_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int ProcedureID;
   Boolean_t    Success;
   Byte_t       AttributeProtocolErrorCode;
} GLPM_Procedure_Started_Event_Data_t;

#define GLPM_PROCEDURE_STARTED_EVENT_DATA_SIZE                 (sizeof(GLPM_Procedure_Started_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGLPMProcedureStopped event.  */
typedef struct _tagGLPM_Procedure_Stopped_Event_Data_t
{
   BD_ADDR_t                           RemoteDeviceAddress;
   unsigned int                        ProcedureID;
   GLPM_Procedure_Type_t               ProcedureType;
   GLPM_Procedure_Response_Code_Type_t ResponseCode;
   unsigned int                        NumberStoredRecords;
} GLPM_Procedure_Stopped_Event_Data_t;

#define GLPM_PROCEDURE_STOPPED_EVENT_DATA_SIZE                 (sizeof(GLPM_Procedure_Stopped_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a getGLPMGlucoseMeasurement event.*/
typedef struct _tagGLPM_Glucose_Measurement_Event_Data_t
{
   BD_ADDR_t                               RemoteDeviceAddress;
   unsigned int                            ProcedureID;
   GLPM_Measurement_Error_Type_t           MeasurementErrorType;
   unsigned long                           MeasurementFlags;
   GLPM_Glucose_Measurement_Data_t         GlucoseMeasurementData;
   GLPM_Glucose_Measurement_Context_Data_t GlucoseMeasurementContextData;
} GLPM_Glucose_Measurement_Event_Data_t;

#define GLPM_GLUCOSE_MEASUREMENT_EVENT_DATA_SIZE               (sizeof(GLPM_Glucose_Measurement_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Glucose Profile (GLPM) Manager Event (and Event Data) of a GLP    */
   /* Manager Event.                                                    */
typedef struct _tagGLPM_Event_Data_t
{
   GLPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      GLPM_Connected_Event_Data_t           ConnectedEventData;
      GLPM_Disconnected_Event_Data_t        DisconnectedEventData;
      GLPM_Procedure_Started_Event_Data_t   ProcedureStartedEventData;
      GLPM_Procedure_Stopped_Event_Data_t   ProcedureStoppedEventData;
      GLPM_Glucose_Measurement_Event_Data_t GlucoseMeasurementEventData;
   } EventData;
} GLPM_Event_Data_t;

#define GLPM_EVENT_DATA_SIZE                                   (sizeof(GLPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Glucose Profile (GLPM) Manager dispatches an event (and the client*/
   /* has registered for events).  This function passes to the caller   */
   /* the GLP Manager Event and the Callback Parameter that was         */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the Event Data ONLY in the context of this    */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer.  This function is guaranteed NOT to be invoked more  */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be reentrant).  Because of     */
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
typedef void (BTPSAPI *GLPM_Event_Callback_t)(GLPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager GLP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI GLPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI GLPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Glucose Profile  */
   /* (GLPM) Manager Service.  This Callback will be dispatched by the  */
   /* GLP Manager when various GLP Manager Events occur.  This function */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a GLP Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          GLPM_Un_Register_Collector_Event_Callback() function to  */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI GLPM_Register_Collector_Event_Callback(GLPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLPM_Register_Collector_Event_Callback_t)(GLPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered GLP Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* GLPM_Register_Collector_Event_Callback() function).  This function*/
   /* accepts as input the GLP Manager Event Callback ID (return value  */
   /* from GLPM_Register_Collector_Event_Callback() function).          */
BTPSAPI_DECLARATION void BTPSAPI GLPM_Un_Register_Collector_Event_Callback(unsigned int GLPMCollectorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_GLPM_Un_Register_Collector_Event_Callback_t)(unsigned int GLPMCollectorCallbackID);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* starting a Glucose Procedure to a remote Glucose Device.  This    */
   /* function accepts as input the GLP Manager Data Event Callback ID  */
   /* (return value from GLPM_Register_Collector_Event_Callback()       */
   /* function), the BD_ADDR of the remote Glucose Device and a pointer */
   /* to a structure containing the procedure data.  This function      */
   /* returns the positive, non-zero, Procedure ID of the request on    */
   /* success or a negative error code.                                 */
   /* * NOTE * The getGLPMProcedureStarted event will be generated when */
   /*          the remote Glucose Device responds to the Start Procedure*/
   /*          Request.                                                 */
   /* * NOTE * Only 1 Glucose procedure can be outstanding at a time for*/
   /*          each remote Glucose device.  A procedure is completed    */
   /*          when either the getGLPMProcedureStarted event is received*/
   /*          with a error code or if the getGLPMProcedureStopped event*/
   /*          is received for a procedure that started successfully.   */
BTPSAPI_DECLARATION int BTPSAPI GLPM_Start_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLPM_Start_Procedure_Request_t)(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, GLPM_Procedure_Data_t *ProcedureData);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* stopping a previouly started Glucose Procedure to a remote Glucose*/
   /* Device.  This function accepts as input the GLP Manager Data Event*/
   /* Callback ID (return value from                                    */
   /* GLPM_Register_Collector_Event_Callback() function), the BD_ADDR of*/
   /* the remote Glucose Device and the Procedure ID that was returned  */
   /* via a successfull call to GLPM_Start_Procedure_Request().  This   */
   /* function returns zero on success or a negative error code.        */
   /* * NOTE * The getGLPMProcedureStoped event will be generated when  */
   /*          the remote Glucse Device responds to the Stop Procedure  */
   /*          Request.                                                 */
BTPSAPI_DECLARATION int BTPSAPI GLPM_Stop_Procedure_Request(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_GLPM_Stop_Procedure_Request_t)(unsigned int GLPMCollectorCallbackID, BD_ADDR_t RemoteDeviceAddress, unsigned int ProcedureID);
#endif


#endif
