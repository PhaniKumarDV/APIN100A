/*****< hogmapi.h >************************************************************/
/*      Copyright 2013 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HOGMAPI - Human Interface Device over GATT (HOGP) Manager API for         */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/16/13  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __HOGMAPIH__
#define __HOGMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "SS1BTPM.h"             /* Platform Manager Prototypes/Constants.    */

#include "HOGMMSG.h"             /* BTPM HOGP Manager Message Formats.        */

   /* The following structure is the structure that is used to pass     */
   /* initialization information when the HOG Module is initialized.    */
typedef struct _tagHOGM_Initialization_Data_t
{
   unsigned long SupportedFeaturesFlags;
} HOGM_Initialization_Data_t;

#define HOGM_INITIALIZATION_DATA_SIZE                          (sizeof(HOGM_Initialization_Data_t))

   /* The following define the supported features flags that may be set */
   /* in the SupportedFeaturesFlags member of the                       */
   /* HOGM_Initialization_Data_t structure.                             */
#define HOGM_SUPPORTED_FEATURES_FLAGS_REPORT_MODE              0x00000001

   /* The following enumerated type represents the HOG Manager Event    */
   /* Types that are dispatched by this module to inform other modules  */
   /* of HOG Manager Changes.                                           */
typedef enum
{
   hetHOGMHIDDeviceConnected,
   hetHOGMHIDDeviceDisconnected,
   hetHOGMHIDGetReportResponse,
   hetHOGMHIDSetReportResponse,
   hetHOGMHIDDataIndication
} HOGM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHOGMHIDDeviceConnected event.*/
typedef struct _tagHOGM_HID_Device_Connected_Event_Data_t
{
   BD_ADDR_t               RemoteDeviceAddress;
   unsigned long           SupportedFeatures;
   HOGM_HID_Information_t  HIDInformation;
   unsigned int            ReportDescriptorLength;
   Byte_t                 *ReportDescriptor;
} HOGM_HID_Device_Connected_Event_Data_t;

#define HOGM_HID_DEVICE_CONNECTED_EVENT_DATA_SIZE              (sizeof(HOGM_HID_Device_Connected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHOGMHIDDeviceDisconnected    */
   /* event.                                                            */
typedef struct _tagHOGM_HID_Device_Disconnected_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} HOGM_HID_Device_Disconnected_Event_Data_t;

#define HOGM_HID_DEVICE_DISCONNECTED_EVENT_DATA_SIZE           (sizeof(HOGM_HID_Device_Disconnected_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* hetHOGMHIDSetProtocolModeResponse event.                          */
typedef struct _tagHOGM_HID_Set_Protocol_Mode_Response_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   unsigned int TransactionID;
   Boolean_t    Success;
   Byte_t       AttributeErrorCode;
} HOGM_HID_Set_Protocol_Mode_Response_Event_Data_t;

#define HOGM_HID_SET_PROTOCOL_MODE_RESPONSE_EVENT_DATA_SIZE    (sizeof(HOGM_HID_Set_Protocol_Mode_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHOGMHIDGetReportResponse     */
   /* event.                                                            */
   /* * NOTE * The ReportID will automatically be appended to the Report*/
   /*          Data if the ReportID is present for the report, and      */
   /*          therefore no action is required on the part of the       */
   /*          receiver of this event before passing the data to a HID  */
   /*          Class driver.                                            */
typedef struct _tagHOGM_HID_Get_Report_Response_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   unsigned int                   TransactionID;
   Boolean_t                      Success;
   Byte_t                         AttributeErrorCode;
   HOGM_HID_Report_Information_t  ReportInformation;
   unsigned int                   ReportDataLength;
   Byte_t                        *ReportData;
} HOGM_HID_Get_Report_Response_Event_Data_t;

#define HOGM_HID_GET_REPORT_RESPONSE_EVENT_DATA_SIZE           (sizeof(HOGM_HID_Get_Report_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHOGMHIDSetReportResponse     */
   /* event.                                                            */
typedef struct _tagHOGM_HID_Set_Report_Response_Event_Data_t
{
   BD_ADDR_t                     RemoteDeviceAddress;
   unsigned int                  TransactionID;
   Boolean_t                     Success;
   Byte_t                        AttributeErrorCode;
   HOGM_HID_Report_Information_t ReportInformation;
} HOGM_HID_Set_Report_Response_Event_Data_t;

