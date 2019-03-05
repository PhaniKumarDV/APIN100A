/*****< pxpmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PXPMAPI - Proximity Profile (PXP) Manager API for Stonestreet One         */
/*            Bluetooth Protocol Stack Platform Manager.                      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/20/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __PXPMAPIH__
#define __PXPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "PXPMMSG.h"             /* BTPM PXP Manager Message Formats.         */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the PXP Module is initialized.    */
typedef struct _tagPXPM_Initialization_Data_t
{
   unsigned int       DefaultRefreshTime;
   PXPM_Alert_Level_t DefaultAlertLevel;
   int                DefaultPathLossThreshold;
} PXPM_Initialization_Info_t;

#define PXPM_INITIALIZATION_DATA_SIZE                          (sizeof(PXPM_Initialization_Info_t))

   /* The following enumerated type represents the PXP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of PXP Manager Changes.                                           */
typedef enum
{
   etPXPConnected,
   etPXPDisconnected,
   etPXPPathLossAlert,
   etPXPLinkLossAlert
} PXPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPXPConnected event.           */
typedef struct _tagPXPM_Connected_Event_Data_t
{
   unsigned int           CallbackID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   unsigned long          SupportedFeatures;
} PXPM_Connected_Event_Data_t;

#define PXPM_CONNECTED_EVENT_DATA_SIZE                   (sizeof(PXPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPXPDisconnected event.        */
typedef struct _tagPXPM_Disconnected_Event_Data_t
{
   unsigned int           CallbackID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} PXPM_Disconnected_Event_Data_t;

#define PXPM_DISCONNECTED_EVENT_DATA_SIZE               (sizeof(PXPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPXPPathLossAlert event.       */
typedef struct _tagPXPM_Path_Loss_Alert_Event_Data_t
{
   unsigned int           CallbackID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   PXPM_Alert_Level_t     AlertLevel;
} PXPM_Path_Loss_Alert_Event_Data_t;

#define PXPM_PATH_LOSS_ALERT_EVENT_DATA_SIZE             (sizeof(PXPM_Path_Loss_Alert_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etPXPLinkLossAlert event.       */
typedef struct _tagPXPM_Link_Loss_Alert_Event_Data_t
{
   unsigned int           CallbackID;
   PXPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
   PXPM_Alert_Level_t     AlertLevel;
} PXPM_Link_Loss_Alert_Event_Data_t;

#define PXPM_LINK_LOSS_ALERT_EVENT_DATA_SIZE             (sizeof(PXPM_Link_Loss_Alert_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Proximity Profile (PXP) Manager Event (and Event Data) of a PXP   */
   /* Manager Event.                                                    */
typedef struct _tagPXPM_Event_Data_t
{
   PXPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      PXPM_Connected_Event_Data_t       ConnectedEventData;
      PXPM_Disconnected_Event_Data_t    DisconnectedEventData;
      PXPM_Path_Loss_Alert_Event_Data_t PathLossAlertEventData;
      PXPM_Link_Loss_Alert_Event_Data_t LinkLossAlertEventData;
   } EventData;
} PXPM_Event_Data_t;

#define PXPM_EVENT_DATA_SIZE                                   (sizeof(PXPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Proximity Profile (PXP) Manager dispatches an event (and the      */
   /* client has registered for events).  This function passes to the   */
   /* caller the PXP Manager Event and the Callback Parameter that was  */
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
typedef void (BTPSAPI *PXPM_Event_Callback_t)(PXPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager PXP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI PXPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI PXPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a Monitor callback function with the Proximity*/
   /* Profile (PXP) Manager Service.  This Callback will be dispatched  */
   /* by the PXP Manager when various PXP Manager Monitor Events occur. */
   /* This function accepts the Callback Function and Callback Parameter*/
   /* (respectively) to call when a PXP Manager Monitor Event needs to  */
   /* be dispatched.  This function returns a positive (non-zero) value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          PXPM_Un_Register_Monitor_Event_Callback() function to    */
   /*          un-register the callback from this module.               */
   /* * NOTE * Only 1 Monitor Event Callback can be registered in the   */
   /*          system at a time.                                        */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Register_Monitor_Event_Callback(PXPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Register_Monitor_Event_Callback_t)(PXPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered PXP Manager Monitor Event     */
   /* Callback (registered via a successful call to the                 */
   /* PXPM_Register_Monitor_Event_Callback() function).  This function  */
   /* accepts as input the PXP Manager Event Callback ID (return value  */
   /* from PXPM_Register_Monitor_Event_Callback() function).            */
BTPSAPI_DECLARATION void BTPSAPI PXPM_Un_Register_Monitor_Event_Callback(unsigned int MonitorCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_PXPM_Un_Register_Monitor_Event_Callback_t)(unsigned int MonitorCallbackID);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the refresh time for checking the Path Loss (the time    */
   /* between checking the path loss for a given link).  This function  */
   /* accepts as it's parameter the MonitorCallbackID that was returned */
   /* from a successful call to PXPM_Register_Monitor_Event_Callback()  */
   /* and the Refresh Time (in milliseconds).  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * Decreasing the refresh rate will increase the power      */
   /*          consumption of the local Bluetooth device as it will     */
   /*          involve reading the RSSI at a faster rate.               */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Set_Path_Loss_Refresh_Time(unsigned int MonitorCallbackID, unsigned int RefreshTime);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Set_Path_Loss_Refresh_Time_t)(unsigned int MonitorCallbackID, unsigned int RefreshTime);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Threshold for a specified PXP Monitor      */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert.  This   */
   /* function accepts as it's parameter the MonitorCallbackID that was */
   /* returned from a successful call to                                */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss for, and the Path Loss Threshold  */
   /* to set.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The Path Loss Threshold should be specified in units of  */
   /*          dBm.                                                     */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Set_Path_Loss_Threshold(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int PathLossThreshold);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Set_Path_Loss_Threshold_t)(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int PathLossThreshold);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* querying the current Path Loss for a specified PXP Monitor        */
   /* Connection.  This function accepts as it's parameter the          */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss for, and a pointer to a buffer to */
   /* return the current Path Loss in (if successful).  This function   */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * The Path Loss Threshold will be specified in units of    */
   /*          dBm.                                                     */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Query_Current_Path_Loss(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int *PathLossResult);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Query_Current_Path_Loss_t)(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, int *PathLossResult);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the Path Loss Alert Level for a specified PXP Monitor    */
   /* Connection.  If the Path Loss exceeds this threshold then an event*/
   /* will be generated to inform the caller to sound an alert at the   */
   /* specified level.  This function accepts as it's parameter the     */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the path loss alert level for, and the Path Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Set_Path_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Set_Path_Loss_Alert_Level_t)(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* changing the Link Loss Alert Level for a specified PXP Monitor    */
   /* Connection.  If the link to the proximity device is lost then an  */
   /* event will be generated to inform the caller to sound an alert at */
   /* the specified level.  This function accepts as it's parameter the */
   /* MonitorCallbackID that was returned from a successful call to     */
   /* PXPM_Register_Monitor_Event_Callback(), the BD_ADDR of the Monitor*/
   /* Connection to set the link loss alert level for, and the Link Loss*/
   /* Alert Level to set.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI PXPM_Set_Link_Loss_Alert_Level(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PXPM_Set_Link_Loss_Alert_Level_t)(unsigned int MonitorCallbackID, BD_ADDR_t BD_ADDR, PXPM_Alert_Level_t AlertLevel);
#endif

#endif
