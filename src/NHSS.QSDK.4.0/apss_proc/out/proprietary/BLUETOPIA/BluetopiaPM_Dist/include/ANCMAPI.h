/*****< ancmapi.h >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANCMAPI - Apple Notification Center Service (ANCS) Manager API for        */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Matt Buckley                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/03/13  M. Buckley     Initial creation.                               */
/******************************************************************************/
#ifndef __ANCMAPIH__
#define __ANCMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

   /* The following enumerated type represents the different Category   */
   /* IDs that can be used.                                             */
typedef enum
{
   cidOther,
   cidIncomingCall,
   cidMissedCall,
   cidVoicemail,
   cidSocial,
   cidSchedule,
   cidEmail,
   cidNews,
   cidHealthAndFitness,
   cidBusinessAndFinance,
   cidLocation,
   cidEntertainment
} ANCM_Category_ID_t;

   /* The following enumerated type represents the different Event IDs  */
   /* that can be used.                                                 */
typedef enum
{
   eidNotificationAdded,
   eidNotificationModified,
   eidNotificationRemoved
} ANCM_Event_ID_t;

   /* The following enumerated type represents the different Command    */
   /* IDs that can be used.                                             */
typedef enum
{
   cidGetNotificationAttributes,
   cidGetAppAttributes
} ANCM_Command_ID_t;

   /* The following enumerated type represents the different            */
   /* Notification Attribute IDs that can be used.                      */
typedef enum
{
   naidAppIdentifier,
   naidTitle,
   naidSubtitle,
   naidMessage,
   naidMessageSize,
   naidDate
} ANCM_Notification_Attribute_ID_t;

   /* The following enumerated type represents the different App        */
   /* Attribute IDs that can be used.                                   */
typedef enum
{
   aaidDisplayName
} ANCM_App_Attribute_ID_t;

   /* The following structure is a container structure that holds the   */
   /* information related to returned app attribute data.               */
typedef struct _tagANCM_App_Attribute_Data_t
{
   ANCM_App_Attribute_ID_t  AttributeID;
   unsigned int             AttributeLength;
   Byte_t                  *AttributeData;
} ANCM_App_Attribute_Data_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCM_App_Attribute_Data_t struct     */
   /* based on the length of its Attribute Data.  The first parameter   */
   /* to this MACRO is the length of the member AttributeData in bytes. */
#define ANCM_APP_ATTRIBUTE_DATA_SIZE(_x)                 (sizeof(ANCM_App_Attribute_Data_t) + ((_x)*BYTE_SIZE))

   /* The following structure is a container structure that holds the   */
   /* information related to returned notification attribute data.      */
typedef struct _tagANCM_Notification_Attribute_Data_t
{
   ANCM_Notification_Attribute_ID_t  AttributeID;
   unsigned int                      AttributeLength;
   Byte_t                           *AttributeData;
} ANCM_Notification_Attribute_Data_t;

   /* The following MACRO is a utility MACRO that exists to aid in      */
   /* calculating the length of an ANCM_Notification_Attribute_Data_t   */
   /* based on the length of its Attribute Data.  The first parameter   */
   /* to this MACRO is the length of the member AttributeData in bytes. */
#define ANCM_NOTIFICATION_ATTRIBUTE_DATA_SIZE(_x)        (sizeof(ANCM_Notification_Attribute_Data_t) + ((_x)*BYTE_SIZE))

   /* The following enumerated type represents the different types of   */
   /* Attribute Data that can exist.                                    */
typedef enum
{
   adtNotification,
   adtApplication
} ANCM_Attribute_Data_Type_t;

   /* The following structure holds parsed attribute data for either    */
   /* Notification Attributes or App Attributes.  This data structure   */
   /* is used when calling the                                          */
   /* ANCM_ConvertRawAttributeDataToParsedAttributeData API function.   */
typedef struct _tagANCM_Parsed_Attribute_Data_t
{
   ANCM_Attribute_Data_Type_t Type;
   unsigned int               NumberOfAttributes;
   union
   {
      ANCM_Notification_Attribute_Data_t *NotificationAttributeData;
      ANCM_App_Attribute_Data_t          *AppAttributeData;
   } AttributeData;
} ANCM_Parsed_Attribute_Data_t;