#define HOGM_HID_SET_REPORT_RESPONSE_EVENT_DATA_SIZE           (sizeof(HOGM_HID_Set_Report_Response_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a hetHOGMHIDDataIndication event. */
   /* * NOTE * The ReportID will automatically be appended to the Report*/
   /*          Data if the ReportID is present for the report, and      */
   /*          therefore no action is required on the part of the       */
   /*          receiver of this event before passing the data to a HID  */
   /*          Class driver.                                            */
typedef struct _tagHOGM_HID_Data_Indication_Event_Data_t
{
   BD_ADDR_t                      RemoteDeviceAddress;
   HOGM_HID_Report_Information_t  ReportInformation;
   unsigned int                   ReportDataLength;
   Byte_t                        *ReportData;
} HOGM_HID_Data_Indication_Event_Data_t;

#define HOGM_HID_DATA_INDICATION_EVENT_DATA_SIZE               (sizeof(HOGM_HID_Data_Indication_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Human Interface Device over GATT (HOGP) Manager Event (and Event  */
   /* Data) of a HOG Manager Event.                                     */
typedef struct _tagHOGM_Event_Data_t
{
   HOGM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      HOGM_HID_Device_Connected_Event_Data_t           DeviceConnectedEventData;
      HOGM_HID_Device_Disconnected_Event_Data_t        DeviceDisconnectedEventData;
      HOGM_HID_Set_Protocol_Mode_Response_Event_Data_t SetProtocolModeResponseEventData;
      HOGM_HID_Get_Report_Response_Event_Data_t        GetReportResponseEventData;
      HOGM_HID_Set_Report_Response_Event_Data_t        SetReportResponseEventData;
      HOGM_HID_Data_Indication_Event_Data_t            DataIndicationEventData;
   } EventData;
} HOGM_Event_Data_t;

