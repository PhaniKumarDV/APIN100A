/*****< anpmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPMAPI - Alert Notification Profile (ANP) Manager API for Stonestreet    */
/*            One Bluetooth Protocol Stack Platform Manager.                  */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/05/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __ANPMAPIH__
#define __ANPMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "ANPMMSG.h"             /* BTPM ANP Manager Message Formats.         */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the ANP Module is initialized.    */
typedef struct _tagANPM_Initialization_Data_t
{
   Word_t SupportedNewAlertCategories;
   Word_t SupportedUnReadAlertCategories;
} ANPM_Initialization_Info_t;

#define ANPM_INITIALIZATION_DATA_SIZE                          (sizeof(ANPM_Initialization_Info_t))

   /* The following enumerated type represents the ANP Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of ANP Manager Changes.                                           */
typedef enum
{
   /* Shared client and server events.                                  */
   aetANPConnected,
   aetANPDisconnected,

   /* ANP Server events.                                                */
   aetANPNewAlertCategoryEnabled,
   aetANPNewAlertCategoryDisabled,
   aetANPUnReadAlertCategoryEnabled,
   aetANPUnReadAlertCategoryDisabled,

   /* ANP Client events.                                                */
   aetANPSupportedNewAlertCategoriesResult,
   aetANPSupportedUnreadCategoriesResult,
   aetANPNewAlertNotification,
   aetANPUnreadStatusNotification,
   aetANPCommandResult
} ANPM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPConnected event.          */
typedef struct _tagANPM_Connected_Event_Data_t
{
   ANPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} ANPM_Connected_Event_Data_t;

#define ANPM_CONNECTED_EVENT_DATA_SIZE                   (sizeof(ANPM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPDisconnected event.       */
typedef struct _tagANPM_Disconnected_Event_Data_t
{
   ANPM_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDeviceAddress;
} ANPM_Disconnected_Event_Data_t;

#define ANPM_DISCONNECTED_EVENT_DATA_SIZE               (sizeof(ANPM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPNewAlertCategoryEnabled   */
   /* event.                                                            */
typedef struct _tagANPM_New_Alert_Category_Enabled_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryEnabled;
   Word_t                         EnabledCategories;
} ANPM_New_Alert_Category_Enabled_Event_Data_t;

#define ANPM_NEW_ALERT_CATEGORY_ENABLED_EVENT_DATA_SIZE   (sizeof(ANPM_New_Alert_Category_Enabled_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPNewAlertCategoryDisabled  */
   /* event.                                                            */
typedef struct _tagANPM_New_Alert_Category_Disabled_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryDisabled;
   Word_t                         EnabledCategories;
} ANPM_New_Alert_Category_Disabled_Event_Data_t;

#define ANPM_NEW_ALERT_CATEGORY_DISABLED_EVENT_DATA_SIZE   (sizeof(ANPM_New_Alert_Category_Disabled_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPUnReadAlertCategoryEnabled*/
   /* event.                                                            */
typedef struct _tagANPM_Un_Read_Alert_Category_Enabled_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryEnabled;
   Word_t                         EnabledCategories;
} ANPM_Un_Read_Alert_Category_Enabled_Event_Data_t;

#define ANPM_UN_READ_ALERT_CATEGORY_ENABLED_EVENT_DATA_SIZE   (sizeof(ANPM_Un_Read_Alert_Category_Enabled_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* aetANPUnReadAlertCategoryDisabled event.                          */
typedef struct _tagANPM_Un_Read_Alert_Category_Disabled_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryDisabled;
   Word_t                         EnabledCategories;
} ANPM_Un_Read_Alert_Category_Disabled_Event_Data_t;

#define ANPM_UN_READ_ALERT_CATEGORY_DISABLED_EVENT_DATA_SIZE   (sizeof(ANPM_Un_Read_Alert_Category_Disabled_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetANPSupportedNewAlertCategoriesResult event.                    */
typedef struct _tagANPM_Supported_New_Alert_Categories_Result_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  TransactionID;
   unsigned int  Status;
   unsigned int  AttProtocolErrorCode;
   unsigned long SupportedCategories;
} ANPM_Supported_New_Alert_Categories_Result_Event_Data_t;

