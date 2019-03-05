/*****< tdsmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  TDSMAPI - Local 3D Synchronization Profile API for Stonestreet One        */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   02/09/15  M. Seabold     Initial creation.                               */
/******************************************************************************/
#ifndef __TDSMAPIH__
#define __TDSMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "TDSMMSG.h"             /* BTPM 3D Sync Profile Message Formats.     */

   /* The following enumerated type represents the 3D Sync Manager Event*/
   /* Types that are dispatched by this module.                         */
typedef enum
{
   tetTDSM_Display_Connection_Announcement,
   tetTDSM_Display_Synchronization_Train_Complete,
   tetTDSM_Display_CSB_Supervision_Timeout,
   tetTDSM_Display_Channel_Map_Change,
   tetTDSM_Display_Slave_Page_Response_Timeout,
} TDSM_Event_Type_t;

   /* The following structure contains the information dispatched       */
   /* in a tetTDSM_Display_Connection_Announcement event. The           */
   /* RemoteDeviceAddress is the Bluetooth Address of the device which  */
   /* sent an association announcement. The Flags is a bitmask that     */
   /* indicates the reason for the announcement. The Battery is an      */
   /* integer from 0-100 which indicates the percentage of battery      */
   /* remaining on the glasses.                                         */
typedef struct _tagTDSM_Display_Connection_Announcement_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Flags;
   unsigned int BatteryLevel;
} TDSM_Display_Connection_Announcement_Data_t;

#define TDSM_DISPLAY_CONNECTION_ANNOUNCEMENT_DATA_SIZE         sizeof(TDSM_Display_Connection_Announcement_Data_t)

   /* The following structure contains the information dispatched in a  */
   /* tetTDSM_Display_Synchronization_Train_Complete event. The Status  */
   /* indicates the reason why the synchronization train has stopped.   */
typedef struct _tagTDSM_Display_Synchronization_Train_Complete_Data_t
{
   unsigned int Status;
} TDSM_Display_Synchronization_Train_Complete_Data_t;

#define TDSM_DISPLAY_SYNCHRONIZATION_TRAIN_COMPLETE_DATA_SIZE  sizeof(TDSM_Display_Synchronization_Train_Complete_Data_t)

   /* The following structure contains the information dispatched in a  */
   /* tetTDSM_Display_Channel_Map_Change event. The ChannelMap indicates*/
   /* the new channel map.                                              */
   /* * NOTE * The application will likely want to start the            */
   /*          synchronization train after this event in order for the  */
   /*          glasses to re-sync to the new channel map.               */
typedef struct _tagTDSM_Display_Channel_Map_Change_Data_t
{
   AFH_Channel_Map_t ChannelMap;
} TDSM_Display_Channel_Map_Change_Data_t;

#define TDSM_DISPLAY_CHANNEL_MAP_CHANGE_DATA_SIZE              sizeof(TDSM_Display_Channel_Map_Change_Data_t)

   /* The following structure represnts the data dispatched for a TDSM  */
   /* event.                                                            */
typedef struct _tagTDSM_Event_Data_t
{
   TDSM_Event_Type_t EventType;
   unsigned int      EventDataSize;
   union
   {
      TDSM_Display_Connection_Announcement_Data_t        DisplayConnectionAnnouncementData;
      TDSM_Display_Synchronization_Train_Complete_Data_t DisplaySynchronizationTrainCompleteData;
      TDSM_Display_Channel_Map_Change_Data_t             DisplayChannelMapChangeData;
   } EventData;
} TDSM_Event_Data_t;