#define HOGM_EVENT_DATA_SIZE                                   (sizeof(HOGM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* Human Interface Device (HOG) Manager dispatches an event (and the */
   /* client has registered for events).  This function passes to the   */
   /* caller the HOG Manager Event and the Callback Parameter that was  */
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
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *HOGM_Event_Callback_t)(HOGM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following function is responsible for initializing/cleaning up*/
   /* the Bluetopia Platform Manager HOG Manager Module.  This function */
   /* should be registered with the Bluetopia Platform Manager Module   */
   /* Handler and will be called when the Platform Manager is           */
   /* initialized (or shut down).                                       */
void BTPSAPI HOGM_InitializationHandlerFunction(Boolean_t Initialize, void *InitializationData);

   /* The following function is responsible for processing asynchronous */
   /* Device Manager (DEVM) Events (including Power On/Off events).     */
   /* This function should be registered with the Bluetopia Platform    */
   /* Manager Module Handler and will be called when an asynchronous    */
   /* Device Manager event is dispatched.                               */
void BTPSAPI HOGM_DeviceManagerHandlerFunction(DEVM_Event_Data_t *EventData);

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service.  This Callback will be dispatched by*/
   /* the HOG Manager when various HOG Manager Events occur.  This      */
   /* function accepts the Callback Function and Callback Parameter     */
   /* (respectively) to call when a HOG Manager Event needs to be       */
   /* dispatched.  This function returns a positive (non-zero) value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          HOGM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Register_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Register_Event_Callback_t)(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_RegisterEventCallback() function).  This function accepts as */
   /* input the HOG Manager Event Callback ID (return value from        */
   /* HOGM_RegisterEventCallback() function).                           */
BTPSAPI_DECLARATION void BTPSAPI HOGM_Un_Register_Event_Callback(unsigned int HOGManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HOGM_Un_Register_Event_Callback_t)(unsigned int HOGManagerCallbackID);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Human Interface  */
   /* Device (HOG) Manager Service to explicitly process HOG report     */
   /* data.  This Callback will be dispatched by the HOG Manager when   */
   /* various HOG Manager Events occur.  This function accepts the      */
   /* Callback Function and Callback Parameter (respectively) to call   */
   /* when a HOG Manager Event needs to be dispatched.  This function   */
   /* returns a positive (non-zero) value if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * The return value from this function (if successful) is   */
   /*          the value that is must be passed to the                  */
   /*          HOGM_Send_Report_Data() function to send report data.    */
   /* * NOTE * There can only be a single Report Data event handler     */
   /*          registered.                                              */
   /* * NOTE * If this function returns success (greater than zero) then*/
   /*          this value can be passed to the                          */
   /*          HOGM_Un_Register_Data_Event_Callback() function to       */
   /*          un-register the callback from this module.               */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Register_Data_Event_Callback(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Register_Data_Event_Callback_t)(HOGM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HOG Manager Event Callback    */
   /* (registered via a successful call to the                          */
   /* HOGM_Register_Data_Event_Callback() function).  This function     */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function).         */
BTPSAPI_DECLARATION void BTPSAPI HOGM_Un_Register_Data_Event_Callback(unsigned int HOGManagerDataCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_HOGM_Un_Register_Data_Event_Callback_t)(unsigned int HOGManagerDataCallbackID);
#endif

   /* The following function is provided to allow a mechanism of setting*/
   /* the HID Protocol Mode on a remote HID Device.  This function      */
   /* accepts as input the HOG Manager Data Event Callback ID (return   */
   /* value from HOGM_Register_Data_Event_Callback() function), the     */
   /* BD_ADDR of the remote HID Device and the Protocol Mode to set.    */
   /* This function returns zero on success or a negative error code.   */
   /* * NOTE * On each connection to a HID Device the Protocol Mode     */
   /*          defaults to Report Mode.                                 */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Set_Protocol_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Set_Protocol_Mode_t)(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Protocol_Mode_t ProtocolMode);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* informing the specified remote HID Device that the local HID Host */
   /* is entering/exiting the Suspend State.  This function accepts as  */
   /* input the HOG Manager Data Event Callback ID (return value from   */
   /* HOGM_Register_Data_Event_Callback() function), the BD_ADDR of the */
   /* remote HID Device and the a Boolean that indicates if the Host is */
   /* entering suspend state (TRUE) or exiting suspend state (FALSE).   */
   /* This function returns zero on success or a negative error code.   */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Set_Suspend_Mode(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Set_Suspend_Mode_t)(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, Boolean_t Suspend);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Get Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device and a pointer to a*/
   /* structure containing information on the Report to set.  This      */
   /* function returns the positive, non-zero, Transaction ID of the    */
   /* request on success or a negative error code.                      */
   /* * NOTE * The hetHOGMHIDGetReportResponse event will be generated  */
   /*          when the remote HID Device responds to the Get Report    */
   /*          Request.                                                 */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Get_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Get_Report_Request_t)(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* performing a HID Set Report procedure to a remote HID Device.     */
   /* This function accepts as input the HOG Manager Data Event Callback*/
   /* ID (return value from HOGM_Register_Data_Event_Callback()         */
   /* function), the BD_ADDR of the remote HID Device, a pointer to a   */
   /* structure containing information on the Report to set, a Boolean  */
   /* that indicates if a response is expected, and the Report Data to  */
   /* set.  This function returns the positive, non-zero, Transaction ID*/
   /* of the request (if a Response is expected, ZERO if no response is */
   /* expected) on success or a negative error code.                    */
   /* * NOTE * If a response is expected the hetHOGMHIDSetReportResponse*/
   /*          event will be generated when the remote HID Device       */
   /*          responds to the Set Report Request.                      */
   /* * NOTE * The ResponseExpected parameter can be set to FALSE to    */
   /*          indicate that no response is expected.  This can only be */
   /*          set to FALSE if the Report to set is an Output Report and*/
   /*          corresponds to a HID Data Output procedure.              */
   /* * NOTE * The ReportID, if valid for the specified Report, MUST be */
   /*          prefixed to the data that is passed to this function.    */
BTPSAPI_DECLARATION int BTPSAPI HOGM_Set_Report_Request(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_HOGM_Set_Report_Request_t)(unsigned int HOGManagerDataCallbackID, BD_ADDR_t RemoteDeviceAddress, HOGM_HID_Report_Information_t *ReportInformation, Boolean_t ResponseExpected, unsigned int ReportDataLength, Byte_t *ReportData);
#endif

#endif