#define ANCM_PARSED_ATTRIBUTE_DATA_SIZE                  (sizeof(ANCM_Parsed_Attribute_Data_t))

   /* The following structure defines the format of the data required   */
   /* for each Notification Attribute being requested when calling the  */
   /* API function ANCM_Get_Notification_Attributes().                  */
   /* * NOTE * The member AttributeMaxLength is only required when      */
   /*          requesting specific Notification Attributes.  The        */
   /*          attributes which require this Max Length Parameter are   */
   /*          specified in the Apple Notification Center Service       */
   /*          Specification.                                           */
typedef struct _tagANCM_Notification_Attribute_Request_Data_t
{
   ANCM_Notification_Attribute_ID_t NotificationAttributeID;
   unsigned int                     AttributeMaxLength;
} ANCM_Notification_Attribute_Request_Data_t;

   /* The following structure defines the format of the data required   */
   /* for each App Attribute being requested when calling the API       */
   /* function ANCM_Get_Notification_Attributes().                      */
   /* * NOTE * The member AttributeMaxLength is only required when      */
   /*          requesting specific App Attributes.  The attributes      */
   /*          which require this Max Length Parameter are specified in */
   /*          the Apple Notification Center Service Specification.     */
   /* * NOTE * As of the ANCS Specification v1.0, no App Attribute      */
   /*          types require a Max Length Parameter.  This              */
   /*          functionality is put in place in case future versions of */
   /*          the Specification require it.                            */
typedef struct _tagANCM_App_Attribute_Request_Data_t
{
   ANCM_App_Attribute_ID_t AppAttributeID;
   unsigned int            AttributeMaxLength;
} ANCM_App_Attribute_Request_Data_t;

   /* The following enumerated type represents the ANCS Manager Event   */
   /* Types that are dispatched by this module to inform other modules  */
   /* of ANCS Manager Changes.                                          */
typedef enum
{
   etANCMConnected,
   etANCMDisconnected,
   etANCMNotificationsEnabledStatus,
   etANCMNotificationReceived,
   etANCMGetNotificationAttributesResponse,
   etANCMGetAppAttributesResponse
} ANCM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etANCMConnected event.          */
typedef struct _tagANCM_Connected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} ANCM_Connected_Event_Data_t;

#define ANCM_CONNECTED_EVENT_DATA_SIZE                   (sizeof(ANCM_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etANCMDisconnected event.       */
typedef struct _tagANCM_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} ANCM_Disconnected_Event_Data_t;

#define ANCM_DISCONNECTED_EVENT_DATA_SIZE                (sizeof(ANCM_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* etANCMNotificationsEnabledStatus event.                           */
typedef struct _tagANCM_Notifications_Enabled_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int Status;
} ANCM_Notifications_Enabled_Status_Event_Data_t;

#define ANCM_NOTIFICATIONS_ENABLED_STATUS_EVENT_DATA_SIZE   (sizeof(ANCM_Notifications_Enabled_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a etANCMNotificationReceived      */
   /* event.                                                            */
typedef struct _tagANCM_Notification_Received_Event_Data_t
{
   BD_ADDR_t       RemoteDeviceAddress;
   ANCM_Event_ID_t EventID;
   Byte_t          EventFlags;
   unsigned int    CategoryID;
   unsigned int    CategoryCount;
   DWord_t         NotificationUID;
} ANCM_Notification_Received_Event_Data_t;

#define ANCM_NOTIFICATION_RECEIVED_EVENT_DATA_SIZE       (sizeof(ANCM_Notification_Received_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* etANCMGetNotificationAttributesResponse event.                    */
typedef struct _tagANCM_Get_Notification_Attributes_Response_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  Status;
   DWord_t       NotificationUID;
   unsigned int  AttributeDataLength;
   Byte_t       *AttributeData;
} ANCM_Get_Notification_Attributes_Response_Event_Data_t;

#define ANCM_GET_NOTIFICATION_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE   (sizeof(ANCM_Get_Notification_Attributes_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* etANCMGetAppAttributesResponse event.                             */
   /* * NOTE * The member AppIdentifier should be a NULL-terminated     */
   /* UTF-8 string.                                                     */
typedef struct _tagANCM_Get_App_Attributes_Response_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned int  Status;
   char         *AppIdentifier;
   unsigned int  AttributeDataLength;
   Byte_t       *AttributeData;
} ANCM_Get_App_Attributes_Response_Event_Data_t;

#define ANCM_GET_APP_ATTRIBUTES_RESPONSE_EVENT_DATA_SIZE (sizeof(ANCM_Get_App_Attributes_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Apple Notification Center Service (ANCS) Manager Event (and Event */
   /* Data) of an ANCS Manager Event.                                   */