#define ANPM_SUPPORTED_NEW_ALERT_CATEGORIES_RESULT_EVENT_DATA_SIZE   (sizeof(ANPM_Supported_New_Alert_Categories_Result_Event_Data_t))

   /* The following structure is a container structure                  */
   /* that holds the information that is returned in a                  */
   /* aetANPSupportedUnreadCategoriesResult event.                      */
typedef struct _tagANPM_Supported_Unread_Categories_Result_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  TransactionID;
   unsigned int  Status;
   unsigned int  AttProtocolErrorCode;
   unsigned long SupportedCategories;
} ANPM_Supported_Unread_Categories_Result_Event_Data_t;

#define ANPM_SUPPORTED_UNREAD_CATEGORIES_RESULT_EVENT_DATA_SIZE (sizeof(ANPM_Supported_Unread_Categories_Result_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPNewAlertNotification      */
   /* event.                                                            */
   /* * NOTE * LastAlertText could be NULL if there is no text          */
   /*          information.                                             */
typedef struct _tagANPM_New_Alert_Notification_Event_Data_t
{
   BD_ADDR_t                       RemoteDeviceAddress;
   ANPM_Category_Identification_t  CategoryID;
   unsigned int                    NumberNewAlerts;
   char                           *LastAlertText;
} ANPM_New_Alert_Notification_Event_Data_t;

#define ANPM_NEW_ALERT_NOTIFICATION_EVENT_DATA_SIZE            (sizeof(ANPM_New_Alert_Notification_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPUnreadStatusNotification  */
   /* event.                                                            */
typedef struct _tagANPM_Unread_Status_Notification_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   ANPM_Category_Identification_t CategoryID;
   unsigned int                   NumberUnreadAlerts;
} ANPM_Unread_Status_Notification_Event_Data_t;

#define ANPM_UNREAD_STATUS_NOTIFICATION_EVENT_DATA_SIZE        (sizeof(ANPM_Unread_Status_Notification_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a aetANPCommandResult event.      */
typedef struct _tagANPM_Command_Result_Event_Data_t
{
   BD_ADDR_t                         RemoteDeviceAddress;
   unsigned int                      TransactionID;
   unsigned int                      AttProtocolErrorCode;
   unsigned int                      Status;
} ANPM_Command_Result_Event_Data_t;

#define ANPM_COMMAND_RESULT_EVENT_DATA_SIZE                    (sizeof(ANPM_Command_Result_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Alert Notification Profile (ANP) Manager Event (and Event Data) of*/
   /* a ANP Manager Event.                                              */
typedef struct _tagANPM_Event_Data_t
{
   ANPM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      ANPM_Connected_Event_Data_t                             ConnectedEventData;
      ANPM_Disconnected_Event_Data_t                          DisconnectedEventData;
      ANPM_New_Alert_Category_Enabled_Event_Data_t            NewAlertCategoryEnabledEventData;
      ANPM_New_Alert_Category_Disabled_Event_Data_t           NewAlertCategoryDisabledEventData;
      ANPM_Un_Read_Alert_Category_Enabled_Event_Data_t        UnReadAlertCategoryEnabledEventData;
      ANPM_Un_Read_Alert_Category_Disabled_Event_Data_t       UnReadAlertCategoryDisabledEventData;
      ANPM_Supported_New_Alert_Categories_Result_Event_Data_t SupportedNewAlertCategoriesEventData;
      ANPM_Supported_Unread_Categories_Result_Event_Data_t    SupportedUnreadCategoriesEventData;
      ANPM_New_Alert_Notification_Event_Data_t                NewAlertNotificationEventData;
      ANPM_Unread_Status_Notification_Event_Data_t            UnreadStatusNotificationEventData;
      ANPM_Command_Result_Event_Data_t                        CommandResultEventData;
   } EventData;
} ANPM_Event_Data_t;

