/*****< pasmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PASMAPI - Phone Alert Status (PAS) Manager API for Stonestreet One        */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/28/12  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __PASMAPIH__
#define __PASMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "PASMMSG.h"             /* BTPM Phone Alert Manager Message Formats. */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the Phone Alert Status Module is  */
   /* initialized.                                                      */
typedef struct _tagPASM_Initialization_Data_t
{
   PASM_Alert_Status_t   DefaultAlertStatus;
   PASM_Ringer_Setting_t DefaultRingerSetting;
} PASM_Initialization_Info_t;

#define PASM_INITIALIZATION_DATA_SIZE                          (sizeof(PASM_Initialization_Info_t))

   /* The following enumerated type represents the Phone Alert Status   */
   /* Manager Event Types that are dispatched by this module to inform  */
   /* other modules of PAS Manager Changes.                             */
typedef enum
{
   etPASConnected,
   etPASDisconnected,
   etPASRingerControlPointCommand
} PASM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPASConnected event.           */
typedef struct _tagPASM_Connected_Event_Data_t
{
   unsigned int           CallbackID;
   PASM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PASM_Connected_Event_Data_t;

#define PASM_CONNECTED_EVENT_DATA_SIZE                         (sizeof(PASM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPASDisconnected event.        */
typedef struct _tagPASM_Disconnected_Event_Data_t
{
   unsigned int           CallbackID;
   PASM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PASM_Disconnected_Event_Data_t;

#define PASM_DISCONNECTED_EVENT_DATA_SIZE               (sizeof(PASM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPASRingerControlPointCommand  */
   /* event.                                                            */
typedef struct _tagPASM_Ringer_Control_Point_Command_Event_Data_t
{
   unsigned int                  ServerCallbackID;
   BD_ADDR_t                     RemoteDeviceAddress;
   PASM_Ringer_Control_Command_t RingerControlCommand;
} PASM_Ringer_Control_Point_Command_Event_Data_t;

#define PASM_RINGER_CONTROL_POINT_COMMAND_EVENT_DATA_SIZE      (sizeof(PASM_Ringer_Control_Point_Command_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Phone Alert Status (PAS) Manager Event (and Event Data) of a PAS  */
   /* Manager Event.                                                    */
typedef struct _tagPASM_Event_Data_t
{
   PASM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      PASM_Connected_Event_Data_t                    ConnectedEventData;
      PASM_Disconnected_Event_Data_t                 DisconnectedEventData;
      PASM_Ringer_Control_Point_Command_Event_Data_t RingerControlPointCommandEventData;
   } EventData;
} PASM_Event_Data_t;

#define PASM_EVENT_DATA_SIZE                                   (sizeof(PASM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Phone Alert Status (PAS) Manager (PASM) Event Callback.  This   */
   /* function will be called whenever the PAS Manager dispatches an    */
   /* event (and the client has registered for events).  This function  */
   /* passes to the caller the PAS Manager Event and the Callback       */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the Event Data ONLY in  */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  Because of this, the processing in this function     */
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another Message will not be processed while this function call is */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *PASM_Event_Callback_t)(PASM_Event_Data_t *EventData, void *CallbackParameter);

   /* Phone Alert Status (PAS) Module Installation/Support Functions.   */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager Phone Alert Status (PAS) Manager   */
   /* Module.  This function should be registered with the Bluetopia    */
   /* Platform Manager Module Handler and will be called when the       */
   /* Platform Manager is initialized (or shut down).                   */
void BTPSAPI PASM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PASM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* Phone Alert Status (PAS) Server Role Functions.                   */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Phone Alert Status Server callback function */
   /* with the Phone Alert Status (PAS) Manager Service.  This Callback */
   /* will be dispatched by the PAS Manager when various PAS Manager    */
   /* Server Events occur.  This function accepts the Callback Function */
   /* and Callback Parameter (respectively) to call when a PAS Manager  */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PASM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Server Event Callback can be registered in the    */
   /*          system at a time.                                        */
BTPSAPI_DECLARATION int BTPSAPI PASM_Register_Server_Event_Callback(PASM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PASM_Register_Server_Event_Callback_t)(PASM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Phone Alert Status (PAS)      */
   /* Manager Server Event Callback (registered via a successful call to*/
   /* the PASM_Register_Server_Event_Callback() function).  This        */
   /* function accepts as input the PAS Manager Event Callback ID       */
   /* (return value from PASM_Register_Server_Event_Callback()          */
   /* function).                                                        */
BTPSAPI_DECLARATION void BTPSAPI PASM_Un_Register_Server_Event_Callback(unsigned int ServerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_PASM_Un_Register_Server_Event_Callback_t)(unsigned int ServerCallbackID);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS).  This function is  */
   /* responsible for updating the Alert Status internally, as well as  */
   /* dispatching any Alert Notifications that have been registered by  */
   /* Phone Alert Status (PAS) clients.  This function accepts as it's  */
   /* parameter the Server callback ID that was returned from a         */
   /* successful call to PASM_Register_Server_Event_Callback() followed */
   /* by the Alert Status value to set.  This function returns zero if  */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI PASM_Set_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PASM_Set_Alert_Status_t)(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) alert       */
   /* status.  This function accepts as it's parameter the Server       */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured alert status upon successful   */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the AlertStatus    */
   /*          buffer will contain the currently configured alert       */
   /*          status.  If this function returns an error then the      */
   /*          contents of the AlertStatus buffer will be undefined.    */
BTPSAPI_DECLARATION int BTPSAPI PASM_Query_Alert_Status(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PASM_Query_Alert_Status_t)(unsigned int ServerCallbackID, PASM_Alert_Status_t *AlertStatus);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the current Phone Alert Status (PAS) ringer setting as   */
   /* well as dispatching any Alert Notifications that have been        */
   /* registered by PAS clients.  This function accepts as it's         */
   /* parameter the PAS Server callback ID that was returned from a     */
   /* successful call to PASM_Register_Server_Event_Callback() and the  */
   /* ringer value to configure.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI PASM_Set_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PASM_Set_Ringer_Setting_t)(unsigned int ServerCallbackID, PASM_Ringer_Setting_t RingerSetting);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* querying the last configured Phone Alert Status (PAS) ringer      */
   /* setting.  This function accepts as it's parameter the Server      */
   /* callback ID that was returned from a successful call to           */
   /* PASM_Register_Server_Event_Callback() followed by a buffer that   */
   /* will hold the currently configured ringer setting upon successful */
   /* execution of this function.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success then the RingerSetting  */
   /*          buffer will contain the currently configured ringer      */
   /*          setting.  If this function returns an error then the     */
   /*          contents of the RingerSetting buffer will be undefined.  */
BTPSAPI_DECLARATION int BTPSAPI PASM_Query_Ringer_Setting(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PASM_Query_Ringer_Setting_t)(unsigned int ServerCallbackID, PASM_Ringer_Setting_t *RingerSetting);
#endif

#endif