typedef struct _tagANCM_Event_Data_t
{
   ANCM_Event_Type_t    EventType;
   unsigned int         EventLength;
   union
   {
      ANCM_Connected_Event_Data_t                             ConnectedEventData;
      ANCM_Disconnected_Event_Data_t                          DisconnectedEventData;
      ANCM_Notification_Received_Event_Data_t                 NotificationReceivedEventData;
      ANCM_Get_Notification_Attributes_Response_Event_Data_t  GetNotificationAttributesResponseEventData;
      ANCM_Get_App_Attributes_Response_Event_Data_t           GetAppAttributesResponseEventData;
   } EventData;
} ANCM_Event_Data_t;

#define ANCM_EVENT_DATA_SIZE                             (sizeof(ANCM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Apple Notification Center Service (ANCS) Manager dispatches an    */
   /* event (and the client has registered for events).  This function  */
   /* passes to the caller the ANCS Manager Event and the Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the Event Data ONLY in  */
   /* the context of this callback.  If the caller requires the Data    */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* re-entrant).  Because of this, the processing in this function    */
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that    */
   /* the User does NOT own.  Therefore, processing in this function    */
   /* should be as efficient as possible (this argument holds anyway    */
   /* because another Message will not be processed while this function */
   /* call is outstanding).                                             */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will be */
   /*            issued while this function is currently outstanding.   */
