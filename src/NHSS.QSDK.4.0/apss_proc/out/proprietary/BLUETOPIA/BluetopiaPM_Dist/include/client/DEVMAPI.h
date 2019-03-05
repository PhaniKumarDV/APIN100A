/*****< devmapi.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DEVMAPI - Local Device Manager API for Stonestreet One Bluetooth Protocol */
/*            Stack Platform Manager.                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/26/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __DEVMAPIH__
#define __DEVMAPIH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "DEVMMSG.h"             /* BTPM Device Manager Message Formats.      */

   /* The following structure is used to hold parsed information about: */
   /*    - Extended Inquiry Response (EIR) Data                         */
   /*    - Advertising Data                                             */
   /*    - Scan Response Data                                           */
   /* In each of the above cases, the data simply consists of tuples    */
   /* that are made up of the three elements listed below.  This        */
   /* structure is used with the DEVM_ConvertRawEIRDataToParsedEIRData()*/
   /* and DEVM_ConvertRawAdvertisingDataToParsedAdvertisingData()       */
   /* functions to convert the respective raw data into a more easily   */
   /* traversed/parsable format.                                        */
typedef struct _tagDEVM_Tag_Length_Value_t
{
   Byte_t  DataType;
   Byte_t  DataLength;
   Byte_t *DataBuffer;
} DEVM_Tag_Length_Value_t;

#define DEVM_TAG_LENGTH_VALUE_SIZE                             (sizeof(DEVM_Tag_Length_Value_t))

   /* The following structure is used with the                          */
   /* DEVM_ConvertRawSDPStreamToParsedSDPData() and                     */
   /* DEVM_ConvertParsedSDPDataToRawSDPStream() functions to convert    */
   /* Parsed SDP Information to a raw SDP Stream (and vice-versa).      */
typedef struct _tagDEVM_Parsed_SDP_Data_t
{
   unsigned int                           NumberServiceRecords;
   SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponseData;
   void                                  *RESERVED;
} DEVM_Parsed_SDP_Data_t;

#define DEVM_PARSED_SDP_DATA_SIZE                              (sizeof(DEVM_Parsed_SDP_Data_t))

   /* The following structure is used with the                          */
   /* DEVM_ConvertRawServicesStreamToParsedServicesData() and           */
   /* DEVM_ConvertParsedServicesDataToRawServicesStream() functions to  */
   /* convert Parsed Bluetopia PM Services Information to a raw Services*/
   /* Stream (and vice-versa).                                          */
typedef struct _tagDEVM_Parsed_Services_Data_t
{
   unsigned int                              NumberServices;
   GATT_Service_Discovery_Indication_Data_t *GATTServiceDiscoveryIndicationData;
   void                                     *RESERVED;
} DEVM_Parsed_Services_Data_t;

#define DEVM_PARSED_SERVICES_DATA_SIZE                         (sizeof(DEVM_Parsed_Services_Data_t))

   /* The following MACRO is a utility MACRO that assigns the special   */
   /* Class of Device that is used to signify a Low Energy device.  This*/
   /* Class of Device is used with the DEVM_AddRemoteDevice() function. */
   /* This MACRO accepts one parameter which is the Class_of_Device_t   */
   /* variable that is receive the Low Energy Class of Device Constant  */
   /* value.                                                            */
#define DEVM_ASSIGN_ADD_REMOTE_DEVICE_LOW_ENERGY_CLASS_OF_DEVICE(_x) \
   ASSIGN_CLASS_OF_DEVICE((_x), 0xFF, 0xFF, 0xFF)

   /* The following MACRO is a utility MACRO that assigns the special   */
   /* Class of Device that is used to signify a Dual Mode device.  This */
   /* Class of Device is used with the DEVM_AddRemoteDevice() function. */
   /* This MACRO accepts one parameter which is the Class_of_Device_t   */
   /* variable that is receive the Dual Mode Class of Device Constant   */
   /* value.                                                            */
#define DEVM_ASSIGN_ADD_REMOTE_DEVICE_DUAL_MODE_CLASS_OF_DEVICE(_x) \
   ASSIGN_CLASS_OF_DEVICE((_x), 0xFF, 0xFF, 0xFE)

   /* The following enumerated type represents the Device Manager Event */
   /* Types that are dispatched by this module to inform other modules  */
   /* of Bluetooth Device Changes.                                      */
typedef enum
{
   detDevicePoweredOn,
   detDevicePoweringOff,
   detDevicePoweredOff,
   detLocalDevicePropertiesChanged,
   detDeviceDiscoveryStarted,
   detDeviceDiscoveryStopped,
   detRemoteDeviceFound,
   detRemoteDeviceDeleted,
   detRemoteDevicePropertiesChanged,
   detRemoteDevicePropertiesStatus,
   detRemoteDeviceServicesStatus,
   detRemoteDevicePairingStatus,
   detRemoteDeviceAuthenticationStatus,
   detRemoteDeviceEncryptionStatus,
   detRemoteDeviceConnectionStatus,
   detDeviceScanStarted,
   detDeviceScanStopped,
   detRemoteDeviceAddressChanged,
   detDeviceAdvertisingStarted,
   detDeviceAdvertisingStopped,
   detAdvertisingTimeout,
   detDeviceObservationScanStarted,
   detDeviceObservationScanStopped,
   detInterleavedAdvertisementComplete,
   detInterleavedAdvertisingSuspended,
   detInterleavedAdvertisingResumed
} DEVM_Event_Type_t;

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detDevicePoweringOff event.     */
typedef struct _tagDEVM_Device_Powering_Off_Event_Data_t
{
   unsigned int PoweringOffTimeout;
} DEVM_Device_Powering_Off_Event_Data_t;

#define DEVM_DEVICE_POWERING_OFF_EVENT_DATA_SIZE               (sizeof(DEVM_Device_Powering_Off_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detLocalDevicePropertiesChanged */
   /* event.                                                            */
typedef struct _tagDEVM_Local_Device_Properties_Changed_Event_Data_t
{
   unsigned long                  ChangedMemberMask;
   DEVM_Local_Device_Properties_t LocalDeviceProperties;
} DEVM_Local_Device_Properties_Changed_Event_Data_t;

#define DEVM_LOCAL_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE   (sizeof(DEVM_Local_Device_Properties_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceFound event.     */
typedef struct _tagDEVM_Remote_Device_Found_Event_Data_t
{
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Found_Event_Data_t;

#define DEVM_REMOTE_DEVICE_FOUND_EVENT_DATA_SIZE               (sizeof(DEVM_Remote_Device_Found_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceDeleted event.   */
typedef struct _tagDEVM_Remote_Device_Deleted_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
} DEVM_Remote_Device_Deleted_Event_Data_t;