#define ANPM_EVENT_DATA_SIZE                                   (sizeof(ANPM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Alert Notification Profile (ANP) Manager dispatches an event (and */
   /* the client has registered for events).  This function passes to   */
   /* the caller the ANP Manager Event and the Callback Parameter that  */
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
typedef void (BTPSAPI *ANPM_Event_Callback_t)(ANPM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANP Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI ANPM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANPM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of New Alerts for a specific category and*/
   /* the text of the last alert for the specified category.  This      */
   /* function accepts as the Category ID of the specific category, the */
   /* number of new alerts for the specified category and a text string */
   /* that describes the last alert for the specified category (if any).*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Set_New_Alert(ANPM_Category_Identification_t CategoryID, unsigned int NewAlertCount, char *LastAlertText);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Set_New_Alert_t)(ANPM_Category_Identification_t CategoryID, unsigned int NewAlertCount, char *LastAlertText);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* module to set the number of Un-Read Alerts for a specific         */
   /* category.  This function accepts as the Category ID of the        */
   /* specific category, and the number of un-read alerts for the       */
   /* specified category.  This function returns zero if successful, or */
   /* a negative return error code if there was an error.               */
   /* * NOTE * ciAllSupportedCategories is not a valid value for the    */
   /*          CategoryID parameter.                                    */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Set_Un_Read_Alert(ANPM_Category_Identification_t CategoryID, unsigned int UnReadAlertCount);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Set_Un_Read_Alert_t)(ANPM_Category_Identification_t CategoryID, unsigned int UnReadAlertCount);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Server     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a ANP Manager      */
   /* Server Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Server_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Register_Server_Event_Callback(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Register_Server_Event_Callback_t)(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Server Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Server_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Server_Event_Callback() function).             */