typedef void (BTPSAPI *ANCM_Event_Callback_t)(ANCM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager ANCS Manager Module.  This         */
   /* function should be registered with the Bluetopia Platform Manager */
   /* Module Handler and will be called when the Platform Manager is    */
   /* initialized (or shut down).                                       */
void BTPSAPI ANCM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI ANCM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the ANCS             */
   /* Manager Service.  This Callback will be dispatched by the ANCS    */
   /* Manager when various ANCS Manager Events occur.  This function    */
   /* accepts the Callback Function and Callback Parameter              */
   /* (respectively) to call when an ANCS Manager Event needs to be     */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          ANCM_Un_Register_Consumer_Event_Callback() function to   */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI ANCM_Register_Consumer_Event_Callback(ANCM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Register_Consumer_Event_Callback_t)(ANCM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered ANCS Manager Event Callback   */
   /* (registered via a successful call to the                          */
   /* ANCM_Register_Consumer_Event_Callback() function).  This          */
   /* function accepts as input the ANCS Manager Event Callback ID      */
   /* (return value from ANCM_Register_Consumer_Event_Callback()        */
   /* function).                                                        */
BTPSAPI_DECLARATION void BTPSAPI ANCM_Un_Register_Consumer_Event_Callback(unsigned int ConsumerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANCM_Un_Register_Consumer_Event_Callback_t)(unsigned int ConsumerCallbackID);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get Notification Attributes request to a Notification Provider. */
   /* This function accepts as input the Callback ID (the return value  */
   /* from the ANCM_Register_Consumer_Event_Callback() function), the   */
   /* Bluetooth address of the Notification Provider, the Notification  */
   /* UID of the Notification for which Attributes are being requested, */
   /* the number of attributes that are being requested, and an array   */
   /* of ANCM_Notification_Attribute_Request_Data_t structures          */
   /* representing each attribute that is being requested.  This        */
   /* function returns a positive (non-zero) value if successful, or a  */
   /* negative error code if an error occurred.                         */
   /* * NOTE * Only one Get Notification Attributes or Get App          */
   /*          Attributes request can be pending at a time.  This       */
   /*          function will return an error if a request is still      */
   /*          pending.                                                 */
BTPSAPI_DECLARATION int BTPSAPI ANCM_Get_Notification_Attributes(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, DWord_t NotificationUID, unsigned int NumberOfAttributes, ANCM_Notification_Attribute_Request_Data_t *AttributeRequestData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Get_Notification_Attributes_t)(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, DWord_t NotificationUID, unsigned int NumberOfAttributes, ANCM_Notification_Attribute_Request_Data_t *AttributeRequestData);
#endif

   /* The following function is provided to allow a mechanism to submit */
   /* a Get App Attributes request to a Notification Provider. This     */
   /* function accepts as input the Callback ID (the return value from  */
   /* the ANCM_Register_Consumer_Event_Callback() function), the        */
   /* Bluetooth address of the Notification Provider, the App           */
   /* Identifier of the application for which Attributes are being      */
   /* requested, the number of attributes that are being requested, and */
   /* an array of App Attribute IDs for each attribute that is being    */
   /* requested.  This function returns a positive (non-zero) value if  */
   /* successful, or a negative error code if an error occurred.        */
   /* * NOTE * Only one Get Notification Attributes or Get App          */
   /*          Attributes request can be pending at a time.  This       */
   /*          function will return an error if a request is still      */
   /*          pending.                                                 */
BTPSAPI_DECLARATION int BTPSAPI ANCM_Get_App_Attributes(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, char *AppIdentifier, unsigned int NumberOfAttributes, ANCM_App_Attribute_Request_Data_t *AttributeRequestData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Get_App_Attributes_t)(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress, char *AppIdentifier, unsigned int NumberOfAttributes, ANCM_App_Attribute_Request_Data_t *AttributeRequestData);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Attribute Data Stream into Parsed Attribute     */
   /* Data in the form of ANCM_Parsed_Attribute_Data_t.  This function  */
   /* accepts as input the type of attribute data that is being parsed, */
   /* the length of the Raw stream (must be greater than zero),         */
   /* followed by a pointer to the actual Raw Attribute Data Stream.    */
   /* The final parameter is a pointer to a buffer that will contain    */
   /* the header information for the parsed data.  This function        */
   /* returns zero if successful or a negative value if an error        */
   /* occurred.                                                         */
   /* * NOTE * If this function is successful the final parameter *MUST**/
   /*          be passed to the ANCM_FreeParsedAttributeData() to free  */
   /*          any allocated resources that were allocated to track the */
   /*          Parsed Attribute Data Stream.                            */
   /* * NOTE * The Raw Attribute Stream Buffer (third parameter) *MUST* */
   /*          remain active while the data is being processed.         */
BTPSAPI_DECLARATION int BTPSAPI ANCM_Convert_Raw_Attribute_Data_To_Parsed_Attribute_Data(ANCM_Attribute_Data_Type_t DataType, unsigned int RawDataLength, Byte_t *RawAttributeData, ANCM_Parsed_Attribute_Data_t *ParsedAttributeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Convert_Raw_Attribute_Data_To_Parsed_Attribute_Data_t)(ANCM_Attribute_Data_Type_t DataType, unsigned int RawDataLength, Byte_t *RawAttributeData, ANCM_Parsed_Attribute_Data_t *ParsedAttributeData);
#endif

   /* The following function is provided to allow a mechanism to free   */
   /* all resources that were allocated to parse a Raw Attribute Data   */
   /* Stream into Parsed Attribute Data.  See the                       */
   /* ANCM_ConvertRawAttributeDataToParsedAttributeData() function for  */
   /* more information.                                                 */
BTPSAPI_DECLARATION void BTPSAPI ANCM_Free_Parsed_Attribute_Data(ANCM_Parsed_Attribute_Data_t *ParsedAttributeData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_ANCM_Free_Parsed_Attribute_Data_t)(ANCM_Parsed_Attribute_Data_t *ParsedAttributeData);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected ANCS    */
   /* devices.  This function accepts the buffer information to receive */
   /* any currently connected devices.  The first parameter specifies   */
   /* the maximum number of BD_ADDR entries that the buffer will        */
   /* support (i.e. can be copied into the buffer).  The next parameter */
   /* is optional and, if specified, will be populated with the         */
   /* Bluetooth addresses of any connected devices if the function is   */
   /* successful.  The final parameter can be used to retrieve the      */
   /* total number of connected devices (regardless of the size of the  */
   /* list specified by the first two parameters).  This function       */
   /* returns a non-negative value if successful which represents the   */
   /* number of connected devices that were copied into the specified   */
   /* input buffer.  This function returns a negative return error code */
   /* if there was an error.                                            */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Remote Device Entries, in which case the final parameter */
   /*          *MUST* be specified.                                     */
int BTPSAPI ANCM_Query_Connected_Devices(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Query_Connected_Devices_t)(unsigned int MaximumRemoteDeviceListEntries, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberConnectedDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current notifications available on the remote*/
   /* ANCS device.  This function accepts as input the Callback ID (the */
   /* return value from the ANCM_Register_Consumer_Event_Callback()     */
   /* function) and the Bluetooth address of the Notification           */
   /* Provider. This functions returns zero if successful and a negative*/
   /* return error code if there was an error.                          */
   /* * NOTE * The results of this function will be returned in an      */
   /*          etANCMNotificationReceived event for each current        */
   /*          notification.                                            */
BTPSAPI_DECLARATION int BTPSAPI ANCM_Query_Current_Notifications(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_ANCM_Query_Current_Notifications_t)(unsigned int ConsumerCallbackID, BD_ADDR_t RemoteDeviceAddress);
#endif

#endif