#define DEVM_REMOTE_DEVICE_DELETED_EVENT_DATA_SIZE             (sizeof(DEVM_Remote_Device_Deleted_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDevicePropertiesChanged*/
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Properties_Changed_Event_Data_t
{
   unsigned long                   ChangedMemberMask;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Properties_Changed_Event_Data_t;

#define DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_EVENT_DATA_SIZE  (sizeof(DEVM_Remote_Device_Properties_Changed_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDevicePropertiesStatus */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Properties_Status_Event_Data_t
{
   Boolean_t                       Success;
   DEVM_Remote_Device_Properties_t RemoteDeviceProperties;
} DEVM_Remote_Device_Properties_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_PROPERTIES_STATUS_EVENT_DATA_SIZE   (sizeof(DEVM_Remote_Device_Properties_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceServicesStatus   */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Services_Status_Event_Data_t
{
   BD_ADDR_t     RemoteDeviceAddress;
   unsigned long StatusFlags;
} DEVM_Remote_Device_Services_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_SERVICES_STATUS_EVENT_DATA_SIZE     (sizeof(DEVM_Remote_Device_Services_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDevicePairingStatus    */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Pairing_Status_Event_Data_t
{
   BD_ADDR_t    RemoteDeviceAddress;
   Boolean_t    Success;
   unsigned int AuthenticationStatus;
} DEVM_Remote_Device_Pairing_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_PAIRING_STATUS_EVENT_DATA_SIZE      (sizeof(DEVM_Remote_Device_Pairing_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* detRemoteDeviceAuthenticationStatus event.                        */
typedef struct _tagDEVM_Remote_Device_Authentication_Status_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   int       Status;
} DEVM_Remote_Device_Authentication_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_AUTHENTICATION_STATUS_EVENT_DATA_SIZE  (sizeof(DEVM_Remote_Device_Authentication_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceEncryptionStatus */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Encryption_Status_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   int       Status;
} DEVM_Remote_Device_Encryption_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_ENCRYPTION_STATUS_EVENT_DATA_SIZE   (sizeof(DEVM_Remote_Device_Encryption_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceConnectionStatus */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Connection_Status_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   int       Status;
} DEVM_Remote_Device_Connection_Status_Event_Data_t;

#define DEVM_REMOTE_DEVICE_CONNECTION_STATUS_EVENT_DATA_SIZE   (sizeof(DEVM_Remote_Device_Connection_Status_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detRemoteDeviceAddressChanged   */
   /* event.                                                            */
typedef struct _tagDEVM_Remote_Device_Address_Change_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   BD_ADDR_t PreviousRemoteDeviceAddress;
} DEVM_Remote_Device_Address_Change_Event_Data_t;

#define DEVM_REMOTE_DEVICE_ADDRESS_CHANGE_EVENT_DATA_SIZE      (sizeof(DEVM_Remote_Device_Address_Change_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detDeviceObservationScanStarted */
   /* event.                                                            */
typedef struct _tagDEVM_Device_Observation_Scan_Started_Event_Data_t
{
   unsigned long ObservationScanFlags;
} DEVM_Device_Observation_Scan_Started_Event_Data_t;

#define DEVM_DEVICE_OBSERVATION_SCAN_STARTED_EVENT_DATA_SIZE   (sizeof(DEVM_Device_Observation_Scan_Started_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a detDeviceObservationScanStopped */
   /* event.                                                            */
typedef struct _tagDEVM_Device_Observation_Scan_Stopped_Event_Data_t
{
   unsigned long ObservationScanFlags;
} DEVM_Device_Observation_Scan_Stopped_Event_Data_t;

#define DEVM_DEVICE_OBSERVATION_SCAN_STOPPED_EVENT_DATA_SIZE   (sizeof(DEVM_Device_Observation_Scan_Stopped_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* information that is returned in a                                 */
   /* detInterleavedAdvertisementComplete event.  This event is         */
   /* dispatched only to the callback that successfully scheduled the   */
   /* interleaved advertisement via the                                 */
   /* DEVM_ScheduleInterleavedAdvertisement() API.  The Status member is*/
   /* a PM error code that will be zero if the interleaved advertisement*/
   /* was successfully scheduled or a negative error code otherwise.    */
typedef struct _tagDEVM_Interleaved_Advertisement_Complete_Event_Data_t
{
   unsigned int DeviceManagerCallbackID;
   unsigned int InterleavedAdvertisementID;
   int          Status;
} DEVM_Interleaved_Advertisement_Complete_Event_Data_t;

#define DEVM_INTERLEAVED_ADVERTISEMENT_COMPLETE_EVENT_DATA_SIZE   (sizeof(DEVM_Interleaved_Advertisement_Complete_Event_Data_t))

   /* The following structure is a container structure that holds the   */
   /* Local Device Manager Event (and Event Data) of a Local Device     */
   /* Manager Event.                                                    */
typedef struct _tagDEVM_Event_Data_t
{
   DEVM_Event_Type_t EventType;
   unsigned int      EventLength;
   union
   {
      DEVM_Device_Powering_Off_Event_Data_t                 DevicePoweringOffEventData;
      DEVM_Local_Device_Properties_Changed_Event_Data_t     LocalDevicePropertiesChangedEventData;
      DEVM_Remote_Device_Found_Event_Data_t                 RemoteDeviceFoundEventData;
      DEVM_Remote_Device_Deleted_Event_Data_t               RemoteDeviceDeletedEventData;
      DEVM_Remote_Device_Properties_Changed_Event_Data_t    RemoteDevicePropertiesChangedEventData;
      DEVM_Remote_Device_Properties_Status_Event_Data_t     RemoteDevicePropertiesStatusEventData;
      DEVM_Remote_Device_Services_Status_Event_Data_t       RemoteDeviceServicesStatusEventData;
      DEVM_Remote_Device_Pairing_Status_Event_Data_t        RemoteDevicePairingStatusEventData;
      DEVM_Remote_Device_Authentication_Status_Event_Data_t RemoteDeviceAuthenticationStatusEventData;
      DEVM_Remote_Device_Encryption_Status_Event_Data_t     RemoteDeviceEncryptionStatusEventData;
      DEVM_Remote_Device_Connection_Status_Event_Data_t     RemoteDeviceConnectionStatusEventData;
      DEVM_Remote_Device_Address_Change_Event_Data_t        RemoteDeviceAddressChangeEventData;
      DEVM_Device_Observation_Scan_Started_Event_Data_t     ObservationScanStartedEventData;
      DEVM_Device_Observation_Scan_Stopped_Event_Data_t     ObservationScanStoppedEventData;
      DEVM_Interleaved_Advertisement_Complete_Event_Data_t  InterleavedAdvertisementCompleteEventData;
   } EventData;
} DEVM_Event_Data_t;

#define DEVM_EVENT_DATA_SIZE                                   (sizeof(DEVM_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* an Event Callback.  This function will be called whenever the     */
   /* local Device Manager dispatches an event (and the client has      */
   /* registered for events).  This function passes to the caller the   */
   /* Device Manager Event and the Callback Parameter that was specified*/
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e. this    */
   /* function DOES NOT have be reentrant).  Because of this, the       */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another Message will not be   */
   /* processed while this function call is outstanding).               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *DEVM_Event_Callback_t)(DEVM_Event_Data_t *EventData, void *CallbackParameter);

   /* The following declared type represents the Prototype Function for */
   /* an Authentication Callback.  This function will be called whenever*/
   /* the local Device Manager needs to have an Authentication Request  */
   /* action serviced (and the client has registered an Authentication  */
   /* handler via the DEVM_RegisterAuthentication() function).  This    */
   /* function passes to the caller the Authentication Request          */
   /* Information and the Callback Parameter that was specified when    */
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
   /* ** NOTE ** There can only be a SINGLE Authentication Callback     */
   /*            Handler installed in the entire system at any given    */
   /*            instant.                                               */
   /* ** NOTE ** This function MUST NOT block and wait for events that  */
   /*            can only be satisfied by Receiving other Events.  A    */
   /*            deadlock WILL occur because NO Event Callbacks will    */
   /*            be issued while this function is currently outstanding.*/
typedef void (BTPSAPI *DEVM_Authentication_Callback_t)(DEVM_Authentication_Information_t *AuthenticationRequestInformation, void *CallbackParameter);

   /* The following function is a utility function that exists to       */
   /* retrieve the human readable string value for the specified Device */
   /* Manufacturer (first parameter).  This function will ALWAYS return */
   /* a NON-NULL value that is a pointer to a static string.  The memory*/
   /* that this string points to owned by the error handler module and  */
   /* cannot be changed (i.e. the pointer must be treated as pointing   */
   /* to constant data.                                                 */
BTPSAPI_DECLARATION char *BTPSAPI DEVM_ConvertManufacturerNameToString(unsigned int DeviceManufacturer);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef char *(BTPSAPI *PFN_DEVM_ConvertManufacturerNameToString_t)(unsigned int DeviceManufacturer);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Extended Inquiry Response (EIR) data into a more*/
   /* easily accessable/programmer friendly format.  This format can    */
   /* easily be traversed using array logic and frees the caller of     */
   /* having to worry about invalid formatted data.  This function      */
   /* accepts as input the Raw EIR data to parse (first parameter),     */
   /* followed by the maximum number of list entries that are pointed to*/
   /* by the buffer that is passed as the final parameter.  This        */
   /* function returns the number of entries that were parsed and found */
   /* to be contained in the Raw EIR data (if successful).  This        */
   /* function returns a negative return error code if there was an     */
   /* error parsing the data.                                           */
   /* * NOTE * If zero and NULL are passed as the final two parameters  */
   /*          (respectively), then this function will determine the    */
   /*          total number of parsed entries that are contained in the */
   /*          EIR data.  This number will be returned to the caller    */
   /*          via a successful return from this function.              */
   /* * NOTE * This function, if parsing the data (i.e. the final two   */
   /*          parameters are specified as non zero and NOT NULL,       */
   /*          respectively), will copy up to the MaximumNumberEntries  */
   /*          into the specified buffer.  If there are more entries    */
   /*          contained in the data, they will be ignored, and only    */
   /*          the first MaximumNumberEntries will be parsed (and the   */
   /*          return value will be the same value as the               */
   /*          MaximumNumberEntries parameter).                         */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertRawEIRDataToParsedEIRData(Extended_Inquiry_Response_Data_t *EIRData, unsigned int MaximumNumberEntries, DEVM_Tag_Length_Value_t *EntryList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertRawEIRDataToParsedEIRData_t)(Extended_Inquiry_Response_Data_t *EIRData, unsigned int MaximumNumberEntries, DEVM_Tag_Length_Value_t *EntryList);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Advertising data (either Advertising data or    */
   /* Scan Response data as they both have the same format) into a more */
   /* easily accessable/programmer friendly format.  This format can    */
   /* easily be traversed using array logic and frees the caller of     */
   /* having to worry about invalid formatted data.  This function      */
   /* accepts as input the Raw Advertising data to parse (first         */
   /* parameter), followed by the maximum number of list entries that   */
   /* are pointed to by the buffer that is passed as the final          */
   /* parameter.  This function returns the number of entries that were */
   /* parsed and found to be contained in the Raw Advertising data (if  */
   /* successful).  This function returns a negative return error code  */
   /* if there was an error parsing the data.                           */
   /* * NOTE * If zero and NULL are passed as the final two parameters  */
   /*          (respectively), then this function will determine the    */
   /*          total number of parsed entries that are contained in the */
   /*          Advertising data.  This number will be returned to the   */
   /*          caller via a successful return from this function.       */
   /* * NOTE * This function, if parsing the data (i.e. the final two   */
   /*          parameters are specified as non zero and NOT NULL,       */
   /*          respectively), will copy up to the MaximumNumberEntries  */
   /*          into the specified buffer.  If there are more entries    */
   /*          contained in the data, they will be ignored, and only    */
   /*          the first MaximumNumberEntries will be parsed (and the   */
   /*          return value will be the same value as the               */
   /*          MaximumNumberEntries parameter).                         */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertRawAdvertisingDataToParsedAdvertisingData(Advertising_Data_t *AdvertisingData, unsigned int MaximumNumberEntries, DEVM_Tag_Length_Value_t *EntryList);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertRawAdvertisingDataToParsedAdvertisingData_t)(Advertising_Data_t *AdvertisingData, unsigned int MaximumNumberEntries, DEVM_Tag_Length_Value_t *EntryList);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw SDP Data Stream into the Bluetopia SDP API      */
   /* (Parsed) format.  This function accepts as input the length of the*/
   /* Raw stream (must be greater than zero), followed by a pointer to  */
   /* the actual Raw SDP Stream.  The final parameter is a pointer to a */
   /* buffer that will contain the header information for the parsed    */
   /* data.  This function returns zero if successful or a negative     */
   /* value if an error occurred.                                       */
   /* * NOTE * If this function is successful the final parameter *MUST**/
   /*          be passed to the DEVM_FreeParsedSDPData() to free any    */
   /*          allocated resources that were allocated to track the     */
   /*          Parsed SDP Stream.                                       */
   /* * NOTE * The Raw SDP Stream Buffer (second parameter) *MUST*      */
   /*          remain active while the data is processed as well as even*/
   /*          during the call to the DEVM_FreeParsedSDPData() function.*/
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertRawSDPStreamToParsedSDPData(unsigned int RawSDPDataLength, unsigned char *RawSDPData, DEVM_Parsed_SDP_Data_t *ParsedSDPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertRawSDPStreamToParsedSDPData_t)(unsigned int RawSDPDataLength, unsigned char *RawSDPData, DEVM_Parsed_SDP_Data_t *ParsedSDPData);
#endif

   /* The following function is provided to allow a mechanism to free   */
   /* all resources that were allocated to parse a Raw SDP Stream into  */
   /* Bluetopia Parsed SDP Data.  See the                               */
   /* DEVM_ConvertRawSDPStreamToParsedSDPData() function for more       */
   /* information.                                                      */
BTPSAPI_DECLARATION void BTPSAPI DEVM_FreeParsedSDPData(DEVM_Parsed_SDP_Data_t *ParsedSDPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_FreeParsedSDPData_t)(DEVM_Parsed_SDP_Data_t *ParsedSDPData);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Tag-Length-Value structure into the raw format that */
   /* is expected by some APIs.  This function accepts as the Number of */
   /* TLV entries and a pointer to the TLV array (first and second      */
   /* parameters respectively), followed by the maximum number of bytes */
   /* that are pointed to by the buffer that is passed as the final     */
   /* parameter.  This function returns the number of bytes that were   */
   /* formatted into the raw data.  This function returns a negative    */
   /* return error code if there was an error parsing the data.         */
   /* * NOTE * If zero and NULL are passed as the final two parameters  */
   /*          (respectively), then this function will determine the    */
   /*          total number of bytes that are contained in the TLV data.*/
   /*          This number will be returned to the caller via a         */
   /*          successful return from this function.                    */
   /* * NOTE * This function, if parsing the data (i.e.  the final two  */
   /*          parameters are specified as non zero and NOT NULL,       */
   /*          respectively), will copy up to the RawDataLength into the*/
   /*          specified buffer.  If there are more entries contained in*/
   /*          the data, they will be ignored, and only the first       */
   /*          RawDataLength will be parsed (and the return value will  */
   /*          be the same value as the RawDataLength parameter).       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertParsedTLVDataToRaw(unsigned int NumberOfEntries, DEVM_Tag_Length_Value_t *EntryList, unsigned int RawDataLength, Byte_t *RawData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertParsedTLVDataToRaw_t)(unsigned int NumberOfEntries, DEVM_Tag_Length_Value_t *EntryList, unsigned int RawDataLength, Byte_t *RawData);
#endif

   /* The following function is a utility function that exists to       */
   /* convert the specified parsed SDP Data Information to the internal */
   /* Raw SDP Stream format that is used to store the representation.   */
   /* This function accepts as input, the Parsed SDP Data to convert    */
   /* (first parameter), followed by an optional buffer to build the    */
   /* stream into.  If the buffer is not specified, then this function  */
   /* can be used to determine the size (in bytes) that will be required*/
   /* to hold the converted stream.  If a buffer is specified (last two */
   /* parameters are not zero and NULL (respectively) then the buffer is*/
   /* required to be large enough to hold the data.  This function      */
   /* returns a positive value if the data was able to be converted (and*/
   /* possibly written to a given buffer), or a negative error code if  */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertParsedSDPDataToRawSDPStream(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int RawSDPDataLength, unsigned char *RawSDPData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertParsedSDPDataToRawSDPStream_t)(DEVM_Parsed_SDP_Data_t *ParsedSDPData, unsigned int RawSDPDataLength, unsigned char *RawSDPData);
#endif

   /* The following function is a utility function that exists to parse */
   /* the specified Raw Bluetooth Low Energy (BLE) Data Stream into the */
   /* Bluetopia PM API (Parsed) format.  This function accepts as input */
   /* the length of the Raw stream (must be greater than zero), followed*/
   /* by a pointer to the actual Raw Services Stream.  The final        */
   /* parameter is a pointer to a buffer that will contain the header   */
   /* information for the parsed data.  This function returns zero if   */
   /* successful or a negative value if an error occurred.              */
   /* * NOTE * If this function is successful the final parameter *MUST**/
   /*          be passed to the DEVM_FreeParsedServicesData() to free   */
   /*          any allocated resources that were allocated to track the */
   /*          Parsed Services Stream.                                  */
   /* * NOTE * The Raw Services Stream Buffer (second parameter) *MUST* */
   /*          remain active while the data is processed as well as even*/
   /*          during the call to the DEVM_FreeParsedServicesData()     */
   /*          function.                                                */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertRawServicesStreamToParsedServicesData(unsigned int RawServicesDataLength, unsigned char *RawServicesData, DEVM_Parsed_Services_Data_t *ParsedServicesData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertRawServicesStreamToParsedServicesData_t)(unsigned int RawServicesDataLength, unsigned char *RawServicesData, DEVM_Parsed_Services_Data_t *ParsedServicesData);
#endif

   /* The following function is provided to allow a mechanism to free   */
   /* all resources that were allocated to parse a Raw Bluetooth Low    */
   /* Energy (BLE) Services stream into Bluetopia PM Parsed Services    */
   /* Data.  See the DEVM_ConvertRawServicesStreamToParsedServicesData()*/
   /* function for more information.                                    */
BTPSAPI_DECLARATION void BTPSAPI DEVM_FreeParsedServicesData(DEVM_Parsed_Services_Data_t *ParsedServicesData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_FreeParsedServicesData_t)(DEVM_Parsed_Services_Data_t *ParsedServicesData);
#endif

   /* The following function is a utility function that exists to       */
   /* convert the specified parsed Service Data Information to the      */
   /* internal Raw Service Stream format that is used to store the      */
   /* representation.  This function accepts as input, the Parsed       */
   /* Service Data to convert (first parameter), followed by an optional*/
   /* buffer to build the stream into.  If the buffer is not specified, */
   /* then this function can be used to determine the size (in bytes)   */
   /* that will be required to hold the converted stream.  If a buffer  */
   /* is specified (last two parameters are not zero and NULL           */
   /* (respectively) then the buffer is required to be large enough to  */
   /* hold the data.  This function returns a positive value if the data*/
   /* was able to be converted (and possibly written to a given buffer),*/
   /* or a negative error code if there was an error.                   */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConvertParsedServicesDataToRawServicesStream(DEVM_Parsed_Services_Data_t *ParsedServiceData, unsigned int RawServiceDataLength, unsigned char *RawServiceData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConvertParsedServicesDataToRawServicesStream_t)(DEVM_Parsed_Services_Data_t *ParsedServiceData, unsigned int RawServiceDataLength, unsigned char *RawServiceData);
#endif

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the Local Device     */
   /* Manager Service.  This Callback will be dispatched by the Local   */
   /* Device Manager when various Local Device Manager Events occur.    */
   /* This function accepts the Callback Function and Callback Parameter*/
   /* (respectively) to call when a Local Device Manager Event needs to */
   /* be dispatched.  This function returns a positive (non-zero)       */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * If this function returns success (greater than zero)     */
   /*          then this value can be passed to the                     */
   /*          DEVM_UnRegisterEventCallback() function to un-register   */
   /*          the callback from this module.                           */
BTPSAPI_DECLARATION int BTPSAPI DEVM_RegisterEventCallback(DEVM_Event_Callback_t CallbackFunction, void *CallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_RegisterEventCallback_t)(DEVM_Event_Callback_t CallbackFunction, void *CallbackParameter);
#endif

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered Device Manager Event Callback */
   /* (registered via a successful call to the                          */
   /* DEVM_RegisterEventCallback() function).  This function accepts as */
   /* input the Device Manager Event Callback ID (return value from     */
   /* DEVM_RegisterEventCallback() function).                           */
BTPSAPI_DECLARATION void BTPSAPI DEVM_UnRegisterEventCallback(unsigned int DeviceManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_UnRegisterEventCallback_t)(unsigned int DeviceManagerCallbackID);
#endif

   /* The following function is provided to allow a mechanism to lock   */
   /* the Device Manager itself (this is needed to prevent simultaneous */
   /* thread access from corrupting internal resources).                */
BTPSAPI_DECLARATION Boolean_t BTPSAPI DEVM_AcquireLock(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef Boolean_t (BTPSAPI *PFN_DEVM_AcquireLock_t)(void);
#endif

   /* The following function is provided to allow a mechansim to release*/
   /* a previously acquired lock (via the DEVM_ReleaseLock() function). */
BTPSAPI_DECLARATION void BTPSAPI DEVM_ReleaseLock(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_ReleaseLock_t)(void);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually instruct the Device Manager to power on the   */
   /* local device (i.e. open the device).  This function returns zero  */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI DEVM_PowerOnDevice(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_PowerOnDevice_t)(void);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually instruct the Device Manager to power off the  */
   /* local device (i.e. close the device).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI DEVM_PowerOffDevice(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_PowerOffDevice_t)(void);
#endif

   /* The following function is provided to allow a mechansim for local */
   /* modules to determine the current Power On State of the local      */
   /* device.  This function returns:                                   */
   /*    - 0 Device is Powered Off                                      */
   /*    - 1 Device is Powered On                                       */
   /*    - Negative return error code                                   */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryDevicePowerState(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryDevicePowerState_t)(void);
#endif

   /* The following function is provided to allow a mechansim for local */
   /* modules that have registered callbacks to inform the Local Device */
   /* Manager that they have received the detDevicePoweringOff event and*/
   /* have completed their necessary cleanup.                           */
   /* * NOTE* When the Device Manager is powering down the local device */
   /*         it will dispatch a detDevicePoweringOff event to all      */
   /*         registered event callbacks.  All event callbacks that have*/
   /*         received this event *MUST* call this function to          */
   /*         acknowledge the Power Down event (usually after any       */
   /*         cleanup that might be needed).  This mechanism allows all */
   /*         modules to attempt to clean up any resources (e.g. close  */
   /*         an active connection) before the device is actually       */
   /*         powered off.  If a registered event callback does NOT     */
   /*         acknowledge the powering down event (i.e. does NOT call   */
   /*         this function), the device will still be shutdown after   */
   /*         a timeout has elapsed.  Using this mechansism allows the  */
   /*         timeout to be cut short because no modules will be using  */
   /*         the active connections any longer.                        */
BTPSAPI_DECLARATION void BTPSAPI DEVM_AcknowledgeDevicePoweringDown(unsigned int DeviceManagerCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_AcknowledgeDevicePoweringDown_t)(unsigned int DeviceManagerCallbackID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the current Local Device Properties of the   */
   /* Local Device.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryLocalDeviceProperties(DEVM_Local_Device_Properties_t *LocalDeviceProperties);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryLocalDeviceProperties_t)(DEVM_Local_Device_Properties_t *LocalDeviceProperties);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to update the current Local Device Properties of the Local*/
   /* Device.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI DEVM_UpdateLocalDeviceProperties(unsigned long UpdateMemberFlag, DEVM_Local_Device_Properties_t *LocalDeviceProperties);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_UpdateLocalDeviceProperties_t)(unsigned long UpdateMemberFlag, DEVM_Local_Device_Properties_t *LocalDeviceProperties);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the current Local Device ID Information of   */
   /* the Local Device.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryLocalDeviceIDInformation(DEVM_Device_ID_Information_t *LocalDeviceIDInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryLocalDeviceIDInformation_t)(DEVM_Device_ID_Information_t *LocalDeviceIDInformation);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* enabling a specified feature for the local device.  This function */
   /* accepts as input the feature that is to be enabled.               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_EnableLocalDeviceFeature(unsigned long Feature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_EnableLocalDeviceFeature_t)(unsigned long Feature);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* disabling a specified feature for the local device.  This function*/
   /* accepts as input the feature that is to be disabled.              */
BTPSAPI_DECLARATION int BTPSAPI DEVM_DisableLocalDeviceFeature(unsigned long Feature);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DisableLocalDeviceFeature_t)(unsigned long Feature);
#endif

   /* The following function is provided to allow a mechanism of        */
   /* querying the currently active features for the local device.  This*/
   /* function accepts as input a pointer to return the currently active*/
   /* features.                                                         */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryActiveLocalDeviceFeatures(unsigned long *ActiveFeatures);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryActiveLocalDeviceFeatures_t)(unsigned long *ActiveFeatures);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin a Device Discovery Process.  Device Discovery, in*/
   /* this context, means inquiry and name discovery.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StartDeviceDiscovery(unsigned long DiscoveryDuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StartDeviceDiscovery_t)(unsigned long DiscoveryDuration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Device Discovery Process.     */
   /* Device Discovery, in this context, means inquiry and name         */
   /* discovery.  This function returns zero if successful, or a        */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StopDeviceDiscovery(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StopDeviceDiscovery_t)(void);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin a Device Scanning Process.  Device Scanning, in  */
   /* this context, means Bluetooth Low Energy Active Scanning.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StartDeviceScan(unsigned long ScanDuration);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StartDeviceScan_t)(unsigned long ScanDuration);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Device Scanning Process.      */
   /* Device Scanning, in this context, means Bluetooth Low Energy      */
   /* Active Scanning.  This function returns zero if successful, or a  */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StopDeviceScan(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StopDeviceScan_t)(void);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to observation scan process.  This process allows for     */
   /* applications to observe devices that are discoverable and within  */
   /* range of the local device.  This function returns zero if         */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The first time a particular device is observed during an */
   /*          observation process it will be reported via the Remote   */
   /*          Device found event (detRemoteDeviceFound).  Subsequent   */
   /*          observations of the same device will be reported via     */
   /*          either of the following flags:                           */
   /*                                                                   */
   /*       DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_LE_LAST_OBSERVED      */
   /*       DEVM_REMOTE_DEVICE_PROPERTIES_CHANGED_CLASSIC_LAST_OBSERVED */
   /*                                                                   */
   /*          of the Remote Device Properties Changed event            */
   /*          (detRemoteDevicePropertiesChanged) based on the type of  */
   /*          observation process that was started via the Flags       */
   /*          parameter to this function (LE or Classic).              */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StartObservationScan(unsigned long Flags, DEVM_Observation_Parameters_t *ObservationParameters);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StartObservationScan_t)(unsigned long Flags, DEVM_Observation_Parameters_t *ObservationParameters);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing observation scan.  This       */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StopObservationScan(unsigned long Flags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StopObservationScan_t)(unsigned long Flags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin an Advertising Process.  This function returns   */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StartAdvertising(DEVM_Advertising_Information_t *AdvertisingInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StartAdvertising_t)(DEVM_Advertising_Information_t *AdvertisingInformation);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Advertising Process.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_StopAdvertising(unsigned long AdvertisingFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_StopAdvertising_t)(unsigned long AdvertisingFlags);
#endif

   /* The following function is provided to scheduled an interleaved    */
   /* advertisement.  An interleaved advertisement is a non-connectable */
   /* advertisement that uses the entire advertising data with the      */
   /* specified address for the specified duration (in milliseconds).   */
   /* When the interleaved advertisement has completed an event will be */
   /* dispatched to the callback identified by DeviceManagerCallbackID  */
   /* that the advertisement is completed.  Interleaved advertisements  */
   /* only run for Duration milliseconds and will not be re-run (i.e.   */
   /* they are not periodic) unless restarted at a higher layer.  This  */
   /* function returns the positive non-zero Interleaved Advertisement  */
   /* ID, which can be used to cancel and track the status of the       */
   /* operation, on success or a negative error code on failure.        */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ScheduleInterleavedAdvertisement(unsigned int DeviceManagerCallbackID, unsigned long Flags, unsigned int Duration, BD_ADDR_t RandomAddress, unsigned int DataLength, Byte_t *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ScheduleInterleavedAdvertisement_t)(unsigned int DeviceManagerCallbackID, unsigned long Flags, unsigned int Duration, BD_ADDR_t RandomAddress, unsigned int DataLength, Byte_t *Data);
#endif

   /* The following function is used to cancel a previously scheduled   */
   /* interleaved advertisement using the ID returned via a previously  */
   /* successful call to DEVM_ScheduleInterleavedAdvertisement().  This */
   /* function returns zero on success or a negative error code on      */
   /* failure.                                                          */
BTPSAPI_DECLARATION int BTPSAPI DEVM_CancelInterleavedAdvertisement(unsigned int DeviceManagerCallbackID, unsigned int InterleavedAdvertisementID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_CancelInterleavedAdvertisement_t)(unsigned int DeviceManagerCallbackID, unsigned int InterleavedAdvertisementID);
#endif

   /* The following function is used to suspend the scheduling of       */
   /* interleaved advertisements.  When interleaved advertisements are  */
   /* suspended none will be scheduled until scheduling has been resumed*/
   /* with the DEVM_ResumeInterleavedAdvertisements() API is called.    */
   /* This function returns zero on success or a negative error code on */
   /* failure.                                                          */
BTPSAPI_DECLARATION int BTPSAPI DEVM_SuspendInterleavedAdvertisements(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_SuspendInterleavedAdvertisements_t)(void);
#endif

   /* The following function is used to resume the scheduling of        */
   /* interleaved advertisements.  This function returns zero on success*/
   /* or a negative error code on failure.                              */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ResumeInterleavedAdvertisements(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ResumeInterleavedAdvertisements_t)(void);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current List of Remote Devices that the      */
   /* Bluetopia Platform Manager services currently knows about.  The   */
   /* return value from this function will be the number of addresses   */
   /* that that were copied into the specified buffer (if specified),   */
   /* zero signifies success, but no bytes copied.  A negative value    */
   /* represents a return error code if there was an error.             */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceList(unsigned int RemoteDeviceFilter, Class_of_Device_t ClassOfDeviceFilter, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceList_t)(unsigned int RemoteDeviceFilter, Class_of_Device_t ClassOfDeviceFilter, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the device properties of a specific Remote Device*/
   /* (based upon the specified Bluetooth Device Address).  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceProperties(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceProperties_t)(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the BR/EDR Extended Inquiry Response (EIR) data  */
   /* of a specific Remote Device (based upon the specified Bluetooth   */
   /* Device Address).  This function returns zero if there is no known */
   /* EIR Data for the device, a positive value if there is known EIR   */
   /* data for the device (and it will populated in the supplied        */
   /* buffer), or a negative return error code if there was an error.   */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceEIRData(BD_ADDR_t RemoteDevice, Extended_Inquiry_Response_Data_t *EIRData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceEIRData_t)(BD_ADDR_t RemoteDevice, Extended_Inquiry_Response_Data_t *EIRData);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the Bluetooth Low Energy Advertising and Scan    */
   /* Response data of a specific Remote Device (based upon the         */
   /* specified Bluetooth Device Address).  This function returns the   */
   /* number of Advertising Reports that were populated (zero, one or   */
   /* two) if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * If this function returns zero it means that no           */
   /*          Advertising or Scan Response data is know for the device.*/
   /*          If this function returns one, then it means that         */
   /*          advertising data is known (and is returned in the        */
   /*          supplied Advertising Data buffer).  If this function     */
   /*          returns two it means that both Advertising and Scan      */
   /*          Response data is known (and are returned in the          */
   /*          respective buffers).                                     */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceAdvertisingData(BD_ADDR_t RemoteDevice, Advertising_Data_t *AdvertisingData, Advertising_Data_t *ScanResponseData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceAdvertisingData_t)(BD_ADDR_t RemoteDevice, Advertising_Data_t *AdvertisingData, Advertising_Data_t *ScanResponseData);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the services provided by a specific Remote Device*/
   /* (based upon the specified Bluetooth Device Address).  The return  */
   /* value from this function will be the number of bytes that were    */
   /* copied into the specified buffer (if specified), zero signifies   */
   /* success, but no bytes copied.  A negative value represents a      */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceServices(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, unsigned int ServiceDataBufferSize, unsigned char *ServiceDataBuffer, unsigned int *TotalServiceDataLength);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceServices_t)(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, unsigned int ServiceDataBufferSize, unsigned char *ServiceDataBuffer, unsigned int *TotalServicesDataLength);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query whether a particular service is supported by a   */
   /* specific Remote Device (based upon the specified Bluetooth Device */
   /* Address). The return value from this function will a positive     */
   /* non-zero value if the device claims to support the service or zero*/
   /* if the device does NOT support the given service. A negative value*/
   /* represents a return error code if there was an error.             */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, this function will return an error. The   */
   /*          caller can check whether services are known using the    */
   /*          DEVM_QueryRemoteDeviceProperties() API and, if necessary,*/
   /*          can update the cached service records for a device using */
   /*          the DEVM_QueryRemoteDeviceServices() API.                */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR_t RemoteDevice, SDP_UUID_Entry_t ServiceUUID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceServiceSupported_t)(BD_ADDR_t RemoteDevice, SDP_UUID_Entry_t ServiceUUID);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query which devices claim support for a specific       */
   /* service. The return value from this function will be the number of*/
   /* Device Addresses copied into the buffer (if specified), where zero*/
   /* signifies success but no Device Addresses copied. A negative value*/
   /* represents a return error code if there was an error.             */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, that device will not be included in the   */
   /*          results. The caller can check whether services are known */
   /*          using the DEVM_QueryRemoteDeviceProperties() API and, if */
   /*          necessary, can update the cached service records for a   */
   /*          device using the DEVM_QueryRemoteDeviceServices() API.   */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDevicesForService(SDP_UUID_Entry_t ServiceUUID, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDevicesForService_t)(SDP_UUID_Entry_t ServiceUUID, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the Service Classes supported by a specific      */
   /* Remote Device (based upon the specified Bluetooth Device Address).*/
   /* The return value from this function will be the number of Service */
   /* Classes copied into the buffer (if specified), where zero         */
   /* signifies success but no Service Classes copied. A negative value */
   /* represents a return error code if there was an error.             */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, this function will return an error. The   */
   /*          caller can check whether services are known using the    */
   /*          DEVM_QueryRemoteDeviceProperties() API and, if necessary,*/
   /*          can update the cached service records for a device using */
   /*          the DEVM_QueryRemoteDeviceServices() API.                */
BTPSAPI_DECLARATION int BTPSAPI DEVM_QueryRemoteDeviceServiceClasses(BD_ADDR_t RemoteDevice, unsigned int MaximumNumberServiceClasses, SDP_UUID_Entry_t *ServiceClassList, unsigned int *TotalNumberServiceClasses);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_QueryRemoteDeviceServiceClasses_t)(BD_ADDR_t RemoteDevice, unsigned int MaximumNumberServiceClasses, SDP_UUID_Entry_t *ServiceClassList, unsigned int *TotalNumberServiceClasses);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually add a specific remote device entry into the   */
   /* list of remote devices that the Bluetopia Platform Manager        */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * To specify no Application Data pass NULL for the         */
   /*          Application Data parameter.                              */
   /* * NOTE * To Add a Bluetooth Low Energy (BLE) device, the Class of */
   /*          device should be set to a special value.  A MACRO exists */
   /*          to assign this value:                                    */
   /*           DEVM_ASSIGN_ADD_REMOTE_DEVICE_LOW_ENERGY_CLASS_OF_DEVICE*/
   /*          This constant is simply a non standard class of device   */
   /*          value that is used to signify this special case.         */
   /* * NOTE * To Add a Bluetooth Dual Mode device (BR/EDR plus LE), the*/
   /*          Class of device should be set a special value.  A MACRO  */
   /*          exists to assign this value:                             */
   /*           DEVM_ASSIGN_ADD_REMOTE_DEVICE_DUAL_MODE_CLASS_OF_DEVICE */
   /*          This constant is simply a non standard class of device   */
   /*          value that is used to signify this special case.         */
BTPSAPI_DECLARATION int BTPSAPI DEVM_AddRemoteDevice(BD_ADDR_t RemoteDevice, Class_of_Device_t ClassOfDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_AddRemoteDevice_t)(BD_ADDR_t RemoteDevice, Class_of_Device_t ClassOfDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove a specific remote device entry from the*/
   /* list of remote devices that the Bluetopia Platform Manager        */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI DEVM_DeleteRemoteDevice(BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DeleteRemoteDevice_t)(BD_ADDR_t RemoteDevice);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to set the Application Data for a specific Remote Device. */
   /* This data will be stored along with the Remote Device Information */
   /* in the list of Remote Devices that the Bluetopia Platform Manager */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * To specify no Application Data pass NULL for the         */
   /*          ApplicationData parameter.                               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_UpdateRemoteDeviceApplicationData(BD_ADDR_t RemoteDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_UpdateRemoteDeviceApplicationData_t)(BD_ADDR_t RemoteDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove a specific group of remote devices from*/
   /* the list of remote devices that the Bluetopia Platform Manager    */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI DEVM_DeleteRemoteDevices(unsigned int DeleteDevicesFilter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DeleteRemoteDevices_t)(unsigned int DeleteDevicesFilter);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually begin a pairing process with the specified    */
   /* remote device.  This function returns zero if successful, or a    */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The pairing process itself is asychronous.  The caller   */
   /*          can determine the status of the Pairing procedure by     */
   /*          waiting for the detRemoteDevicePairingStatus event to    */
   /*          be dispatched.                                           */
   /* * NOTE * Bluetooth Low Energy pairing can only be accomplished    */
   /*          if the device is already connected (i.e. this function   */
   /*          will not attempt to connect to the device for pairing).  */
BTPSAPI_DECLARATION int BTPSAPI DEVM_PairWithRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long PairFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_PairWithRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long PairFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually cancel an on-going pairing process with the   */
   /* specified remote device.  This function returns zero if           */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The pairing process itself is asychronous.  The caller   */
   /*          can determine the status of the Pairing procedure by     */
   /*          waiting for the detRemoteDevicePairingStatus event to be */
   /*          dispatched.                                              */
BTPSAPI_DECLARATION int BTPSAPI DEVM_CancelPairWithRemoteDevice(BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_CancelPairWithRemoteDevice_t)(BD_ADDR_t RemoteDevice);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove all stored pairing information with the*/
   /* specified remote device.  This function returns zero if           */
   /* successful, or a negative return error code if there was an error.*/
BTPSAPI_DECLARATION int BTPSAPI DEVM_UnPairRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long UnPairFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_UnPairRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long UnPairFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually Authenticate with a remote device (the device */
   /* *MUST* already be connected - i.e. this function will not make a  */
   /* connection and then attempt to authenticate the device).  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_AuthenticateRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long AuthenticateFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_AuthenticateRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long AuthenticateFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually force Encryption with a remote device (the    */
   /* device *MUST* already be connected - i.e. this function will not  */
   /* make a connection and then attempt to encrypt the device).  This  */
   /* function function returns zero if successful, or a negative return*/
   /* error code if there was an error.                                 */
   /* * NOTE * Bluetooth Encryption *REQUIRES* that a Link Key be       */
   /*          established between two devices before the link can be   */
   /*          encrypted.  Because of this, the caller *MUST* issue this*/
   /*          function call *AFTER* the link has been authenticated    */
   /*          (either locally or remotely).                            */
BTPSAPI_DECLARATION int BTPSAPI DEVM_EncryptRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long EncryptFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_EncryptRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long EncryptFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually connect with a remote device (and optionally  */
   /* specify connection parameters (authentication and encryption).    */
   /* This function function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * This function is envisioned to be used to force an       */
   /*          authentication on an outgoing link before a profile is   */
   /*          connected.                                               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_ConnectWithRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long ConnectFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_ConnectWithRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long ConnectFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually disconnect a remote device.  This function    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function should be used sparingly because it will   */
   /*          force all active service connections to be closed in     */
   /*          addition to forcing a disconnection of the device.       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_DisconnectRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long DisconnectFlags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DisconnectRemoteDevice_t)(BD_ADDR_t RemoteDevice, unsigned long DisconnectFlags);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to force the current link mode/state to active.  This     */
   /* means that if the current device is in:                           */
   /*    - Park Mode                                                    */
   /*    - Hold Mode                                                    */
   /*    - Sniff Mode                                                   */
   /* then this function will attempt to change the current mode/state  */
   /* to the active mode.  This function returns zero if the request    */
   /* was submitted, a positive value if the link is already in active  */
   /* mode, or a negative return error code if there was an error.      */
BTPSAPI_DECLARATION int BTPSAPI DEVM_SetRemoteDeviceLinkActive(BD_ADDR_t RemoteDevice);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_SetRemoteDeviceLinkActive_t)(BD_ADDR_t RemoteDevice);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to create an SDP Service Record in the Local Devices SDP  */
   /* Database.  This function returns a positive value if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If this function returns success then the return value   */
   /*          represents the actual Service Record Handle of the       */
   /*          created Service Record.                                  */
   /* * NOTE * Upon Client De-registration all Registered SDP Records   */
   /*          (registered by the client) are deleted from the SDP      */
   /*          Database if the PersistClient parameters is FALSE.  If   */
   /*          PersistClient is TRUE the record will stay in the SDP    */
   /*          database until it is either explicitly deleted OR the    */
   /*          device is powered down.                                  */
BTPSAPI_DECLARATION long BTPSAPI DEVM_CreateServiceRecord(Boolean_t PersistClient, unsigned int NumberServiceClassUUID, SDP_UUID_Entry_t SDP_UUID_Entry[]);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef long (BTPSAPI *PFN_DEVM_CreateServiceRecord_t)(Boolean_t PersistClient, unsigned int NumberServiceClassUUID, SDP_UUID_Entry_t SDP_UUID_Entry[]);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to delete a previously registered SDP Service Record from */
   /* the Local Devices SDP Database.  This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * Upon Client De-registration all Registered SDP Records   */
   /*          (registered by the client) are deleted from the SDP      */
   /*          Database.                                                */
BTPSAPI_DECLARATION int BTPSAPI DEVM_DeleteServiceRecord(unsigned long ServiceRecordHandle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DeleteServiceRecord_t)(unsigned long ServiceRecordHandle);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to add a Service Record Attribute to a Service Record in  */
   /* the Local Devices SDP Database.  This function returns a zero if  */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI DEVM_AddServiceRecordAttribute(unsigned long ServiceRecordHandle, unsigned int AttributeID, SDP_Data_Element_t *SDP_Data_Element);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_AddServiceRecordAttribute_t)(unsigned long ServiceRecordHandle, unsigned int AttributeID, SDP_Data_Element_t *SDP_Data_Element);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to delete a Service Record Attribute from a Service Record*/
   /* in the Local Devices SDP Database.  This function returns a zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI DEVM_DeleteServiceRecordAttribute(unsigned long ServiceRecordHandle, unsigned int AttributeID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_DeleteServiceRecordAttribute_t)(unsigned long ServiceRecordHandle, unsigned int AttributeID);
#endif

   /* The following function is a utility function that allows the      */
   /* caller to instruct the Local Device Manager to enable/disable     */
   /* Bluetooth Debugging.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
BTPSAPI_DECLARATION int BTPSAPI DEVM_EnableBluetoothDebug(Boolean_t Enable, unsigned int DebugType, unsigned long DebugFlags, unsigned int DebugParameterLength, unsigned char *DebugParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_EnableBluetoothDebug_t)(Boolean_t Enable, unsigned int DebugType, unsigned long DebugFlags, unsigned int DebugParameterLength, unsigned char *DebugParameter);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* local module to register an Authentication Callback.  This        */
   /* callback will be called whenever an Authentication Request needs  */
   /* to be serviced.  This function returns a positive, non-zero, value*/
   /* if successful, or a negative return error code if there was an    */
   /* error.  The return value from this function (if successful) can be*/
   /* passed to the DEVM_UnRegisterAuthentication() function to         */
   /* Un-Register the Authentication Callback.                          */
   /* * NOTE * There can ONLY be a SINGLE Registered Authentication     */
   /*          Callback Handler in the entire system at any given time. */
BTPSAPI_DECLARATION int BTPSAPI DEVM_RegisterAuthentication(DEVM_Authentication_Callback_t AuthenticationCallbackFunction, void *AuthenticationCallbackParameter);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_RegisterAuthentication_t)(DEVM_Authentication_Callback_t AuthenticationCallbackFunction, void *AuthenticationCallbackParameter);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* local module to Un-Register a previously registered Authentication*/
   /* Callback.  This function accepts as input the Authentication      */
   /* Callback ID of the successfully registered Authentication Callback*/
   /* (successful return value from the DEVM_RegisterAuthentication()   */
   /* function).                                                        */
BTPSAPI_DECLARATION void BTPSAPI DEVM_UnRegisterAuthentication(unsigned int AuthenticationCallbackID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef void (BTPSAPI *PFN_DEVM_UnRegisterAuthentication_t)(unsigned int AuthenticationCallbackID);
#endif

   /* The following function is provided to allow a mechanism for the   */
   /* local module that is processing Authentication Requests to respond*/
   /* to an Authentication Request.  This function accepts as input the */
   /* Authentication Callback ID of the successfully registered         */
   /* Authentication Callback (successful return value from the         */
   /* DEVM_RegisterAuthentication() function) followed by the           */
   /* Authentication Response Information.                              */
BTPSAPI_DECLARATION int BTPSAPI DEVM_AuthenticationResponse(unsigned int AuthenticationCallbackID, DEVM_Authentication_Information_t *AuthenticationInformation);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_AuthenticationResponse_t)(unsigned int AuthenticationCallbackID, DEVM_Authentication_Information_t *AuthenticationInformation);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to send raw HCI commands to the Bluetooth Device. This    */
   /* function accepts as input the Bluetooth Command OGF and OCF, data */
   /* (with length) to send with the command, and optional buffers in   */
   /* which to store command results. The final parameter indicates     */
   /* whether the function should wait for a response from the Bluetooth*/
   /* Device. If the caller knows that the Device does NOT return data  */
   /* for the given command, this parameter can be set to FALSE and the */
   /* function will return immediately after sending the command. If    */
   /* specified, LengthResult should indicate the maximum amount of data*/
   /* to be stored in BufferResult. Otherwise, the function will block  */
   /* until either a response is received or a timeout expires. This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI DEVM_SendRawHCICommand(unsigned char OGF, unsigned short OCF, unsigned char CommandLength, unsigned char CommandData[], unsigned char *ResultStatus, unsigned char *ResultLength, unsigned char *ResultBuffer, Boolean_t WaitForResponse);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_SendRawHCICommand_t)(unsigned char OGF, unsigned short OCF, unsigned char CommandLength, unsigned char CommandData[], unsigned char *ResultStatus, unsigned char *ResultLength, unsigned char *ResultBuffer, Boolean_t WaitForResponse);
#endif

   /* The following function is provided to allow a mechanism for local */
   /* modules to add arbitrary structures (length-tag-value structures) */
   /* to the local Extended Inquiry Response (EIR) data that is sent by */
   /* the local Bluetooth Device in response to remote inquiries.  This */
   /* function takes the length of the data (the total length of all LTV*/
   /* structures) and the data to add.  This function returns zero if   */
   /* successful or a negative return error code if there was an error. */
BTPSAPI_DECLARATION int BTPSAPI DEVM_AddLocalEIRData(unsigned int DataLength, unsigned char *Data);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_AddLocalEIRData_t)(unsigned int DataLength, unsigned char *Data);
#endif

   /* The following function allows local modules to request a Bluetooth*/
   /* Classic Role Switch. The RemoteDeviceAddress parameters is the    */
   /* addres of the remote device. The Role parameter is the requested  */
   /* role to switch to. This function returns zero if successful or a  */
   /* negative error code if there was an error.                        */
BTPSAPI_DECLARATION int BTPSAPI DEVM_RoleSwitch(BD_ADDR_t RemoteDeviceAddress, DEVM_Role_t Role);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DEVM_RoleSwitch_t)(BD_ADDR_t RemoteDeviceAddress, DEVM_Role_t Role);
#endif

#endif
