/*****< basmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BASMAPI - Battery Service (BAS) Manager API for Stonestreet One Bluetooth */
/*            Protocol Stack Platform Manager.                                */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  R. Byrne       Initial creation.                               */
/******************************************************************************/
#ifndef __BASMAPIH__
#define __BASMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "BASMMSG.h"             /* BTPM BAS Manager Message Formats.         */

   /* The following enumerated type represents the BAS Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of BAS Manager Changes.                                           */
typedef enum
{
   betBASConnected,
   betBASDisconnected,
   betBASBatteryLevel,
   betBASBatteryLevelNotification,
   betBASBatteryIdentification
} BASM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBASConnected event.          */
typedef struct _tagBASM_Connected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   BASM_Connection_Type_t ConnectionType;
   unsigned long          ConnectedFlags;
   unsigned int           NumberOfInstances;
} BASM_Connected_Event_Data_t;

#define BASM_CONNECTED_EVENT_DATA_SIZE   (sizeof(BASM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBASDisconnected event.       */
typedef struct _tagBASM_Disconnected_Event_Data_t
{
   BD_ADDR_t              RemoteDeviceAddress;
   BASM_Connection_Type_t ConnectionType;
   unsigned long          DisconnectedFlags;
} BASM_Disconnected_Event_Data_t;

#define BASM_DISCONNECTED_EVENT_DATA_SIZE   (sizeof(BASM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBASBatteryLevel event.       */
typedef struct _tagBASM_Battery_Level_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int TransactionID;
   int          Status;
   Byte_t       BatteryLevel;
} BASM_Battery_Level_Event_Data_t;

#define BASM_BATTERY_LEVEL_EVENT_DATA_SIZE   (sizeof(BASM_Battery_Level_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBASBatteryLevelNotification  */
   /* event.                                                            */
typedef struct _tagBASM_Battery_Level_Notification_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   Byte_t       BatteryLevel;
} BASM_Battery_Level_Notification_Event_Data_t;

#define BASM_BATTERY_LEVEL_NOTIFICATION_EVENT_DATA_SIZE   (sizeof(BASM_Battery_Level_Notification_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a betBASBatteryIdentification     */
   /* event.                                                            */
typedef struct _tagBASM_Battery_Identification_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int InstanceID;
   unsigned int TransactionID;
   int          Status;
   Byte_t       Namespace;
   Word_t       Description;
} BASM_Battery_Identification_Event_Data_t;

#define BASM_BATTERY_IDENTIFICATION_EVENT_DATA_SIZE   (sizeof(BASM_Battery_Identification_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Battery Service (BAS) Manager Event (and Event Data) of a BAS     */
   /* Manager Event.                                                    */
typedef struct _tagBASM_Event_Data_t
{
   BASM_Event_Type_t EventType;
   unsigned int      EventLength;
   unsigned int      EventCallbackID;
   union
   {
      BASM_Connected_Event_Data_t                  ConnectedEventData;
      BASM_Disconnected_Event_Data_t               DisconnectedEventData;
      BASM_Battery_Level_Event_Data_t              BatteryLevelEventData;
      BASM_Battery_Level_Notification_Event_Data_t BatteryLevelNotificationEventData;
      BASM_Battery_Identification_Event_Data_t     BatteryIdentificationEventData;
   } EventData;
} BASM_Event_Data_t;

#define BASM_EVENT_DATA_SIZE                                   (sizeof(BASM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Battery Service (BAS) Manager dispatches an event (and the client */
   /* has registered for events).  This function passes to the caller   */
   /* the BAS Manager Event and the Callback Parameter that was         */
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
typedef void (BTPSAPI *BASM_Event_Callback_t)(BASM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager BAS Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI BASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Battery Service  */
   /* Manager (BASM).  This Callback will be dispatched by the BAS      */
   /* Manager when various BAS Manager Events occur.  This function     */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when a BAS Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          BASM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI BASM_Register_Client_Event_Callback(BASM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Register_Client_Event_Callback_t)(BASM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered BAS Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* BASM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the BAS Manager Event Callback ID (return value  */
   /* from BASM_Register_Client_Event_Callback() function).             */
BTPSAPI_DECLARATION void BTPSAPI BASM_Un_Register_Client_Event_Callback(unsigned int ClientCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_BASM_Un_Register_Client_Event_Callback_t)(unsigned int ClientCallbackID);
#endif

   /* The following function is provided to allow a mechanism to enable */
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
   /* * NOTE * Battery Level Notification support is optional for a     */
   /*          given Battery Server Instance.  This function will return*/
   /*          BTPM_ERROR_CODE_BATTERY_NOTIFY_UNSUPPORTED if the        */
   /*          specified instance does not support Battery Level        */
   /*          Notifications.                                           */
BTPSAPI_DECLARATION int BTPSAPI BASM_Enable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Enable_Notifications)(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism to disable*/
   /* notifications on a specified Battery Server instance.  This       */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
BTPSAPI_DECLARATION int BTPSAPI BASM_Disable_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Disable_Notifications)(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Level Request to a remote server.  This function    */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Battery Level from.  The third       */
   /* parameter is the Instance ID of the Battery Server.  This function*/
   /* returns a positive Transaction ID on success; otherwise, a        */
   /* negative error value is returned.                                 */
BTPSAPI_DECLARATION int BTPSAPI BASM_Get_Battery_Level(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Get_Battery_Level_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Battery Identification Request to a remote server.  This    */
   /* will retrieve the Namespace and Description values from the       */
   /* Presentation Format Descriptor of the remote server.  This        */
   /* function accepts as input the Callback ID (return value from      */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Bluetooth Address of the  */
   /* remote device to request the Identification information from.  The*/
   /* third parameter is the Instance ID of the Battery Server.  This   */
   /* function returns a positive Transaction ID on success; otherwise, */
   /* a negative error value is returned.                               */
   /* * NOTE * Identification Information is only available on a remote */
   /*          device that have multiple BAS Server Instances.  This    */
   /*          function will return                                     */
   /*          BTPM_ERROR_CODE_BATTERY_IDENTIFICATION_UNSUPPORTED if the*/
   /*          specified remote device does not have multiple BAS Server*/
   /*          Instances.                                               */
BTPSAPI_DECLARATION int BTPSAPI BASM_Get_Battery_Identification(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Get_Battery_Identification_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteServer, unsigned int InstanceID);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Cancel Transaction Request to a remote server.  This function   */
   /* accepts as input the Callback ID (return value from               */
   /* BASM_Register_Client_Event_Callback() function) as the first      */
   /* parameter.  The second parameter is the Transaction ID returned by*/
   /* a previously called function in this module.  This function       */
   /* returns zero on success; otherwise, a negative error value is     */
   /* returned.                                                         */
BTPSAPI_DECLARATION int BTPSAPI BASM_Cancel_Transaction(unsigned int ClientCallbackID, unsigned int TransactionID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_BASM_Cancel_Transaction_t)(unsigned int ClientCallbackID, unsigned int TransactionID);
#endif

#endif