BTPSAPI_DECLARATION void BTPSAPI ANPM_Un_Register_Server_Event_Callback(unsigned int ANPManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANPM_Un_Register_Server_Event_Callback_t)(unsigned int ANPManagerCallbackID);
#endif

   /* Alert Notification Client API definitions. */

   /* This functions submits a request to a remote ANP Server to get the*/
   /* supported New Alert Categories.  The ClientCallbackID parameter   */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  This function returns a positive value indicating */
   /* the Transaction ID if successful or a negative return error code  */
   /* if there was an error.                                            */
   /* * NOTE * An aetANPSupportedNewAlertCategoriesResult event will be */
   /*          dispatched when this request completes.                  */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Get_Supported_New_Alert_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Get_Supported_New_Alert_Categories_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions will enable New Alert Notifications from a         */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Enable_New_Alert_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Enable_New_Alert_Notifications_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions will disable New Alert Notifications from a        */
   /* remote ANP server.  The ClientCallbackID parameter should be      */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Disable_New_Alert_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Disable_New_Alert_Notifications_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions submits a request to a remote ANP Server           */
   /* to get the supported Unread Alery Status Categories.  The         */
   /* ClientCallbackID parameter should be an ID return from            */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  This    */
   /* function returns a positive value indicating the Transaction ID if*/
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * An aetANPSupportedUnreadCategoriesResult event will be   */
   /*          dispatched when this request completes.                  */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Get_Supported_Unread_Status_Categories(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Get_Supported_Unread_Status_Categories_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions will enable Unread Alert Status Notifications from */
   /* a remote ANP server.  The ClientCallbackID parameter should be    */
   /* an ID return from ANPM_Register_Client_Event_Callback().  The     */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  If successful and a GATT write was triggered to   */
   /* enabled notifications on the remote ANP server, this function     */
   /* will return a positive value representing the Transaction ID of   */
   /* the submitted write. If this function successfully registers the  */
   /* callback, but a GATT write is not necessary, it will return 0. If */
   /* an error occurs, this function will return a negative error code. */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Enable_Unread_Status_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Enable_Unread_Status_Notifications_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions will disable Unread Alert Status Notifications     */
   /* from a remote ANP server.  The ClientCallbackID parameter should  */
   /* be an ID return from ANPM_Register_Client_Event_Callback().  The  */
   /* RemoteDeviceAddress parameter is the Bluetooth Address of the     */
   /* remote server.  This function returns zero if successful or a     */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Disable_Unread_Status_Notifications(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Disable_Unread_Status_Notifications_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Enable_New_Alert_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Enable_New_Alert_Category_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* This functions submits a request to a remote ANP Server to disable*/
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Disable_New_Alert_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Disable_New_Alert_Category_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* This functions submits a request to a remote ANP Server to enable */
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  If successful and a GATT write was triggered to enabled  */
   /* notifications on the remote ANP server, this function will return */
   /* a positive value representing the Transaction ID of the submitted */
   /* write. If this function successfully registers the callback, but a*/
   /* GATT write is not necessary, it will return 0. If an error occurs,*/
   /* this function will return a negative error code.                  */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value if positive.                            */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Enable_Unread_Status_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Enable_Unread_Status_Category_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* This functions submits a request to a remote ANP Server to disable*/
   /* notifications for a given category The ClientCallbackID parameter */
   /* should be an ID return from ANPM_Register_Client_Event_Callback().*/
   /* The RemoteDeviceAddress parameter is the Bluetooth Address of the */
   /* remote server.  The CategoryID paremter indicates the category to */
   /* enable.  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Disable_Unread_Status_Category(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Disable_Unread_Status_Category_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* This functions submits a request to a remote ANP Server           */
   /* to request an immediate New Alert Notification.  The              */
   /* ClientCallbackID parameter should be an ID return from            */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  The     */
   /* CategoryID paremter indicates the category to enable.  This       */
   /* function returns a positive value representing the Transaction    */
   /* ID if successful or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Request_New_Alert_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Request_New_Alert_Notification_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* This functions submits a request to a remote ANP Server           */
   /* to request an immediate Unread Alert Status Notification.         */
   /* The ClientCallbackID parameter should be an ID return from        */
   /* ANPM_Register_Client_Event_Callback().  The RemoteDeviceAddress   */
   /* parameter is the Bluetooth Address of the remote server.  The     */
   /* CategoryID paremter indicates the category to enable.  This       */
   /* function returns a positive value representing the Transaction    */
   /* ID if successful or a negative return error code if there was an  */
   /* error.                                                            */
   /* * NOTE * The status of the request will be returned in an         */
   /*          aetANPCommandResult event with Transaction ID matching   */
   /*          the return value.                                        */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Request_Unread_Status_Notification(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Request_Unread_Status_Notification_t)(unsigned int ClientCallbackID, BD_ADDR_t RemoteDeviceAddress, ANPM_Category_Identification_t CategoryID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a server callback function with the Alert     */
   /* Notification (ANP) Manager Service.  This Callback will be        */
   /* dispatched by the ANP Manager when various ANP Manager Client     */
   /* Events occur.  This function accepts the Callback Function and    */
   /* Callback Parameter (respectively) to call when a ANP Manager      */
   /* Client Event needs to be dispatched.  This function returns a     */
   /* positive (non-zero) value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANPM_Un_Register_Client_Event_Callback() function to     */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI ANPM_Register_Client_Event_Callback(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANPM_Register_Client_Event_Callback_t)(ANPM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANP Manager Client Event      */
   /* Callback (registered via a successful call to the                 */
   /* ANPM_Register_Client_Event_Callback() function).  This function   */
   /* accepts as input the ANP Manager Event Callback ID (return value  */
   /* from ANPM_Register_Client_Event_Callback() function).             */
BTPSAPI_DECLARATION void BTPSAPI ANPM_Un_Register_Client_Event_Callback(unsigned int ANPManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANPM_Un_Register_Client_Event_Callback_t)(unsigned int ANPManagerCallbackID);
#endif

#endif