#define TDSM_EVENT_DATA_SIZE                                   sizeof(TDSM_Event_Data_t)

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* 3D Sync Manager dispatches an event (and the client has registered*/
   /* for events).  This function passes to the caller the 3D Sync      */
   /* Manager Event and the Callback Parameter that was specified when  */
   /* this Callback was installed.  The caller is free to use the       */
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
   /* * NOTE * This function MUST NOT block and wait for events that can*/
   /*          only be satisfied by Receiving other Events.  A deadlock */
   /*          WILL occur because NO Event Callbacks will be issued     */
   /*          while this function is currently outstanding.            */
typedef void (BTPSAPI *TDSM_Event_Callback_t)(TDSM_Event_Data_t *EventData, void *CallbackParameter);

   /* 3D Sync Module Installation/Support Functions.                    */

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager 3D Sync Manager module.  This      */
   /* function should be registered with the Bluetopia Platform Manager */
   /* module handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI TDSM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager module handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI TDSM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* 3D Synchronization Profile API Functions.                         */

   /* The following function will configure the Synchronization Train   */
   /* parameters on the Bluetooth controller.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The SyncTrainParams parameter is a pointer to the       */
   /* Synchronization Train parameters to be set.  The IntervalResult   */
   /* parameter is a pointer to the variable that will be populated with*/
   /* the Synchronization Interval chosen by the Bluetooth controller.  */
   /* This function will return zero on success; otherwise, a negative  */
   /* error value will be returned.                                     */
   /* * NOTE * The Timout value in the                                  */
   /*          TDS_Synchronization_Train_Parameters_t structure must be */
   /*          a value of at least 120 seconds, or the function call    */
   /*          will fail.                                               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Write_Synchronization_Train_Parameters(unsigned int TDSMControlCallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Write_Synchronization_Train_Parameters_t)(unsigned int TDSMControlCallbackID, TDSM_Synchronization_Train_Parameters_t *SyncTrainParams, Word_t *IntervalResult);
#endif

   /* The following function will enable the Synchronization Train. The */
   /* TDSMControlCallbackID parameter is an ID returned from a succesfull*/
   /* call to TDSM_Register_Event_Callback() with the Control parameter */
   /* set to TRUE.  This function will return zero on success;          */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The TDSM_Write_Synchronization_Train_Parameters function */
   /*          should be called at least once after initializing the    */
   /*          stack and before calling this function.                  */
   /* * NOTE * The tetTDSM_Display_Synchronization_Train_Complete event */
   /*          will be triggered when the Synchronization Train         */
   /*          completes.  This function can be called again at this    */
   /*          time to restart the Synchronization Train.               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Start_Synchronization_Train(unsigned int TDSMControlCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Start_Synchronization_Train_t)(unsigned int TDSMControlCallbackID);
#endif

   /* The following function will configure and enable                  */
   /* the Connectionless Slave Broadcast channel on the                 */
   /* Bluetooth controller.  The TDSMControlCallbackID                   */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set     */
   /* to TRUE.  The ConnectionlessSlaveBroadcastParams parameter is a   */
   /* pointer to the Connectionless Slave Broadcast parameters to be    */
   /* set.  The IntervalResult parameter is a pointer to the variable   */
   /* that will be populated with the Broadcast Interval chosen by the  */
   /* Bluetooth controller.  This function will return zero on success; */
   /* otherwise, a negative error value will be returned.               */
   /* * NOTE * The MinInterval value should be greater than or equal to */
   /*          50 milliseconds, and the MaxInterval value should be less*/
   /*          than or equal to 100 milliseconds; otherwise, the        */
   /*          function will fail.                                      */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Enable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Enable_Connectionless_Slave_Broadcast_t)(unsigned int TDSMControlCallbackID, TDSM_Connectionless_Slave_Broadcast_Parameters_t *ConnectionlessSlaveBroadcastParams, Word_t *IntervalResult);
#endif

   /* The following function is used to disable the previously enabled  */
   /* Connectionless Slave Broadcast channel.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.                                                             */
   /* * NOTE * Calling this function will terminate the Synchronization */
   /*          Train (if it is currently enabled).                      */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Disable_Connectionless_Slave_Broadcast(unsigned int TDSMControlCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Disable_Connectionless_Slave_Broadcast_t)(unsigned int TDSMControlCallbackID);
#endif

   /* The following function is used to get the current information     */
   /* being used int the synchronization broadcasts.  The               */
   /* CurrentBroadcastInformation parameter to a structure in which the */
   /* current information will be placed.  This function returns zero if*/
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Get_Current_Broadcast_Information(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Get_Current_Broadcast_Information_t)(TDSM_Current_Broadcast_Information_t *CurrentBroadcastInformation);
#endif

   /* The following function is used to update the information being    */
   /* sent in the synchronization broadcasts.  The TDSMControlCallbackID */
   /* parameter is an ID returned from a succesfull call to             */
   /* TDSM_Register_Event_Callback() with the Control parameter set to  */
   /* TRUE.  The BroadcastInformationUpdate parameter is a pointer to a */
   /* structure which contains the information to update.  This function*/
   /* returns zero if successful and a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Update_Broadcast_Information(unsigned int TDSMControlCallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Update_Broadcast_Information_t)(unsigned int TDSMControlCallbackID, TDSM_Broadcast_Information_Update_t *BroadcastInformationUpdate);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the 3D Sync Profile  */
   /* Manager Service.  This Callback will be dispatched by the 3D Sync */
   /* Manager when various 3D Sync Manager events occur.  This function */
   /* accepts the callback function and callback parameter              */
   /* (respectively) to call when a 3D Sync Manager event needs to be   */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          TDSM_Un_Register_Event_Callback() function to un-register*/
   /*          the callback from this module.                           */
   /* * NOTE * Any registered TDSM Callback will get all TDSM events,   */
   /*          but only a callback registered with the Control parameter*/
   /*          set to TRUE can manage the Sync Train and Broadcast      */
   /*          information. There can only be one of these Control      */
   /*          callbacks registered.                                    */
BTPSAPI_DECLARATION int BTPSAPI TDSM_Register_Event_Callback(Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_TDSM_Register_Event_Callback_t)(Boolean_t Control, TDSM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered 3D Sync Manager event Callback*/
   /* (registered via a successful call to the                          */
   /* TDSM_Register_Event_Callback() function.  This function accepts as*/
   /* input the 3D Sync Manager event callback ID (return value from the*/
   /* TDSM_Register_Event_Callback() function).                         */
BTPSAPI_DECLARATION void BTPSAPI TDSM_Un_Register_Event_Callback(unsigned int TDSManagerEventCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_TDSM_Un_Register_Event_Callback_t)(unsigned int TDSManagerEventCallbackID);
#endif

#endif
